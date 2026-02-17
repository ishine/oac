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

/**********************************************************************
* Correlation Matrix Computations for LS estimate.
**********************************************************************/

#include "main_FIX.h"

/* Calculates correlation vector X'*t */
void silk_corrVector_FIX(
    const oac_int16                *x,                                     /* I    x vector [L + order - 1] used to form data matrix X                         */
    const oac_int16                *t,                                     /* I    Target vector [L]                                                           */
    const oac_int L,                                                       /* I    Length of vectors                                                           */
    const oac_int order,                                                   /* I    Max lag for correlation                                                     */
    oac_int32                      *Xt,                                    /* O    Pointer to X'*t correlation vector [order]                                  */
    const oac_int rshifts,                                                 /* I    Right shifts of correlations                                                */
    int arch                                                                /* I    Run-time architecture                                                       */
    ) {
    oac_int lag, i;
    const oac_int16 *ptr1, *ptr2;
    oac_int32 inner_prod;

    ptr1 = &x[ order - 1 ]; /* Points to first sample of column 0 of X: X[:,0] */
    ptr2 = t;
    /* Calculate X'*t */
    if (rshifts > 0) {
        /* Right shifting used */
        for (lag = 0; lag < order; lag++) {
            inner_prod = 0;
            for (i = 0; i < L; i++) {
                inner_prod = silk_ADD_RSHIFT32( inner_prod, silk_SMULBB( ptr1[ i ], ptr2[i] ), rshifts );
            }
            Xt[ lag ] = inner_prod; /* X[:,lag]'*t */
            ptr1--; /* Go to next column of X */
        }
    } else {
        silk_assert( rshifts == 0 );
        for (lag = 0; lag < order; lag++) {
            Xt[ lag ] = silk_inner_prod_aligned( ptr1, ptr2, L, arch ); /* X[:,lag]'*t */
            ptr1--; /* Go to next column of X */
        }
    }
}

/* Calculates correlation matrix X'*X */
void silk_corrMatrix_FIX(
    const oac_int16                *x,                                     /* I    x vector [L + order - 1] used to form data matrix X                         */
    const oac_int L,                                                       /* I    Length of vectors                                                           */
    const oac_int order,                                                   /* I    Max lag for correlation                                                     */
    oac_int32                      *XX,                                    /* O    Pointer to X'*X correlation matrix [ order x order ]                        */
    oac_int32                      *nrg,                                    /* O    Energy of x vector                                                            */
    oac_int                        *rshifts,                               /* O    Right shifts of correlations and energy                                     */
    int arch                                                                /* I    Run-time architecture                                                       */
    ) {
    oac_int i, j, lag;
    oac_int32 energy;
    const oac_int16 *ptr1, *ptr2;

    /* Calculate energy to find shift used to fit in 32 bits */
    silk_sum_sqr_shift( nrg, rshifts, x, L + order - 1 );
    energy = *nrg;

    /* Calculate energy of first column (0) of X: X[:,0]'*X[:,0] */
    /* Remove contribution of first order - 1 samples */
    for (i = 0; i < order - 1; i++) {
        energy -= silk_RSHIFT32( silk_SMULBB( x[ i ], x[ i ] ), *rshifts );
    }

    /* Calculate energy of remaining columns of X: X[:,j]'*X[:,j] */
    /* Fill out the diagonal of the correlation matrix */
    matrix_ptr( XX, 0, 0, order ) = energy;
    silk_assert( energy >= 0 );
    ptr1 = &x[ order - 1 ]; /* First sample of column 0 of X */
    for (j = 1; j < order; j++) {
        energy = silk_SUB32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ L - j ], ptr1[ L - j ] ), *rshifts ));
        energy = silk_ADD32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ -j ], ptr1[ -j ] ), *rshifts ));
        matrix_ptr( XX, j, j, order ) = energy;
        silk_assert( energy >= 0 );
    }

    ptr2 = &x[ order - 2 ]; /* First sample of column 1 of X */
    /* Calculate the remaining elements of the correlation matrix */
    if (*rshifts > 0) {
        /* Right shifting used */
        for (lag = 1; lag < order; lag++) {
            /* Inner product of column 0 and column lag: X[:,0]'*X[:,lag] */
            energy = 0;
            for (i = 0; i < L; i++) {
                energy += silk_RSHIFT32( silk_SMULBB( ptr1[ i ], ptr2[i] ), *rshifts );
            }
            /* Calculate remaining off diagonal: X[:,j]'*X[:,j + lag] */
            matrix_ptr( XX, lag, 0, order ) = energy;
            matrix_ptr( XX, 0, lag, order ) = energy;
            for (j = 1; j < (order - lag); j++) {
                energy = silk_SUB32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ L - j ], ptr2[ L - j ] ), *rshifts ));
                energy = silk_ADD32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ -j ], ptr2[ -j ] ), *rshifts ));
                matrix_ptr( XX, lag + j, j, order ) = energy;
                matrix_ptr( XX, j, lag + j, order ) = energy;
            }
            ptr2--; /* Update pointer to first sample of next column (lag) in X */
        }
    } else {
        for (lag = 1; lag < order; lag++) {
            /* Inner product of column 0 and column lag: X[:,0]'*X[:,lag] */
            energy = silk_inner_prod_aligned( ptr1, ptr2, L, arch );
            matrix_ptr( XX, lag, 0, order ) = energy;
            matrix_ptr( XX, 0, lag, order ) = energy;
            /* Calculate remaining off diagonal: X[:,j]'*X[:,j + lag] */
            for (j = 1; j < (order - lag); j++) {
                energy = silk_SUB32( energy, silk_SMULBB( ptr1[ L - j ], ptr2[ L - j ] ));
                energy = silk_SMLABB( energy, ptr1[ -j ], ptr2[ -j ] );
                matrix_ptr( XX, lag + j, j, order ) = energy;
                matrix_ptr( XX, j, lag + j, order ) = energy;
            }
            ptr2--;/* Update pointer to first sample of next column (lag) in X */
        }
    }
}
