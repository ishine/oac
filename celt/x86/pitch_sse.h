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

/* Copyright (c) 2013 Jean-Marc Valin and John Ridges
   Copyright (c) 2014, Cisco Systems, INC MingXiang WeiZhou MinPeng YanWang*/
/**
   @file pitch_sse.h
   @brief Pitch analysis
 */

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

#ifndef PITCH_SSE_H
#define PITCH_SSE_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#if defined(OAC_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)
void oaci_xcorr_kernel_sse4_1(
    const oac_int16 *x,
    const oac_int16 *y,
    oac_val32 sum[4],
    int len);
#endif

#if defined(OAC_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)
void oaci_xcorr_kernel_sse(
    const oac_val16 *x,
    const oac_val16 *y,
    oac_val32 sum[4],
    int len);
#endif

#if defined(OAC_X86_PRESUME_SSE4_1) && defined(FIXED_POINT)
# define OVERRIDE_XCORR_KERNEL
# define oaci_xcorr_kernel(x, y, sum, len, arch) \
        ((void)arch, oaci_xcorr_kernel_sse4_1(x, y, sum, len))

#elif defined(OAC_X86_PRESUME_SSE) && !defined(FIXED_POINT)
# define OVERRIDE_XCORR_KERNEL
# define oaci_xcorr_kernel(x, y, sum, len, arch) \
        ((void)arch, oaci_xcorr_kernel_sse(x, y, sum, len))

#elif defined(OAC_HAVE_RTCD) &&  ((defined(OAC_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)) \
    || (defined(OAC_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)))

extern void (*const OACI_XCORR_KERNEL_IMPL[OAC_ARCHMASK + 1])(
                    const oac_val16 *x,
                    const oac_val16 *y,
                    oac_val32 sum[4],
                    int len);

# define OVERRIDE_XCORR_KERNEL
# define oaci_xcorr_kernel(x, y, sum, len, arch) \
        ((*OACI_XCORR_KERNEL_IMPL[(arch)&OAC_ARCHMASK])(x, y, sum, len))

#endif

#if defined(OAC_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)
oac_val32 oaci_celt_inner_prod_sse4_1(
    const oac_int16 *x,
    const oac_int16 *y,
    int N);
#endif

#if defined(OAC_X86_MAY_HAVE_SSE2) && defined(FIXED_POINT)
oac_val32 oaci_celt_inner_prod_sse2(
    const oac_int16 *x,
    const oac_int16 *y,
    int N);
#endif

#if defined(OAC_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)
oac_val32 oaci_celt_inner_prod_sse(
    const oac_val16 *x,
    const oac_val16 *y,
    int N);
#endif


#if defined(OAC_X86_PRESUME_SSE4_1) && defined(FIXED_POINT)
# define OVERRIDE_CELT_INNER_PROD
# define oaci_celt_inner_prod(x, y, N, arch) \
        ((void)arch, oaci_celt_inner_prod_sse4_1(x, y, N))

#elif defined(OAC_X86_PRESUME_SSE2) && defined(FIXED_POINT) && !defined(OAC_X86_MAY_HAVE_SSE4_1)
# define OVERRIDE_CELT_INNER_PROD
# define oaci_celt_inner_prod(x, y, N, arch) \
        ((void)arch, oaci_celt_inner_prod_sse2(x, y, N))

#elif defined(OAC_X86_PRESUME_SSE) && !defined(FIXED_POINT)
# define OVERRIDE_CELT_INNER_PROD
# define oaci_celt_inner_prod(x, y, N, arch) \
        ((void)arch, oaci_celt_inner_prod_sse(x, y, N))


#elif defined(OAC_HAVE_RTCD) && (((defined(OAC_X86_MAY_HAVE_SSE4_1) || defined(OAC_X86_MAY_HAVE_SSE2)) \
    && defined(FIXED_POINT)) || \
    (defined(OAC_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)))

extern oac_val32 (*const OACI_CELT_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(
                    const oac_val16 *x,
                    const oac_val16 *y,
                    int N);

# define OVERRIDE_CELT_INNER_PROD
# define oaci_celt_inner_prod(x, y, N, arch) \
        ((*OACI_CELT_INNER_PROD_IMPL[(arch)&OAC_ARCHMASK])(x, y, N))

#endif

#if defined(OAC_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)

void oaci_dual_inner_prod_sse(const oac_val16 *x,
    const oac_val16 *y01,
    const oac_val16 *y02,
    int N,
    oac_val32       *xy1,
    oac_val32       *xy2);

void oaci_comb_filter_const_sse(oac_val32 *y,
    oac_val32 *x,
    int T,
    int N,
    oac_val16 g10,
    oac_val16 g11,
    oac_val16 g12);


# if defined(OAC_X86_PRESUME_SSE)
#  define OVERRIDE_DUAL_INNER_PROD
#  define OVERRIDE_COMB_FILTER_CONST
#  define oaci_dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) \
        ((void)(arch), oaci_dual_inner_prod_sse(x, y01, y02, N, xy1, xy2))

#  define oaci_comb_filter_const(y, x, T, N, g10, g11, g12, arch) \
        ((void)(arch), oaci_comb_filter_const_sse(y, x, T, N, g10, g11, g12))
# elif defined(OAC_HAVE_RTCD)

#  define OVERRIDE_DUAL_INNER_PROD
#  define OVERRIDE_COMB_FILTER_CONST
extern void (*const OACI_DUAL_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(
              const oac_val16 *x,
              const oac_val16 *y01,
              const oac_val16 *y02,
              int N,
              oac_val32       *xy1,
              oac_val32       *xy2);

#  define oaci_dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) \
        ((*OACI_DUAL_INNER_PROD_IMPL[(arch)&OAC_ARCHMASK])(x, y01, y02, N, xy1, xy2))

extern void (*const OACI_COMB_FILTER_CONST_IMPL[OAC_ARCHMASK + 1])(
              oac_val32 *y,
              oac_val32 *x,
              int T,
              int N,
              oac_val16 g10,
              oac_val16 g11,
              oac_val16 g12);

#  define oaci_comb_filter_const(y, x, T, N, g10, g11, g12, arch) \
        ((*OACI_COMB_FILTER_CONST_IMPL[(arch)&OAC_ARCHMASK])(y, x, T, N, g10, g11, g12))

#  define NON_STATIC_COMB_FILTER_CONST_C

# endif

void oaci_celt_pitch_xcorr_avx2(const float *_x, const float *_y, float *xcorr, int len, int max_pitch, int arch);

# if defined(OAC_X86_PRESUME_AVX2)

#  define OVERRIDE_PITCH_XCORR
#  define oaci_celt_pitch_xcorr oaci_celt_pitch_xcorr_avx2

# elif defined(OAC_HAVE_RTCD) && defined(OAC_X86_MAY_HAVE_AVX2)

#  define OVERRIDE_PITCH_XCORR
extern void (*const OACI_PITCH_XCORR_IMPL[OAC_ARCHMASK + 1])(
              const float *_x,
              const float *_y,
              float *xcorr,
              int len,
              int max_pitch,
              int arch
    );

#  define oaci_celt_pitch_xcorr(_x, _y, xcorr, len, max_pitch, arch) \
        ((*OACI_PITCH_XCORR_IMPL[(arch)&OAC_ARCHMASK])(_x, _y, xcorr, len, max_pitch, arch))


# endif /* OAC_X86_PRESUME_AVX2 && !OAC_HAVE_RTCD */

#endif /* OAC_X86_MAY_HAVE_SSE && !FIXED_POINT */

#endif
