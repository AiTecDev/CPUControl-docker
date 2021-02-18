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
 * @file EGLInterop.h
 *
 * Functions dealing with EGL interop.
 */

#ifndef NV_VPI_EGLINTEROP_H
#define NV_VPI_EGLINTEROP_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdint.h>

/**
 * EGL is our main vehicle for interoperability with multimedia, camera and
 * graphics APIs.
 *
 * We provide function to export \ref VPIEvent objects to be consumed
 * by other APIs, as well as function to import images and sync objects from them.
 *
 * @defgroup VPI_EGLInterop EGL Interoperability
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Parameters for customizing EGLImage wrapping.
 * These parameters are used to customize how EGLImage wrapping will be made.
 * Make sure to call \ref vpiInitWrapEGLImageParams to initialize this
 * structure before updating its attributes. This guarantees that new attributes
 * added in future versions will have a suitable default value assigned.
 */
typedef struct
{
    /** Overriden \ref VPIColorSpec of the wrapped EGLImage.
     * It it's a valid color spec, override the color spec information in the wrapped EGLImage.
     * If \ref VPI_COLOR_SPEC_DEFAULT, infer the color spec from the wrapped EGLImage. */
    VPIColorSpec colorSpec;
} VPIWrapEGLImageParams;

/** Initialize \ref VPIWrapEGLImageParams with default values.
 *
 * Default values are:
 * - colorSpec: \ref VPI_COLOR_SPEC_DEFAULT
 *
 * @param[in] params Structure to be initialized with default values.
 *
 * @returns \ref VPI_SUCCESS if function succeeded, or \ref VPI_ERROR_INVALID_ARGUMENT is params is NULL.
 */
VPI_PUBLIC VPIStatus vpiInitWrapEGLImageParams(VPIWrapEGLImageParams *params);

/**
 * Create an image object by wrapping an existing `EGLImage`.
 *
 * The object doesn't own the wrapped memory. The user is still responsible for wrapped memory lifetime, which
 * must be valid until the array object is destroyed.
 *
 * The returned handle must be destroyed when not being used anymore by calling \ref vpiImageDestroy.
 *
 * @ingroup VPI_EGLInterop
 *
 * @param[in] eglImage EGL image created from another client API
 * @param[in] params If not NULL, use the parameters to modify how the EGLImage is wrapped.
 *                   Otherwise, it'll use default EGLImage handling.
 * @param[in] flags Image flags. Here it can be specified in what backends the
 *                  image can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] img Pointer to memory where the created image handle will be written to.
 *
 * @returns
 *     - \ref VPI_SUCCESS on success
 *     - \ref VPI_ERROR_INVALID_IMAGE_FORMAT if the image format is not supported by the backend (check the log)
 *     - \ref VPI_ERROR_INVALID_OPERATION if the backend doesn't support EGL images
 */
VPI_PUBLIC VPIStatus vpiImageCreateEGLImageWrapper(EGLImageKHR eglImage, const VPIWrapEGLImageParams *params,
                                                   uint32_t flags, VPIImage *img);

/**
 * Redefines the wrapped `EGLImage` of an existing \ref VPIArray wrapper.
 *
 * The new `EGLImage` must have the same characteristics (type and dimensions) of the existing
 * `EGLImage` being wrapped.
 *
 * The VPIImage must have been created by \ref vpiImageCreateEGLImageWrapper.
 *
 * This operation is efficient and does not allocate memory. The wrapped memory will be
 * accessible to the same backends specified during wrapper creation.
 *
 * The wrapped `EGLImage` must be valid while it's still being wrapped.
 *
 * @ingroup VPI_EGLInterop
 *
 * @param[in] img Handle to an image created by \ref vpiImageCreateEGLImageWrapper .
 * @param[in] eglImage `EGLImage` object to be wrapped.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiImageSetWrappedEGLImage(VPIImage img, EGLImageKHR eglImage);

/**
 * Create an event object by wrapping around an EGL sync object created from
 * another client API.
 * The created event can be used by \ref vpiEventSync / \ref vpiStreamWaitEvent to synchronize
 * VPI tasks with a command queue from an external API.
 *
 * @ingroup VPI_EGLInterop
 *
 * @warning This function is currently not implemented.
 *
 * @param[in] eglSync EGL sync handle
 * @param[out] event pointer to an event handle
 *
 * @returns Always returns \ref VPI_ERROR_NOT_IMPLEMENTED.
 */
VPI_PUBLIC VPIStatus vpiEventCreateEGLSyncWrapper(EGLSyncKHR eglSync, VPIEvent *event);

/**
 * Export a recorded VPI event to EGL sync object. The resulting object can be
 * used to allow a command queue belonging to an external API to wait on a recorded VPI event.
 *
 * The exported `EGLSync` must be destroyed using `eglDestroySyncKHR` before the VPI
 * event object is destroyed.
 *
 * @ingroup VPI_EGLInterop
 *
 * @warning This function is currently not implemented.

 * @param[in] event  Event to be exported.
 * @param[in] eglDpy EGL display for which the sync object is being created.
 * @param[out] eglSync Exported EGL sync handle
 *
 * @returns Always returns \ref VPI_ERROR_NOT_IMPLEMENTED.
 *
 */
VPI_PUBLIC VPIStatus vpiEventExportEGLSync(VPIEvent event, EGLDisplay eglDpy, EGLSyncKHR *eglSync);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_EGLInterop */

#endif /* NV_VPI_EGLINTEROP_H */
