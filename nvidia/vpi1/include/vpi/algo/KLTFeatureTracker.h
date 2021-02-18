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
 * @file KLTFeatureTracker.h
 *
 * Declares functions that implement the KLT Feature Tracker algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_KLT_BOUNDING_BOX_TRACKER_H
#define NV_VPI_ALGORITHMS_KLT_BOUNDING_BOX_TRACKER_H

/**
 * @defgroup VPI_KLTFeatureTracker KLT Feature Tracker
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs KLT Feature tracking on a sequence of frames.
 */

#include "../Export.h"
#include "../ImageFormat.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Creation parameters of KLT Feature Tracker. */
typedef struct
{
    int32_t maxTemplateCount;  /**< Maximum number of templates to be tracked. */
    int32_t maxTemplateWidth;  /**< Maximum width of each tracked template. */
    int32_t maxTemplateHeight; /**< Maximum height of each tracked templates. */
} VPIKLTFeatureTrackerCreationParams;

/** Initialize \ref VPIKLTFeatureTrackerCreationParams with default values.
 *
 * Default values:
 *   - maxTemplateCount: 64
 *   - maxTemplateWidth: 64
 *   - maxTemplateHeight: 64
 *
 * @param[out] params Structure to be filled with default values.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiInitKLTFeatureTrackerCreationParams(VPIKLTFeatureTrackerCreationParams *params);

/**
 * Creates payload for \ref vpiSubmitKLTFeatureTracker
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] imageWidth, imageHeight Input image dimensions.
 * @param[in] imageFormat Input image format.
 * @param[in] params
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiCreateKLTFeatureTracker(uint32_t backends, int32_t imageWidth, int32_t imageHeight,
                                                VPIImageFormat imageFormat,
                                                const VPIKLTFeatureTrackerCreationParams *params, VPIPayload *payload);

/**
 * KLT Feature Tracker algorithm type.
 */
typedef enum
{
    /** Inverse compositional algorithm for KLT tracker.
        The inverse compositional algorithm is a reformulation of the classic
        Lucas-Kanade algorithm to make the steepest-descent images and Hessian
        constant.<br>
        <table border=0px margin=0px>
        <tr><td valign=top>Ref:</td><td>Simon Baker, Iain Matthew, <a href="http://www.ncorr.com/download/publications/bakerunify.pdf">"Lucas-Kanade 20 Years On: A Unified Framework"</a>.<br>
             International Journal of Computer Vision, February 2004, Volume 56, issue 3, pp. 228-231.</td></tr>
        </table>
    */
    VPI_KLT_INVERSE_COMPOSITIONAL
} VPIKLTFeatureTrackerType;

/**
 * Structure that defines the parameters for \ref vpiCreateKLTFeatureTracker
 */
typedef struct
{
    int32_t numberOfIterationsScaling; /**< Number of Inverse compositional iterations of scale estimations */
    float nccThresholdUpdate;          /**< Threshold for requiring template update. Must be a value between 0 and 1. */
    float nccThresholdKill; /**< Threshold to consider template tracking was lost. Must be a value between 0 and 1. */
    float nccThresholdStop; /**< Threshold to stop estimating. */

    /** Maximum relative scale change.
     * Scale changes larger than this will make KLT consider that tracking was lost. */
    float maxScaleChange;

    /** Maximum relative translation change.
     * Translation changes larger than this will make KLT consider that tracking was lost. */
    float maxTranslationChange;

    /** Type of KLT tracking that will be performed. */
    VPIKLTFeatureTrackerType trackingType;
} VPIKLTFeatureTrackerParams;

/**
 * Runs KLT Feature Tracker on two frames. Outputs tracked bounding boxes and estimated transform array.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload payload created with \ref vpiCreateKLTFeatureTracker
 * @param[in] referenceImage reference image
 * @param[in] inputBoxList Input bounding box array, must have type \ref VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX
 * @param[in] inputPredictionList Input predicted transform array, must have type \ref VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D
 * @param[in] templateImage template image
 * @param[out] outputBoxList output Bounding box array, must have type \ref VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX
 * @param[out] outputEstimationList Sstimated transform array, must have type \ref VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D
 * @param[in] params Control parameters of the KLT feature tracker algorithm.
 * 
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiSubmitKLTFeatureTracker(VPIStream stream, uint32_t backend, VPIPayload payload,
                                                VPIImage templateImage, VPIArray inputBoxList,
                                                VPIArray inputPredictionList, VPIImage referenceImage,
                                                VPIArray outputBoxList, VPIArray outputEstimationList,
                                                const VPIKLTFeatureTrackerParams *params);
/** @} end of VPI_KLTFeatureTracker */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_KLT_BOUNDING_BOX_TRACKER_H */
