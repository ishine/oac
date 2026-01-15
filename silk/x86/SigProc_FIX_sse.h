/* Copyright (c) 2014, Cisco Systems, INC
   Written by XiangMingZhu WeiZhou MinPeng YanWang

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

#ifndef SIGPROC_FIX_SSE_H
#define SIGPROC_FIX_SSE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(OAC_X86_MAY_HAVE_SSE4_1)
void silk_burg_modified_sse4_1(
    oac_int32                  *res_nrg,           /* O    Residual energy                                             */
    oac_int                    *res_nrg_Q,         /* O    Residual energy Q value                                     */
    oac_int32 A_Q16[],                             /* O    Prediction coefficients (length order)                      */
    const oac_int16 x[],                           /* I    Input signal, length: nb_subfr * ( D + subfr_length )       */
    const oac_int32 minInvGain_Q30,                /* I    Inverse of max prediction gain                              */
    const oac_int subfr_length,                    /* I    Input signal subframe length (incl. D preceding samples)    */
    const oac_int nb_subfr,                        /* I    Number of subframes stacked in x                            */
    const oac_int D,                               /* I    Order                                                       */
    int arch                                        /* I    Run-time architecture                                       */
    );

# if defined(OAC_X86_PRESUME_SSE4_1)

#  define OVERRIDE_silk_burg_modified
#  define silk_burg_modified(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch) \
        ((void)(arch), \
         silk_burg_modified_sse4_1(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch))

# elif defined(OAC_HAVE_RTCD)

extern void (*const SILK_BURG_MODIFIED_IMPL[OAC_ARCHMASK + 1])(
    oac_int32                  *res_nrg,           /* O    Residual energy                                             */
    oac_int                    *res_nrg_Q,         /* O    Residual energy Q value                                     */
    oac_int32 A_Q16[],                             /* O    Prediction coefficients (length order)                      */
    const oac_int16 x[],                           /* I    Input signal, length: nb_subfr * ( D + subfr_length )       */
    const oac_int32 minInvGain_Q30,                /* I    Inverse of max prediction gain                              */
    const oac_int subfr_length,                    /* I    Input signal subframe length (incl. D preceding samples)    */
    const oac_int nb_subfr,                        /* I    Number of subframes stacked in x                            */
    const oac_int D,                               /* I    Order                                                       */
    int arch /* I    Run-time architecture                                       */);

#  define OVERRIDE_silk_burg_modified
#  define silk_burg_modified(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch) \
        ((*SILK_BURG_MODIFIED_IMPL[(arch)&OAC_ARCHMASK])(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, \
                                                         nb_subfr, D, arch))

# endif

oac_int64 silk_inner_prod16_sse4_1(
    const oac_int16 *inVec1,
    const oac_int16 *inVec2,
    const oac_int len);


# if defined(OAC_X86_PRESUME_SSE4_1)

#  define OVERRIDE_silk_inner_prod16
#  define silk_inner_prod16(inVec1, inVec2, len, arch) \
        ((void)(arch), silk_inner_prod16_sse4_1(inVec1, inVec2, len))

# elif defined(OAC_HAVE_RTCD)

extern oac_int64 (*const SILK_INNER_PROD16_IMPL[OAC_ARCHMASK + 1])(
                    const oac_int16 *inVec1,
                    const oac_int16 *inVec2,
                    const oac_int len);

#  define OVERRIDE_silk_inner_prod16
#  define silk_inner_prod16(inVec1, inVec2, len, arch) \
        ((*SILK_INNER_PROD16_IMPL[(arch)&OAC_ARCHMASK])(inVec1, inVec2, len))

# endif
#endif
#endif
