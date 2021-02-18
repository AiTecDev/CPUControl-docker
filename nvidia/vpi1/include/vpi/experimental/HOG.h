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
 * @file HOG.h
 *
 * Declares functions that implement the Histogram of Oriented Gradients algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_HOG_H
#define NV_VPI_ALGORITHMS_HOG_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_HOG Histogram of Oriented Gradients
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Extracts Histogram of Oriented Gradients features from input image.
 */

/** @name HOG flags 
 * Defines what features will be returned.
 * @{ */
#define VPI_HOG_CONTRAST_SENSITIVE 0x01   /**< Return contrast sensitive features. */
#define VPI_HOG_CONTRAST_INSENSITIVE 0x02 /**< Return contrast insensitive features. */
#define VPI_HOG_TEXTURE 0x04              /**< Return texture-related features. */

/** Helper flag to return all features. */
#define VPI_HOG_ALL_FEATURES (VPI_HOG_CONTRAST_SENSITIVE | VPI_HOG_CONTRAST_INSENSITIVE | VPI_HOG_TEXTURE)
/** @} */

/**
 * Create a payload for the non-batch version of HOG algorithm.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] width,height Dimensions of the input image to be used.
 * @param[in] features Flags to specify what features will be returned
 * @param[in] cellSize Cell size, typically 8 or 16 for 8x8 and 16x16 cells respectively.
 * @param[in] numOrientations Number of orientations used. This is typically 18.
 * @param[out] outNumFeatures Receives the number of features that will be returned. Pass NULL if not needed.
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 * 
 * @returns \ref VPI_SUCCESS if payload was created successfully, or some other \ref VPIStatus code in case of error.
 */
VPI_PUBLIC VPIStatus vpiCreateExtractHOGFeatures(uint32_t backends, int32_t width, int32_t height, int32_t features,
                                                 int32_t cellSize, int32_t numOrientations, int32_t *outNumFeatures,
                                                 VPIPayload *payload);

/**
 * Create a payload for the batch version of HOG algorithm.
 * The input images are assumed to be laid out in memory as a 2D matrix of images. All images must have the same dimensions.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] maxBatchWidth,maxBatchHeight Maximum number of images horizontally and vertically, respectively.
 * @param[in] imgWidth,imgHeight Dimensions each of the batch images.
 * @param[in] features Flags to specify which features will be returned
 * @param[in] cellSize Cell size, typically 8 or 16 for 8x8 and 16x16 cells respectively.
 * @param[in] numOrientations Number of orientations used. This is typically 18.
 * @param[out] outNumFeatures Receives the number of features that will be returned. Pass NULL if not needed.
 * @param[out] payload Pointer to a payload handle that will receive the allocated payload.
 * 
 * @returns \ref VPI_SUCCESS if payload was created successfully, or some other \ref VPIStatus code in case of error.
 */
VPI_PUBLIC VPIStatus vpiCreateExtractHOGFeaturesBatch(uint32_t backends, int32_t maxBatchWidth, int32_t maxBatchHeight,
                                                      int32_t imgWidth, int32_t imgHeight, int32_t features,
                                                      int32_t cellSize, int32_t numOrientations,
                                                      int32_t *outNumFeatures, VPIPayload *payload);

/**
 * Submit the HOG algorithm to the stream associated with the payload.
 * It handles both batch and non-batch payloads.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload to be submitted along the other parameters.
 * @param[in] input If using a non batch payload, this is the input image to be processed. Its dimensions must match
 *                   what was passed to \ref vpiCreateExtractHOGFeatures. If using a batch payload, the image dimensions
 *                   must be a multiple of (`imgWidth`,`imgHeight`), and it must be at most (`imgWidth*maxBatchWidth`, `imgHeight*maxBatchHeight`).
 * @param[out] outFeatures Pointer to an array of images that will receive the features. The dimensions of each image must be
 *                          (`input.width / cellSize`, `input.height / cellSize`). In case of batch processing, the output features position
 *                          in the 2D matrix be the same position of the corresponding input image.
 * @param[in] numFeatures Number of images in the output array.
 *
 * @returns \ref VPI_SUCCESS if payload was created successfully, or some other \ref VPIStatus code in case of error.
 */
VPI_PUBLIC VPIStatus vpiSubmitExtractHOGFeatures(VPIStream stream, uint32_t backend, VPIPayload payload, VPIImage input,
                                                 VPIImage *outFeatures, int32_t numFeatures);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_HOG_H */
