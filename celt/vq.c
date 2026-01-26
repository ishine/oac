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

/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Written by Jean-Marc Valin */
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
#include "cwrs.h"
#include "vq.h"
#include "arch.h"
#include "os_support.h"
#include "bands.h"
#include "rate.h"
#include "pitch.h"

#if defined(FIXED_POINT)
void oaci_norm_scaleup(celt_norm *X, int N, int shift) {
    int i;
    celt_assert(shift >= 0);
    if (shift <= 0) return;
    for (i = 0; i < N; i++) X[i] = SHL32(X[i], shift);
}

void oaci_norm_scaledown(celt_norm *X, int N, int shift) {
    int i;
    celt_assert(shift >= 0);
    if (shift <= 0) return;
    for (i = 0; i < N; i++) X[i] = PSHR32(X[i], shift);
}

oac_val32 oaci_celt_inner_prod_norm(const celt_norm *x, const celt_norm *y, int len, int arch) {
    int i;
    oac_val32 sum = 0;
    (void)arch;
    for (i = 0; i < len; i++) sum += x[i]*y[i];
    return sum;
}
oac_val32 oaci_celt_inner_prod_norm_shift(const celt_norm *x, const celt_norm *y, int len, int arch) {
    int i;
    oac_val64 sum = 0;
    (void)arch;
    for (i = 0; i < len; i++) sum += x[i]*(oac_val64)y[i];
    return sum>>2*(NORM_SHIFT - 14);
}
#endif

#ifndef OVERRIDE_vq_exp_rotation1
static void oaci_exp_rotation1(celt_norm *X, int len, int stride, oac_val16 c, oac_val16 s) {
    int i;
    oac_val16 ms;
    celt_norm *Xptr;
    Xptr = X;
    ms = NEG16(s);
    oaci_norm_scaledown(X, len, NORM_SHIFT - 14);
    for (i = 0; i < len - stride; i++) {
        celt_norm x1, x2;
        x1 = Xptr[0];
        x2 = Xptr[stride];
        Xptr[stride] = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x2),  s, x1), 15));
        *Xptr++      = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x1), ms, x2), 15));
    }
    Xptr = &X[len - 2*stride - 1];
    for (i = len - 2*stride - 1; i >= 0; i--) {
        celt_norm x1, x2;
        x1 = Xptr[0];
        x2 = Xptr[stride];
        Xptr[stride] = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x2),  s, x1), 15));
        *Xptr--      = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x1), ms, x2), 15));
    }
    oaci_norm_scaleup(X, len, NORM_SHIFT - 14);
}
#endif /* OVERRIDE_vq_exp_rotation1 */

void oaci_exp_rotation(celt_norm *X, int len, int dir, int stride, int K, int spread) {
    static const int SPREAD_FACTOR[3] = {15, 10, 5};
    int i;
    oac_val16 c, s;
    oac_val16 gain, theta;
    int stride2 = 0;
    int factor;

    if (2*K >= len || spread == SPREAD_NONE)
        return;
    factor = SPREAD_FACTOR[spread - 1];

    gain = oaci_celt_div((oac_val32)MULT16_16(Q15_ONE, len), (oac_val32)(len + factor*K));
    theta = HALF16(MULT16_16_Q15(gain, gain));

    c = oaci_celt_cos_norm(EXTEND32(theta));
    s = oaci_celt_cos_norm(EXTEND32(SUB16(Q15ONE, theta))); /*  sin(theta) */

    if (len >= 8*stride) {
        stride2 = 1;
        /* This is just a simple (equivalent) way of computing sqrt(len/stride) with rounding.
           It's basically incrementing long as (stride2+0.5)^2 < len/stride. */
        while ((stride2*stride2 + stride2)*stride + (stride>>2) < len)
            stride2++;
    }
    /*NOTE: As a minor optimization, we could be passing around log2(B), not B, for both this and for
       oaci_extract_collapse_mask().*/
    len = oaci_celt_udiv(len, stride);
    for (i = 0; i < stride; i++) {
        if (dir < 0) {
            if (stride2)
                oaci_exp_rotation1(X + i*len, len, stride2, s, c);
            oaci_exp_rotation1(X + i*len, len, 1, c, s);
        } else {
            oaci_exp_rotation1(X + i*len, len, 1, c, -s);
            if (stride2)
                oaci_exp_rotation1(X + i*len, len, stride2, s, -c);
        }
    }
}

/** Normalizes the decoded integer pvq codeword to unit norm. */
static void oaci_normalise_residual(int * OAC_RESTRICT iy, celt_norm * OAC_RESTRICT X,
                               int N, oac_val32 Ryy, oac_val32 gain, int shift) {
    int i;
#ifdef FIXED_POINT
    int k;
#endif
    oac_val32 t;
    oac_val32 g;

#ifdef FIXED_POINT
    k = oaci_celt_ilog2(Ryy)>>1;
#endif
    t = VSHR32(Ryy, 2*(k - 7) - 15);
    g = MULT32_32_Q31(oaci_celt_rsqrt_norm32(t), gain);
    i = 0;
    (void)shift;
#if defined(FIXED_POINT)
    if (shift > 0) {
        int tot_shift = NORM_SHIFT + 1 - k - shift;
        if (tot_shift >= 0) {
            do X[i] = MULT32_32_Q31(g, SHL32(iy[i], tot_shift));
            while (++i < N);
        } else {
            do X[i] = MULT32_32_Q31(g, PSHR32(iy[i], -tot_shift));
            while (++i < N);
        }
    } else
#endif
    do X[i] = VSHR32(MULT16_32_Q15(iy[i], g), k + 15 - NORM_SHIFT);
    while (++i < N);
}

static unsigned oaci_extract_collapse_mask(int *iy, int N, int B) {
    unsigned collapse_mask;
    int N0;
    int i;
    if (B <= 1)
        return 1;
    /*NOTE: As a minor optimization, we could be passing around log2(B), not B, for both this and for
       oaci_exp_rotation().*/
    N0 = oaci_celt_udiv(N, B);
    collapse_mask = 0;
    i = 0; do {
        int j;
        unsigned tmp = 0;
        j = 0; do {
            tmp |= iy[i*N0 + j];
        } while (++j < N0);
        collapse_mask |= (tmp != 0)<<i;
    } while (++i < B);
    return collapse_mask;
}

oac_val16 oaci_op_pvq_search_c(celt_norm *X, int *iy, int K, int N, int arch) {
    VARDECL(celt_norm, y);
    VARDECL(int, signx);
    int i, j;
    int pulsesLeft;
    oac_val32 sum;
    oac_val32 xy;
    oac_val16 yy;
    SAVE_STACK;

    (void)arch;
    ALLOC(y, N, celt_norm);
    ALLOC(signx, N, int);
#ifdef FIXED_POINT
    {
        int shift = (oaci_celt_ilog2(1 + oaci_celt_inner_prod_norm_shift(X, X, N, arch)) + 1)/2;
        shift = IMAX(0, shift + (NORM_SHIFT - 14) - 14);
        oaci_norm_scaledown(X, N, shift);
    }
#endif
    /* Get rid of the sign */
    sum = 0;
    j = 0; do {
        signx[j] = X[j] < 0;
        /* OPT: Make sure the compiler doesn't use a branch on ABS16(). */
        X[j] = ABS16(X[j]);
        iy[j] = 0;
        y[j] = 0;
    } while (++j < N);

    xy = yy = 0;

    pulsesLeft = K;

    /* Do a pre-search by projecting on the pyramid */
    if (K > (N>>1)) {
        oac_val16 rcp;
        j = 0; do {
            sum += X[j];
        }  while (++j < N);

        /* If X is too small, just replace it with a pulse at 0 */
#ifdef FIXED_POINT
        if (sum <= K)
#else
        /* Prevents infinities and NaNs from causing too many pulses
           to be allocated. 64 is an approximation of infinity here. */
        if (!(sum > EPSILON && sum < 64))
#endif
        {
            X[0] = QCONST16(1.f, 14);
            j = 1; do
                X[j] = 0;
            while (++j < N);
            sum = QCONST16(1.f, 14);
        }
#ifdef FIXED_POINT
        rcp = EXTRACT16(MULT16_32_Q16(K, oaci_celt_rcp(sum)));
#else
        /* Using K+e with e < 1 guarantees we cannot get more than K pulses. */
        rcp = EXTRACT16(MULT16_32_Q16(K + 0.8f, oaci_celt_rcp(sum)));
#endif
        j = 0; do {
#ifdef FIXED_POINT
            /* It's really important to round *towards zero* here */
            iy[j] = MULT16_16_Q15(X[j], rcp);
#else
            iy[j] = (int)floor(rcp*X[j]);
#endif
            y[j] = (celt_norm)iy[j];
            yy = MAC16_16(yy, y[j], y[j]);
            xy = MAC16_16(xy, X[j], y[j]);
            y[j] *= 2;
            pulsesLeft -= iy[j];
        }  while (++j < N);
    }
    celt_sig_assert(pulsesLeft >= 0);

    /* This should never happen, but just in case it does (e.g. on silence)
       we fill the first bin with pulses. */
#ifdef FIXED_POINT_DEBUG
    celt_sig_assert(pulsesLeft <= N + 3);
#endif
    if (pulsesLeft > N + 3) {
        oac_val16 tmp = (oac_val16)pulsesLeft;
        yy = MAC16_16(yy, tmp, tmp);
        yy = MAC16_16(yy, tmp, y[0]);
        iy[0] += pulsesLeft;
        pulsesLeft = 0;
    }

    for (i = 0; i < pulsesLeft; i++) {
        oac_val16 Rxy, Ryy;
        int best_id;
        oac_val32 best_num;
        oac_val16 best_den;
#ifdef FIXED_POINT
        int rshift;
#endif
#ifdef FIXED_POINT
        rshift = 1 + oaci_celt_ilog2(K - pulsesLeft + i + 1);
#endif
        best_id = 0;
        /* The squared magnitude term gets added anyway, so we might as well
           add it outside the loop */
        yy = ADD16(yy, 1);

        /* Calculations for position 0 are out of the loop, in part to reduce
           mispredicted branches (since the if condition is usually false)
           in the loop. */
        /* Temporary sums of the new pulse(s) */
        Rxy = EXTRACT16(SHR32(ADD32(xy, EXTEND32(X[0])), rshift));
        /* We're multiplying y[j] by two so we don't have to do it here */
        Ryy = ADD16(yy, y[0]);

        /* Approximate score: we maximise Rxy/sqrt(Ryy) (we're guaranteed that
           Rxy is positive because the sign is pre-computed) */
        Rxy = MULT16_16_Q15(Rxy, Rxy);
        best_den = Ryy;
        best_num = Rxy;
        j = 1;
        do {
            /* Temporary sums of the new pulse(s) */
            Rxy = EXTRACT16(SHR32(ADD32(xy, EXTEND32(X[j])), rshift));
            /* We're multiplying y[j] by two so we don't have to do it here */
            Ryy = ADD16(yy, y[j]);

            /* Approximate score: we maximise Rxy/sqrt(Ryy) (we're guaranteed that
               Rxy is positive because the sign is pre-computed) */
            Rxy = MULT16_16_Q15(Rxy, Rxy);
            /* The idea is to check for num/den >= best_num/best_den, but that way
               we can do it without any division */
            /* OPT: It's not clear whether a cmov is faster than a branch here
               since the condition is more often false than true and using
               a cmov introduces data dependencies across iterations. The optimal
               choice may be architecture-dependent. */
            if (oac_unlikely(MULT16_16(best_den, Rxy) > MULT16_16(Ryy, best_num))) {
                best_den = Ryy;
                best_num = Rxy;
                best_id = j;
            }
        } while (++j < N);

        /* Updating the sums of the new pulse(s) */
        xy = ADD32(xy, EXTEND32(X[best_id]));
        /* We're multiplying y[j] by two so we don't have to do it here */
        yy = ADD16(yy, y[best_id]);

        /* Only now that we've made the final choice, update y/iy */
        /* Multiplying y[j] by 2 so we don't have to do it everywhere else */
        y[best_id] += 2;
        iy[best_id]++;
    }

    /* Put the original sign back */
    j = 0;
    do {
        /*iy[j] = signx[j] ? -iy[j] : iy[j];*/
        /* OPT: The is more likely to be compiled without a branch than the code above
           but has the same performance otherwise. */
        iy[j] = (iy[j]^-signx[j]) + signx[j];
    } while (++j < N);
    RESTORE_STACK;
    return yy;
}

RESTORE_STACK;
RESTORE_STACK;
RESTORE_STACK;
unsigned oaci_alg_quant(celt_norm *X, int N, int K, int spread, int B, ec_enc *enc,
                   oac_val32 gain, int resynth, int arch) {
    VARDECL(int, iy);
    oac_val32 yy;
    unsigned collapse_mask;
    SAVE_STACK;

    celt_assert2(K > 0, "oaci_alg_quant() needs at least one pulse");
    celt_assert2(N > 1, "oaci_alg_quant() needs at least two dimensions");

    /* Covers vectorization by up to 4. */
    ALLOC(iy, N + 3, int);

    oaci_exp_rotation(X, N, 1, B, K, spread);

    yy = oaci_op_pvq_search(X, iy, K, N, arch);
    collapse_mask = oaci_extract_collapse_mask(iy, N, B);
    oaci_encode_pulses(iy, N, K, enc);
    if (resynth) oaci_normalise_residual(iy, X, N, yy, gain, 0);

    if (resynth)
        oaci_exp_rotation(X, N, -1, B, K, spread);

    RESTORE_STACK;
    return collapse_mask;
}

/** Decode pulse vector and combine the result with the pitch vector to produce
    the final normalised signal in the current band. */
unsigned oaci_alg_unquant(celt_norm *X, int N, int K, int spread, int B,
                     ec_dec *dec, oac_val32 gain) {
    oac_val32 Ryy;
    unsigned collapse_mask;
    VARDECL(int, iy);
    int yy_shift = 0;
    SAVE_STACK;

    celt_assert2(K > 0, "oaci_alg_unquant() needs at least one pulse");
    celt_assert2(N > 1, "oaci_alg_unquant() needs at least two dimensions");
    ALLOC(iy, N, int);
    Ryy = oaci_decode_pulses(iy, N, K, dec);
    oaci_normalise_residual(iy, X, N, Ryy, gain, yy_shift);
    oaci_exp_rotation(X, N, -1, B, K, spread);
    collapse_mask = oaci_extract_collapse_mask(iy, N, B);
    RESTORE_STACK;
    return collapse_mask;
}

#ifndef OVERRIDE_oaci_renormalise_vector
void oaci_renormalise_vector(celt_norm *X, int N, oac_val32 gain, int arch) {
    int i;
# ifdef FIXED_POINT
    int k;
# endif
    oac_val32 E;
    oac_val16 g;
    oac_val32 t;
    celt_norm *xptr;
    oaci_norm_scaledown(X, N, NORM_SHIFT - 14);
    E = EPSILON + oaci_celt_inner_prod_norm(X, X, N, arch);
# ifdef FIXED_POINT
    k = oaci_celt_ilog2(E)>>1;
# endif
    t = VSHR32(E, 2*(k - 7));
    g = MULT32_32_Q31(oaci_celt_rsqrt_norm(t), gain);

    xptr = X;
    for (i = 0; i < N; i++) {
        *xptr = EXTRACT16(PSHR32(MULT16_16(g, *xptr), k + 15 - 14));
        xptr++;
    }
    oaci_norm_scaleup(X, N, NORM_SHIFT - 14);
    /*return oaci_celt_sqrt(E);*/
}
#endif /* OVERRIDE_oaci_renormalise_vector */

oac_int32 oaci_stereo_itheta(const celt_norm *X, const celt_norm *Y, int stereo, int N, int arch) {
    int i;
    int itheta;
    oac_val32 mid, side;
    oac_val32 Emid, Eside;

    Emid = Eside = 0;
    if (stereo) {
        for (i = 0; i < N; i++) {
            celt_norm m, s;
            m = PSHR32(ADD32(X[i], Y[i]), NORM_SHIFT - 13);
            s = PSHR32(SUB32(X[i], Y[i]), NORM_SHIFT - 13);
            Emid = MAC16_16(Emid, m, m);
            Eside = MAC16_16(Eside, s, s);
        }
    } else {
        Emid += oaci_celt_inner_prod_norm_shift(X, X, N, arch);
        Eside += oaci_celt_inner_prod_norm_shift(Y, Y, N, arch);
    }
    mid = oaci_celt_sqrt32(Emid);
    side = oaci_celt_sqrt32(Eside);
#if defined(FIXED_POINT)
    itheta = oaci_celt_atan2p_norm(side, mid);
#else
    itheta = (int)floor(.5f + 65536.f*16384*oaci_celt_atan2p_norm(side, mid));
#endif

    return itheta;
}

static void oaci_cubic_synthesis(celt_norm *X, int *iy, int N, int K, int face, int sign, oac_val32 gain) {
    int i;
    oac_val32 sum = 0;
    oac_val32 mag;
#ifdef FIXED_POINT
    int sum_shift;
    int shift = IMAX(oaci_celt_ilog2(K) + oaci_celt_ilog2(N)/2 - 13, 0);
#endif
    for (i = 0; i < N; i++) {
        X[i] = (1 + 2*iy[i]) - K;
    }
    X[face] = sign ? -K : K;
    for (i = 0; i < N; i++) {
        sum += PSHR32(MULT16_16(X[i], X[i]), 2*shift);
    }
#ifdef FIXED_POINT
    sum_shift = (29 - oaci_celt_ilog2(sum))>>1;
    mag = oaci_celt_rsqrt_norm32(SHL32(sum, 2*sum_shift + 1));
    for (i = 0; i < N; i++) {
        X[i] = VSHR32(MULT16_32_Q15(X[i], MULT32_32_Q31(mag, gain)), shift - sum_shift + 29 - NORM_SHIFT);
    }
#else
    mag = 1.f/sqrt(sum);
    for (i = 0; i < N; i++) {
        X[i] *= mag*gain;
    }
#endif
}

unsigned oaci_cubic_quant(celt_norm *X, int N, int res, int B, ec_enc *enc, oac_val32 gain, int resynth) {
    int i;
    int face = 0;
    int K;
    VARDECL(int, iy);
    celt_norm faceval = -1;
    oac_val32 norm;
    int sign;
    SAVE_STACK;
    ALLOC(iy, N, int);
    K = 1<<res;
    /* Using odd K on transients to avoid adding pre-echo. */
    if (B != 1) K = IMAX(1, K - 1);
    if (K == 1) {
        if (resynth) OAC_CLEAR(X, N);
        RESTORE_STACK;
        return 0;
    }
    for (i = 0; i < N; i++) {
        if (ABS32(X[i]) > faceval) {
            faceval = ABS32(X[i]);
            face = i;
        }
    }
    sign = X[face] < 0;
    oaci_ec_enc_uint(enc, face, N);
    oaci_ec_enc_bits(enc, sign, 1);
#ifdef FIXED_POINT
    if (faceval != 0) {
        int face_shift = 30 - oaci_celt_ilog2(faceval);
        norm = oaci_celt_rcp_norm32(SHL32(faceval, face_shift));
        norm = MULT16_32_Q15(K, norm);
        for (i = 0; i < N; i++) {
            /* By computing X[i]+faceval inside the shift, the result is guaranteed non-negative. */
            iy[i] = IMIN(K - 1, (MULT32_32_Q31(SHL32(X[i] + faceval, face_shift - 1), norm))>>15);
        }
    } else {
        OAC_CLEAR(iy, N);
    }
#else
    norm = .5f*K/(faceval + EPSILON);
    for (i = 0; i < N; i++) {
        iy[i] = IMIN(K - 1, (int)floor((X[i] + faceval)*norm));
    }
#endif
    for (i = 0; i < N; i++) {
        if (i != face) oaci_ec_enc_bits(enc, iy[i], res);
    }
    if (resynth) {
        oaci_cubic_synthesis(X, iy, N, K, face, sign, gain);
    }
    RESTORE_STACK;
    return (1<<B) - 1;
}

unsigned oaci_cubic_unquant(celt_norm *X, int N, int res, int B, ec_dec *dec, oac_val32 gain) {
    int i;
    int face;
    int sign;
    int K;
    VARDECL(int, iy);
    SAVE_STACK;
    ALLOC(iy, N, int);
    K = 1<<res;
    /* Using odd K on transients to avoid adding pre-echo. */
    if (B != 1) K = IMAX(1, K - 1);
    if (K == 1) {
        OAC_CLEAR(X, N);
        RESTORE_STACK;
        return 0;
    }
    face = oaci_ec_dec_uint(dec, N);
    sign = oaci_ec_dec_bits(dec, 1);
    for (i = 0; i < N; i++) {
        if (i != face) iy[i] = oaci_ec_dec_bits(dec, res);
    }
    iy[face] = 0;
    oaci_cubic_synthesis(X, iy, N, K, face, sign, gain);
    RESTORE_STACK;
    return (1<<B) - 1;
}
