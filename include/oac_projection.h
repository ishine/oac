/* Copyright (c) 2017 Google Inc.
   Written by Andrew Allen */
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

/**
 * @file oac_projection.h
 * @brief Oac projection reference API
 */

#ifndef OAC_PROJECTION_H
#define OAC_PROJECTION_H

#include "oac_multistream.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond OAC_INTERNAL_DOC */

/** These are the actual encoder and decoder CTL ID numbers.
 * They should not be used directly by applications.c
 * In general, SETs should be even and GETs should be odd.*/
/**@{*/
#define OAC_PROJECTION_GET_DEMIXING_MATRIX_GAIN_REQUEST    6001
#define OAC_PROJECTION_GET_DEMIXING_MATRIX_SIZE_REQUEST    6003
#define OAC_PROJECTION_GET_DEMIXING_MATRIX_REQUEST         6005
/**@}*/


/** @endcond */

/** @defgroup oac_projection_ctls Projection specific encoder and decoder CTLs
 *
 * These are convenience macros that are specific to the
 * oac_projection_encoder_ctl() and oac_projection_decoder_ctl()
 * interface.
 * The CTLs from @ref oac_genericctls, @ref oac_encoderctls,
 * @ref oac_decoderctls, and @ref oac_multistream_ctls may be applied to a
 * projection encoder or decoder as well.
 */
/**@{*/

/** Gets the gain (in dB. S7.8-format) of the demixing matrix from the encoder.
 * @param[out] x <tt>oac_int32 *</tt>: Returns the gain (in dB. S7.8-format)
 *                                      of the demixing matrix.
 * @hideinitializer
 */
#define OAC_PROJECTION_GET_DEMIXING_MATRIX_GAIN(x) OAC_PROJECTION_GET_DEMIXING_MATRIX_GAIN_REQUEST, oac_check_int_ptr(x)


/** Gets the size in bytes of the demixing matrix from the encoder.
 * @param[out] x <tt>oac_int32 *</tt>: Returns the size in bytes of the
 *                                      demixing matrix.
 * @hideinitializer
 */
#define OAC_PROJECTION_GET_DEMIXING_MATRIX_SIZE(x) OAC_PROJECTION_GET_DEMIXING_MATRIX_SIZE_REQUEST, oac_check_int_ptr(x)


/** Copies the demixing matrix to the supplied pointer location.
 * @param[out] x <tt>unsigned char *</tt>: Returns the demixing matrix to the
 *                                         supplied pointer location.
 * @param y <tt>oac_int32</tt>: The size in bytes of the reserved memory at the
 *                              pointer location.
 * @hideinitializer
 */
#define OAC_PROJECTION_GET_DEMIXING_MATRIX(x, y) OAC_PROJECTION_GET_DEMIXING_MATRIX_REQUEST, x, oac_check_int(y)


/**@}*/

/** Oac projection encoder state.
 * This contains the complete state of a projection Oac encoder.
 * It is position independent and can be freely copied.
 * @see oac_projection_ambisonics_encoder_create
 */
typedef struct OacProjectionEncoder OacProjectionEncoder;


/** Oac projection decoder state.
 * This contains the complete state of a projection Oac decoder.
 * It is position independent and can be freely copied.
 * @see oac_projection_decoder_create
 * @see oac_projection_decoder_init
 */
typedef struct OacProjectionDecoder OacProjectionDecoder;


/**\name Projection encoder functions */
/**@{*/

/** Gets the size of an OacProjectionEncoder structure.
 * @param channels <tt>int</tt>: The total number of input channels to encode.
 *                               This must be no more than 255.
 * @param mapping_family <tt>int</tt>: The mapping family to use for selecting
 *                                     the appropriate projection.
 * @returns The size in bytes on success, or a negative error code
 *          (see @ref oac_errorcodes) on error.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT oac_int32 oac_projection_ambisonics_encoder_get_size(
    int channels,
    int mapping_family);


/** Allocates and initializes a projection encoder state.
 * Call oac_projection_encoder_destroy() to release
 * this object when finished.
 * @param Fs <tt>oac_int32</tt>: Sampling rate of the input signal (in Hz).
 *                                This must be one of 8000, 12000, 16000,
 *                                24000, or 48000.
 * @param channels <tt>int</tt>: Number of channels in the input signal.
 *                               This must be at most 255.
 *                               It may be greater than the number of
 *                               coded channels (<code>streams +
 *                               coupled_streams</code>).
 * @param mapping_family <tt>int</tt>: The mapping family to use for selecting
 *                                     the appropriate projection.
 * @param[out] streams <tt>int *</tt>: The total number of streams that will
 *                                     be encoded from the input.
 * @param[out] coupled_streams <tt>int *</tt>: Number of coupled (2 channel)
 *                                 streams that will be encoded from the input.
 * @param application <tt>int</tt>: The target encoder application.
 *                                  This must be one of the following:
 * <dl>
 * <dt>#OAC_APPLICATION_VOIP</dt>
 * <dd>Process signal for improved speech intelligibility.</dd>
 * <dt>#OAC_APPLICATION_AUDIO</dt>
 * <dd>Favor faithfulness to the original input.</dd>
 * <dt>#OAC_APPLICATION_RESTRICTED_LOWDELAY</dt>
 * <dd>Configure the minimum possible coding delay by disabling certain modes
 * of operation.</dd>
 * </dl>
 * @param[out] error <tt>int *</tt>: Returns #OAC_OK on success, or an error
 *                                   code (see @ref oac_errorcodes) on
 *                                   failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT OacProjectionEncoder *oac_projection_ambisonics_encoder_create(
    oac_int32 Fs,
    int channels,
    int mapping_family,
    int *streams,
    int *coupled_streams,
    int application,
    int *error) OAC_ARG_NONNULL(4) OAC_ARG_NONNULL(5);


/** Initialize a previously allocated projection encoder state.
 * The memory pointed to by \a st must be at least the size returned by
 * oac_projection_ambisonics_encoder_get_size().
 * This is intended for applications which use their own allocator instead of
 * malloc.
 * To reset a previously initialized state, use the #OAC_RESET_STATE CTL.
 * @see oac_projection_ambisonics_encoder_create
 * @see oac_projection_ambisonics_encoder_get_size
 * @param st <tt>OacProjectionEncoder*</tt>: Projection encoder state to initialize.
 * @param Fs <tt>oac_int32</tt>: Sampling rate of the input signal (in Hz).
 *                                This must be one of 8000, 12000, 16000,
 *                                24000, or 48000.
 * @param channels <tt>int</tt>: Number of channels in the input signal.
 *                               This must be at most 255.
 *                               It may be greater than the number of
 *                               coded channels (<code>streams +
 *                               coupled_streams</code>).
 * @param streams <tt>int</tt>: The total number of streams to encode from the
 *                              input.
 *                              This must be no more than the number of channels.
 * @param coupled_streams <tt>int</tt>: Number of coupled (2 channel) streams
 *                                      to encode.
 *                                      This must be no larger than the total
 *                                      number of streams.
 *                                      Additionally, The total number of
 *                                      encoded channels (<code>streams +
 *                                      coupled_streams</code>) must be no
 *                                      more than the number of input channels.
 * @param application <tt>int</tt>: The target encoder application.
 *                                  This must be one of the following:
 * <dl>
 * <dt>#OAC_APPLICATION_VOIP</dt>
 * <dd>Process signal for improved speech intelligibility.</dd>
 * <dt>#OAC_APPLICATION_AUDIO</dt>
 * <dd>Favor faithfulness to the original input.</dd>
 * <dt>#OAC_APPLICATION_RESTRICTED_LOWDELAY</dt>
 * <dd>Configure the minimum possible coding delay by disabling certain modes
 * of operation.</dd>
 * </dl>
 * @returns #OAC_OK on success, or an error code (see @ref oac_errorcodes)
 *          on failure.
 */
OAC_EXPORT int oac_projection_ambisonics_encoder_init(
    OacProjectionEncoder *st,
    oac_int32 Fs,
    int channels,
    int mapping_family,
    int *streams,
    int *coupled_streams,
    int application) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(5) OAC_ARG_NONNULL(6);


/** Encodes a projection Oac frame.
 * @param st <tt>OacProjectionEncoder*</tt>: Projection encoder state.
 * @param[in] pcm <tt>const oac_int16*</tt>: The input signal as interleaved
 *                                            samples.
 *                                            This must contain
 *                                            <code>frame_size*channels</code>
 *                                            samples.
 * @param frame_size <tt>int</tt>: Number of samples per channel in the input
 *                                 signal.
 *                                 This must be an Oac frame size for the
 *                                 encoder's sampling rate.
 *                                 For example, at 48 kHz the permitted values
 *                                 are 120, 240, 480, 960, 1920, and 2880.
 *                                 Passing in a duration of less than 10 ms
 *                                 (480 samples at 48 kHz) will prevent the
 *                                 encoder from using the LPC or hybrid modes.
 * @param[out] data <tt>unsigned char*</tt>: Output payload.
 *                                           This must contain storage for at
 *                                           least \a max_data_bytes.
 * @param [in] max_data_bytes <tt>oac_int32</tt>: Size of the allocated
 *                                                 memory for the output
 *                                                 payload. This may be
 *                                                 used to impose an upper limit on
 *                                                 the instant bitrate, but should
 *                                                 not be used as the only bitrate
 *                                                 control. Use #OAC_SET_BITRATE to
 *                                                 control the bitrate.
 * @returns The length of the encoded packet (in bytes) on success or a
 *          negative error code (see @ref oac_errorcodes) on failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT int oac_projection_encode(
    OacProjectionEncoder *st,
    const oac_int16 *pcm,
    int frame_size,
    unsigned char *data,
    oac_int32 max_data_bytes) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(2) OAC_ARG_NONNULL(4);

/** Encodes a projection Oac frame.
 * @param st <tt>OacProjectionEncoder*</tt>: Projection encoder state.
 * @param[in] pcm <tt>const oac_int32*</tt>: The input signal as interleaved
 *                                            samples representing (or slightly exceeding) 24-bit values.
 *                                            This must contain
 *                                            <code>frame_size*channels</code>
 *                                            samples.
 * @param frame_size <tt>int</tt>: Number of samples per channel in the input
 *                                 signal.
 *                                 This must be an Oac frame size for the
 *                                 encoder's sampling rate.
 *                                 For example, at 48 kHz the permitted values
 *                                 are 120, 240, 480, 960, 1920, and 2880.
 *                                 Passing in a duration of less than 10 ms
 *                                 (480 samples at 48 kHz) will prevent the
 *                                 encoder from using the LPC or hybrid modes.
 * @param[out] data <tt>unsigned char*</tt>: Output payload.
 *                                           This must contain storage for at
 *                                           least \a max_data_bytes.
 * @param [in] max_data_bytes <tt>oac_int32</tt>: Size of the allocated
 *                                                 memory for the output
 *                                                 payload. This may be
 *                                                 used to impose an upper limit on
 *                                                 the instant bitrate, but should
 *                                                 not be used as the only bitrate
 *                                                 control. Use #OAC_SET_BITRATE to
 *                                                 control the bitrate.
 * @returns The length of the encoded packet (in bytes) on success or a
 *          negative error code (see @ref oac_errorcodes) on failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT int oac_projection_encode24(
    OacProjectionEncoder *st,
    const oac_int32 *pcm,
    int frame_size,
    unsigned char *data,
    oac_int32 max_data_bytes) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(2) OAC_ARG_NONNULL(4);


/** Encodes a projection Oac frame from floating point input.
 * @param st <tt>OacProjectionEncoder*</tt>: Projection encoder state.
 * @param[in] pcm <tt>const float*</tt>: The input signal as interleaved
 *                                       samples with a normal range of
 *                                       +/-1.0.
 *                                       Samples with a range beyond +/-1.0
 *                                       are supported but will be clipped by
 *                                       decoders using the integer API and
 *                                       should only be used if it is known
 *                                       that the far end supports extended
 *                                       dynamic range.
 *                                       This must contain
 *                                       <code>frame_size*channels</code>
 *                                       samples.
 * @param frame_size <tt>int</tt>: Number of samples per channel in the input
 *                                 signal.
 *                                 This must be an Oac frame size for the
 *                                 encoder's sampling rate.
 *                                 For example, at 48 kHz the permitted values
 *                                 are 120, 240, 480, 960, 1920, and 2880.
 *                                 Passing in a duration of less than 10 ms
 *                                 (480 samples at 48 kHz) will prevent the
 *                                 encoder from using the LPC or hybrid modes.
 * @param[out] data <tt>unsigned char*</tt>: Output payload.
 *                                           This must contain storage for at
 *                                           least \a max_data_bytes.
 * @param [in] max_data_bytes <tt>oac_int32</tt>: Size of the allocated
 *                                                 memory for the output
 *                                                 payload. This may be
 *                                                 used to impose an upper limit on
 *                                                 the instant bitrate, but should
 *                                                 not be used as the only bitrate
 *                                                 control. Use #OAC_SET_BITRATE to
 *                                                 control the bitrate.
 * @returns The length of the encoded packet (in bytes) on success or a
 *          negative error code (see @ref oac_errorcodes) on failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT int oac_projection_encode_float(
    OacProjectionEncoder *st,
    const float *pcm,
    int frame_size,
    unsigned char *data,
    oac_int32 max_data_bytes) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(2) OAC_ARG_NONNULL(4);


/** Frees an <code>OacProjectionEncoder</code> allocated by
 * oac_projection_ambisonics_encoder_create().
 * @param st <tt>OacProjectionEncoder*</tt>: Projection encoder state to be freed.
 */
OAC_EXPORT void oac_projection_encoder_destroy(OacProjectionEncoder *st);


/** Perform a CTL function on a projection Oac encoder.
 *
 * Generally the request and subsequent arguments are generated by a
 * convenience macro.
 * @param st <tt>OacProjectionEncoder*</tt>: Projection encoder state.
 * @param request This and all remaining parameters should be replaced by one
 *                of the convenience macros in @ref oac_genericctls,
 *                @ref oac_encoderctls, @ref oac_multistream_ctls, or
 *                @ref oac_projection_ctls
 * @see oac_genericctls
 * @see oac_encoderctls
 * @see oac_multistream_ctls
 * @see oac_projection_ctls
 */
OAC_EXPORT int oac_projection_encoder_ctl(OacProjectionEncoder *st, int request, ...) OAC_ARG_NONNULL(1);


/**@}*/

/**\name Projection decoder functions */
/**@{*/

/** Gets the size of an <code>OacProjectionDecoder</code> structure.
 * @param channels <tt>int</tt>: The total number of output channels.
 *                               This must be no more than 255.
 * @param streams <tt>int</tt>: The total number of streams coded in the
 *                              input.
 *                              This must be no more than 255.
 * @param coupled_streams <tt>int</tt>: Number streams to decode as coupled
 *                                      (2 channel) streams.
 *                                      This must be no larger than the total
 *                                      number of streams.
 *                                      Additionally, The total number of
 *                                      coded channels (<code>streams +
 *                                      coupled_streams</code>) must be no
 *                                      more than 255.
 * @returns The size in bytes on success, or a negative error code
 *          (see @ref oac_errorcodes) on error.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT oac_int32 oac_projection_decoder_get_size(
    int channels,
    int streams,
    int coupled_streams);


/** Allocates and initializes a projection decoder state.
 * Call oac_projection_decoder_destroy() to release
 * this object when finished.
 * @param Fs <tt>oac_int32</tt>: Sampling rate to decode at (in Hz).
 *                                This must be one of 8000, 12000, 16000,
 *                                24000, or 48000.
 * @param channels <tt>int</tt>: Number of channels to output.
 *                               This must be at most 255.
 *                               It may be different from the number of coded
 *                               channels (<code>streams +
 *                               coupled_streams</code>).
 * @param streams <tt>int</tt>: The total number of streams coded in the
 *                              input.
 *                              This must be no more than 255.
 * @param coupled_streams <tt>int</tt>: Number of streams to decode as coupled
 *                                      (2 channel) streams.
 *                                      This must be no larger than the total
 *                                      number of streams.
 *                                      Additionally, The total number of
 *                                      coded channels (<code>streams +
 *                                      coupled_streams</code>) must be no
 *                                      more than 255.
 * @param[in] demixing_matrix <tt>const unsigned char[demixing_matrix_size]</tt>: Demixing matrix
 *                         that mapping from coded channels to output channels,
 *                         as described in @ref oac_projection and
 *                         @ref oac_projection_ctls.
 * @param demixing_matrix_size <tt>oac_int32</tt>: The size in bytes of the
 *                                                  demixing matrix, as
 *                                                  described in @ref
 *                                                  oac_projection_ctls.
 * @param[out] error <tt>int *</tt>: Returns #OAC_OK on success, or an error
 *                                   code (see @ref oac_errorcodes) on
 *                                   failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT OacProjectionDecoder *oac_projection_decoder_create(
    oac_int32 Fs,
    int channels,
    int streams,
    int coupled_streams,
    unsigned char *demixing_matrix,
    oac_int32 demixing_matrix_size,
    int *error) OAC_ARG_NONNULL(5);


/** Initialize a previously allocated projection decoder state object.
 * The memory pointed to by \a st must be at least the size returned by
 * oac_projection_decoder_get_size().
 * This is intended for applications which use their own allocator instead of
 * malloc.
 * To reset a previously initialized state, use the #OAC_RESET_STATE CTL.
 * @see oac_projection_decoder_create
 * @see oac_projection_deocder_get_size
 * @param st <tt>OacProjectionDecoder*</tt>: Projection encoder state to initialize.
 * @param Fs <tt>oac_int32</tt>: Sampling rate to decode at (in Hz).
 *                                This must be one of 8000, 12000, 16000,
 *                                24000, or 48000.
 * @param channels <tt>int</tt>: Number of channels to output.
 *                               This must be at most 255.
 *                               It may be different from the number of coded
 *                               channels (<code>streams +
 *                               coupled_streams</code>).
 * @param streams <tt>int</tt>: The total number of streams coded in the
 *                              input.
 *                              This must be no more than 255.
 * @param coupled_streams <tt>int</tt>: Number of streams to decode as coupled
 *                                      (2 channel) streams.
 *                                      This must be no larger than the total
 *                                      number of streams.
 *                                      Additionally, The total number of
 *                                      coded channels (<code>streams +
 *                                      coupled_streams</code>) must be no
 *                                      more than 255.
 * @param[in] demixing_matrix <tt>const unsigned char[demixing_matrix_size]</tt>: Demixing matrix
 *                         that mapping from coded channels to output channels,
 *                         as described in @ref oac_projection and
 *                         @ref oac_projection_ctls.
 * @param demixing_matrix_size <tt>oac_int32</tt>: The size in bytes of the
 *                                                  demixing matrix, as
 *                                                  described in @ref
 *                                                  oac_projection_ctls.
 * @returns #OAC_OK on success, or an error code (see @ref oac_errorcodes)
 *          on failure.
 */
OAC_EXPORT int oac_projection_decoder_init(
    OacProjectionDecoder *st,
    oac_int32 Fs,
    int channels,
    int streams,
    int coupled_streams,
    unsigned char *demixing_matrix,
    oac_int32 demixing_matrix_size) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(6);


/** Decode a projection Oac packet.
 * @param st <tt>OacProjectionDecoder*</tt>: Projection decoder state.
 * @param[in] data <tt>const unsigned char*</tt>: Input payload.
 *                                                Use a <code>NULL</code>
 *                                                pointer to indicate packet
 *                                                loss.
 * @param len <tt>oac_int32</tt>: Number of bytes in payload.
 * @param[out] pcm <tt>oac_int16*</tt>: Output signal, with interleaved
 *                                       samples.
 *                                       This must contain room for
 *                                       <code>frame_size*channels</code>
 *                                       samples.
 * @param frame_size <tt>int</tt>: The number of samples per channel of
 *                                 available space in \a pcm.
 *                                 If this is less than the maximum packet duration
 *                                 (120 ms; 5760 for 48kHz), this function will not be capable
 *                                 of decoding some packets. In the case of PLC (data==NULL)
 *                                 or FEC (decode_fec=1), then frame_size needs to be exactly
 *                                 the duration of audio that is missing, otherwise the
 *                                 decoder will not be in the optimal state to decode the
 *                                 next incoming packet. For the PLC and FEC cases, frame_size
 *                                 <b>must</b> be a multiple of 2.5 ms.
 * @param decode_fec <tt>int</tt>: Flag (0 or 1) to request that any in-band
 *                                 forward error correction data be decoded.
 *                                 If no such data is available, the frame is
 *                                 decoded as if it were lost.
 * @returns Number of samples decoded on success or a negative error code
 *          (see @ref oac_errorcodes) on failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT int oac_projection_decode(
    OacProjectionDecoder *st,
    const unsigned char *data,
    oac_int32 len,
    oac_int16 *pcm,
    int frame_size,
    int decode_fec) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(4);

/** Decode a projection Oac packet.
 * @param st <tt>OacProjectionDecoder*</tt>: Projection decoder state.
 * @param[in] data <tt>const unsigned char*</tt>: Input payload.
 *                                                Use a <code>NULL</code>
 *                                                pointer to indicate packet
 *                                                loss.
 * @param len <tt>oac_int32</tt>: Number of bytes in payload.
 * @param[out] pcm <tt>oac_int32*</tt>: Output signal, with interleaved
 *                                       samples representing (or slightly exceeding) 24-bit values.
 *                                       This must contain room for
 *                                       <code>frame_size*channels</code>
 *                                       samples.
 * @param frame_size <tt>int</tt>: The number of samples per channel of
 *                                 available space in \a pcm.
 *                                 If this is less than the maximum packet duration
 *                                 (120 ms; 5760 for 48kHz), this function will not be capable
 *                                 of decoding some packets. In the case of PLC (data==NULL)
 *                                 or FEC (decode_fec=1), then frame_size needs to be exactly
 *                                 the duration of audio that is missing, otherwise the
 *                                 decoder will not be in the optimal state to decode the
 *                                 next incoming packet. For the PLC and FEC cases, frame_size
 *                                 <b>must</b> be a multiple of 2.5 ms.
 * @param decode_fec <tt>int</tt>: Flag (0 or 1) to request that any in-band
 *                                 forward error correction data be decoded.
 *                                 If no such data is available, the frame is
 *                                 decoded as if it were lost.
 * @returns Number of samples decoded on success or a negative error code
 *          (see @ref oac_errorcodes) on failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT int oac_projection_decode24(
    OacProjectionDecoder *st,
    const unsigned char *data,
    oac_int32 len,
    oac_int32 *pcm,
    int frame_size,
    int decode_fec) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(4);

/** Decode a projection Oac packet with floating point output.
 * @param st <tt>OacProjectionDecoder*</tt>: Projection decoder state.
 * @param[in] data <tt>const unsigned char*</tt>: Input payload.
 *                                                Use a <code>NULL</code>
 *                                                pointer to indicate packet
 *                                                loss.
 * @param len <tt>oac_int32</tt>: Number of bytes in payload.
 * @param[out] pcm <tt>oac_int16*</tt>: Output signal, with interleaved
 *                                       samples.
 *                                       This must contain room for
 *                                       <code>frame_size*channels</code>
 *                                       samples.
 * @param frame_size <tt>int</tt>: The number of samples per channel of
 *                                 available space in \a pcm.
 *                                 If this is less than the maximum packet duration
 *                                 (120 ms; 5760 for 48kHz), this function will not be capable
 *                                 of decoding some packets. In the case of PLC (data==NULL)
 *                                 or FEC (decode_fec=1), then frame_size needs to be exactly
 *                                 the duration of audio that is missing, otherwise the
 *                                 decoder will not be in the optimal state to decode the
 *                                 next incoming packet. For the PLC and FEC cases, frame_size
 *                                 <b>must</b> be a multiple of 2.5 ms.
 * @param decode_fec <tt>int</tt>: Flag (0 or 1) to request that any in-band
 *                                 forward error correction data be decoded.
 *                                 If no such data is available, the frame is
 *                                 decoded as if it were lost.
 * @returns Number of samples decoded on success or a negative error code
 *          (see @ref oac_errorcodes) on failure.
 */
OAC_EXPORT OAC_WARN_UNUSED_RESULT int oac_projection_decode_float(
    OacProjectionDecoder *st,
    const unsigned char *data,
    oac_int32 len,
    float *pcm,
    int frame_size,
    int decode_fec) OAC_ARG_NONNULL(1) OAC_ARG_NONNULL(4);


/** Perform a CTL function on a projection Oac decoder.
 *
 * Generally the request and subsequent arguments are generated by a
 * convenience macro.
 * @param st <tt>OacProjectionDecoder*</tt>: Projection decoder state.
 * @param request This and all remaining parameters should be replaced by one
 *                of the convenience macros in @ref oac_genericctls,
 *                @ref oac_decoderctls, @ref oac_multistream_ctls, or
 *                @ref oac_projection_ctls.
 * @see oac_genericctls
 * @see oac_decoderctls
 * @see oac_multistream_ctls
 * @see oac_projection_ctls
 */
OAC_EXPORT int oac_projection_decoder_ctl(OacProjectionDecoder *st, int request, ...) OAC_ARG_NONNULL(1);


/** Frees an <code>OacProjectionDecoder</code> allocated by
 * oac_projection_decoder_create().
 * @param st <tt>OacProjectionDecoder</tt>: Projection decoder state to be freed.
 */
OAC_EXPORT void oac_projection_decoder_destroy(OacProjectionDecoder *st);


/**@}*/

/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* OAC_PROJECTION_H */
