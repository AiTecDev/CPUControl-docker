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
 * @file Image.h
 *
 * Functions and structures for dealing with VPI images.
 */

#ifndef NV_VPI_IMAGE_H
#define NV_VPI_IMAGE_H

#include "Export.h"
#include "ImageFormat.h"
#include "PixelType.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * An abstract representation of a 2D image.
 *
 * There are two ways of creating 2D image containers with the API. The most
 * basic one is to use \ref vpiImageCreate to allocate and initialize an empty
 * (zeroed) \ref VPIImage object. The memory for the image data is allocated
 * and managed by VPI. Parameters such as width, height and pixel type are
 * immutable and specified at the construction time. The internal memory layout
 * is also backend-specific. More importantly, efficient exchange of image data
 * between different hardware backends might force the implementation to
 * allocate the memory in multiple memory pools (e.g. dGPU and system DRAM). In
 * some scenarios (to optimize performance and memory use), it might be
 * beneficial to constrain the internal allocation policy to support only a
 * particular set of backends. 
 *
 * To enable interop with existing host- or gpu-side code, the user can also
 * create an image object that wraps a user-allocated (and managed) image
 * data. Similarly to \ref vpiImageCreate, image parameters passed to
 * `vpiImageCreate*Wrapper()` are fixed. To prevent excessive copying, users
 * can point to image data that resides directly in the CUDA device memory with
 * \ref vpiImageCreateCUDAMemWrapper.
 *
 * The wrapped memory can be redefined by calling `vpiImageSetWrapped*()`
 * corresponding to the image wrapper creation function used, as long as the
 * new wrapped memory has the same capacity and type as the one originally
 * wrapped. It's more efficient to create the VPIImage wrapper once and reuse
 * it later then creating and destroying it all the time.
 *
 * The set of \ref vpiImageLock / \ref vpiImageUnlock allows the user to read
 * from/write to the image data from the host. These functions are non-blocking
 * and oblivious to the stream command queue, so it's up to the user to make
 * sure that all pending operations using this image as input or output are
 * finished. Also, depending on which device the memory is allocated,
 * lock/unlock operation might be time-consuming and, for example, involve
 * copying data over PCIe bus for dGPUs.
 *
 * VPI allows image interoperability with the following memory types:
 * - CUDA memory: \ref vpiImageCreateCUDAMemWrapper
 * - Host/CPU memory: \ref vpiImageCreateHostMemWrapper
 * - EGLImage: \ref vpiImageCreateEGLImageWrapper
 * - NvBuffer: \ref vpiImageCreateNvBufferWrapper
 *
 * @defgroup VPI_Image Image
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @anchor image_flags @name Image-specific flags. */
/**@{*/

/** 
 * Flag to allow \ref vpiImageLock of block-linear images to succeed.
 * It'll allocate an extra buffer to store the corresponding pitch-linear
 * image.
 */
#define VPI_DISABLE_BL_HOST_LOCK (1u << 31)

/**@}*/

/** Stores information about an image plane.
 *  Image planes represent 2D data where consecutive rows are laid out in pitch-linear format.
 *  This structure has all information needed to address any pixel in the plane. 
 */
typedef struct VPIImagePlaneRec
{
    VPIPixelType pixelType; /**< Type of each pixel within this plane. */
    int32_t width;          /**< Width of this plane in pixels. */
    int32_t height;         /**< Height of this plane in pixels. */
    int32_t pitchBytes;     /**< Difference in bytes of beginning of one row and the beginning of the previous.
                                     This is used to address every row (and ultimately every pixel) in the plane.
                                     @code
                                        T *pix_addr = (T *)((uint8_t *)data + pitchBytes*height)+width;
                                     @endcode
                                     where T is the C type related to pixelType.
                                */

    void *data; /**< Pointer to the first row of this plane.
                     This points to the actual data represented by this plane.
                     Depending on how the plane is used, the pointer might be
                     addressing a GPU memory or host memory. Care is needed to
                     know when it is allowed to dereference this memory. */
} VPIImagePlane;

/** Maximum number of data planes an image can have. */
#define VPI_MAX_PLANE_COUNT (6)

/** Stores information about image characteristics and content. */
typedef struct VPIImageDataRec
{
    VPIImageFormat format; /**< Image format. */
    int32_t numPlanes;     /**< Number of planes. */

    /** Data of all image planes.
     * Only the first \ref numPlanes elements have valid data.
     */
    VPIImagePlane planes[VPI_MAX_PLANE_COUNT];
} VPIImageData;

/**
 * Create an empty image instance with the specified flags. Image data is zeroed.
 *
 * @param[in] width,height Image dimensions.
 * @param[in] fmt Image format.
 * @param[in] flags Image flags. Here it can be specified in what backends the
 *                  image can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageCreate(int32_t width, int32_t height, VPIImageFormat fmt, uint32_t flags, VPIImage *img);

/**
 * Create an image object by wrapping around an existing host memory block.
 *
 * Only pitch-linear memory layout is supported. The underlying image object does not own/claim the memory
 * block.
 *
 * @ingroup VPI_CPUInterop
 *
 * @param[in] hostData Pointer to structure with host memory to be wrapped.
 * @param[in] flags Image flags. Here it can be specified in what backends the
 *                  mage can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] img Pointer to memory that will receive the created image handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageCreateHostMemWrapper(const VPIImageData *hostData, uint32_t flags, VPIImage *img);

/**
 * Redefines the wrapped host memory in an existing \ref VPIImage wrapper.
 *
 * The old wrapped memory and the new one must have same dimensions, format and must point
 * to host-side (cpu-accessible) memory.
 *
 * The VPIImage must have been created by \ref vpiImageCreateHostMemWrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped memory must not be deallocated while it's still being wrapped.
 *
 * @ingroup VPI_CPUInterop
 *
 * @param[in] img Handle to image created by \ref vpiImageCreateHostMemWrapper.
 * @param[in] hostData \ref VPIImageData pointing to the new host memory block to be wrapped.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageSetWrappedHostMem(VPIImage img, const VPIImageData *hostData);

/**
 * Destroy an image instance.
 *
 * This function deallocates all resources allocated by the image creation function.
 * When destroying an VPIImager wrapper, the wrapped memory itself isn't deallocated.
 *
 * @param[in] img Image handle to be destroyed. Passing NULL is allowed, to which the function simply does nothing.
 */
VPI_PUBLIC void vpiImageDestroy(VPIImage img);

/**
 * Informs that the image's wrapped memory was updated outside VPI.
 *
 * This method is useful for unmanaged images only (created with
 * `vpiImageWrap*`). If the underlying image data has been modified outside the
 * API, use this method to mark the image as invalid. This will force the API
 * to update its internal representation (e.g., re-upload to CUDA memory) when
 * necessary.
 *
 * @param[in] img An image handle created by one of the wrapper image creation functions.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageInvalidate(VPIImage img);

/**
 * Get the image size in pixels.
 *
 * @param[in] img an image handle
 * @param[out] width, height Pointers to memories where the image width and height (respectively) will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageGetSize(VPIImage img, int32_t *width, int32_t *height);

/**
 * Get the image format.
 *
 * @param[in] img An image handle
 * @param[out] format A pointer to where the mage format will be written to
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageGetFormat(VPIImage img, VPIImageFormat *format);

/**
 * Get the image flags.
 *
 * @param[in] img Nn image handle
 * @param[out] flags A pointer to where the flags will be written to.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageGetFlags(VPIImage img, uint32_t *flags);

/**
 * Acquires the lock on an image object and returns a pointer to the image planes.
 *
 * Depending on the internal image representation, as well as the actual location in memory, this
 * function might have a significant performance overhead (format conversion, layout conversion,
 * device-to-host memory copy).
 *
 * The image can be locked multiple times. Each lock operation increments a
 * counter and must be matched by a corresponding \ref vpiImageUnlock
 * call. Lock will fail if the image is being used by an algorithm.
 *
 * @param[in] img An image handle
 * @param[in] mode Lock mode, depending on whether the memory will be written to and/or read from.
 * @param[out] hostData A pointer to a structure that will be filled with image memory information.
 *                      If it's NULL, the image will still be locked. This is useful to make sure
 *                      wrapped image is updated.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageLock(VPIImage img, VPILockMode mode, VPIImageData *hostData);

/**
 * Releases the lock on an image object.
 *
 * This function might have a significant performance overhead (format conversion, layout
 * conversion, host-to-device memory copy).
 *
 * The mage is effectively unlocked when the internal lock counter reaches 0.
 *
 * @param[in] img An image handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiImageUnlock(VPIImage img);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Image */

#endif /* NV_VPI_IMAGE_H */
