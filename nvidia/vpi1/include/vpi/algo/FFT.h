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
 * @file FFT.h
 *
 * Declares functions that implement the Fast Fourier Transform algorithm and its inverse.
 */

#ifndef NV_VPI_ALGORITHMS_FFT_H
#define NV_VPI_ALGORITHMS_FFT_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_FFT Fast Fourier Transform
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Operations that applies the Fast Fourier Transform and its inverse to 2D images.
 *
 * Both FFT and inverse FFT need a payload created during application
 * initialization phase, where image dimensions, input and output types are
 * defined. The payload then can be used to submit operations on different
 * images, as long as their dimensions and types match what was defined during
 * payload creation.
 *
 */

/**
 * Creates payload for direct Fast Fourier Transform algorithm.
 * The created payload can then be used with \ref vpiSubmitFFT.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] inputWidth, inputHeight Dimensions of the input/output images to be used.
 * @param[in] inFormat Input image format.
 * @param[in] outFormat Output image format.
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiCreateFFT(uint32_t backends, int32_t inputWidth, int32_t inputHeight,
                                  const VPIImageFormat inFormat, const VPIImageFormat outFormat, VPIPayload *payload);

/**
 * Runs the direct Fast Fourier Transform on single image.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload created with \ref vpiCreateFFT
 * @param[in] input Input image in space domain.
 * @param[out] output Image where the result in frequency domain will be written to.
 * @param[in] flags Control flags, currently it must be set to 0.
 * 
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiSubmitFFT(VPIStream stream, uint32_t backend, VPIPayload payload, VPIImage input,
                                  VPIImage output, uint32_t flags);

/**
 * Creates payload for inverse Fast Fourier Transform algorithm.
 * The created payload can then be used with \ref vpiSubmitIFFT.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[in] outputWidth, outputHeight Output image dimensions.
 * @param[in] inFormat Input image format.
 * @param[in] outFormat Output image format.
 * @param[out] payload Pointer to memory where the created payload handle will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiCreateIFFT(uint32_t backends, int32_t outputWidth, int32_t outputHeight,
                                   const VPIImageFormat inFormat, const VPIImageFormat outFormat, VPIPayload *payload);

/**
 * Runs the inverse Fast Fourier Transform on single image.
 *
 * @param[in] stream A stream handle where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload created with \ref vpiCreateIFFT
 * @param[in] input Input image in frequency domain.
 * @param[out] output Image where the result in space domain will be written to.
 * @param[in] flags Control flags. Valid values:
 *              - 0 for default processing, where output scaled. 
 *              - \ref VPI_DENORMALIZED_OUTPUT : Output is left denormalized. This leads to
 *                                               faster execution as normalization isn't usually needed.
 *                                               Absense of this flag will scale the output, dividing it by the
 *                                               total pixel count. This makes the output the exact inverse of
 *                                               direct Fast Fourier Transform.
 * 
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiSubmitIFFT(VPIStream stream, uint32_t backend, VPIPayload payload, VPIImage input,
                                   VPIImage output, uint32_t flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_FFT */

#endif /* NV_VPI_ALGORITHMS_FFT_H */
