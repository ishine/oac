/*  Copyright (c) 2026, Alliance for Open Media
    All rights reserved. */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted (subject to the limitations in the
    disclaimer below) provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    - Neither the name of the Alliance for Open Media nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
    GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
    HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
#include "mapping_matrix.h"
#include "stack_alloc.h"

struct OacProjectionDecoder {
    oac_int32 demixing_matrix_size_in_bytes;
    /* Encoder states go here */
};

#if !defined(DISABLE_FLOAT_API)
static void oac_projection_copy_channel_out_float(
    void *dst,
    int dst_stride,
    int dst_channel,
    const oac_res *src,
    int src_stride,
    int frame_size,
    void *user_data) {
    float *float_dst;
    const MappingMatrix *matrix;
    float_dst = (float *)dst;
    matrix = (const MappingMatrix *)user_data;

    if (dst_channel == 0)
        OAC_CLEAR(float_dst, frame_size*dst_stride);

    if (src != NULL)
        oaci_mapping_matrix_multiply_channel_out_float(matrix, src, dst_channel,
      src_stride, float_dst, dst_stride, frame_size);
}
#endif

static void oac_projection_copy_channel_out_short(
    void *dst,
    int dst_stride,
    int dst_channel,
    const oac_res *src,
    int src_stride,
    int frame_size,
    void *user_data) {
    oac_int16 *short_dst;
    const MappingMatrix *matrix;
    short_dst = (oac_int16 *)dst;
    matrix = (const MappingMatrix *)user_data;
    if (dst_channel == 0)
        OAC_CLEAR(short_dst, frame_size*dst_stride);

    if (src != NULL)
        oaci_mapping_matrix_multiply_channel_out_short(matrix, src, dst_channel,
      src_stride, short_dst, dst_stride, frame_size);
}

static void oac_projection_copy_channel_out_int24(
    void *dst,
    int dst_stride,
    int dst_channel,
    const oac_res *src,
    int src_stride,
    int frame_size,
    void *user_data) {
    oac_int32 *short_dst;
    const MappingMatrix *matrix;
    short_dst = (oac_int32 *)dst;
    matrix = (const MappingMatrix *)user_data;
    if (dst_channel == 0)
        OAC_CLEAR(short_dst, frame_size*dst_stride);

    if (src != NULL)
        oaci_mapping_matrix_multiply_channel_out_int24(matrix, src, dst_channel,
      src_stride, short_dst, dst_stride, frame_size);
}

static MappingMatrix *get_dec_demixing_matrix(OacProjectionDecoder *st) {
    /* void* cast avoids clang -Wcast-align warning */
    return (MappingMatrix*)(void*)((char*)st
                                   + align(sizeof(OacProjectionDecoder)));
}

static OacMSDecoder *oaci_get_multistream_decoder(OacProjectionDecoder *st) {
    /* void* cast avoids clang -Wcast-align warning */
    return (OacMSDecoder*)(void*)((char*)st
                                  + align(sizeof(OacProjectionDecoder)
                                      + st->demixing_matrix_size_in_bytes));
}

oac_int32 oac_projection_decoder_get_size(int channels, int streams,
                                          int coupled_streams) {
    oac_int32 matrix_size;
    oac_int32 decoder_size;

    matrix_size =
        oaci_mapping_matrix_get_size(streams + coupled_streams, channels);
    if (!matrix_size)
        return 0;

    decoder_size = oac_multistream_decoder_get_size(streams, coupled_streams);
    if (!decoder_size)
        return 0;

    return align(sizeof(OacProjectionDecoder)) + matrix_size + decoder_size;
}

int oac_projection_decoder_init(OacProjectionDecoder *st, oac_int32 Fs,
                                int channels, int streams, int coupled_streams,
                                unsigned char *demixing_matrix, oac_int32 demixing_matrix_size) {
    int nb_input_streams;
    oac_int32 expected_matrix_size;
    int i, ret;
    unsigned char mapping[255];
    VARDECL(oac_int16, buf);
    ALLOC_STACK;

    /* Verify supplied matrix size. */
    nb_input_streams = streams + coupled_streams;
    expected_matrix_size = nb_input_streams*channels*sizeof(oac_int16);
    if (expected_matrix_size != demixing_matrix_size) {
        RESTORE_STACK;
        return OAC_BAD_ARG;
    }

    /* Convert demixing matrix input into internal format. */
    ALLOC(buf, nb_input_streams*channels, oac_int16);
    for (i = 0; i < nb_input_streams*channels; i++) {
        int s = demixing_matrix[2*i + 1]<<8|demixing_matrix[2*i];
        s = ((s&0xFFFF)^0x8000) - 0x8000;
        buf[i] = (oac_int16)s;
    }

    /* Assign demixing matrix. */
    st->demixing_matrix_size_in_bytes =
        oaci_mapping_matrix_get_size(channels, nb_input_streams);
    if (!st->demixing_matrix_size_in_bytes) {
        RESTORE_STACK;
        return OAC_BAD_ARG;
    }

    oaci_mapping_matrix_init(get_dec_demixing_matrix(st), channels, nb_input_streams, 0,
    buf, demixing_matrix_size);

    /* Set trivial mapping so each input channel pairs with a matrix column. */
    for (i = 0; i < channels; i++)
        mapping[i] = i;

    ret = oac_multistream_decoder_init(
    oaci_get_multistream_decoder(st), Fs, channels, streams, coupled_streams, mapping);
    RESTORE_STACK;
    return ret;
}

OacProjectionDecoder *oac_projection_decoder_create(
    oac_int32 Fs, int channels, int streams, int coupled_streams,
    unsigned char *demixing_matrix, oac_int32 demixing_matrix_size, int *error) {
    int size;
    int ret;
    OacProjectionDecoder *st;

    /* Allocate space for the projection decoder. */
    size = oac_projection_decoder_get_size(channels, streams, coupled_streams);
    if (!size) {
        if (error)
            *error = OAC_ALLOC_FAIL;
        return NULL;
    }
    st = (OacProjectionDecoder *)oac_alloc(size);
    if (!st) {
        if (error)
            *error = OAC_ALLOC_FAIL;
        return NULL;
    }

    /* Initialize projection decoder with provided settings. */
    ret = oac_projection_decoder_init(st, Fs, channels, streams, coupled_streams,
                                     demixing_matrix, demixing_matrix_size);
    if (ret != OAC_OK) {
        oac_free(st);
        st = NULL;
    }
    if (error)
        *error = ret;
    return st;
}

#ifdef FIXED_POINT
# define OPTIONAL_CLIP 0
#else
# define OPTIONAL_CLIP 1
#endif

int oac_projection_decode(OacProjectionDecoder *st, const unsigned char *data,
                          oac_int32 len, oac_int16 *pcm, int frame_size,
                          int decode_fec) {
    return oac_multistream_decode_native(oaci_get_multistream_decoder(st), data, len,
    pcm, oac_projection_copy_channel_out_short, frame_size, decode_fec, OPTIONAL_CLIP,
    get_dec_demixing_matrix(st));
}

int oac_projection_decode24(OacProjectionDecoder *st, const unsigned char *data,
                            oac_int32 len, oac_int32 *pcm, int frame_size,
                            int decode_fec) {
    return oac_multistream_decode_native(oaci_get_multistream_decoder(st), data, len,
    pcm, oac_projection_copy_channel_out_int24, frame_size, decode_fec, 0,
    get_dec_demixing_matrix(st));
}

#ifndef DISABLE_FLOAT_API
int oac_projection_decode_float(OacProjectionDecoder *st, const unsigned char *data,
                                oac_int32 len, float *pcm, int frame_size, int decode_fec) {
    return oac_multistream_decode_native(oaci_get_multistream_decoder(st), data, len,
    pcm, oac_projection_copy_channel_out_float, frame_size, decode_fec, 0,
    get_dec_demixing_matrix(st));
}
#endif

int oac_projection_decoder_ctl(OacProjectionDecoder *st, int request, ...) {
    va_list ap;
    int ret = OAC_OK;

    va_start(ap, request);
    ret = oac_multistream_decoder_ctl_va_list(oaci_get_multistream_decoder(st),
    request, ap);
    va_end(ap);
    return ret;
}

void oac_projection_decoder_destroy(OacProjectionDecoder *st) {
    oac_free(st);
}
