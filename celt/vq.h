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
/**
   @file vq.h
   @brief Vector quantisation of the residual
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

#ifndef VQ_H
#define VQ_H

#include "entenc.h"
#include "entdec.h"
#include "modes.h"

#if (defined(OAC_X86_MAY_HAVE_SSE2) && !defined(FIXED_POINT))
# include "x86/vq_sse.h"
#endif

#if defined(FIXED_POINT)
oac_val32 oaci_celt_inner_prod_norm(const celt_norm *x, const celt_norm *y, int len, int arch);
oac_val32 oaci_celt_inner_prod_norm_shift(const celt_norm *x, const celt_norm *y, int len, int arch);

void oaci_norm_scaleup(celt_norm *X, int N, int shift);
void oaci_norm_scaledown(celt_norm *X, int N, int shift);

#else
# define oaci_celt_inner_prod_norm oaci_celt_inner_prod
# define oaci_celt_inner_prod_norm_shift oaci_celt_inner_prod
# define oaci_norm_scaleup(X, N, shift)
# define oaci_norm_scaledown(X, N, shift)
#endif

void oaci_exp_rotation(celt_norm *X, int len, int dir, int stride, int K, int spread);

oac_val16 oaci_op_pvq_search_c(celt_norm *X, int *iy, int K, int N, int arch);

#if !defined(OVERRIDE_OP_PVQ_SEARCH)
# define oaci_op_pvq_search(x, iy, K, N, arch) \
        (oaci_op_pvq_search_c(x, iy, K, N, arch))
#endif

/** Algebraic pulse-vector quantiser. The signal x is replaced by the sum of
 * the pitch and a combination of pulses such that its norm is still equal
 * to 1. This is the function that will typically require the most CPU.
 * @param X Residual signal to quantise/encode (returns quantised version)
 * @param N Number of samples to encode
 * @param K Number of pulses to use
 * @param enc Entropy encoder state
 * @ret A mask indicating which blocks in the band received pulses
 */
unsigned oaci_alg_quant(celt_norm *X, int N, int K, int spread, int B, ec_enc *enc,
    oac_val32 gain, int resynth, int arch);

/** Algebraic pulse decoder
 * @param X Decoded normalised spectrum (returned)
 * @param N Number of samples to decode
 * @param K Number of pulses to use
 * @param dec Entropy decoder state
 * @ret A mask indicating which blocks in the band received pulses
 */
unsigned oaci_alg_unquant(celt_norm *X, int N, int K, int spread, int B,
    ec_dec *dec, oac_val32 gain);

void oaci_renormalise_vector(celt_norm *X, int N, oac_val32 gain, int arch);

oac_int32 oaci_stereo_itheta(const celt_norm *X, const celt_norm *Y, int stereo, int N, int arch);

unsigned oaci_cubic_quant(celt_norm *X, int N, int K, int B, ec_enc *enc, oac_val32 gain, int resynth);
unsigned oaci_cubic_unquant(celt_norm *X, int N, int K, int B, ec_dec *dec, oac_val32 gain);

#endif /* VQ_H */
