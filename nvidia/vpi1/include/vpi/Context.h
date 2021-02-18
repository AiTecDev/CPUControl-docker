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
 * @file Context.h
 *
 * Functions and structures for dealing with VPI contexts.
 */

#ifndef NV_VPI_CONTEXT_H
#define NV_VPI_CONTEXT_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * Context is the implicit owner of objects created when it's active and their properties.
 *
 * This is a top-level class that manages the lifetime and properties of all
 * other API objects.
 * Specifically, it has the following properties:
 * - Thread-specific - every thread within an application has one current \ref VPIContext instance set.
 *   If no instance has been explicitly set for a particular thread, all API calls from this thread
 *   will use a process-specific default context instance instead.
 * - Configurable - the user can specify during construction which kinds of backends are enabled
 *   for this context. This effectively allows to mask support for a particular hardware. For
 *   example, creating a \ref VPIStream instance for a CUDA backend will fail if the current context
 *   doesn't have the \ref VPI_BACKEND_CUDA flag set. These flags are also automatically updated to reflect
 *   the actual platform support for a particular backend. For example, if the PVA backend is not
 *   available, \ref vpiContextGetFlags will return the flag without \ref VPI_BACKEND_PVA set. The CPU backend
 *   cannot be masked and is always supported as a fallback implementation though.
 * - Allows for association with a specific CUDA driver API `CUcontext` instance - this should permit
 *   multi-GPU processing.
 * - Sharing buffers between different contexts is not permitted.
 *
 * @defgroup VPI_Context Context
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @private
 * Forward declaration of CUDA context handle
 */
typedef struct CUctx_st *CUcontext;

/** @name Special contexts */
/**@{*/
#define VPI_GLOBAL_CONTEXT ((VPIContext)0x610BA1C1D) /**< Global context identifier */
/**@}*/

/**
 * Represents a context was destroyed or is invalid for some other reason.
 */
#define VPI_INVALID_CONTEXT ((VPIContext)-1)

/**
 * Create a context instance.
 *
 * @param[in] flags Context flags. User can pass a set of \ref VPIBackend flags or-ed together. This will
 *                  restrict the enabled backends of objects created while this context is active.
 * @param[out] ctx Pointer to memory that will receive the new context handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiContextCreate(uint32_t flags, VPIContext *ctx);

/**
 * Create a context instance that wraps a CUDA context.
 * CUDA operations will all under the wrapped context whenever the created
 * context is active.
 * \note Currently VPI doesn't do anything with the CUDA context, so the
 *       call performs exactly like vpiContextCreate().
 *
 * @param[in] flags Context flags. It's assumed that \ref VPI_BACKEND_CUDA is always enabled.
 * @param[in] cudaCtx CUDA context handle to be wrapped.
 * @param[out] ctx Pointer to memory that will receive the new context handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextCreateCudaContextWrapper(uint32_t flags, CUcontext cudaCtx, VPIContext *ctx);

/**
 * Destroy a context instance as well as all resources it owns.
 *
 * The context ctx will be destroyed regardless of how many threads it is active in. It is the
 * responsibility of the calling function to ensure that no API call issues using ctx while
 * \ref vpiContextDestroy is executing.
 *
 * If ctx is current to the calling thread, then ctx will also be popped from the current thread's
 * context stack (as though \ref vpiContextPop was called). If ctx is current to other threads, then
 * ctx will remain current to those threads, and attempting to access ctx from those threads will
 * result in the error \ref VPI_ERROR_INVALID_CONTEXT.
 *
 * @param[in] ctx Context handle. Passing NULL is accepted. In this case no operation is performed.
 */
VPI_PUBLIC void vpiContextDestroy(VPIContext ctx);

/**
 * Controls low-level task parallelism of CPU devices owned by the context. This function allows
 * the user to overload the parallel_for implementation used by CPU devices. Changing this parallel_for
 * implementation on a context that is performing CPU processing is undefined and might lead to
 * application crashes.
 *
 * @param[in] ctx Context handle.
 * @param[in] config A pointer to parallel_for configuration. Passing NULL will make the context to
 * fallback to its default internal implementation.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextSetParallelFor(VPIContext ctx, const VPIParallelForConfig *config);

/**
 * Returns parameters set by \ref vpiContextSetParallelFor.
 *
 * @param[in] ctx Context handle
 * @param[out] config A pointer to parallel_for configuration.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextGetParallelFor(VPIContext ctx, VPIParallelForConfig *config);

/**
 * Gets the context for the calling thread.
 *
 * If there is no error and current context is NULL, the thread is using the global context.
 *
 * @param[out] ctx pointer to a context handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextGetCurrent(VPIContext *ctx);

/**
 * Sets the context for the calling thread.
 * If there are items on the context stack this will replace the top of the stack.
 * If ctx is NULL this will clear the context stack and make the thread use the global context.
 *
 * @param[in] ctx a context handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextSetCurrent(VPIContext ctx);

/**
 * Pushes the context to a per-thread context stack and sets this context as the current context
 * for the calling thread.
 *
 * @param[in] ctx Context handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextPush(VPIContext ctx);

/**
 * Pops a context from a per-thread context stack and saves it to the ctx variable. The top of the
 * context stack is set as the current context for the calling thread.
 *
 * @param[out] ctx pointer to a context handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextPop(VPIContext *ctx);

/**
 * Get the current context flags. This function can be used to verify underlying backend support.
 *
 * @param[in] ctx a context handle
 * @param[out] flags a pointer to a variable which will be set to the current context flags
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiContextGetFlags(VPIContext ctx, uint32_t *flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Context */

#endif /* NV_VPI_CONTEXT_H */
