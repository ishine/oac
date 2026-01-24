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

#ifndef SILK_NSQ_DEL_DEC_ARM_H
#define SILK_NSQ_DEL_DEC_ARM_H

#include "celt/arm/armcpu.h"

#if defined(OAC_ARM_MAY_HAVE_NEON_INTR)
void oaci_silk_NSQ_del_dec_neon(
    const silk_encoder_state *psEncC, silk_nsq_state *NSQ,
    SideInfoIndices *psIndices, const oac_int16 x16[], oac_int8 pulses[],
    const oac_int16 *PredCoef_Q12,
    const oac_int16 LTPCoef_Q14[LTP_ORDER*MAX_NB_SUBFR],
    const oac_int16 AR_Q13[MAX_NB_SUBFR*MAX_SHAPE_LPC_ORDER],
    const oac_int HarmShapeGain_Q14[MAX_NB_SUBFR],
    const oac_int Tilt_Q14[MAX_NB_SUBFR],
    const oac_int32 LF_shp_Q14[MAX_NB_SUBFR],
    const oac_int32 Gains_Q16[MAX_NB_SUBFR],
    const oac_int pitchL[MAX_NB_SUBFR], const oac_int Lambda_Q10,
    const oac_int LTP_scale_Q14);

# if !defined(OAC_HAVE_RTCD)
#  define OVERRIDE_oaci_silk_NSQ_del_dec (1)
#  define oaci_silk_NSQ_del_dec(psEncC, NSQ, psIndices, x16, pulses, PredCoef_Q12,  \
                           LTPCoef_Q14, AR_Q13, HarmShapeGain_Q14, Tilt_Q14,   \
                           LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10,          \
                           LTP_scale_Q14, arch)                                \
        ((void)(arch),                                                           \
         PRESUME_NEON(oaci_silk_NSQ_del_dec)(                                         \
             psEncC, NSQ, psIndices, x16, pulses, PredCoef_Q12, LTPCoef_Q14,     \
             AR_Q13, HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, \
             Lambda_Q10, LTP_scale_Q14))
# endif
#endif

#if !defined(OVERRIDE_oaci_silk_NSQ_del_dec)
/*Is run-time CPU detection enabled on this platform?*/
# if defined(OAC_HAVE_RTCD) && (defined(OAC_ARM_MAY_HAVE_NEON_INTR) && \
    !defined(OAC_ARM_PRESUME_NEON_INTR))
extern void (*const OACI_SILK_NSQ_DEL_DEC_IMPL[OAC_ARCHMASK + 1])(
    const silk_encoder_state *psEncC, silk_nsq_state *NSQ,
    SideInfoIndices *psIndices, const oac_int16 x16[], oac_int8 pulses[],
    const oac_int16 *PredCoef_Q12,
    const oac_int16 LTPCoef_Q14[LTP_ORDER * MAX_NB_SUBFR],
    const oac_int16 AR_Q13[MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER],
    const oac_int HarmShapeGain_Q14[MAX_NB_SUBFR],
    const oac_int Tilt_Q14[MAX_NB_SUBFR],
    const oac_int32 LF_shp_Q14[MAX_NB_SUBFR],
    const oac_int32 Gains_Q16[MAX_NB_SUBFR],
    const oac_int pitchL[MAX_NB_SUBFR], const oac_int Lambda_Q10,
    const oac_int LTP_scale_Q14);
#  define OVERRIDE_oaci_silk_NSQ_del_dec (1)
#  define oaci_silk_NSQ_del_dec(psEncC, NSQ, psIndices, x16, pulses, PredCoef_Q12, \
                           LTPCoef_Q14, AR_Q13, HarmShapeGain_Q14, Tilt_Q14,  \
                           LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10,         \
                           LTP_scale_Q14, arch)                               \
        ((*OACI_SILK_NSQ_DEL_DEC_IMPL[(arch)&OAC_ARCHMASK])(                        \
             psEncC, NSQ, psIndices, x16, pulses, PredCoef_Q12, LTPCoef_Q14,     \
             AR_Q13, HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, \
             Lambda_Q10, LTP_scale_Q14))
# elif defined(OAC_ARM_PRESUME_NEON_INTR)
#  define OVERRIDE_oaci_silk_NSQ_del_dec (1)
#  define oaci_silk_NSQ_del_dec(psEncC, NSQ, psIndices, x16, pulses, PredCoef_Q12,   \
                           LTPCoef_Q14, AR_Q13, HarmShapeGain_Q14, Tilt_Q14,    \
                           LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10,           \
                           LTP_scale_Q14, arch)                                 \
        ((void)(arch),                                                            \
         oaci_silk_NSQ_del_dec_neon(psEncC, NSQ, psIndices, x16, pulses, PredCoef_Q12, \
                           LTPCoef_Q14, AR_Q13, HarmShapeGain_Q14, Tilt_Q14,  \
                           LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10,         \
                           LTP_scale_Q14))
# endif
#endif

#endif /* end SILK_NSQ_DEL_DEC_ARM_H */
