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

#ifndef SILK_CONTROL_H
#define SILK_CONTROL_H

#include "typedef.h"


/* Decoder API flags */
#define FLAG_DECODE_NORMAL                      0
#define FLAG_PACKET_LOST                        1
#define FLAG_DECODE_LBRR                        2

/***********************************************/
/* Structure for controlling encoder operation */
/***********************************************/
typedef struct {
    /* I:   Number of channels; 1/2                                                         */
    oac_int32 nChannelsAPI;

    /* I:   Number of channels; 1/2                                                         */
    oac_int32 nChannelsInternal;

    /* I:   Input signal sampling rate in Hertz; 8000/12000/16000/24000/32000/44100/48000   */
    oac_int32 API_sampleRate;

    /* I:   Maximum internal sampling rate in Hertz; 8000/12000/16000                       */
    oac_int32 maxInternalSampleRate;

    /* I:   Minimum internal sampling rate in Hertz; 8000/12000/16000                       */
    oac_int32 minInternalSampleRate;

    /* I:   Soft request for internal sampling rate in Hertz; 8000/12000/16000              */
    oac_int32 desiredInternalSampleRate;

    /* I:   Number of samples per packet in milliseconds; 10/20/40/60                       */
    oac_int payloadSize_ms;

    /* I:   Bitrate during active speech in bits/second; internally limited                 */
    oac_int32 bitRate;

    /* I:   Uplink packet loss in percent (0-100)                                           */
    oac_int packetLossPercentage;

    /* I:   Complexity mode; 0 is lowest, 10 is highest complexity                          */
    oac_int complexity;

    /* I:   Flag to enable in-band Forward Error Correction (FEC); 0/1                      */
    oac_int useInBandFEC;

    /* I:   Flag to enable in-band Deep REDundancy (DRED); 0/1                              */
    oac_int useDRED;

    /* I:   Flag to actually code in-band Forward Error Correction (FEC) in the current packet; 0/1 */
    oac_int LBRR_coded;

    /* I:   Flag to enable discontinuous transmission (DTX); 0/1                            */
    oac_int useDTX;

    /* I:   Flag to use constant bitrate                                                    */
    oac_int useCBR;

    /* I:   Maximum number of bits allowed for the frame                                    */
    oac_int maxBits;

    /* I:   Causes a smooth oaci_downmix to mono                                                 */
    oac_int toMono;

    /* I:   Oac encoder is allowing us to switch bandwidth                                 */
    oac_int oacCanSwitch;

    /* I: Make frames as independent as possible (but still use LPC)                        */
    oac_int reducedDependency;

    /* O:   Internal sampling rate used, in Hertz; 8000/12000/16000                         */
    oac_int32 internalSampleRate;

    /* O: Flag that bandwidth switching is allowed (because low voice activity)             */
    oac_int allowBandwidthSwitch;

    /* O:   Flag that SILK runs in WB mode without variable LP filter (use for switching between WB/SWB/FB) */
    oac_int inWBmodeWithoutVariableLP;

    /* O:   Stereo width */
    oac_int stereoWidth_Q14;

    /* O:   Tells the Oac encoder we're ready to switch                                    */
    oac_int switchReady;

    /* O: SILK Signal type */
    oac_int signalType;

    /* O: SILK offset (dithering) */
    oac_int offset;
} silk_EncControlStruct;

/**************************************************************************/
/* Structure for controlling decoder operation and reading decoder status */
/**************************************************************************/
typedef struct {
    /* I:   Number of channels; 1/2                                                         */
    oac_int32 nChannelsAPI;

    /* I:   Number of channels; 1/2                                                         */
    oac_int32 nChannelsInternal;

    /* I:   Output signal sampling rate in Hertz; 8000/12000/16000/24000/32000/44100/48000  */
    oac_int32 API_sampleRate;

    /* I:   Internal sampling rate used, in Hertz; 8000/12000/16000                         */
    oac_int32 internalSampleRate;

    /* I:   Number of samples per packet in milliseconds; 10/20/40/60                       */
    oac_int payloadSize_ms;

    /* O:   Pitch lag of previous frame (0 if unvoiced), measured in samples at 48 kHz      */
    oac_int prevPitchLag;

    /* I:   Enable Deep PLC                                                                 */
    oac_int enable_deep_plc;

#ifdef ENABLE_OSCE
    /* I: OSCE method */
    oac_int osce_method;

# ifdef ENABLE_OSCE_BWE
    /* I: OSCE bandwidth extension method */
    oac_int enable_osce_bwe;

    /* I: extended mode */
    oac_int osce_extended_mode;

    /* O: previous extended mode */
    oac_int prev_osce_extended_mode;
# endif
#endif
} silk_DecControlStruct;

#endif
