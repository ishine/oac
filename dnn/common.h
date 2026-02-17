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



#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "oac_defines.h"

#define LOG256 5.5451774445f
static OAC_INLINE float log2_approx(float x) {
    int integer;
    float frac;
    union {
        float f;
        int i;
    } in;
    in.f = x;
    integer = (in.i>>23) - 127;
    in.i -= integer<<23;
    frac = in.f - 1.5f;
    frac = -0.41445418f + frac*(0.95909232f
                                + frac*(-0.33951290f + frac*0.16541097f));
    return 1 + integer + frac;
}

#define log_approx(x) (0.69315f*log2_approx(x))

static OAC_INLINE float ulaw2lin(float u) {
    float s;
    float scale_1 = 32768.f/255.f;
    u = u - 128.f;
    s = u >= 0.f ? 1.f : -1.f;
    u = fabs(u);
    return s*scale_1*(exp(u/128.*LOG256) - 1);
}

static OAC_INLINE int lin2ulaw(float x) {
    float u;
    float scale = 255.f/32768.f;
    int s = x >= 0 ? 1 : -1;
    x = fabs(x);
    u = (s*(128*log_approx(1 + scale*x)/LOG256));
    u = 128 + u;
    if (u < 0) u = 0;
    if (u > 255) u = 255;
    return (int)floor(.5 + u);
}



#endif
