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

/***********************************************************************
   Copyright (c) 2006-2011, Skype Limited. All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   - Neither the name of Internet Society, IETF or IETF Trust, nor the
   names of specific contributors, may be used to endorse or promote
   products derived from this software without specific prior written
   permission.
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "main.h"

/* Encode side-information parameters to payload */
void oaci_silk_encode_indices(
    silk_encoder_state          *psEncC,                        /* I/O  Encoder state                               */
    ec_enc                      *psRangeEnc,                    /* I/O  Compressor data structure                   */
    oac_int FrameIndex,                                        /* I    Frame number                                */
    oac_int encode_LBRR,                                       /* I    Flag indicating LBRR data is being encoded  */
    oac_int condCoding                                         /* I    The type of conditional coding to use       */
    ) {
    oac_int i, k, typeOffset;
    oac_int encode_absolute_lagIndex, delta_lagIndex;
    oac_int16 ec_ix[ MAX_LPC_ORDER ];
    oac_uint8 pred_Q8[ MAX_LPC_ORDER ];
    const SideInfoIndices *psIndices;

    if (encode_LBRR) {
        psIndices = &psEncC->indices_LBRR[ FrameIndex ];
    } else {
        psIndices = &psEncC->indices;
    }

    /*******************************************/
    /* Encode signal type and quantizer offset */
    /*******************************************/
    typeOffset = 2*psIndices->signalType + psIndices->quantOffsetType;
    celt_assert( typeOffset >= 0 && typeOffset < 6 );
    celt_assert( encode_LBRR == 0 || typeOffset >= 2 );
    if (encode_LBRR || typeOffset >= 2) {
        oaci_ec_enc_icdf( psRangeEnc, typeOffset - 2, oaci_silk_type_offset_VAD_iCDF, 8 );
    } else {
        oaci_ec_enc_icdf( psRangeEnc, typeOffset, oaci_silk_type_offset_no_VAD_iCDF, 8 );
    }

    /****************/
    /* Encode gains */
    /****************/
    /* first subframe */
    if (condCoding == CODE_CONDITIONALLY) {
        /* conditional coding */
        silk_assert( psIndices->GainsIndices[ 0 ] >= 0
            && psIndices->GainsIndices[ 0 ] < MAX_DELTA_GAIN_QUANT - MIN_DELTA_GAIN_QUANT + 1 );
        oaci_ec_enc_icdf( psRangeEnc, psIndices->GainsIndices[ 0 ], oaci_silk_delta_gain_iCDF, 8 );
    } else {
        /* independent coding, in two stages: MSB bits followed by 3 LSBs */
        silk_assert( psIndices->GainsIndices[ 0 ] >= 0 && psIndices->GainsIndices[ 0 ] < N_LEVELS_QGAIN );
        oaci_ec_enc_icdf( psRangeEnc, silk_RSHIFT( psIndices->GainsIndices[ 0 ], 3 ),
        oaci_silk_gain_iCDF[ psIndices->signalType ], 8 );
        oaci_ec_enc_icdf( psRangeEnc, psIndices->GainsIndices[ 0 ]&7, oaci_silk_uniform8_iCDF, 8 );
    }

    /* remaining subframes */
    for (i = 1; i < psEncC->nb_subfr; i++) {
        silk_assert( psIndices->GainsIndices[ i ] >= 0
            && psIndices->GainsIndices[ i ] < MAX_DELTA_GAIN_QUANT - MIN_DELTA_GAIN_QUANT + 1 );
        oaci_ec_enc_icdf( psRangeEnc, psIndices->GainsIndices[ i ], oaci_silk_delta_gain_iCDF, 8 );
    }

    /****************/
    /* Encode NLSFs */
    /****************/
    oaci_ec_enc_icdf( psRangeEnc, psIndices->NLSFIndices[ 0 ],
    &psEncC->psNLSF_CB->CB1_iCDF[ (psIndices->signalType>>1)*psEncC->psNLSF_CB->nVectors ], 8 );
    oaci_silk_NLSF_unpack( ec_ix, pred_Q8, psEncC->psNLSF_CB, psIndices->NLSFIndices[ 0 ] );
    celt_assert( psEncC->psNLSF_CB->order == psEncC->predictLPCOrder );
    for (i = 0; i < psEncC->psNLSF_CB->order; i++) {
        if (psIndices->NLSFIndices[ i + 1 ] >= NLSF_QUANT_MAX_AMPLITUDE) {
            oaci_ec_enc_icdf( psRangeEnc, 2*NLSF_QUANT_MAX_AMPLITUDE, &psEncC->psNLSF_CB->ec_iCDF[ ec_ix[ i ] ], 8 );
            oaci_ec_enc_icdf( psRangeEnc, psIndices->NLSFIndices[ i + 1 ] - NLSF_QUANT_MAX_AMPLITUDE, oaci_silk_NLSF_EXT_iCDF,
            8 );
        } else if (psIndices->NLSFIndices[ i + 1 ] <= -NLSF_QUANT_MAX_AMPLITUDE) {
            oaci_ec_enc_icdf( psRangeEnc, 0, &psEncC->psNLSF_CB->ec_iCDF[ ec_ix[ i ] ], 8 );
            oaci_ec_enc_icdf( psRangeEnc, -psIndices->NLSFIndices[ i + 1 ] - NLSF_QUANT_MAX_AMPLITUDE, oaci_silk_NLSF_EXT_iCDF,
            8 );
        } else {
            oaci_ec_enc_icdf( psRangeEnc, psIndices->NLSFIndices[ i + 1 ] + NLSF_QUANT_MAX_AMPLITUDE,
            &psEncC->psNLSF_CB->ec_iCDF[ ec_ix[ i ] ], 8 );
        }
    }

    /* Encode NLSF interpolation factor */
    if (psEncC->nb_subfr == MAX_NB_SUBFR) {
        silk_assert( psIndices->NLSFInterpCoef_Q2 >= 0 && psIndices->NLSFInterpCoef_Q2 < 5 );
        oaci_ec_enc_icdf( psRangeEnc, psIndices->NLSFInterpCoef_Q2, oaci_silk_NLSF_interpolation_factor_iCDF, 8 );
    }

    if (psIndices->signalType == TYPE_VOICED) {
        /*********************/
        /* Encode pitch lags */
        /*********************/
        /* lag index */
        encode_absolute_lagIndex = 1;
        if (condCoding == CODE_CONDITIONALLY && psEncC->ec_prevSignalType == TYPE_VOICED) {
            /* Delta Encoding */
            delta_lagIndex = psIndices->lagIndex - psEncC->ec_prevLagIndex;
            if (delta_lagIndex < -8 || delta_lagIndex > 11) {
                delta_lagIndex = 0;
            } else {
                delta_lagIndex = delta_lagIndex + 9;
                encode_absolute_lagIndex = 0; /* Only use delta */
            }
            silk_assert( delta_lagIndex >= 0 && delta_lagIndex < 21 );
            oaci_ec_enc_icdf( psRangeEnc, delta_lagIndex, oaci_silk_pitch_delta_iCDF, 8 );
        }
        if (encode_absolute_lagIndex) {
            /* Absolute encoding */
            oac_int32 pitch_high_bits, pitch_low_bits;
            pitch_high_bits = silk_DIV32_16( psIndices->lagIndex, silk_RSHIFT( psEncC->fs_kHz, 1 ));
            pitch_low_bits = psIndices->lagIndex - silk_SMULBB( pitch_high_bits, silk_RSHIFT( psEncC->fs_kHz, 1 ));
            silk_assert( pitch_low_bits < psEncC->fs_kHz/2 );
            silk_assert( pitch_high_bits < 32 );
            oaci_ec_enc_icdf( psRangeEnc, pitch_high_bits, oaci_silk_pitch_lag_iCDF, 8 );
            oaci_ec_enc_icdf( psRangeEnc, pitch_low_bits, psEncC->pitch_lag_low_bits_iCDF, 8 );
        }
        psEncC->ec_prevLagIndex = psIndices->lagIndex;

        /* Contour index */
        silk_assert(   psIndices->contourIndex  >= 0 );
        silk_assert((psIndices->contourIndex < 34 && psEncC->fs_kHz  > 8 && psEncC->nb_subfr == 4)
            || (psIndices->contourIndex < 11 && psEncC->fs_kHz == 8 && psEncC->nb_subfr == 4)
            || (psIndices->contourIndex < 12 && psEncC->fs_kHz  > 8 && psEncC->nb_subfr == 2)
            || (psIndices->contourIndex <  3 && psEncC->fs_kHz == 8 && psEncC->nb_subfr == 2));
        oaci_ec_enc_icdf( psRangeEnc, psIndices->contourIndex, psEncC->pitch_contour_iCDF, 8 );

        /********************/
        /* Encode LTP gains */
        /********************/
        /* PERIndex value */
        silk_assert( psIndices->PERIndex >= 0 && psIndices->PERIndex < 3 );
        oaci_ec_enc_icdf( psRangeEnc, psIndices->PERIndex, oaci_silk_LTP_per_index_iCDF, 8 );

        /* Codebook Indices */
        for (k = 0; k < psEncC->nb_subfr; k++) {
            silk_assert( psIndices->LTPIndex[ k ] >= 0 && psIndices->LTPIndex[ k ] < (8<<psIndices->PERIndex));
            oaci_ec_enc_icdf( psRangeEnc, psIndices->LTPIndex[ k ], oaci_silk_LTP_gain_iCDF_ptrs[ psIndices->PERIndex ], 8 );
        }

        /**********************/
        /* Encode LTP scaling */
        /**********************/
        if (condCoding == CODE_INDEPENDENTLY) {
            silk_assert( psIndices->LTP_scaleIndex >= 0 && psIndices->LTP_scaleIndex < 3 );
            oaci_ec_enc_icdf( psRangeEnc, psIndices->LTP_scaleIndex, oaci_silk_LTPscale_iCDF, 8 );
        }
        silk_assert( !condCoding || psIndices->LTP_scaleIndex == 0 );
    }

    psEncC->ec_prevSignalType = psIndices->signalType;

    /***************/
    /* Encode seed */
    /***************/
    silk_assert( psIndices->Seed >= 0 && psIndices->Seed < 4 );
    oaci_ec_enc_icdf( psRangeEnc, psIndices->Seed, oaci_silk_uniform4_iCDF, 8 );
}
