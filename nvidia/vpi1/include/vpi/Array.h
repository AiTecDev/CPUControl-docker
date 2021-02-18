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
 * @file Array.h
 *
 * Functions and structures for dealing with VPI arrays.
 */

#ifndef NV_VPI_ARRAY_H
#define NV_VPI_ARRAY_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * An abstract representation of a generic 1D array.
 *
 * There are two ways of creating 1D array containers with the API. The most
 * basic one is to use \ref vpiArrayCreate to allocate and initialize an empty
 * (zeroed) \ref VPIArray object. The memory for the array data is allocated
 * and managed by the backend implementation. Parameters such as capacity
 * format are immutable and specified at the construction time. The internal
 * memory layout is also backend-specific. More importantly, efficient exchange
 * of array data between different hardware blocks might force the
 * implementation to allocate the memory in multiple memory pools (e.g. dGPU
 * and system DRAM). In some scenarios (to optimize performance and memory use)
 * it might be beneficial to constrain the internal allocation policy to
 * support only a particular set of backends.
 *
 * To enable interop with existing host- or gpu-side code, the user can also
 * create an array object that wraps a user-allocated (and managed) array
 * data. Similarly to \ref vpiArrayCreate, array parameters passed to
 * `vpiArrayCreate*Wrapper()` are fixed. To prevent excessive copying, users
 * can point to array data that resides directly in the CUDA device memory with
 * \ref vpiArrayCreateCUDAMemWrapper.
 *
 * The wrapped memory can be redefined by calling \ref
 * vpiArraySetWrappedHostMem or \ref vpiArraySetWrappedCUDAMem as long as the
 * new wrapped memory has the same capacity and format as the one originally
 * wrapped. It's more efficient to create the VPIArray wrapper once and reuse
 * it later then creating and destroying it all the time.
 *
 * The set of \ref vpiArrayLock / \ref vpiArrayUnlock allows to read from/write
 * to the array data from host. These functions are non-blocking and oblivious
 * to the stream command queue so it's up to the user to make sure that all
 * pending operations using this array as input or output are finished. Also,
 * depending on which device the memory is allocated, lock/unlock operation
 * might be time-consuming and, for example, involve copying data over PCIe bus
 * for dGPUs.
 *
 * VPI allows array interoperability with the following memory types:
 * - CUDA memory: \ref vpiArrayCreateCUDAMemWrapper
 * - Host/CPU memory: \ref vpiArrayCreateHostMemWrapper
 *
 * @defgroup VPI_Array Array
 * @ingroup VPI_API_Core
 * @{
 *
 */

#include "ArrayType.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Stores information about array characteristics and content. */
typedef struct VPIArrayDataRec
{
    VPIArrayType format;  /**< Format of each array element. */
    int32_t *sizePointer; /**< Points to the number of elements in the array. */
    int32_t capacity;     /**< Maximum number of elements that the array can hold. */
    int32_t strideBytes;  /**< Size in bytes of each array element. */
    void *data;           /**< Points to the first element of the array. */
} VPIArrayData;

/**
 * Create an empty array instance.
 *
 * Array data is zeroed. Maximum capacity of the array is fixed and defined at the
 * construction-time. The VPIArray object owns the allocated memory.
 *
 * @param[in] capacity Array capacity in elements.
 * @param[in] type Type of each element.
 * @param[in] flags Array flags. Here it can be specified in what backends the
 *                  array can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] array Pointer to memory that will receive the created array handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiArrayCreate(int32_t capacity, VPIArrayType type, uint32_t flags, VPIArray *array);

/**
 * Create an array object by wrapping an existing host memory block.
 *
 * Stride between elements has to be at least as large as the element structure size. It also has
 * to respect alignment requirements of the element data structure.
 *
 * The returned handle must be destroyed when not being used anymore by calling \ref vpiArrayDestroy.
 *
 * The object doesn't own the wrapped memory. The user is still responsible for wrapped memory lifetime, which
 * must be valid until the array object is destroyed.
 *
 * @ingroup VPI_CPUInterop
 *
 * @param[in] arrayData \ref VPIArrayData pointing to the host memory block to be wrapped.
 * @param[in] flags Array flags. Here it can be specified in what backends the
 *                  array can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] array Pointer to memory that will receive the created array handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiArrayCreateHostMemWrapper(const VPIArrayData *arrayData, uint32_t flags, VPIArray *array);

/**
 * Redefines the wrapped host memory in an existing \ref VPIArray wrapper.
 *
 * The old wrapped memory and the new one must have same capacity, element format and must point
 * to host-side (cpu-accessible) memory.
 *
 * The VPIArray must have been created by \ref vpiArrayCreateHostMemWrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped memory must not be deallocated while it's still being wrapped.
 *
 * @ingroup VPI_CPUInterop
 *
 * @param[in] array Handle to array created by \ref vpiArrayCreateHostMemWrapper.
 * @param[in] arrayData \ref VPIArrayData pointing to the new host memory block to be wrapped.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiArraySetWrappedHostMem(VPIArray array, const VPIArrayData *arrayData);

/**
 * Destroy an array instance.
 *
 * This function deallocates all resources allocated by the array creation function.
 * When destroying an VPIArray wrapper, the wrapped memory itself isn't deallocated.
 *
 * @param[in] array Array handle to be destroyed. Passing NULL is allowed, to which the function simply does nothing.
 */
VPI_PUBLIC void vpiArrayDestroy(VPIArray array);

/**
 * Informs that the array's wrapped memory was updated outside VPI.
 *
 * This method is used with wrapped arrays only, i.e. created with
 * 'vpiArrayCreate*Wrapper*` functions. If the underlying array data has been
 * modified outside VPI, use this method to mark the array as invalid. This
 * will force the API to update its internal representation (e.g., re-upload to
 * CUDA memory) when necessary.
 *
 * @param[in] array An array handle created by one of the wrapper array creation functions.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayInvalidate(VPIArray array);

/**
 * Returns the array size in elements.
 *
 * @param[in] array A valid array handle.
 * @param[out] size A pointer to a variable which will be set to the size of the array.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayGetSize(VPIArray array, int32_t *size);

/**
 * Set the array size in elements.
 *
 * @param[in] array A valid array handle.
 * @param[in] size The new size of the array. Must be less or equal than array's capacity.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArraySetSize(VPIArray array, int32_t size);

/**
 * Returns the array capacity in elements.
 *
 * @param[in] array A valid array handle
 * @param[out] capacity A pointer to a variable which will be set to the capacity of the array.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayGetCapacity(VPIArray array, int32_t *capacity);

/**
 * Returns the array stride (distance between two consecutive elements) in bytes.
 *
 * @param[in] array A valid array handle.
 * @param[out] strideBytes A pointer to a variable which will be set to the stride of the array element, in bytes.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayGetStrideBytes(VPIArray array, int32_t *strideBytes);

/**
 * Returns the array flags.
 *
 * @param[in] array A valid array handle.
 * @param[out] flags A pointer where the flags will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayGetFlags(VPIArray array, uint32_t *flags);

/**
 * Returns the array type.
 *
 * @param[in] array A valid array handle.
 * @param[out] type A pointer where the array type will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayGetFormat(VPIArray array, VPIArrayType *type);

/**
 * Acquires the lock on array object and returns a pointer to array data.
 *
 * Depending on the internal array representation, as well as the actual location in memory, this
 * function might have a significant performance overhead (format conversion, layout conversion,
 * device-to-host memory copy).
 *
 * The array can be locked multiple times. Each lock operation increments a
 * counter and must be matched by a corresponding \ref vpiArrayUnlock
 * call. Lock will fail if the array is being used by an algorithm.
 *
 * @param[in] array A valid array handle.
 * @param[in] mode Lock mode, depending on whether the memory will be written to and/or read from.
 * @param[out] arrayData A pointer to a structure that will be filled with information about the array memory.
 *                       If it's NULL, the array will still be locked. This is useful to make sure
 *                       wrapped array is updated.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayLock(VPIArray array, VPILockMode mode, VPIArrayData *arrayData);

/**
 * Releases the lock on array object.
 *
 * This function might have a significant performance overhead (format conversion, layout
 * conversion, host-to-device memory copy).
 *
 * The array is effectively unlocked when the internal lock counter reaches 0.
 *
 * @param[in] array A valid array handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiArrayUnlock(VPIArray array);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Array */

#endif /* NV_VPI_ARRAY_H */
