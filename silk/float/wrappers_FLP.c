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

#include "main_FLP.h"

/* Wrappers. Calls flp / fix code */

/* Convert AR filter coefficients to NLSF parameters */
void oaci_silk_A2NLSF_FLP(
    oac_int16                      *NLSF_Q15,                          /* O    NLSF vector      [ LPC_order ]              */
    const silk_float                *pAR,                               /* I    LPC coefficients [ LPC_order ]              */
    const oac_int LPC_order                                            /* I    LPC order                                   */
    ) {
    oac_int i;
    oac_int32 a_fix_Q16[ MAX_LPC_ORDER ];

    for (i = 0; i < LPC_order; i++) {
        a_fix_Q16[ i ] = silk_float2int( pAR[ i ]*65536.0f );
    }

    oaci_silk_A2NLSF( NLSF_Q15, a_fix_Q16, LPC_order );
}

/* Convert LSF parameters to AR prediction filter coefficients */
void oaci_silk_NLSF2A_FLP(
    silk_float                      *pAR,                               /* O    LPC coefficients [ LPC_order ]              */
    const oac_int16                *NLSF_Q15,                          /* I    NLSF vector      [ LPC_order ]              */
    const oac_int LPC_order,                                           /* I    LPC order                                   */
    int arch                                                            /* I    Run-time architecture                       */
    ) {
    oac_int i;
    oac_int16 a_fix_Q12[ MAX_LPC_ORDER ];

    oaci_silk_NLSF2A( a_fix_Q12, NLSF_Q15, LPC_order, arch );

    for (i = 0; i < LPC_order; i++) {
        pAR[ i ] = ( silk_float )a_fix_Q12[ i ]*(1.0f/4096.0f);
    }
}

/******************************************/
/* Floating-point NLSF processing wrapper */
/******************************************/
void oaci_silk_process_NLSFs_FLP(
    silk_encoder_state              *psEncC,                            /* I/O  Encoder state                               */
    silk_float PredCoef[ 2 ][ MAX_LPC_ORDER ],                          /* O    Prediction coefficients                     */
    oac_int16 NLSF_Q15[      MAX_LPC_ORDER ],                          /* I/O  Normalized LSFs (quant out) (0 - (2^15-1))  */
    const oac_int16 prev_NLSF_Q15[ MAX_LPC_ORDER ]                     /* I    Previous Normalized LSFs (0 - (2^15-1))     */
    ) {
    oac_int i, j;
    oac_int16 PredCoef_Q12[ 2 ][ MAX_LPC_ORDER ];

    oaci_silk_process_NLSFs( psEncC, PredCoef_Q12, NLSF_Q15, prev_NLSF_Q15);

    for (j = 0; j < 2; j++) {
        for (i = 0; i < psEncC->predictLPCOrder; i++) {
            PredCoef[ j ][ i ] = ( silk_float )PredCoef_Q12[ j ][ i ]*(1.0f/4096.0f);
        }
    }
}

/****************************************/
/* Floating-point Silk NSQ wrapper      */
/****************************************/
void oaci_silk_NSQ_wrapper_FLP(
    silk_encoder_state_FLP          *psEnc,                             /* I/O  Encoder state FLP                           */
    silk_encoder_control_FLP        *psEncCtrl,                         /* I/O  Encoder control FLP                         */
    SideInfoIndices                 *psIndices,                         /* I/O  Quantization indices                        */
    silk_nsq_state                  *psNSQ,                             /* I/O  Noise Shaping Quantzation state             */
    oac_int8 pulses[],                                                 /* O    Quantized pulse signal                      */
    const silk_float x[]                                                /* I    Prefiltered input signal                    */
    ) {
    oac_int i, j;
    oac_int16 x16[ MAX_FRAME_LENGTH ];
    oac_int32 Gains_Q16[ MAX_NB_SUBFR ];
    silk_DWORD_ALIGN oac_int16 PredCoef_Q12[ 2 ][ MAX_LPC_ORDER ];
    oac_int16 LTPCoef_Q14[ LTP_ORDER*MAX_NB_SUBFR ];
    oac_int LTP_scale_Q14;

    /* Noise shaping parameters */
    oac_int16 AR_Q13[ MAX_NB_SUBFR*MAX_SHAPE_LPC_ORDER ];
    oac_int32 LF_shp_Q14[ MAX_NB_SUBFR ];           /* Packs two int16 coefficients per int32 value             */
    oac_int Lambda_Q10;
    oac_int Tilt_Q14[ MAX_NB_SUBFR ];
    oac_int HarmShapeGain_Q14[ MAX_NB_SUBFR ];

    /* Convert control struct to fix control struct */
    /* Noise shape parameters */
    for (i = 0; i < psEnc->sCmn.nb_subfr; i++) {
        for (j = 0; j < psEnc->sCmn.shapingLPCOrder; j++) {
            AR_Q13[ i*MAX_SHAPE_LPC_ORDER + j ] = silk_float2int( psEncCtrl->AR[ i*MAX_SHAPE_LPC_ORDER + j ]*8192.0f );
        }
    }

    for (i = 0; i < psEnc->sCmn.nb_subfr; i++) {
        LF_shp_Q14[ i ] =   silk_LSHIFT32( silk_float2int( psEncCtrl->LF_AR_shp[ i ]*16384.0f ), 16 )
                          |(oac_uint16)silk_float2int( psEncCtrl->LF_MA_shp[ i ]*16384.0f );
        Tilt_Q14[ i ]   =        (oac_int)silk_float2int( psEncCtrl->Tilt[ i ]*16384.0f );
        HarmShapeGain_Q14[ i ] = (oac_int)silk_float2int( psEncCtrl->HarmShapeGain[ i ]*16384.0f );
    }
    Lambda_Q10 = ( oac_int )silk_float2int( psEncCtrl->Lambda*1024.0f );

    /* prediction and coding parameters */
    for (i = 0; i < psEnc->sCmn.nb_subfr*LTP_ORDER; i++) {
        LTPCoef_Q14[ i ] = (oac_int16)silk_float2int( psEncCtrl->LTPCoef[ i ]*16384.0f );
    }

    for (j = 0; j < 2; j++) {
        for (i = 0; i < psEnc->sCmn.predictLPCOrder; i++) {
            PredCoef_Q12[ j ][ i ] = (oac_int16)silk_float2int( psEncCtrl->PredCoef[ j ][ i ]*4096.0f );
        }
    }

    for (i = 0; i < psEnc->sCmn.nb_subfr; i++) {
        Gains_Q16[ i ] = silk_float2int( psEncCtrl->Gains[ i ]*65536.0f );
        silk_assert( Gains_Q16[ i ] > 0 );
    }

    if (psIndices->signalType == TYPE_VOICED) {
        LTP_scale_Q14 = oaci_silk_LTPScales_table_Q14[ psIndices->LTP_scaleIndex ];
    } else {
        LTP_scale_Q14 = 0;
    }

    /* Convert input to fix */
    for (i = 0; i < psEnc->sCmn.frame_length; i++) {
        x16[ i ] = silk_float2int( x[ i ] );
    }

    /* Call NSQ */
    if (psEnc->sCmn.nStatesDelayedDecision > 1 || psEnc->sCmn.warping_Q16 > 0) {
        oaci_silk_NSQ_del_dec( &psEnc->sCmn, psNSQ, psIndices, x16, pulses, PredCoef_Q12[ 0 ], LTPCoef_Q14,
            AR_Q13, HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, psEncCtrl->pitchL, Lambda_Q10, LTP_scale_Q14,
        psEnc->sCmn.arch );
    } else {
        oaci_silk_NSQ( &psEnc->sCmn, psNSQ, psIndices, x16, pulses, PredCoef_Q12[ 0 ], LTPCoef_Q14,
            AR_Q13, HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, psEncCtrl->pitchL, Lambda_Q10, LTP_scale_Q14,
        psEnc->sCmn.arch );
    }
}

/***********************************************/
/* Floating-point Silk LTP quantiation wrapper */
/***********************************************/
void oaci_silk_quant_LTP_gains_FLP(
    silk_float B[ MAX_NB_SUBFR * LTP_ORDER ],                           /* O    Quantized LTP gains                            */
    oac_int8 cbk_index[ MAX_NB_SUBFR ],                                /* O    Codebook index                              */
    oac_int8                       *periodicity_index,                 /* O    Periodicity index                           */
    oac_int32                      *sum_log_gain_Q7,                   /* I/O  Cumulative max prediction gain  */
    silk_float                      *pred_gain_dB,                        /* O    LTP prediction gain                            */
    const silk_float XX[ MAX_NB_SUBFR * LTP_ORDER * LTP_ORDER ],                /* I    Correlation matrix                    */
    const silk_float xX[ MAX_NB_SUBFR * LTP_ORDER ],                       /* I    Correlation vector                            */
    const oac_int subfr_len,                                               /* I    Number of samples per subframe                */
    const oac_int nb_subfr,                                              /* I    Number of subframes                            */
    int arch                                                            /* I    Run-time architecture                       */
    ) {
    oac_int i, pred_gain_dB_Q7;
    oac_int16 B_Q14[ MAX_NB_SUBFR*LTP_ORDER ];
    oac_int32 XX_Q17[ MAX_NB_SUBFR*LTP_ORDER*LTP_ORDER ];
    oac_int32 xX_Q17[ MAX_NB_SUBFR*LTP_ORDER ];

    i = 0;
    do {
        XX_Q17[ i ] = (oac_int32)silk_float2int( XX[ i ]*131072.0f );
    } while (++i < nb_subfr*LTP_ORDER*LTP_ORDER);
    i = 0;
    do {
        xX_Q17[ i ] = (oac_int32)silk_float2int( xX[ i ]*131072.0f );
    } while (++i < nb_subfr*LTP_ORDER);

    oaci_silk_quant_LTP_gains( B_Q14, cbk_index, periodicity_index, sum_log_gain_Q7, &pred_gain_dB_Q7, XX_Q17, xX_Q17,
    subfr_len, nb_subfr, arch );

    for (i = 0; i < nb_subfr*LTP_ORDER; i++) {
        B[ i ] = (silk_float)B_Q14[ i ]*(1.0f/16384.0f);
    }

    *pred_gain_dB = (silk_float)pred_gain_dB_Q7*(1.0f/128.0f);
}
