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
 * @file ColorNames.h
 *
 * Declares functions that implement the Color Names algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_COLORNAMES_H
#define NV_VPI_ALGORITHMS_COLORNAMES_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Colornames Color Names Features Extractor
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Extracts the Color Names features of the input image.
 */

/**
 * Creates the payload for the Color Names algorithm.
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] outType Type of the output images. It must be one of:
 *                     - \ref VPI_IMAGE_FORMAT_S8
 *                     - \ref VPI_IMAGE_FORMAT_U8
 *                     - \ref VPI_IMAGE_FORMAT_S16
 *                     - \ref VPI_IMAGE_FORMAT_U16 
 *                     
 *                     16-bit types will return more precise values, in exchange of some minor performance hit. 
 *                     8-bit types will be less precise, but typically leads to faster execution time.
 * @param[out] payload Pointer to a handle that will receive the created payload.
 * 
 * @returns \ref VPI_SUCCESS if payload creation succeeded, or any other \ref VPIStatus value in case of error.
 */
VPI_PUBLIC VPIStatus vpiCreateExtractColorNameFeatures(uint32_t backends, VPIImageFormat outType, VPIPayload *payload);

/** Submits the Color Names algorithm to the stream associated with the payload.
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload to be submitted along the other parameters.
 * @param[in] input Input image. The following image types are accepted:
 *                   - \ref VPI_IMAGE_FORMAT_RGB8
 *                   - \ref VPI_IMAGE_FORMAT_RGBA8
 *                   - \ref VPI_IMAGE_FORMAT_NV12
 * @param[out] output Pointer to an array of \ref VPIImage where the features will be written to.
 *                    The images must have the same type as set during payload creation.
 * @param[in] numOutputs Number of images in output array. It must be equal to 10.
 *
 * @returns \ref VPI_SUCCESS if payload creation succeeded, or any other \ref VPIStatus value in case of error.
 */
VPI_PUBLIC VPIStatus vpiSubmitExtractColorNameFeatures(VPIStream stream, uint32_t backend, VPIPayload payload,
                                                       VPIImage input, VPIImage *output, int32_t numOutputs);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_COLORNAMES_H */
