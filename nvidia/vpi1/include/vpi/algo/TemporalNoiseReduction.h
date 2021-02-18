/*
 * Copyright 2020-2021 NVIDIA Corporation. All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee. Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE. IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users. These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item. Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

/**
 * @file TemporalNoiseReduction.h
 *
 * Declares functions that implement the Temporal Noise Reduction algorithm
 */

#ifndef NV_VPI_ALGORITHMS_TEMPORAL_NOISE_REDUCTION_H
#define NV_VPI_ALGORITHMS_TEMPORAL_NOISE_REDUCTION_H

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_TNR Temporal Noise Reduction
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Performs temporal noise reduction in a sequence of images.
 */

/** Defines scene presets for temporal noise reduction.
 * These presets are used by \ref algo_tnr to define its internal parameters suitable
 * for noise reduction of scenes captured with given characteristics.
 */
typedef enum
{
    VPI_TNR_PRESET_DEFAULT,              /**< Default preset, suitable for most scenes. */
    VPI_TNR_PRESET_OUTDOOR_LOW_LIGHT,    /**< Low light outdoor scene. */
    VPI_TNR_PRESET_OUTDOOR_MEDIUM_LIGHT, /**< Medium light outdoor scene. */
    VPI_TNR_PRESET_OUTDOOR_HIGH_LIGHT,   /**< Bright light outdoor scene. */
    VPI_TNR_PRESET_INDOOR_LOW_LIGHT,     /**< Low light indoor scene. */
    VPI_TNR_PRESET_INDOOR_MEDIUM_LIGHT,  /**< Medium light indoor scene. */
    VPI_TNR_PRESET_INDOOR_HIGH_LIGHT     /**< Bright light indoor scene. */
} VPITNRPreset;

/** Defines the version of the \ref algo_tnr algorithm to be used.
 * Higher version numbers usually achieve better quality. Some versions
 * might not be available for a given device/backend.
 */
typedef enum
{
    VPI_TNR_DEFAULT, /**< Chooses the version with best quality available in the current device and given backend. */
    VPI_TNR_V1, /**< Version 1, without scene control and somewhat poor noise reduction capability specially in dark scenes, but runs fast. */
    VPI_TNR_V2, /**< Version 2, offers noise reduction strength control with decent processing speed. */
    VPI_TNR_V3  /**< Version 3, offers quite good quality overall, specially in dark scenes.*/
} VPITNRVersion;

/**
 * Creates a payload for \ref algo_tnr algorithm.
 * This function allocates all resources needed by the algorithm and ties the
 * returned payload to the given backend.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] width,height Dimensions of frames to be processed.
 * @param[in] imgFormat Format of the images to be processed. 
 * @param[in] version Version of the algorithm to be used.
 * @param[in] preset Scene preset to be used.
 * @param[in] strength Noise reduction strength. It's a floating point value between 0 and 1, inclusive.
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 *
 * @returns \ref VPI_SUCCESS if payload was created successfully, or some other \ref VPIStatus value in case of error.
 */
VPI_PUBLIC VPIStatus vpiCreateTemporalNoiseReduction(uint32_t backends, int32_t width, int32_t height,
                                                     VPIImageFormat imgFormat, VPITNRVersion version,
                                                     VPITNRPreset preset, float strength, VPIPayload *payload);

/**
 * Submits a \ref algo_tnr operation to the stream associated with the given payload.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload to be submitted along the other parameters.
 * @param[in] prevFrame When processing the first frame of a video sequence, this must be NULL. For subsequent frames, user must
 *                      pass the result of the previous run of the algorithm.
 * @param[in] curFrame Current frame to be processed.
 * @param[in] outFrame Output frame, where de-noised image will be written to.
 *
 * @returns \ref VPI_SUCCESS if algorithm was submitted successfully, or some other \ref VPIStatus value in case of error.
 *               Some errors will only be reported asynchronously by synchronization functions or subsequent algorithm
 *               submissions.
 */
VPI_PUBLIC VPIStatus vpiSubmitTemporalNoiseReduction(VPIStream stream, uint32_t backend, VPIPayload payload,
                                                     VPIImage prevFrame, VPIImage curFrame, VPIImage outFrame);

/** @} end of VPI_TNR */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_TEMPORAL_NOISE_REDUCTION_H */
