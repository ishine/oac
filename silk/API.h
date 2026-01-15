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

#ifndef SILK_API_H
#define SILK_API_H

#include "control.h"
#include "typedef.h"
#include "errors.h"
#include "entenc.h"
#include "entdec.h"

#ifdef ENABLE_DEEP_PLC
# include "lpcnet_private.h"
#endif

#define SILK_MAX_FRAMES_PER_PACKET  3

/* Struct for TOC (Table of Contents) */
typedef struct {
    oac_int VADFlag;                                   /* Voice activity for packet                            */
    oac_int VADFlags[ SILK_MAX_FRAMES_PER_PACKET ];    /* Voice activity for each frame in packet              */
    oac_int inbandFECFlag;                             /* Flag indicating if packet contains in-band FEC       */
} silk_TOC_struct;

/****************************************/
/* Encoder functions                    */
/****************************************/

/***********************************************/
/* Get size in bytes of the Silk encoder state */
/***********************************************/
oac_int silk_Get_Encoder_Size(                         /* O    Returns error code                              */
    oac_int                        *encSizeBytes,      /* O    Number of bytes in SILK encoder state           */
    oac_int channels                                   /* I    Number of channels                              */
    );

/*************************/
/* Init or reset encoder */
/*************************/
oac_int silk_InitEncoder(                              /* O    Returns error code                              */
    void                            *encState,          /* I/O  State                                           */
    int channels,                                       /* I    Number of channels                              */
    int arch,                                           /* I    Run-time architecture                           */
    silk_EncControlStruct           *encStatus          /* O    Encoder Status                                  */
    );

/**************************/
/* Encode frame with Silk */
/**************************/
/* Note: if prefillFlag is set, the input must contain 10 ms of audio, irrespective of what                     */
/* encControl->payloadSize_ms is set to                                                                         */
oac_int silk_Encode(                                   /* O    Returns error code                              */
    void                            *encState,          /* I/O  State                                           */
    silk_EncControlStruct           *encControl,        /* I    Control status                                  */
    const oac_res                  *samplesIn,         /* I    Speech sample input vector                      */
    oac_int nSamplesIn,                                /* I    Number of samples in input vector               */
    ec_enc                          *psRangeEnc,        /* I/O  Compressor data structure                       */
    oac_int32                      *nBytesOut,         /* I/O  Number of bytes in payload (input: Max bytes)   */
    const oac_int prefillFlag,                         /* I    Flag to indicate prefilling buffers no coding   */
    int activity                                        /* I    Decision of Oac voice activity detector        */
    );

/****************************************/
/* Decoder functions                    */
/****************************************/


/***********************************************/
/* Load OSCE models from external data pointer */
/***********************************************/
oac_int silk_LoadOSCEModels(
    void *decState,                                     /* O    I/O State                                       */
    const unsigned char *data,                          /* I    pointer to binary blob                          */
    int len                                             /* I    length of binary blob data                      */
    );

/***********************************************/
/* Get size in bytes of the Silk decoder state */
/***********************************************/
oac_int silk_Get_Decoder_Size(                         /* O    Returns error code                              */
    oac_int                        *decSizeBytes       /* O    Number of bytes in SILK decoder state           */
    );

/*************************/
/* Init and Reset decoder */
/*************************/
oac_int silk_ResetDecoder(                              /* O    Returns error code                              */
    void                            *decState            /* I/O  State                                           */
    );

oac_int silk_InitDecoder(                              /* O    Returns error code                              */
    void                            *decState           /* I/O  State                                           */
    );

/******************/
/* Decode a frame */
/******************/
oac_int silk_Decode(                                   /* O    Returns error code                              */
    void*                           decState,           /* I/O  State                                           */
    silk_DecControlStruct*decControl,                   /* I/O  Control Structure                               */
    oac_int lostFlag,                                  /* I    0: no loss, 1 loss, 2 decode fec                */
    oac_int newPacketFlag,                             /* I    Indicates first decoder call for this packet    */
    ec_dec*psRangeDec,                                  /* I/O  Compressor data structure                       */
    oac_res*samplesOut,                                /* O    Decoded output speech vector                    */
    oac_int32*nSamplesOut,                             /* O    Number of samples decoded                       */
#ifdef ENABLE_DEEP_PLC
    LPCNetPLCState*lpcnet,
#endif
    int arch                                            /* I    Run-time architecture                           */
    );

#if 0
/**************************************/
/* Get table of contents for a packet */
/**************************************/
oac_int silk_get_TOC(
    const oac_uint8                *payload,           /* I    Payload data                                */
    const oac_int nBytesIn,                            /* I    Number of input bytes                       */
    const oac_int nFramesPerPayload,                   /* I    Number of SILK frames per payload           */
    silk_TOC_struct                 *Silk_TOC           /* O    Type of content                             */
    );
#endif


#endif
