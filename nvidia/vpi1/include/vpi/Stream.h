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
 * @file Stream.h
 *
 * Declares functions dealing with VPI streams.
 */

#ifndef NV_VPI_STREAM_H
#define NV_VPI_STREAM_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

/**
 * The main entry-point to the API is the \ref VPIStream object.
 *
 * This object represents a command queue (FIFO) storing a list of
 * commands to execute. Commands might comprise running a particular CV
 * algorithm or signaling an event in a particular backend. This allows the API
 * functions to be executed asynchronously with respect to the calling thread.
 * Invoking any CV function simply pushes a corresponding command to the
 * command queue in the \ref VPIStream instance and immediately returns. The
 * queued commands then get consumed and dispatched to the associated backend
 * device.
 *
 * @defgroup VPI_Stream Stream
 * @ingroup VPI_API_Core
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a stream instance.
 *
 * @param[in] flags Stream flags. Here it can be specified in what backends are
 *                  enabled in it by or-ing \ref VPIBackend flags together.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context.
 * @param[out] stream pointer to stream handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiStreamCreate(uint32_t flags, VPIStream *stream);

/**
 * Destroy a stream instance and deallocate all HW resources.
 *
 * This operation will implicitly synchronize the stream to guarantee that all operations
 * submitted to it are completed.
 *
 * @param[in] stream Stream handle. If can be NULL, in this case the function won't do anything.
 */
VPI_PUBLIC void vpiStreamDestroy(VPIStream stream);

/**
 * Blocks the calling thread until all submitted commands in this stream queue
 * are done (queue is empty). This function call is equivalent to atomically
 * calling \ref vpiEventRecord followed by \ref vpiStreamWaitEvent.
 *
 * @param[in] stream stream handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiStreamSync(VPIStream stream);

/**
 * Pushes a command that blocks the processing of all future commands submitted to the stream until the
 * event is signaled.
 *
 * @param[in] stream Stream handle
 * @param[in] event event handle
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiStreamWaitEvent(VPIStream stream, VPIEvent event);

/**
 * Returns OS-specific handle of the background stream processing thread.
 * @param[in] stream stream handle
 * @param[out] handle stream processing thread handle
 * 
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiStreamGetThreadHandle(VPIStream stream, VPINativeThreadHandle *handle);

/** Gets the VPIStream flags passed during its creation.
 *
 * @param[in] stream Stream handle.
 * @param[out] flags Pointer to variable where the flags will be stored into.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiStreamGetFlags(VPIStream stream, uint32_t *flags);

/**
 * Returns the backend associated with a payload.
 *
 * @ingroup VPI_Payload
 *
 * @param[in] payload Payload to be queried
 * @param[out] backend The backend associated with a payload
 * 
 * @returns an error code on failure else \ref VPI_SUCCESS
 *
 */
VPI_PUBLIC VPIStatus vpiPayloadGetBackend(VPIPayload payload, VPIBackend *backend);

/**
 * Deallocates the payload object and all associated resources.
 *
 * @ingroup VPI_Payload
 *
 * @param[in] payload payload handle
 *
 */
VPI_PUBLIC void vpiPayloadDestroy(VPIPayload payload);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Stream */

#endif /* NV_VPI_STREAM_H */
