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
 * @file GaussianFilter.h
 *
 * Declares functions that implement the Gaussian Filter algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_GAUSSIAN_FILTER_H
#define NV_VPI_ALGORITHMS_GAUSSIAN_FILTER_H

#include "vpi/Export.h"
#include "vpi/Status.h"
#include "vpi/Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_GaussianFilter Gaussian Filter
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Runs a generic 2D Gaussian filter over the input image.
 */

/**
 * Runs a 2D Gaussian filter over an image.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 * @param[in] input Input image to be filtered.
 * @param[out] output Image where the result will be written to.
 * @param[in] kernelSizeX, kernelSizeY Gaussian kernel size in X and Y
 *                                     directions respectively. If 0, it'll be
 *                                     `2*ceil(3*sigma)` rounded to the next
 *                                     odd size. Limited between 3 and 11, and
 *                                     must be odd.
 * @param[in] sigmaX, sigmaY Standard deviation of the Gaussian kernel in the X
 *                           and Y directions respectively.
 *                           If `kernelSize==0`, sigma is limited to 2.
 * @param[in] border Border extension to be used.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiSubmitGaussianFilter(VPIStream stream, uint32_t backend, VPIImage input, VPIImage output,
                                             int32_t kernelSizeX, int32_t kernelSizeY, float sigmaX, float sigmaY,
                                             VPIBorderExtension border);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_GaussianFilter */

#endif /* NV_VPI_ALGORITHMS_GAUSSIAN_FILTER_H */
