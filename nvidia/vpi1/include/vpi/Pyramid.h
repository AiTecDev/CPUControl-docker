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
 * @file Pyramid.h
 *
 * Functions and structures for dealing with VPI pyramids.
 */

#ifndef NV_VPI_PYRAMID_H
#define NV_VPI_PYRAMID_H

#include "Export.h"
#include "Image.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * An abstract representation of a 2D image pyramid.
 *
 * 2D image pyramid containers are created by calling \ref vpiPyramidCreate to
 * allocate and initialize an empty (zeroed) \ref VPIPyramid object. The memory
 * for the image pyramid data is allocated and managed by VPI.
 *
 * Image formats match the ones supported by image container. The pyramid is
 * not necessarily dyadic. The scale between levels is defined in the
 * constructor.
 *
 * Parameters such as levels, scale, width, height and image format are
 * immutable and specified at the construction time. The internal memory layout
 * is also backend-specific. More importantly, efficient exchange of image
 * pyramid data between different hardware blocks might force the
 * implementation to allocate the memory in multiple memory pools (e.g. dGPU
 * and system DRAM). In some scenarios (to optimize performance and memory
 * use), it might be beneficial to constrain the internal allocation policy to
 * support only a particular set of backends. 
 *
 * The set of \ref vpiPyramidLock / \ref vpiPyramidUnlock calls allows the user
 * to read from/write to the image data from the host. These functions are
 * non-blocking and oblivious to the device command queue, so it's up to the
 * user to make sure that all pending operations using this image pyramid as
 * input or output are finished. Also, depending on the enabled backends
 * lock/unlock * operation might be time-consuming and, for example, involve
 * copying data over PCIe bus for dGPUs.
 *
 * @defgroup VPI_Pyramid Pyramid
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of pyramid levels */
#define VPI_MAX_PYRAMID_LEVEL_COUNT (10)

/** Stores the pyramid contents.
 * Every level is represented by an entire \ref VPIImageData. There are `numLevels` levels, and
 * they can be accessed from `levels[0]` to `levels[numLevels-1]`.
 */
typedef struct VPIPyramidDataRec
{
    int32_t numLevels; /**< Number of levels (i.e. height) of the pyramid. */

    /** Contents of every pyramid level.
     *  Only the first numLevels levels has valid data. */
    VPIImageData levels[VPI_MAX_PYRAMID_LEVEL_COUNT];
} VPIPyramidData;

/**
 * Create an empty image pyramid instance with the specified flags. Pyramid data is zeroed.
 *
 * @param[in] width, height Dimensions of the finest pyramid level.
 * @param[in] numLevels Number of levels.
 * @param[in] scale Downscale factor from one level and the next.
 * @param[in] fmt Image format of each level.
 * @param[in] flags Pyramid flags. Here it can be specified in what backends the
 *                  pyramid can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] pyr Pointer to memory that will receive the created pyramid handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiPyramidCreate(int32_t width, int32_t height, VPIImageFormat fmt, int32_t numLevels, float scale,
                                      uint32_t flags, VPIPyramid *pyr);

/**
 * Destroy an image pyramid instance as well as all resources it owns.
 *
 * @param[in] pyr pyramid handle
 */
VPI_PUBLIC void vpiPyramidDestroy(VPIPyramid pyr);

/**
 * Returns the flags associated with the pyramid.
 *
 * @param[in] pyr pyramid handle
 * @param[out] flags pointer to where the flags will be written.
 *
 * @returns \ref VPI_SUCCESS if function was able to return the flags, or any other \ref VPIStatus code in case of error.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetFlags(VPIPyramid pyr, uint32_t *flags);

/**
 * Returns the image format of the pyramid levels.
 *
 * @param[in] pyr pyramid handle
 * @param[out] fmt pointer to where the image format will be written.
 *
 * @returns \ref VPI_SUCCESS if function was able to return the flags, or any other \ref VPIStatus code in case of error.
 */
VPI_PUBLIC VPIStatus vpiPyramidGetFormat(VPIPyramid pyr, VPIImageFormat *fmt);

/**
 * Get the image pyramid level count.
 *
 * @param[in] pyr an image pyramid handle
 * @param[out] numLevels a pointer to a variable which will be set to the number of levels of the image
 *                        pyramid
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiPyramidGetNumLevels(VPIPyramid pyr, int32_t *numLevels);

/**
 * Get the image width and height in pixels (*for all levels at once*).
 *
 * @param[in] pyr an image pyramid handle
 * @param[in] outSize size of the output arrays, in elements.
 * @param[out] outWidth, outHeight Pointers to a C array which will be filled widths and heights (respectively) of all image pyramid levels. If any of them is NULL, the corresponding data isn't returned.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiPyramidGetSize(VPIPyramid pyr, int32_t outSize, int32_t *outWidth, int32_t *outHeight);

/**
 * Acquires the lock on a pyramid object and returns pointers to each level of the pyramid.
 * Depending on the internal image representation, as well as the actual location in memory, this
 * function might have a significant performance overhead (format conversion, layout conversion,
 * device-to-host memory copy).
 *
 * @param[in] pyr an image pyramid handle
 * @param[in] lock lock mode
 * @param[out] out a pointer to a structure that will receive the pyramid data to be accessed by host.
 *             If it's NULL, the pyramid will still be locked. This is useful to make sure
 *             wrapped image is updated.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiPyramidLock(VPIPyramid pyr, VPILockMode lock, VPIPyramidData *out);

/**
 * Releases the lock on a image pyramid object.
 * This function might have a significant performance overhead (format conversion, layout
 * conversion, host-to-device memory copy).
 *
 * @param[in] pyr an image pyramid handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiPyramidUnlock(VPIPyramid pyr);

/**
 * Creates an image that wraps one pyramid level.
 * The created image doesn't own its contents. Destroying the pyramid while there
 * are images wrapping its levels leads to undefined behavior. If image wraps the
 * base pyramid level, locking the pyramid will also lock the image. Once the image
 * isn't needed anymore, call \ref vpiImageDestroy to free resources.
 *
 * @param[in] pyr The pyramid whose level will be wrapped.
 * @param[in] level Pyramid level to wrap.
 * @param[out] img Pointer to an image handle that will hold the created image.
 * 
 * @returns \ref VPI_SUCCESS if image was created successfully, or any other \ref VPIStatus error code in case of error.
 */
VPI_PUBLIC VPIStatus vpiImageCreatePyramidLevelWrapper(VPIPyramid pyr, int32_t level, VPIImage *img);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Pyramid */

#endif /* NV_VPI_PYRAMID_H */
