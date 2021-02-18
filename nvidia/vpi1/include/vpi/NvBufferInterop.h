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
 * @file NvBufferInterop.h
 *
 * Functions and structures for dealing with NvBuffer interop with VPI.
 */

#ifndef NV_VPI_NVBUFFER_INTEROP_H
#define NV_VPI_NVBUFFER_INTEROP_H

#include "Export.h"
#include "ImageFormat.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * @ingroup VPI_NvBufferInterop NvBuffer interop
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Parameters for customizing NvBuffer wrapping.
 * These parameters are used to customize how NvBuffer wrapping will be made.
 * Make sure to call \ref vpiInitWrapNvBufferParams to initialize this
 * structure before updating its attributes. This guarantees that new attributes
 * added in future versions will have a suitable default value assigned.
 */
typedef struct
{
    /** Overriden \ref VPIColorSpec of the wrapped Nvbuffer.
     * It it's a valid color spec, override the color spec information in the wrapped NvBuffer.
     * If \ref VPI_COLOR_SPEC_DEFAULT, infer the color spec from the wrapped NvBuffer. */
    VPIColorSpec colorSpec;
} VPIWrapNvBufferParams;

/** Initialize \ref VPIWrapNvBufferParams with default values.
 *
 * Default values are:
 * - colorSpec: \ref VPI_COLOR_SPEC_DEFAULT
 *
 * @param[in] params Structure to be initialized with default values.
 * 
 * @returns \ref VPI_SUCCESS if function succeeded, or \ref VPI_ERROR_INVALID_ARGUMENT is params is NULL.
 */
VPI_PUBLIC VPIStatus vpiInitWrapNvBufferParams(VPIWrapNvBufferParams *params);

/**
 * Create an image object by wrapping an existing `NvBuffer` given its dmabuf_fd handle.
 *
 * The underlying image object does not own/claim the memory block.
 *
 * @ingroup VPI_NvBufferInterop
 *
 * @param[in] dmabuf_fd Handle to the `NvBuffer` to be wrapped.
 * @param[in] params If not NULL, use the parameters to modify how the NvBuffer is wrapped.
 *                   Otherwise, it'll use default NvBuffer handling.
 * @param[in] flags Image flags. Here it can be specified in what backends the
 *                  image can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageCreateNvBufferWrapper(int32_t dmabuf_fd, const VPIWrapNvBufferParams *params,
                                                   uint32_t flags, VPIImage *img);

/**
 * Redefines the wrapped `NvBuffer` memory in an existing \ref VPIImage wrapper.
 *
 * The old wrapped `NvBuffer` and the new one must have same dimensions and format.
 *
 * The VPIImage must have been created by \ref vpiImageCreateNvBufferWrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped `NvBuffer` will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped `NvBuffer` must not be deallocated while it's still being wrapped.
 *
 * @ingroup VPI_NvBufferInterop
 *
 * @param[in] img Handle to image created by \ref vpiImageCreateNvBufferWrapper.
 * @param[in] dmabuf_fd The new NvBuffer to be wrapped.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageSetWrappedNvBuffer(VPIImage img, int dmabuf_fd);

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_NvBufferInterop

#endif // NV_VPI_NVBUFFER_INTEROP_H
