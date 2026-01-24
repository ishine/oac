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

/* Copyright (c) 2010 Xiph.Org Foundation
 * Copyright (c) 2013 Parrot */
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

#if !defined(PITCH_ARM_H)
#define PITCH_ARM_H

#include "armcpu.h"

#if defined(OAC_ARM_MAY_HAVE_NEON_INTR)
oac_val32 oaci_celt_inner_prod_neon(const oac_val16 *x, const oac_val16 *y, int N);
void oaci_dual_inner_prod_neon(const oac_val16 *x, const oac_val16 *y01,
    const oac_val16 *y02, int N, oac_val32 *xy1, oac_val32 *xy2);

# if !defined(OAC_HAVE_RTCD) && defined(OAC_ARM_PRESUME_NEON)
#  define OVERRIDE_CELT_INNER_PROD (1)
#  define OVERRIDE_DUAL_INNER_PROD (1)
#  define oaci_celt_inner_prod(x, y, N, arch) ((void)(arch), PRESUME_NEON(oaci_celt_inner_prod)(x, y, N))
#  define oaci_dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) ((void)(arch), \
                                                           PRESUME_NEON(oaci_dual_inner_prod)(x, y01, y02, N, xy1, xy2))
# endif
#endif

#if !defined(OVERRIDE_CELT_INNER_PROD)
# if defined(OAC_HAVE_RTCD) && (defined(OAC_ARM_MAY_HAVE_NEON_INTR) && !defined(OAC_ARM_PRESUME_NEON_INTR))
extern oac_val32 (*const OACI_CELT_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(const oac_val16 *x, const oac_val16 *y, int N);
#  define OVERRIDE_CELT_INNER_PROD (1)
#  define oaci_celt_inner_prod(x, y, N, arch) ((*OACI_CELT_INNER_PROD_IMPL[(arch)&OAC_ARCHMASK])(x, y, N))
# elif defined(OAC_ARM_PRESUME_NEON_INTR)
#  define OVERRIDE_CELT_INNER_PROD (1)
#  define oaci_celt_inner_prod(x, y, N, arch) ((void)(arch), oaci_celt_inner_prod_neon(x, y, N))
# endif
#endif

#if !defined(OVERRIDE_DUAL_INNER_PROD)
# if defined(OAC_HAVE_RTCD) && (defined(OAC_ARM_MAY_HAVE_NEON_INTR) && !defined(OAC_ARM_PRESUME_NEON_INTR))
extern void (*const OACI_DUAL_INNER_PROD_IMPL[OAC_ARCHMASK + 1])(const oac_val16 *x,
        const oac_val16 *y01, const oac_val16 *y02, int N, oac_val32 *xy1, oac_val32 *xy2);
#  define OVERRIDE_DUAL_INNER_PROD (1)
#  define oaci_dual_inner_prod(x, y01, y02, N, xy1, xy2, \
                          arch) ((*OACI_DUAL_INNER_PROD_IMPL[(arch)&OAC_ARCHMASK])(x, y01, y02, N, xy1, xy2))
# elif defined(OAC_ARM_PRESUME_NEON_INTR)
#  define OVERRIDE_DUAL_INNER_PROD (1)
#  define oaci_dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) ((void)(arch), oaci_dual_inner_prod_neon(x, y01, y02, N, xy1, xy2))
# endif
#endif

#if defined(FIXED_POINT)

# if defined(OAC_ARM_MAY_HAVE_NEON)
oac_val32 oaci_celt_pitch_xcorr_neon(const oac_val16 *_x, const oac_val16 *_y,
    oac_val32 *xcorr, int len, int max_pitch, int arch);
# endif

# if defined(OAC_ARM_MAY_HAVE_MEDIA)
#  define oaci_celt_pitch_xcorr_media MAY_HAVE_EDSP(oaci_celt_pitch_xcorr)
# endif

# if defined(OAC_ARM_MAY_HAVE_EDSP)
oac_val32 oaci_celt_pitch_xcorr_edsp(const oac_val16 *_x, const oac_val16 *_y,
    oac_val32 *xcorr, int len, int max_pitch, int arch);
# endif

# if defined(OAC_HAVE_RTCD) && \
    ((defined(OAC_ARM_MAY_HAVE_NEON) && !defined(OAC_ARM_PRESUME_NEON)) || \
    (defined(OAC_ARM_MAY_HAVE_MEDIA) && !defined(OAC_ARM_PRESUME_MEDIA)) || \
    (defined(OAC_ARM_MAY_HAVE_EDSP) && !defined(OAC_ARM_PRESUME_EDSP)))
extern oac_val32
(*const OACI_CELT_PITCH_XCORR_IMPL[OAC_ARCHMASK + 1])(const oac_val16 *,
      const oac_val16 *, oac_val32 *, int, int, int);
#  define OVERRIDE_PITCH_XCORR (1)
#  define oaci_celt_pitch_xcorr(_x, _y, xcorr, len, max_pitch, arch) \
        ((*OACI_CELT_PITCH_XCORR_IMPL[(arch)&OAC_ARCHMASK])(_x, _y, \
                                                       xcorr, len, max_pitch, arch))

# elif defined(OAC_ARM_PRESUME_EDSP) || \
    defined(OAC_ARM_PRESUME_MEDIA) || \
    defined(OAC_ARM_PRESUME_NEON)
#  define OVERRIDE_PITCH_XCORR (1)
#  define oaci_celt_pitch_xcorr (PRESUME_NEON(oaci_celt_pitch_xcorr))

# endif

# if defined(OAC_ARM_MAY_HAVE_NEON_INTR)
void oaci_xcorr_kernel_neon_fixed(
    const oac_val16 *x,
    const oac_val16 *y,
    oac_val32 sum[4],
    int len);
# endif

# if defined(OAC_HAVE_RTCD) && \
    (defined(OAC_ARM_MAY_HAVE_NEON_INTR) && !defined(OAC_ARM_PRESUME_NEON_INTR))

extern void (*const OACI_XCORR_KERNEL_IMPL[OAC_ARCHMASK + 1])(
                    const oac_val16 *x,
                    const oac_val16 *y,
                    oac_val32 sum[4],
                    int len);

#  define OVERRIDE_XCORR_KERNEL (1)
#  define oaci_xcorr_kernel(x, y, sum, len, arch) \
        ((*OACI_XCORR_KERNEL_IMPL[(arch)&OAC_ARCHMASK])(x, y, sum, len))

# elif defined(OAC_ARM_PRESUME_NEON_INTR)
#  define OVERRIDE_XCORR_KERNEL (1)
#  define oaci_xcorr_kernel(x, y, sum, len, arch) \
        ((void)arch, oaci_xcorr_kernel_neon_fixed(x, y, sum, len))

# endif

#else /* Start !FIXED_POINT */
/* Float case */
# if defined(OAC_ARM_MAY_HAVE_NEON_INTR)
void oaci_celt_pitch_xcorr_float_neon(const oac_val16 *_x, const oac_val16 *_y,
    oac_val32 *xcorr, int len, int max_pitch, int arch);
# endif

# if defined(OAC_HAVE_RTCD) && \
    (defined(OAC_ARM_MAY_HAVE_NEON_INTR) && !defined(OAC_ARM_PRESUME_NEON_INTR))
extern void
(*const OACI_CELT_PITCH_XCORR_IMPL[OAC_ARCHMASK + 1])(const oac_val16 *,
      const oac_val16 *, oac_val32 *, int, int, int);

#  define OVERRIDE_PITCH_XCORR (1)
#  define oaci_celt_pitch_xcorr(_x, _y, xcorr, len, max_pitch, arch) \
        ((*OACI_CELT_PITCH_XCORR_IMPL[(arch)&OAC_ARCHMASK])(_x, _y, \
                                                       xcorr, len, max_pitch, arch))

# elif defined(OAC_ARM_PRESUME_NEON_INTR)

#  define OVERRIDE_PITCH_XCORR (1)
#  define oaci_celt_pitch_xcorr oaci_celt_pitch_xcorr_float_neon

# endif

#endif /* end !FIXED_POINT */

#endif
