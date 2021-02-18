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
 * @file CUDAInterop.h
 *
 * Functions and structures for dealing with CUDA interop with VPI.
 */

#ifndef NV_VPI_CUDAINTEROP_H
#define NV_VPI_CUDAINTEROP_H

#include "Array.h"
#include "Event.h"
#include "Export.h"
#include "Image.h"
#include "ImageFormat.h"
#include "Status.h"
#include "Stream.h"
#include "Types.h"

#include <cuda.h>
#include <stdint.h>

/**
 * @ingroup VPI_CUDAInterop CUDA interop
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create an image object by wrapping around an existing device (CUDA) memory block.
 *
 * Only pitch-linear format is supported. The underlying image object does not own/claim the memory
 * block.
 *
 * @param[in] cudaData Pointer to structure with cuda memory to be wrapped.
 * @param[in] flags Image flags. Here it can be specified in what backends the
 *                  image can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageCreateCUDAMemWrapper(const VPIImageData *cudaData, uint32_t flags, VPIImage *img);

/**
 * Redefines the wrapped device (CUDA) memory in an existing \ref VPIImage wrapper.
 *
 * The old wrapped memory and the new one must have same dimensions, format and must point
 * to device-side (cuda-accessible) memory.
 *
 * The VPIImage must have been created by \ref vpiImageCreateCUDAMemWrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped memory must not be deallocated while it's still being wrapped.
 *
 * @param[in] img Handle to image created by \ref vpiImageCreateCUDAMemWrapper.
 * @param[in] hostData \ref VPIImageData pointing to the new device memory block to be wrapped.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageSetWrappedCUDAMem(VPIImage img, const VPIImageData *hostData);

/**
 * Create an event object by wrapping around an existing CUDA `CUevent` object.
 * The created event can be used by \ref vpiEventSync / \ref vpiStreamWaitEvent to synchronize on a
 * previously recorded CUDA event, or CUDA synchronization functions can be used to synchronize on
 * events captured with vpiEventRecord().
 *
 * @warning This function is currently not implemented.
 *
 * @param[in] cudaEvent CUDA event handle to be wrapped.
 * @param[out] event Pointer to memory that will receive the created event handle.
 *
 * @returns Always returns \ref VPI_ERROR_NOT_IMPLEMENTED.
 */
VPI_PUBLIC VPIStatus vpiEventCreateCUDAEventWrapper(CUevent cudaEvent, VPIEvent *event);

/**
 * Create an array object by wrapping an existing device (CUDA) memory block.
 *
 * Stride between elements has to be at least as large as the element structure size. It also has
 * to respect alignment requirements of the element data structure.
 *
 * The returned handle must be destroyed when not being used anymore by calling \ref vpiArrayDestroy.
 *
 * The object doesn't own the wrapped memory. The user is still responsible for wrapped memory lifetime, which
 * must be valid until the array object is destroyed.
 *
 * @ingroup VPI_CUDAInterop
 *
 * @param[in] arrayData \ref VPIArrayData pointing to the device (CUDA) memory block to be wrapped.
 * @param[in] flags Array flags. Here it can be specified in what backends the
 *                  array can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] array Pointer to memory that will receive the created array handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiArrayCreateCUDAMemWrapper(const VPIArrayData *arrayData, uint32_t flags, VPIArray *array);

/**
 * Redefines the wrapped device (CUDA) memory in an existing \ref VPIArray wrapper.
 *
 * The old wrapped memory and the new one must have same capacity, element format and must point
 * to device-side memory.
 * The VPIArray must have been created by \ref vpiArrayCreateCUDAMemWrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped memory must not be deallocated while it's still being wrapped.
 *
 * @ingroup VPI_CUDAInterop
 *
 * @param[in] array Handle to array created by \ref vpiArrayCreateCUDAMemWrapper.
 * @param[in] arrayData \ref VPIArrayData pointing to the new host memory block to be wrapped.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArraySetWrappedCUDAMem(VPIArray array, const VPIArrayData *arrayData);

/**
 * Wraps an existing `cudaStream_t` into a VPI stream.
 *
 * CUDA algorithms are submitted for execution in the wrapped `cudaStream_t`.
 * This allows to insert a VPI-driven processing into an existing CUDA pipeline.
 * Algorithms can still be submitted to other backends.
 *
 * The VPIStream doesn't own the `cudaStream_t`. It must be valid during VPIStream lifetime.
 *
 * CUDA kernels can only be submitted directly to cudaStream_t if it's guaranteed that
 * all tasks submitted to VPIStream are finished.
 *
 * @ingroup VPI_CUDAInterop
 *
 * @param[in] cudaStream The CUDA stream handle to be wrapped.
 * @param[in] flags Stream flags. VPI_BACKEND_CUDA is always added, but other backends can be specified as well
 *                  by or-ing together \ref VPIBackend flags.
 * @param[out] stream Pointer that will receive the newly created VPIStream.
 */
VPI_PUBLIC VPIStatus vpiStreamCreateCUDAStreamWrapper(CUstream cudaStream, uint32_t flags, VPIStream *stream);

#ifdef __cplusplus
}
#endif

/** @} */ // end of VPI_CUDAInterop

#endif /* NV_VPI_CUDAINTEROP_H */
