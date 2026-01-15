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

#ifndef SILK_MAIN_FIX_H
#define SILK_MAIN_FIX_H

#include "SigProc_FIX.h"
#include "structs_FIX.h"
#include "control.h"
#include "main.h"
#include "PLC.h"
#include "debug.h"
#include "entenc.h"

#if ((defined(OAC_ARM_ASM) && defined(FIXED_POINT)) \
   || defined(OAC_ARM_MAY_HAVE_NEON_INTR))
#include "fixed/arm/warped_autocorrelation_FIX_arm.h"
#endif


#define silk_encoder_state_Fxx      silk_encoder_state_FIX
#define silk_encode_do_VAD_Fxx      silk_encode_do_VAD_FIX
#define silk_encode_frame_Fxx       silk_encode_frame_FIX

#define QC  10
#define QS  13

/*********************/
/* Encoder Functions */
/*********************/

/* High-pass filter with cutoff frequency adaptation based on pitch lag statistics */
void silk_HP_variable_cutoff(
    silk_encoder_state_Fxx          state_Fxx[]                             /* I/O  Encoder states                                                              */
);

/* Encoder main function */
void silk_encode_do_VAD_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  Pointer to Silk FIX encoder state                                           */
    oac_int                        activity                                /* I    Decision of Oac voice activity detector                                    */
);

/* Encoder main function */
oac_int silk_encode_frame_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  Pointer to Silk FIX encoder state                                           */
    oac_int32                      *pnBytesOut,                            /* O    Pointer to number of payload bytes;                                         */
    ec_enc                          *psRangeEnc,                            /* I/O  compressor data structure                                                   */
    oac_int                        condCoding,                             /* I    The type of conditional coding to use                                       */
    oac_int                        maxBits,                                /* I    If > 0: maximum number of output bits                                       */
    oac_int                        useCBR                                  /* I    Flag to force constant-bitrate operation                                    */
);

/* Initializes the Silk encoder state */
oac_int silk_init_encoder(
    silk_encoder_state_Fxx          *psEnc,                                 /* I/O  Pointer to Silk FIX encoder state                                           */
    int                              arch                                   /* I    Run-time architecture                                                       */
);

/* Control the Silk encoder */
oac_int silk_control_encoder(
    silk_encoder_state_Fxx          *psEnc,                                 /* I/O  Pointer to Silk encoder state                                               */
    silk_EncControlStruct           *encControl,                            /* I    Control structure                                                           */
    const oac_int                  allow_bw_switch,                        /* I    Flag to allow switching audio bandwidth                                     */
    const oac_int                  channelNb,                              /* I    Channel number                                                              */
    const oac_int                  force_fs_kHz
);

/**************************/
/* Noise shaping analysis */
/**************************/
/* Compute noise shaping coefficients and initial gain values */
void silk_noise_shape_analysis_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  Encoder state FIX                                                           */
    silk_encoder_control_FIX        *psEncCtrl,                             /* I/O  Encoder control FIX                                                         */
    const oac_int16                *pitch_res,                             /* I    LPC residual from pitch analysis                                            */
    const oac_int16                *x,                                     /* I    Input signal [ frame_length + la_shape ]                                    */
    int                              arch                                   /* I    Run-time architecture                                                       */
);

/* Autocorrelations for a warped frequency axis */
void silk_warped_autocorrelation_FIX_c(
          oac_int32                *corr,                                  /* O    Result [order + 1]                                                          */
          oac_int                  *scale,                                 /* O    Scaling of the correlation vector                                           */
    const oac_int16                *input,                                 /* I    Input data to correlate                                                     */
    const oac_int                  warping_Q16,                            /* I    Warping coefficient                                                         */
    const oac_int                  length,                                 /* I    Length of input                                                             */
    const oac_int                  order                                   /* I    Correlation order (even)                                                    */
);

#if !defined(OVERRIDE_silk_warped_autocorrelation_FIX)
#define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
        ((void)(arch), silk_warped_autocorrelation_FIX_c(corr, scale, input, warping_Q16, length, order))
#endif

/* Calculation of LTP state scaling */
void silk_LTP_scale_ctrl_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  encoder state                                                               */
    silk_encoder_control_FIX        *psEncCtrl,                             /* I/O  encoder control                                                             */
    oac_int                        condCoding                              /* I    The type of conditional coding to use                                       */
);

/**********************************************/
/* Prediction Analysis                        */
/**********************************************/
/* Find pitch lags */
void silk_find_pitch_lags_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  encoder state                                                               */
    silk_encoder_control_FIX        *psEncCtrl,                             /* I/O  encoder control                                                             */
    oac_int16                      res[],                                  /* O    residual                                                                    */
    const oac_int16                x[],                                    /* I    Speech signal                                                               */
    int                             arch                                    /* I    Run-time architecture                                                       */
);

/* Find LPC and LTP coefficients */
void silk_find_pred_coefs_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  encoder state                                                               */
    silk_encoder_control_FIX        *psEncCtrl,                             /* I/O  encoder control                                                             */
    const oac_int16                res_pitch[],                            /* I    Residual from pitch analysis                                                */
    const oac_int16                x[],                                    /* I    Speech signal                                                               */
    oac_int                        condCoding                              /* I    The type of conditional coding to use                                       */
);

/* LPC analysis */
void silk_find_LPC_FIX(
    silk_encoder_state              *psEncC,                                /* I/O  Encoder state                                                               */
    oac_int16                      NLSF_Q15[],                             /* O    NLSFs                                                                       */
    const oac_int16                x[],                                    /* I    Input signal                                                                */
    const oac_int32                minInvGain_Q30                          /* I    Inverse of max prediction gain                                              */
);

/* LTP analysis */
void silk_find_LTP_FIX(
    oac_int32                      XXLTP_Q17[ MAX_NB_SUBFR * LTP_ORDER * LTP_ORDER ], /* O    Correlation matrix                                               */
    oac_int32                      xXLTP_Q17[ MAX_NB_SUBFR * LTP_ORDER ],  /* O    Correlation vector                                                          */
    const oac_int16                r_lpc[],                                /* I    Residual signal after LPC                                                   */
    const oac_int                  lag[ MAX_NB_SUBFR ],                    /* I    LTP lags                                                                    */
    const oac_int                  subfr_length,                           /* I    Subframe length                                                             */
    const oac_int                  nb_subfr,                               /* I    Number of subframes                                                         */
    int                             arch                                    /* I    Run-time architecture                                                       */
);

void silk_LTP_analysis_filter_FIX(
    oac_int16                      *LTP_res,                               /* O    LTP residual signal of length MAX_NB_SUBFR * ( pre_length + subfr_length )  */
    const oac_int16                *x,                                     /* I    Pointer to input signal with at least max( pitchL ) preceding samples       */
    const oac_int16                LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],/* I    LTP_ORDER LTP coefficients for each MAX_NB_SUBFR subframe                   */
    const oac_int                  pitchL[ MAX_NB_SUBFR ],                 /* I    Pitch lag, one for each subframe                                            */
    const oac_int32                invGains_Q16[ MAX_NB_SUBFR ],           /* I    Inverse quantization gains, one for each subframe                           */
    const oac_int                  subfr_length,                           /* I    Length of each subframe                                                     */
    const oac_int                  nb_subfr,                               /* I    Number of subframes                                                         */
    const oac_int                  pre_length                              /* I    Length of the preceding samples starting at &x[0] for each subframe         */
);

/* Calculates residual energies of input subframes where all subframes have LPC_order   */
/* of preceding samples                                                                 */
void silk_residual_energy_FIX(
          oac_int32                nrgs[ MAX_NB_SUBFR ],                   /* O    Residual energy per subframe                                                */
          oac_int                  nrgsQ[ MAX_NB_SUBFR ],                  /* O    Q value per subframe                                                        */
    const oac_int16                x[],                                    /* I    Input signal                                                                */
          oac_int16                a_Q12[ 2 ][ MAX_LPC_ORDER ],            /* I    AR coefs for each frame half                                                */
    const oac_int32                gains[ MAX_NB_SUBFR ],                  /* I    Quantization gains                                                          */
    const oac_int                  subfr_length,                           /* I    Subframe length                                                             */
    const oac_int                  nb_subfr,                               /* I    Number of subframes                                                         */
    const oac_int                  LPC_order,                              /* I    LPC order                                                                   */
    int                             arch                                    /* I    Run-time architecture                                                       */
);

/* Residual energy: nrg = wxx - 2 * wXx * c + c' * wXX * c */
oac_int32 silk_residual_energy16_covar_FIX(
    const oac_int16                *c,                                     /* I    Prediction vector                                                           */
    const oac_int32                *wXX,                                   /* I    Correlation matrix                                                          */
    const oac_int32                *wXx,                                   /* I    Correlation vector                                                          */
    oac_int32                      wxx,                                    /* I    Signal energy                                                               */
    oac_int                        D,                                      /* I    Dimension                                                                   */
    oac_int                        cQ                                      /* I    Q value for c vector 0 - 15                                                 */
);

/* Processing of gains */
void silk_process_gains_FIX(
    silk_encoder_state_FIX          *psEnc,                                 /* I/O  Encoder state                                                               */
    silk_encoder_control_FIX        *psEncCtrl,                             /* I/O  Encoder control                                                             */
    oac_int                        condCoding                              /* I    The type of conditional coding to use                                       */
);

/******************/
/* Linear Algebra */
/******************/
/* Calculates correlation matrix X'*X */
void silk_corrMatrix_FIX(
    const oac_int16                *x,                                     /* I    x vector [L + order - 1] used to form data matrix X                         */
    const oac_int                  L,                                      /* I    Length of vectors                                                           */
    const oac_int                  order,                                  /* I    Max lag for correlation                                                     */
    oac_int32                      *XX,                                    /* O    Pointer to X'*X correlation matrix [ order x order ]                        */
    oac_int32                      *nrg,                                   /* O    Energy of x vector                                                          */
    oac_int                        *rshifts,                               /* O    Right shifts of correlations                                                */
    int                              arch                                   /* I    Run-time architecture                                                       */
);

/* Calculates correlation vector X'*t */
void silk_corrVector_FIX(
    const oac_int16                *x,                                     /* I    x vector [L + order - 1] used to form data matrix X                         */
    const oac_int16                *t,                                     /* I    Target vector [L]                                                           */
    const oac_int                  L,                                      /* I    Length of vectors                                                           */
    const oac_int                  order,                                  /* I    Max lag for correlation                                                     */
    oac_int32                      *Xt,                                    /* O    Pointer to X'*t correlation vector [order]                                  */
    const oac_int                  rshifts,                                /* I    Right shifts of correlations                                                */
    int                             arch                                    /* I    Run-time architecture                                                       */
);

#endif /* SILK_MAIN_FIX_H */
