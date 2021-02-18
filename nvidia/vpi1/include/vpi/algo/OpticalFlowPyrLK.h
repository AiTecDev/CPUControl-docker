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
 * @file OpticalFlowPyrLK.h
 *
 * Declares functions that implement the Pyramidal LK Optical Flow algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_OPTICALFLOWPYRLK_H
#define NV_VPI_ALGORITHMS_OPTICALFLOWPYRLK_H

/**
 * @defgroup VPI_OpticalFlowPyrLK Pyramidal LK Optical Flow
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Algorithm used to track points from one frame to the next.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines the error measurement types
 */
typedef enum
{
    /** L1 distance between previous feature and a next feature
     */
    VPI_LK_ERROR_L1
} VPIEpsilonType;

/**
 * Structure that defines the parameters for \ref vpiSubmitOpticalFlowPyrLK
 */
typedef struct
{
    /** Uses initial estimations stored in curPts when this flag is not 0,
     * otherwise prevPts is copied to curPts and is considered the initial estimate.
     */
    uint32_t useInitialFlow;

    /** Specifies the termination criteria. It can be bitwise combination of \ref VPI_TERMINATION_CRITERIA_ITERATIONS
     * and \ref VPI_TERMINATION_CRITERIA_EPSILON.
     */
    uint32_t termination;

    /** Specifies the tracking error type. */
    VPIEpsilonType epsilonType;

    /** Specifies the error for terminating the algorithm. */
    float epsilon;

    /** Specifies the number of iterations. */
    int32_t numIterations;

    /** Specifies the size of the window on which to perform the algorithm.
     *  This field must be greater than
     *  or equal to 6 and less than or equal to 32. */
    int32_t windowDimension;

} VPIOpticalFlowPyrLKParams;

/**
 * Creates payload for \ref vpiSubmitOpticalFlowPyrLK
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] fmt Specifies the format for the pyramid.
 * @param[in] width, height  Specifies the dimensions of the pyramid on the finest resolution.
 * @param[in] levels Specifies the level of the pyramid.
 * @param[in] scale Specifies the scale of the pyramid.
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiCreateOpticalFlowPyrLK(uint32_t backends, int32_t width, int32_t height, VPIImageFormat fmt,
                                               int32_t levels, float scale, VPIPayload *payload);

/**
 * Runs Pyramidal LK Optical Flow on two frames. Outputs estimated feature points and tracking status.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload created with \ref vpiCreateOpticalFlowPyrLK
 * @param[in] prevPyr Previous frame, represented as a gaussian pyramid.
 * @param[in] curPyr  Current frame, represented as a gaussian pyramid.
 * @param[in] prevPts array of keypoints in the prevPyr high resolution pyramid. The type of array must be \ref VPI_ARRAY_TYPE_KEYPOINT.
 * @param[in,out] curPts array of keypoints in the curPyr high resolution pyramid. It is used as starting
 *                          search point in curPyr if useInitialFlow is not 0.
 *                          The type of array must be \ref VPI_ARRAY_TYPE_KEYPOINT.
 * @param[in,out] trackingStatus Status array for tracking status for each
 *                               input in prevPts. If element is zero, the corresponding keypoint isn't being
 *                               tracked, otherwise, it is.
 *                               The type of the array must be \ref VPI_ARRAY_TYPE_U8.
 * @param[in] params parameters for the LK tracker
 * 
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiSubmitOpticalFlowPyrLK(VPIStream stream, uint32_t backend, VPIPayload payload,
                                               VPIPyramid prevPyr, VPIPyramid curPyr, VPIArray prevPts, VPIArray curPts,
                                               VPIArray trackingStatus, const VPIOpticalFlowPyrLKParams *params);

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_OpticalFlowPyrLK

#endif // NV_VPI_ALGORITHMS_OPTICALFLOWPYRLK_H
