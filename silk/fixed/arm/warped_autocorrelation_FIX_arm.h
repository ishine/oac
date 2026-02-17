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
   Copyright (c) 2017 Google Inc.
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

#ifndef SILK_WARPED_AUTOCORRELATION_FIX_ARM_H
#define SILK_WARPED_AUTOCORRELATION_FIX_ARM_H

#include "celt/arm/armcpu.h"

#if defined(FIXED_POINT)

# if defined(OAC_ARM_MAY_HAVE_NEON_INTR)
void silk_warped_autocorrelation_FIX_neon(
    oac_int32                *corr,                                        /* O    Result [order + 1]                                                          */
    oac_int                  *scale,                                       /* O    Scaling of the correlation vector                                           */
    const oac_int16                *input,                                 /* I    Input data to correlate                                                     */
    const oac_int warping_Q16,                                             /* I    Warping coefficient                                                         */
    const oac_int length,                                                  /* I    Length of input                                                             */
    const oac_int order                                                    /* I    Correlation order (even)                                                    */
    );

#  if !defined(OAC_HAVE_RTCD) && defined(OAC_ARM_PRESUME_NEON)
#   define OVERRIDE_silk_warped_autocorrelation_FIX (1)
#   define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
        ((void)(arch), PRESUME_NEON(silk_warped_autocorrelation_FIX)(corr, scale, input, warping_Q16, length, order))
#  endif
# endif

# if !defined(OVERRIDE_silk_warped_autocorrelation_FIX)
/*Is run-time CPU detection enabled on this platform?*/
#  if defined(OAC_HAVE_RTCD) && (defined(OAC_ARM_MAY_HAVE_NEON_INTR) && !defined(OAC_ARM_PRESUME_NEON_INTR))
extern void (*const SILK_WARPED_AUTOCORRELATION_FIX_IMPL[OAC_ARCHMASK + 1])(oac_int32*, oac_int*, const oac_int16*,
const oac_int, const oac_int, const oac_int);
#   define OVERRIDE_silk_warped_autocorrelation_FIX (1)
#   define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
        ((*SILK_WARPED_AUTOCORRELATION_FIX_IMPL[(arch)&OAC_ARCHMASK])(corr, scale, input, warping_Q16, length, order))
#  elif defined(OAC_ARM_PRESUME_NEON_INTR)
#   define OVERRIDE_silk_warped_autocorrelation_FIX (1)
#   define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
        ((void)(arch), silk_warped_autocorrelation_FIX_neon(corr, scale, input, warping_Q16, length, order))
#  endif
# endif

#endif /* end FIXED_POINT */

#endif /* end SILK_WARPED_AUTOCORRELATION_FIX_ARM_H */
