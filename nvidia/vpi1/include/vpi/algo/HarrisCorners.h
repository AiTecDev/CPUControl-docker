/*
 * Copyright 2019-2021 NVIDIA Corporation. All rights reserved.
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
 * @file HarrisCorners.h
 *
 * Declares functions that implement the Harris Corner Detector algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_HARRIS_CORNERS_H
#define NV_VPI_ALGORITHMS_HARRIS_CORNERS_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_HarrisCorners Harris Corners
 * @ingroup VPI_API_Algorithms
 * @{
 *
 */

/**
 * Structure that defines the parameters for \ref vpiSubmitHarrisCornerDetector
 */
typedef struct
{
    /** Gradient window size. Must be 3, 5 or 7. */
    int32_t gradientSize;

    /** Block window size used to compute the Harris Corner score. Must be 3, 5 or 7. */
    int32_t blockSize;

    /** Specifies the minimum threshold with which to eliminate Harris Corner scores */
    float strengthThresh;

    /** Specifies sensitivity threshold from the Harris-Stephens equation. */
    float sensitivity;

    /** Non-maximum suppression radius, set to 0 to disable it. For PVA backend, this must be set to 8. */
    float minNMSDistance;
} VPIHarrisCornerDetectorParams;

/**
 * Creates a \ref algo_harris_corners "Harris Corner Detector" payload.
 * This function allocates all temporary memory needed by the algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] inputWidth, inputHeight Dimensions of the input image that will be used with this payload.
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 * 
 * @returns \ref VPI_SUCCESS if payload was created successfully, some other \ref VPIStatus value in case of error.
 */
VPI_PUBLIC VPIStatus vpiCreateHarrisCornerDetector(uint32_t backends, int32_t inputWidth, int32_t inputHeight,
                                                   VPIPayload *payload);

/**
 * Submits \ref algo_harris_corners "Harris Corner Detector" operation to the stream associated with
 * the payload.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload to be submitted along the other parameters.
 * @param[in] input Input image from where the Harris corners will be extracted.
 * @param[out] outFeatures Array that will receive the detected corners. It must have type \ref VPI_ARRAY_TYPE_KEYPOINT.
 * @param[out] outScores Array that will receive the corners' scores. It must have type \ref VPI_ARRAY_TYPE_U32.
 * @param[in] params Pointer to a \ref VPIHarrisCornerDetectorParams with parameters for this algorithm invocation. These parameters can
 *                    vary in every call and will be copied internally, no need to keep the object around.
 *                    
 * @returns \ref VPI_SUCCESS if the algorithm submitted was created successfully,
 *          some other \ref VPIStatus value in case of error. Even if \ref VPI_SUCCESS is
 *          returned, the actual algorithm execution might still fail, due to
 *          invalid parameters. The actual error will only be returned by some
 *          synchronization call or another algorithm submission.
 */
VPI_PUBLIC VPIStatus vpiSubmitHarrisCornerDetector(VPIStream stream, uint32_t backend, VPIPayload payload,
                                                   VPIImage input, VPIArray outFeatures, VPIArray outScores,
                                                   const VPIHarrisCornerDetectorParams *params);

/** @} end of VPI_HarrisCorners */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_HARRIS_CORNERS_H */
