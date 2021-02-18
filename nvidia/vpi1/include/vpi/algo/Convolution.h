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
 * @file Convolution.h
 *
 * Declares functions to perform image filtering with convolution kernels.
 *
 */

#ifndef NV_VPI_ALGORITHMS_CONVOLUTION_H
#define NV_VPI_ALGORITHMS_CONVOLUTION_H

#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_Convolution Convolution
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Convolves an image with a 2D kernel.
 * \ref vpiSubmitConvolution is used for generic 2D kernels, separable or not. For separable kernels,
 * it's usually more efficient to use \ref vpiSubmitSeparableConvolution.
 *
 */

/**
 * Runs a generic 2D convolution over an image.
 *
 * @param[in] stream The stream handle where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 * @param[in] input Input image to be convolved with the kernel.
 * @param[out] output Output image where the result is written to. Its image format and dimensions must be the same as input.
 * @param[in] kernelWidth, kernelHeight Kernel dimensions.
 * @param[in] kernelData Convolution kernel coefficients, in row-major layout.
 * @param[in] border Border extension.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiSubmitConvolution(VPIStream stream, uint32_t backend, VPIImage input, VPIImage output,
                                          const float *kernelData, int32_t kernelWidth, int32_t kernelHeight,
                                          VPIBorderExtension border);

/**
 * Runs a generic 2D convolution operation over an image, optimized for separable kernels.
 *
 * @param[in] stream The stream handle where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 * @param[in] input Input image to be convolved with the kernel.
 * @param[out] output Output image where the result is written to. Its image format and dimensions must be the same as input's.
 * @param[in] kernelXSize, kernelYSize Kernel dimensions in X and Y directions.
 * @param[in] kernelXData, kernelYData Convolution kernel coefficients, in both X and Y directions respectively.
 * @param[in] border Border extension.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiSubmitSeparableConvolution(VPIStream stream, uint32_t backend, VPIImage input, VPIImage output,
                                                   const float *kernelXData, int32_t kernelXSize,
                                                   const float *kernelYData, int32_t kernelYSize,
                                                   VPIBorderExtension border);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_CONVOLUTION_H */
