/* Copyright (c) 2017 Google Inc.
   Written by Andrew Allen */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mathops.h"
#include "os_support.h"
#include "oac_private.h"
#include "oac_defines.h"
#include "oac_projection.h"
#include "oac_multistream.h"
#include "stack_alloc.h"
#include "mapping_matrix.h"

struct OacProjectionEncoder {
    oac_int32 mixing_matrix_size_in_bytes;
    oac_int32 demixing_matrix_size_in_bytes;
    /* Encoder states go here */
};

#if !defined(DISABLE_FLOAT_API)
static void oac_projection_copy_channel_in_float(
    oac_res *dst,
    int dst_stride,
    const void *src,
    int src_stride,
    int src_channel,
    int frame_size,
    void *user_data)                  {
    oaci_mapping_matrix_multiply_channel_in_float((const MappingMatrix*)user_data,
        (const float*)src, src_stride, dst, src_channel, dst_stride, frame_size);
}
#endif

static void oac_projection_copy_channel_in_short(
    oac_res *dst,
    int dst_stride,
    const void *src,
    int src_stride,
    int src_channel,
    int frame_size,
    void *user_data)                  {
    oaci_mapping_matrix_multiply_channel_in_short((const MappingMatrix*)user_data,
        (const oac_int16*)src, src_stride, dst, src_channel, dst_stride, frame_size);
}

static void oac_projection_copy_channel_in_int24(
    oac_res *dst,
    int dst_stride,
    const void *src,
    int src_stride,
    int src_channel,
    int frame_size,
    void *user_data)                  {
    oaci_mapping_matrix_multiply_channel_in_int24((const MappingMatrix*)user_data,
        (const oac_int32*)src, src_stride, dst, src_channel, dst_stride, frame_size);
}

static int oaci_get_order_plus_one_from_channels(int channels, int *order_plus_one) {
    int order_plus_one_;
    int acn_channels;
    int nondiegetic_channels;

    /* Allowed numbers of channels:
     * (1 + n)^2 + 2j, for n = 0...14 and j = 0 or 1.
     */
    if (channels < 1 || channels > 227)
        return OAC_BAD_ARG;

    order_plus_one_ = oaci_isqrt32(channels);
    acn_channels = order_plus_one_*order_plus_one_;
    nondiegetic_channels = channels - acn_channels;
    if (nondiegetic_channels != 0 && nondiegetic_channels != 2)
        return OAC_BAD_ARG;

    if (order_plus_one)
        *order_plus_one = order_plus_one_;
    return OAC_OK;
}

static int oaci_get_streams_from_channels(int channels, int mapping_family,
                                     int *streams, int *coupled_streams,
                                     int *order_plus_one) {
    if (mapping_family == 3) {
        if (oaci_get_order_plus_one_from_channels(channels, order_plus_one) != OAC_OK)
            return OAC_BAD_ARG;
        if (streams)
            *streams = (channels + 1)/2;
        if (coupled_streams)
            *coupled_streams = channels/2;
        return OAC_OK;
    }
    return OAC_BAD_ARG;
}

static MappingMatrix *oaci_get_mixing_matrix(OacProjectionEncoder *st) {
    /* void* cast avoids clang -Wcast-oaci_align warning */
    return (MappingMatrix *)(void*)((char*)st
                                    + oaci_align(sizeof(OacProjectionEncoder)));
}

static MappingMatrix *oaci_get_enc_demixing_matrix(OacProjectionEncoder *st) {
    /* void* cast avoids clang -Wcast-oaci_align warning */
    return (MappingMatrix *)(void*)((char*)st
                                    + oaci_align(sizeof(OacProjectionEncoder)
                                        + st->mixing_matrix_size_in_bytes));
}

static OacMSEncoder *oaci_get_multistream_encoder(OacProjectionEncoder *st) {
    /* void* cast avoids clang -Wcast-oaci_align warning */
    return (OacMSEncoder *)(void*)((char*)st
                                   + oaci_align(sizeof(OacProjectionEncoder)
                                       + st->mixing_matrix_size_in_bytes
                                       + st->demixing_matrix_size_in_bytes));
}

oac_int32 oac_projection_ambisonics_encoder_get_size(int channels,
                                                     int mapping_family) {
    int nb_streams;
    int nb_coupled_streams;
    int order_plus_one;
    int mixing_matrix_rows, mixing_matrix_cols;
    int demixing_matrix_rows, demixing_matrix_cols;
    oac_int32 mixing_matrix_size, demixing_matrix_size;
    oac_int32 encoder_size;
    int ret;

    ret = oaci_get_streams_from_channels(channels, mapping_family, &nb_streams,
                                  &nb_coupled_streams, &order_plus_one);
    if (ret != OAC_OK)
        return 0;

    if (order_plus_one == 2) {
        mixing_matrix_rows = oaci_mapping_matrix_foa_mixing.rows;
        mixing_matrix_cols = oaci_mapping_matrix_foa_mixing.cols;
        demixing_matrix_rows = oaci_mapping_matrix_foa_demixing.rows;
        demixing_matrix_cols = oaci_mapping_matrix_foa_demixing.cols;
    } else if (order_plus_one == 3)   {
        mixing_matrix_rows = oaci_mapping_matrix_soa_mixing.rows;
        mixing_matrix_cols = oaci_mapping_matrix_soa_mixing.cols;
        demixing_matrix_rows = oaci_mapping_matrix_soa_demixing.rows;
        demixing_matrix_cols = oaci_mapping_matrix_soa_demixing.cols;
    } else if (order_plus_one == 4)   {
        mixing_matrix_rows = oaci_mapping_matrix_toa_mixing.rows;
        mixing_matrix_cols = oaci_mapping_matrix_toa_mixing.cols;
        demixing_matrix_rows = oaci_mapping_matrix_toa_demixing.rows;
        demixing_matrix_cols = oaci_mapping_matrix_toa_demixing.cols;
    } else if (order_plus_one == 5)   {
        mixing_matrix_rows = oaci_mapping_matrix_fourthoa_mixing.rows;
        mixing_matrix_cols = oaci_mapping_matrix_fourthoa_mixing.cols;
        demixing_matrix_rows = oaci_mapping_matrix_fourthoa_demixing.rows;
        demixing_matrix_cols = oaci_mapping_matrix_fourthoa_demixing.cols;
    } else if (order_plus_one == 6)   {
        mixing_matrix_rows = oaci_mapping_matrix_fifthoa_mixing.rows;
        mixing_matrix_cols = oaci_mapping_matrix_fifthoa_mixing.cols;
        demixing_matrix_rows = oaci_mapping_matrix_fifthoa_demixing.rows;
        demixing_matrix_cols = oaci_mapping_matrix_fifthoa_demixing.cols;
    } else
        return 0;

    mixing_matrix_size =
        oaci_mapping_matrix_get_size(mixing_matrix_rows, mixing_matrix_cols);
    if (!mixing_matrix_size)
        return 0;

    demixing_matrix_size =
        oaci_mapping_matrix_get_size(demixing_matrix_rows, demixing_matrix_cols);
    if (!demixing_matrix_size)
        return 0;

    encoder_size =
        oac_multistream_encoder_get_size(nb_streams, nb_coupled_streams);
    if (!encoder_size)
        return 0;

    return oaci_align(sizeof(OacProjectionEncoder))
           + mixing_matrix_size + demixing_matrix_size + encoder_size;
}

int oac_projection_ambisonics_encoder_init(OacProjectionEncoder *st, oac_int32 Fs,
                                           int channels, int mapping_family,
                                           int *streams, int *coupled_streams,
                                           int application) {
    MappingMatrix *mixing_matrix;
    MappingMatrix *demixing_matrix;
    OacMSEncoder *ms_encoder;
    int i;
    int ret;
    int order_plus_one;
    unsigned char mapping[255];

    if (streams == NULL || coupled_streams == NULL) {
        return OAC_BAD_ARG;
    }

    if (oaci_get_streams_from_channels(channels, mapping_family, streams,
    coupled_streams, &order_plus_one) != OAC_OK)
        return OAC_BAD_ARG;

    if (mapping_family == 3) {
        /* Assign mixing matrix based on available pre-computed matrices. */
        mixing_matrix = oaci_get_mixing_matrix(st);
        if (order_plus_one == 2) {
            oaci_mapping_matrix_init(mixing_matrix, oaci_mapping_matrix_foa_mixing.rows,
        oaci_mapping_matrix_foa_mixing.cols, oaci_mapping_matrix_foa_mixing.gain,
        oaci_mapping_matrix_foa_mixing_data,
        sizeof(oaci_mapping_matrix_foa_mixing_data));
        } else if (order_plus_one == 3)   {
            oaci_mapping_matrix_init(mixing_matrix, oaci_mapping_matrix_soa_mixing.rows,
        oaci_mapping_matrix_soa_mixing.cols, oaci_mapping_matrix_soa_mixing.gain,
        oaci_mapping_matrix_soa_mixing_data,
        sizeof(oaci_mapping_matrix_soa_mixing_data));
        } else if (order_plus_one == 4)   {
            oaci_mapping_matrix_init(mixing_matrix, oaci_mapping_matrix_toa_mixing.rows,
        oaci_mapping_matrix_toa_mixing.cols, oaci_mapping_matrix_toa_mixing.gain,
        oaci_mapping_matrix_toa_mixing_data,
        sizeof(oaci_mapping_matrix_toa_mixing_data));
        } else if (order_plus_one == 5)   {
            oaci_mapping_matrix_init(mixing_matrix, oaci_mapping_matrix_fourthoa_mixing.rows,
        oaci_mapping_matrix_fourthoa_mixing.cols, oaci_mapping_matrix_fourthoa_mixing.gain,
        oaci_mapping_matrix_fourthoa_mixing_data,
        sizeof(oaci_mapping_matrix_fourthoa_mixing_data));
        } else if (order_plus_one == 6)   {
            oaci_mapping_matrix_init(mixing_matrix, oaci_mapping_matrix_fifthoa_mixing.rows,
        oaci_mapping_matrix_fifthoa_mixing.cols, oaci_mapping_matrix_fifthoa_mixing.gain,
        oaci_mapping_matrix_fifthoa_mixing_data,
        sizeof(oaci_mapping_matrix_fifthoa_mixing_data));
        } else
            return OAC_BAD_ARG;

        st->mixing_matrix_size_in_bytes = oaci_mapping_matrix_get_size(
      mixing_matrix->rows, mixing_matrix->cols);
        if (!st->mixing_matrix_size_in_bytes)
            return OAC_BAD_ARG;

        /* Assign demixing matrix based on available pre-computed matrices. */
        demixing_matrix = oaci_get_enc_demixing_matrix(st);
        if (order_plus_one == 2) {
            oaci_mapping_matrix_init(demixing_matrix, oaci_mapping_matrix_foa_demixing.rows,
        oaci_mapping_matrix_foa_demixing.cols, oaci_mapping_matrix_foa_demixing.gain,
        oaci_mapping_matrix_foa_demixing_data,
        sizeof(oaci_mapping_matrix_foa_demixing_data));
        } else if (order_plus_one == 3)   {
            oaci_mapping_matrix_init(demixing_matrix, oaci_mapping_matrix_soa_demixing.rows,
        oaci_mapping_matrix_soa_demixing.cols, oaci_mapping_matrix_soa_demixing.gain,
        oaci_mapping_matrix_soa_demixing_data,
        sizeof(oaci_mapping_matrix_soa_demixing_data));
        } else if (order_plus_one == 4)   {
            oaci_mapping_matrix_init(demixing_matrix, oaci_mapping_matrix_toa_demixing.rows,
        oaci_mapping_matrix_toa_demixing.cols, oaci_mapping_matrix_toa_demixing.gain,
        oaci_mapping_matrix_toa_demixing_data,
        sizeof(oaci_mapping_matrix_toa_demixing_data));
        } else if (order_plus_one == 5) {
            oaci_mapping_matrix_init(demixing_matrix, oaci_mapping_matrix_fourthoa_demixing.rows,
        oaci_mapping_matrix_fourthoa_demixing.cols, oaci_mapping_matrix_fourthoa_demixing.gain,
        oaci_mapping_matrix_fourthoa_demixing_data,
        sizeof(oaci_mapping_matrix_fourthoa_demixing_data));
        } else if (order_plus_one == 6)   {
            oaci_mapping_matrix_init(demixing_matrix, oaci_mapping_matrix_fifthoa_demixing.rows,
        oaci_mapping_matrix_fifthoa_demixing.cols, oaci_mapping_matrix_fifthoa_demixing.gain,
        oaci_mapping_matrix_fifthoa_demixing_data,
        sizeof(oaci_mapping_matrix_fifthoa_demixing_data));
        } else
            return OAC_BAD_ARG;

        st->demixing_matrix_size_in_bytes = oaci_mapping_matrix_get_size(
      demixing_matrix->rows, demixing_matrix->cols);
        if (!st->demixing_matrix_size_in_bytes)
            return OAC_BAD_ARG;
    } else
        return OAC_UNIMPLEMENTED;

    /* Ensure matrices are large enough for desired coding scheme. */
    if (*streams + *coupled_streams > mixing_matrix->rows
        || channels > mixing_matrix->cols
        || channels > demixing_matrix->rows
        || *streams + *coupled_streams > demixing_matrix->cols)
        return OAC_BAD_ARG;

    /* Set trivial mapping so each input channel pairs with a matrix column. */
    for (i = 0; i < channels; i++)
        mapping[i] = i;

    /* Initialize multistream encoder with provided settings. */
    ms_encoder = oaci_get_multistream_encoder(st);
    ret = oac_multistream_encoder_init(ms_encoder, Fs, channels, *streams,
                                      *coupled_streams, mapping, application);
    return ret;
}

OacProjectionEncoder *oac_projection_ambisonics_encoder_create(
    oac_int32 Fs, int channels, int mapping_family, int *streams,
    int *coupled_streams, int application, int *error) {
    int size;
    int ret;
    OacProjectionEncoder *st;

    /* Allocate space for the projection encoder. */
    size = oac_projection_ambisonics_encoder_get_size(channels, mapping_family);
    if (!size) {
        if (error)
            *error = OAC_ALLOC_FAIL;
        return NULL;
    }
    st = (OacProjectionEncoder *)oac_alloc(size);
    if (!st) {
        if (error)
            *error = OAC_ALLOC_FAIL;
        return NULL;
    }

    /* Initialize projection encoder with provided settings. */
    ret = oac_projection_ambisonics_encoder_init(st, Fs, channels,
     mapping_family, streams, coupled_streams, application);
    if (ret != OAC_OK) {
        oac_free(st);
        st = NULL;
    }
    if (error)
        *error = ret;
    return st;
}

int oac_projection_encode(OacProjectionEncoder *st, const oac_int16 *pcm,
                          int frame_size, unsigned char *data,
                          oac_int32 max_data_bytes) {
    return oac_multistream_encode_native(oaci_get_multistream_encoder(st),
    oac_projection_copy_channel_in_short, pcm, frame_size, data,
    max_data_bytes, 16, oaci_downmix_int, 0, oaci_get_mixing_matrix(st));
}

int oac_projection_encode24(OacProjectionEncoder *st, const oac_int32 *pcm,
                            int frame_size, unsigned char *data,
                            oac_int32 max_data_bytes) {
    return oac_multistream_encode_native(oaci_get_multistream_encoder(st),
    oac_projection_copy_channel_in_int24, pcm, frame_size, data,
    max_data_bytes, MAX_ENCODING_DEPTH, oaci_downmix_int, 0, oaci_get_mixing_matrix(st));
}

#ifndef DISABLE_FLOAT_API
int oac_projection_encode_float(OacProjectionEncoder *st, const float *pcm,
                                int frame_size, unsigned char *data,
                                oac_int32 max_data_bytes) {
    return oac_multistream_encode_native(oaci_get_multistream_encoder(st),
    oac_projection_copy_channel_in_float, pcm, frame_size, data,
    max_data_bytes, MAX_ENCODING_DEPTH, oaci_downmix_float, 1, oaci_get_mixing_matrix(st));
}
#endif

void oac_projection_encoder_destroy(OacProjectionEncoder *st) {
    oac_free(st);
}

int oac_projection_encoder_ctl(OacProjectionEncoder *st, int request, ...) {
    va_list ap;
    MappingMatrix *demixing_matrix;
    OacMSEncoder *ms_encoder;
    int ret = OAC_OK;

    ms_encoder = oaci_get_multistream_encoder(st);
    demixing_matrix = oaci_get_enc_demixing_matrix(st);

    va_start(ap, request);
    switch (request) {
        case OAC_PROJECTION_GET_DEMIXING_MATRIX_SIZE_REQUEST:
        {
            oac_int32 *value = va_arg(ap, oac_int32*);
            if (!value) {
                goto bad_arg;
            }
            *value =
                ms_encoder->layout.nb_channels*(ms_encoder->layout.nb_streams
                                                + ms_encoder->layout.nb_coupled_streams)*sizeof(oac_int16);
        }
        break;
        case OAC_PROJECTION_GET_DEMIXING_MATRIX_GAIN_REQUEST:
        {
            oac_int32 *value = va_arg(ap, oac_int32*);
            if (!value) {
                goto bad_arg;
            }
            *value = demixing_matrix->gain;
        }
        break;
        case OAC_PROJECTION_GET_DEMIXING_MATRIX_REQUEST:
        {
            int i, j, k, l;
            int nb_input_streams;
            int nb_output_streams;
            unsigned char *external_char;
            oac_int16 *internal_short;
            oac_int32 external_size;
            oac_int32 internal_size;

            /* (I/O is in relation to the decoder's perspective). */
            nb_input_streams = ms_encoder->layout.nb_streams
                               + ms_encoder->layout.nb_coupled_streams;
            nb_output_streams = ms_encoder->layout.nb_channels;

            external_char = va_arg(ap, unsigned char *);
            external_size = va_arg(ap, oac_int32);
            if (!external_char) {
                goto bad_arg;
            }
            internal_short = oaci_mapping_matrix_get_data(demixing_matrix);
            internal_size = nb_input_streams*nb_output_streams*sizeof(oac_int16);
            if (external_size != internal_size) {
                goto bad_arg;
            }

            /* Copy demixing matrix subset to output destination. */
            l = 0;
            for (i = 0; i < nb_input_streams; i++) {
                for (j = 0; j < nb_output_streams; j++) {
                    k = demixing_matrix->rows*i + j;
                    external_char[2*l] = (unsigned char)internal_short[k];
                    external_char[2*l + 1] = (unsigned char)(internal_short[k]>>8);
                    l++;
                }
            }
        }
        break;
        default:
        {
            ret = oac_multistream_encoder_ctl_va_list(ms_encoder, request, ap);
        }
        break;
    }
    va_end(ap);
    return ret;

bad_arg:
    va_end(ap);
    return OAC_BAD_ARG;
}
