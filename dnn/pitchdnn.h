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

#ifndef PITCHDNN_H
#define PITCHDNN_H


typedef struct PitchDNN PitchDNN;

#include "pitchdnn_data.h"

#define PITCH_MIN_PERIOD 32
#define PITCH_MAX_PERIOD 256

#define NB_XCORR_FEATURES (PITCH_MAX_PERIOD - PITCH_MIN_PERIOD)


typedef struct {
    PitchDNN model;
    float gru_state[GRU_1_STATE_SIZE];
    float xcorr_mem1[(NB_XCORR_FEATURES + 2)*2];
    float xcorr_mem2[(NB_XCORR_FEATURES + 2)*2*8];
    float xcorr_mem3[(NB_XCORR_FEATURES + 2)*2*8];
} PitchDNNState;


void oaci_pitchdnn_init(PitchDNNState *st);
int oaci_pitchdnn_load_model(PitchDNNState *st, const void *data, int len);

float oaci_compute_pitchdnn(
    PitchDNNState *st,
    const float *if_features,
    const float *xcorr_features,
    int arch);

#endif
