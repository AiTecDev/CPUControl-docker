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
 * @file Event.h
 *
 * Functions and structures for dealing with VPI events.
 */

#ifndef NV_VPI_EVENT_H
#define NV_VPI_EVENT_H

#include "Export.h"
#include "Status.h"
#include "Types.h"

#include <stdint.h>

/**
 * A representation of events used in stream synchronization and timing.
 *
 * Each compute function in the API is executed asynchronously with respect to the calling thread,
 * i.e., returns immediately without waiting for the completion. There are two ways of
 * synchronizing with the backend. One is to wait until all the commands in the \ref VPIStream queue
 * are finished, with the \ref vpiStreamSync call. This approach, while simple, doesn't allow for
 * fine-grained ("wait for until function X is completed") or inter-device ("before running
 * function A in device B, wait until function C in device D finishes") synchronization. That's
 * where \ref VPIEvent objects come in. Conceptually, they correspond to binary semaphores and are
 * designed to closely mimic events in CUDA API:
 *
 * - Users can capture all commands submitted to a \ref VPIStream instance in an event instance (see
 *   \ref vpiEventRecord). The event is considered completed when all captured commands have been
 *   processed and removed from \ref VPIStream command queue.
 * - Inter-device synchronization is possible with \ref vpiStreamWaitEvent call that pushes a command
 *   to \ref VPIStream queue that blocks processing of future queued commands until given event is
 *   completed.
 * - Host threads can query the event's state with \ref vpiEventQuery
 * - Host threads can block until event is completed with \ref vpiEventSync.
 * - Events can be time-stamped when completed.
 * - Users can compute time-stamp difference between completed events in the same device as well as
 *   between different devices. This implies that internally, the API has to have a notion of
 *   unified time source.
 *
 * @defgroup VPI_Event Event
 * @ingroup VPI_API_Core
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @anchor event_flags @name Event-specific flags. */
/**@{*/
#define VPI_EVENT_DISABLE_TIMESTAMP (1 << 18) /**< disable time-stamping of event signaling for better performance. */
/**@}*/

/**
 * Create an event instance with the specified flags.
 *
 * @param[in] flags Event flags. Here it can be specified in what backends the
 *                  image can be used by or-ing together \ref VPIBackend flags.
 *                  Set flags to 0 to enable it in all backends supported by
 *                  the active VPI context. \ref event_flags "Additional flags" can
 *                  also be specified.
 * @param[out] event Pointer to memory that will receive the created event handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiEventCreate(uint32_t flags, VPIEvent *event);

/**
 * Destroy an event instance as well as all resources it owns.
 *
 * @param[in] event event handle
 */
VPI_PUBLIC void vpiEventDestroy(VPIEvent event);

/**
 * Captures in the event the contents of the stream command queue at the time of this call.
 *
 * When all tasks recorded are finished, the event will be signaled and calls that are waiting
 * for it will be unblocked.
 *
 * This function can be called multiple times on the same event, however existing
 * \ref vpiEventSync / \ref vpiStreamWaitEvent calls are not affected by later calls to \ref vpiEventRecord.
 * This means they will wait for the event completion of the command queue tasks described by the
 * event at the time of the initial \ref vpiEventSync / \ref vpiStreamWaitEvent call.
 *
 * @param[in] event An event handle
 * @param[in] stream A stream handle whose command queue with tasks yet to be
 *            executed will be recorded in the event.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiEventRecord(VPIEvent event, VPIStream stream);

/**
 * Blocks the calling thread until the event is signaled. The event is considered signaled when
 * all the tasks captured by \ref vpiEventRecord are completed or when no tasks were captured.
 *
 * @param[in] event An event handle.
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiEventSync(VPIEvent event);

/**
 * Queries the status of all work currently captured by the event. If all tasks are finished,
 * \ref VPI_SUCCESS is returned, otherwise \ref VPI_ERROR_NOT_READY.
 *
 * @param[in] event An event handle.
 * @param[out] state Pointer to memory that will receive the states of the event. State is only valid
 *                   when the return status is \ref VPI_SUCCESS.
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiEventQuery(VPIEvent event, VPIEventState *state);

/**
 * Computes the elapsed time in (msec) between two completed events. If one or both are not
 * completed at the time of the call, \ref VPI_ERROR_NOT_READY is returned, otherwise \ref VPI_SUCCESS.
 *
 * @param[in] start an event handle marking the start of the time interval
 * @param[in] end an event handle marking the end of the time interval
 * @param[out] msec a pointer to a variable which will be set to the time difference between the
 *                  events
 *
 * @returns an error code on failure, \ref VPI_ERROR_NOT_READY if at least one event is not ready,
 *          else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiEventElapsedTimeMillis(VPIEvent start, VPIEvent end, float *msec);

/**
 * Returns the event flags passed during event creation.
 *
 * @param[in] event An event handle.
 * @param[out] flags Pointer to memory that will hold the event flags. It can't be NULL.
 *
 * @returns An error code on failure, or else \ref VPI_SUCCESS.
 */
VPI_PUBLIC VPIStatus vpiEventGetFlags(VPIEvent event, uint32_t *flags);

#ifdef __cplusplus
}
#endif

/** @} end of VPI_Event */

#endif /* NV_VPI_EVENT_H */
