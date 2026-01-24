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

#include <stdio.h>
#include <math.h>
#include "oac_types.h"
#include "arch.h"
#include "common.h"
#include "tansig_table.h"

#define LPCNET_TEST

// we need to call two versions of each functions that have the same
// name, so use #defines to temp rename them

#define lpcnet_exp2 lpcnet_exp2_fast
#define tansig_approx tansig_approx_fast
#define oaci_sigmoid_approx sigmoid_approx_fast
#define oaci_softmax softmax_fast
#define oaci_vec_tanh vec_tanh_fast
#define oaci_vec_sigmoid vec_sigmoid_fast
#define sgemv_accum16 sgemv_accum16_fast
#define sparse_sgemv_accum16 sparse_sgemv_accum16_fast

#ifdef __AVX__
# include "vec_avx.h"
# ifdef __AVX2__
const char simd[] = "AVX2";
# else
const char simd[] = "AVX";
# endif
#elif __ARM_NEON__
# include "vec_neon.h"
const char simd[] = "NEON";
#else
const char simd[] = "none";

#endif

#undef lpcnet_exp2
#undef tansig_approx
#undef oaci_sigmoid_approx
#undef oaci_softmax
#undef oaci_vec_tanh
#undef oaci_vec_sigmoid
#undef sgemv_accum16
#undef sparse_sgemv_accum16
#include "vec.h"

#define ROW_STEP 16
#define ROWS     ROW_STEP*10
#define COLS     2
#define ENTRIES  2

int test_sgemv_accum16() {
    float weights[ROWS*COLS];
    float x[COLS];
    float out[ROWS], out_fast[ROWS];
    int i;

    printf("sgemv_accum16.....................: ");
    for (i = 0; i < ROWS*COLS; i++) {
        weights[i] = i;
    }
    for (i = 0; i < ROWS; i++) {
        out[i] = 0;
        out_fast[i] = 0;
    }

    for (i = 0; i < COLS; i++) {
        x[i] = i + 1;
    }

    sgemv_accum16(out, weights, ROWS, COLS, 1, x);
    sgemv_accum16_fast(out_fast, weights, ROWS, COLS, 1, x);

    for (i = 0; i < ROWS; i++) {
        if (out[i] != out_fast[i]) {
            printf("fail\n");
            for (i = 0; i < ROWS; i++) {
                printf("%d %f %f\n", i, out[i], out_fast[i]);
                if (out[i] != out_fast[i])
                    return 1;
            }
        }
    }

    printf("pass\n");
    return 0;
}


int test_sparse_sgemv_accum16() {
    int rows = ROW_STEP*ENTRIES;
    int indx[] = {1, 0, 2, 0, 1};
    float w[ROW_STEP*(1 + 2)];
    float x[ENTRIES] = {1, 2};
    float out[ROW_STEP*(1 + 2)], out_fast[ROW_STEP*(1 + 2)];
    int i;

    printf("sparse_sgemv_accum16..............: ");
    for (i = 0; i < ROW_STEP*(1 + 2); i++) {
        w[i] = i;
        out[i] = 0;
        out_fast[i] = 0;
    }

    sparse_sgemv_accum16(out, w, rows, indx, x);
    sparse_sgemv_accum16_fast(out_fast, w, rows, indx, x);

    for (i = 0; i < ROW_STEP*ENTRIES; i++) {
        if (out[i] != out_fast[i]) {
            printf("fail\n");
            for (i = 0; i < ROW_STEP*ENTRIES; i++) {
                printf("%d %f %f\n", i, out[i], out_fast[i]);
                if (out[i] != out_fast[i])
                    return 1;
            }
        }
    }

    printf("pass\n");
    return 0;
}

int main() {
    printf("testing vector routines on SIMD: %s\n", simd);
    int test1 = test_sgemv_accum16();
    int test2 = test_sparse_sgemv_accum16();
    return test1 || test2;
}
