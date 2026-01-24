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

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "x86/x86cpu.h"
#include "celt_lpc.h"
#include "pitch.h"
#include "pitch_sse.h"
#include "vq.h"

#if defined(OAC_HAVE_RTCD)

# if defined(FIXED_POINT)

#  if defined(OAC_X86_MAY_HAVE_SSE4_1) && !defined(OAC_X86_PRESUME_SSE4_1)

void (*const OACI_CELT_FIR_IMPL[OAC_ARCHMASK + 1])(
         const oac_val16 *x,
         const oac_val16 *num,
         oac_val16       *y,
         int N,
         int ord,
         int arch
    ) = {
    oaci_celt_fir_c,              /* non-sse */
    oaci_celt_fir_c,
    oaci_celt_fir_c,
    MAY_HAVE_SSE4_1(oaci_celt_fir), /* sse4.1  */
    MAY_HAVE_SSE4_1(oaci_celt_fir) /* avx  */
};

void (*const OACI_XCORR_KERNEL_IMPL[OAC_ARCHMASK + 1])(
         const oac_val16 *x,
         const oac_val16 *y,
         oac_val32 sum[4],
         int len
    ) = {
    oaci_xcorr_kernel_c,              /* non-sse */
    oaci_xcorr_kernel_c,
    oaci_xcorr_kernel_c,
    MAY_HAVE_SSE4_1(oaci_xcorr_kernel), /* sse4.1  */
    MAY_HAVE_SSE4_1(oaci_xcorr_kernel) /* avx  */
};

#  endif

#  if (defined(OAC_X86_MAY_HAVE_SSE4_1) && !defined(OAC_X86_PRESUME_SSE4_1)) ||  \
    (!defined(OAC_X86_MAY_HAVE_SSE_4_1) && defined(OAC_X86_MAY_HAVE_SSE2) && !defined(OAC_X86_PRESUME_SSE2))

oac_val32 (*const OACI_CELT_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(
         const oac_val16 *x,
         const oac_val16 *y,
         int N
    ) = {
    oaci_celt_inner_prod_c,              /* non-sse */
    oaci_celt_inner_prod_c,
    MAY_HAVE_SSE2(oaci_celt_inner_prod),
    MAY_HAVE_SSE4_1(oaci_celt_inner_prod), /* sse4.1  */
    MAY_HAVE_SSE4_1(oaci_celt_inner_prod) /* avx  */
};

#  endif

# else

#  if defined(OAC_X86_MAY_HAVE_AVX2) && !defined(OAC_X86_PRESUME_AVX2)

void (*const OACI_PITCH_XCORR_IMPL[OAC_ARCHMASK + 1])(
         const float *_x,
         const float *_y,
         float *xcorr,
         int len,
         int max_pitch,
         int arch
    ) = {
    oaci_celt_pitch_xcorr_c,              /* non-sse */
    oaci_celt_pitch_xcorr_c,
    oaci_celt_pitch_xcorr_c,
    oaci_celt_pitch_xcorr_c,
    MAY_HAVE_AVX2(oaci_celt_pitch_xcorr)
};

#  endif


#  if defined(OAC_X86_MAY_HAVE_SSE) && !defined(OAC_X86_PRESUME_SSE)

void (*const OACI_XCORR_KERNEL_IMPL[OAC_ARCHMASK + 1])(
         const oac_val16 *x,
         const oac_val16 *y,
         oac_val32 sum[4],
         int len
    ) = {
    oaci_xcorr_kernel_c,              /* non-sse */
    MAY_HAVE_SSE(oaci_xcorr_kernel),
    MAY_HAVE_SSE(oaci_xcorr_kernel),
    MAY_HAVE_SSE(oaci_xcorr_kernel),
    MAY_HAVE_SSE(oaci_xcorr_kernel)
};

oac_val32 (*const OACI_CELT_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(
         const oac_val16 *x,
         const oac_val16 *y,
         int N
    ) = {
    oaci_celt_inner_prod_c,              /* non-sse */
    MAY_HAVE_SSE(oaci_celt_inner_prod),
    MAY_HAVE_SSE(oaci_celt_inner_prod),
    MAY_HAVE_SSE(oaci_celt_inner_prod),
    MAY_HAVE_SSE(oaci_celt_inner_prod)
};

void (*const OACI_DUAL_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(
                    const oac_val16 *x,
                    const oac_val16 *y01,
                    const oac_val16 *y02,
                    int N,
                    oac_val32       *xy1,
                    oac_val32       *xy2
    ) = {
    oaci_dual_inner_prod_c,              /* non-sse */
    MAY_HAVE_SSE(oaci_dual_inner_prod),
    MAY_HAVE_SSE(oaci_dual_inner_prod),
    MAY_HAVE_SSE(oaci_dual_inner_prod),
    MAY_HAVE_SSE(oaci_dual_inner_prod)
};

void (*const OACI_COMB_FILTER_CONST_IMPL[OAC_ARCHMASK + 1])(
              oac_val32 *y,
              oac_val32 *x,
              int T,
              int N,
              oac_val16 g10,
              oac_val16 g11,
              oac_val16 g12
    ) = {
    oaci_comb_filter_const_c,              /* non-sse */
    MAY_HAVE_SSE(oaci_comb_filter_const),
    MAY_HAVE_SSE(oaci_comb_filter_const),
    MAY_HAVE_SSE(oaci_comb_filter_const),
    MAY_HAVE_SSE(oaci_comb_filter_const)
};


#  endif

#  if defined(OAC_X86_MAY_HAVE_SSE2) && !defined(OAC_X86_PRESUME_SSE2)
oac_val16 (*const OACI_OP_PVQ_SEARCH_IMPL[OAC_ARCHMASK + 1])(
      celt_norm *_X, int *iy, int K, int N, int arch
    ) = {
    oaci_op_pvq_search_c,              /* non-sse */
    oaci_op_pvq_search_c,
    MAY_HAVE_SSE2(oaci_op_pvq_search),
    MAY_HAVE_SSE2(oaci_op_pvq_search),
    MAY_HAVE_SSE2(oaci_op_pvq_search)
};
#  endif

# endif
#endif
