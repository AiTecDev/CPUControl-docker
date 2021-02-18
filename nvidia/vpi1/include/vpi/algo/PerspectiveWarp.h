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
 * @file PerspectiveWarp.h
 *
 * Declares functions that implement the Perspective Warp algorithm.
 */

#ifndef NV_VPI_ALGORITHMS_PERSPECTIVE_WARP_H
#define NV_VPI_ALGORITHMS_PERSPECTIVE_WARP_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"
#include "../WarpMap.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_PerspectiveWarp Perspective Warp
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Perspective Warp
 */

/**
 * Creates a payload for \ref algo_persp_warp algorithm.
 * This function allocates all resources needed by the algorithm and
 * ties the returned payload to the given stream.
 *
 * @param[in] backends VPI backends that are eligible to execute the algorithm. Currently only one backend is accepted.
 * @param[out] payload Pointer to a payload handle with the created payload.
 *
 * @returns \ref VPI_SUCCESS if payload was created successfully, or some other \ref VPIStatus value in case of error.
 */
VPI_PUBLIC VPIStatus vpiCreatePerspectiveWarp(uint32_t backends, VPIPayload *payload);

/**
 * Submits the \ref algo_persp_warp operation to the stream associated with the payload.
 *
 * @param[in] stream The stream where the operation will be queued in.
 * @param[in] backend Backend that will execute the algorithm.
 *                    Must be one of the backends specified during payload creation.
 *                    If 0, VPI will select one of the eligible backends from the payload
 *                    that accepts the given parameters, usually the fastest one.
 * @param[in] payload Payload created by \ref vpiCreatePerspectiveWarp.
 * @param[in] input Input image to be warped.
 * @param[in] xform Transform to be applied.
 * @param[out] output Output image where warped image is written to.
 * @param[in] interp Interpolation mode to be used. Valid values:
 *                    - \ref VPI_INTERP_NEAREST
 *                    - \ref VPI_INTERP_LINEAR
 *                    - \ref VPI_INTERP_CATMULL_ROM
 * @param[in] border Border extension to use for samples that fall outsize input's bounds. Value values:
 *                    - \ref VPI_BORDER_ZERO
 * @param[in] flags Flags to modify algorithm behavior. Valid values:
 *                    - 0: default, transform isn't inverted, algorithm shall invert it.
 *                    - \ref VPI_WARP_INVERSE : transform is already inverted, algorithm can use it directly.
 *                    - \ref VPI_PRECISE : precise, but potentially slower implementation.
 *
 * @returns \ref VPI_SUCCESS if algorithm was submitted successfully, or some other \ref VPIStatus value in case of error.
 *               Some errors will only be reported asynchronously by synchronization functions or subsequent algorithm
 *               submissions.
 */
VPI_PUBLIC VPIStatus vpiSubmitPerspectiveWarp(VPIStream stream, uint32_t backend, VPIPayload payload, VPIImage input,
                                              const VPIPerspectiveTransform xform, VPIImage output,
                                              VPIInterpolationType interp, VPIBorderExtension border, uint32_t flags);

/** @} end of VPI_PerspectiveWarp */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_PERSPECTIVE_WARP_H */
