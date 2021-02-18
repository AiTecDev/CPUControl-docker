/*
 * Copyright 2019 NVIDIA Corporation. All rights reserved.
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
 * @file Types.h
 *
 * Defines all types needed for programming with VPI.
 */

#ifndef NV_VPI_TYPES_H
#define NV_VPI_TYPES_H

#include "ImageFormat.h"
#include "Interpolation.h"
#include "PixelType.h"
#include "Status.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines common types used by several components.
 *
 * @defgroup VPI_Types Common Types
 * @ingroup VPI_API_Misc
 */

/** @name Memory creation flags.
 * These flags can be used when creating images, pyramids or arrays.
 *
 * @ingroup VPI_Types
 * @{
 */

/** VPI Backend types.
 *
 * @ingroup VPI_Stream
 *
 */
typedef enum
{
    VPI_BACKEND_CPU     = (1u << 0), /**< CPU backend. */
    VPI_BACKEND_CUDA    = (1u << 1), /**< CUDA backend. */
    VPI_BACKEND_PVA     = (1u << 2), /**< PVA backend. */
    VPI_BACKEND_VIC     = (1u << 3), /**< VIC backend. */
    VPI_BACKEND_INVALID = 0x7FFFFFFF /**< Invalid backend. */
} VPIBackend;

#define VPI_BACKEND_ALL (VPI_BACKEND_CPU | VPI_BACKEND_CUDA | VPI_BACKEND_PVA | VPI_BACKEND_VIC) /**< All backends. */

#define VPI_FLAG_ALL ((1ULL << 32) - 1)

/** @} */

/**
 * Parallel task function pointer type.
 * @ingroup VPI_Context
 */
typedef void (*VPIParallelTask)(int taskId, int threadId, void *vpiData);

/**
 * Parallel for callback function pointer type. A serial (reference) implementation of this
 * function might looks as follows:
 *
 * @code
 * void parallel_for(VPIParallelTask task, int taskCount, void *vpiData, void *userData)
 * {
 *    for (int i = 0; i < taskCount; ++i)
 *    {
 *      task(i, 0, vpiData); 
 *    }
 * }
 * @endcode
 *
 * Parallel implementation should have equivalent behavior; that is, run all tasks
 * between 0 and taskCount-1 and block the calling thread until all tasks have been
 * completed. `threadId` parameter should be between 0 and maxThreads-1 (\see vpiContextSetParallelFor).
 * Implementations that have `maxThreads` set to zero can pass an arbitrary `threadId` value to
 * task functions.
 *
 * @ingroup VPI_Context
 */
typedef void (*VPIParallelForCallback)(VPIParallelTask task, int taskCount, void *vpiData, void *userData);

/** 
 * Stores the ParallelFor configuration.
 *
 * @ingroup VPI_Context
 */
typedef struct
{
    /**
     * The maximum number of threads used by the parallel_for implementation code. Has to be larger than 0.
     * Setting the number to N means that the parallel for implementation will call task functors with task id
     * between 0 and N-1. Calling task functors with thread id outside of this range is not legal and will result
     * in undefined behavior.
     */
    int maxThreads;

    /**
     * A pointer to the parallel_for implementation. If null is passed, the context will fallback to the default
     * internal parallel_for implementation. parallel_for implementation is required to be thread-safe.
     * Internal API implementation might call this function from different threads at the same time.
     */
    VPIParallelForCallback callback;

    /**
     * A user defined opaque pointer passed to callback function unaltered.
     */
    void *userData;
} VPIParallelForConfig;

/**
 * A handle to OS-specific thread handle.
 *
 * @ingroup VPI_Context
 */
typedef void *VPINativeThreadHandle;

/**
 * A handle to an array.
 * @ingroup VPI_Array
 */
typedef struct VPIArrayImpl *VPIArray;

/**
 * A handle to a context.
 * @ingroup VPI_Context
 */
typedef struct VPIContextImpl *VPIContext;

/**
 * A handle to an event.
 * @ingroup VPI_Event
 */
typedef struct VPIEventImpl *VPIEvent;

/**
 * A handle to a stream.
 * @ingroup VPI_Stream
 */
typedef struct VPIStreamImpl *VPIStream;

/**
 * A handle to an image.
 * @ingroup VPI_Image
 */
typedef struct VPIImageImpl *VPIImage;

/**
 * A handle to an image pyramid.
 * @ingroup VPI_Pyramid
 */
typedef struct VPIPyramidImpl *VPIPyramid;

/**
 * A handle to an algorithm payload.
 * @ingroup VPI_Payload
 */
typedef struct VPIPayloadImpl *VPIPayload;

/**
 * Image border extension specify how pixel values outside of the image domain should be
 * constructed.
 *
 * @ingroup VPI_Types
 */
typedef enum
{
    VPI_BORDER_ZERO,    /**< All pixels outside the image are considered to be zero. */
    VPI_BORDER_CLAMP,   /**< Border pixels are repeated indefinitely. */
    VPI_BORDER_REFLECT, /**<  edcba|abcde|edcba */
    VPI_BORDER_MIRROR,  /**< dedcb|abcde|dcbab */
} VPIBorderExtension;

/**
 * Policy used when converting between image types.
 * @ingroup VPI_ConvertImageFormat
 */
typedef enum
{
    /** Clamps input to output's type range. Overflows
        and underflows are mapped to the output type's
        maximum and minimum representable value,
        respectively. When output type is floating point,
        clamp behaves like cast. */
    VPI_CONVERSION_CLAMP = 0,

    /** Casts input to the output type. Overflows and
        underflows are handled as per C specification,
        including situations of undefined behavior. */
    VPI_CONVERSION_CAST,

    /** Invalid conversion. */
    VPI_CONVERSION_INVALID = 255,
} VPIConversionPolicy;

/**
 * Stores a keypoint coordinate.
 * The coordinate is relative to the top-left corner of an image.
 *
 * @ingroup VPI_Types
 */

typedef struct
{
    float x, /**< Keypoint's x coordinate. */
        y;   /**< Keypoint's y coordinate. */
} VPIKeypoint;

/**
 * Stores a generic 2D homography transform.
 * When only scale and translation transformation is needed,
 * these parameters must be arranged in the matrix as follows:
 *
 * \f[ \begin{bmatrix}
 *     s_x &   0 & p_x \\
 *       0 & s_y & p_y \\
 *       0 &   0 &   1
 *   \end{bmatrix}
 * \f]
 *
 * Scaling \f$(s_x,s_y)\f$ is relative to the center of the patch,
 * position \f$(p_x,p_y)\f$ is relative to the top-left of the image.
 *
 * In the general case, given an homogeneous 2D point \f$P(x,y,1)\f$
 * and the matrix \f$M^{3x3}\f$, the Euclidean 2D point \f$O(x,y)\f$ is defined as
 * \f{align}{
 *  T &= M \cdot P \\
 *  O &= (T_x/T_z, T_y/T_z)
 * \f}
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    float mat3[3][3]; /**< 3x3 homogeneous matrix that defines the homography. */
} VPIHomographyTransform2D;

/**
 * Stores a generic 2D bounding box.
 * Although this structure can store a 2D bounding box transformed
 * by any homography, most of the time it stores an axis-aligned bounding box.
 * To retrieve it, do the following:
 *
 * @code
 *   float x = xform.mat3[0][2];
 *   float y = xform.mat3[1][2];
 *   float w = width  * xform.mat3[0][0];
 *   float h = height * xform.mat3[1][1];
 * @endcode
 *
 * @ingroup VPI_Types
 */
typedef struct
{
    VPIHomographyTransform2D xform; /**< Defines the bounding box top left corner and its homography. */
    float width,                    /**< Bounding box width. */
        height;                     /**< Bounding box height. */
} VPIBoundingBox;

/**
 * Stores a bounding box that is being tracked by \ref algo_klt_tracker "KLT Tracker".
 * @ingroup VPI_KLTFeatureTracker
 */
typedef struct
{
    /** Bounding box being tracked. */
    VPIBoundingBox bbox;

    /** Tracking status of this bounding box.
     * Accepted values:
     * - `1` tracking information is invalid and shouldn't be relied upon.
     * - `0` tracking information is valid.
     */
    int8_t trackingStatus;

    /** Status of the template related to this bounding box.
     * Accepted values:
     * - `1` template needs updating.
     * - `0` existing template still can be used for tracking, it doesn't need to be updated.
     */
    int8_t templateStatus;

    /*! @{ */
    /** Reserved for future use. */
    uint8_t reserved1, reserved2;
    /*! @} */
} VPIKLTTrackedBoundingBox;

/**
 * Defines the lock modes used by memory lock functions.
 * @ingroup VPI_Types
 */
typedef enum
{
    /** Lock memory only for reading.
     * Writing to the memory when locking for reading leads to undefined behavior. */
    VPI_LOCK_READ = 1,

    /** Lock memory only for writing.
     * Reading to the memory when locking for reading leads to undefined behavior.
     * It is expected that the whole memory is written to. If there are regions not
     * written, it might not be updated correctly during unlock. In this case, it's
     * better to use VPI_LOCK_READ_WRITE.
     *
     * It might be slightly efficient to lock only for writing, specially when
     * performing non-shared memory mapping.
     */
    VPI_LOCK_WRITE = 2,

    /** Lock memory for reading and writing. */
    VPI_LOCK_READ_WRITE = 3
} VPILockMode;

/**
 * Defines the termination criteria macros.
 * @ingroup VPI_Types
 */
#define VPI_TERMINATION_CRITERIA_ITERATIONS (1u << 0) /**< Termination based on maximum number of iterations. */
#define VPI_TERMINATION_CRITERIA_EPSILON (1u << 1)    /**< Termination based on maximum error (epsilon). */

/** Camera intrinsic matrix.
 * \f{bmatrix}{
 *    f_x & s & c_x \\
 *    0 & f_y & c_y
 * \f}
 *
 * Where:
 * - \f$f_x, f_y\f$: focal length in pixels.
 * - \f$s\f$: skew, used to model slanted pixels.
 * - \f$c_x, c_y\f$: principal point.
 *
 * @ingroup VPI_LDC
 *
 */
typedef float VPICameraIntrinsic[2][3];

/** Camera extrinsic matrix.
 * \f{bmatrix}{
 *   R_{3\times3} & T_{3\times1}
 * \f}
 *
 * Where:
 * \f$R_{3\times3}\f$: Rotation matrix.
 * \f$T_{3\times1}\f$: 3D position of the origin of world coordinate system expressed in coordinates of camera-centered system.
 *
 * @ingroup VPI_LDC
 */
typedef float VPICameraExtrinsic[3][4];

/** Represents a 2D perspective transform.
 * \verbatim
   [vx]   [a11 a12 tx] [ux]
   [vy] = [a21 a22 ty] [uy]
   [vw]   [ p0  p1  p] [ 1]
   u'x  = vx/vw
   u'y  = vy/vw
   \endverbatim
 *
 * In these equations,
 * - ux, uy are the inhomogeneous coordinates in the source image.
 * - u'x, u'y are the inhomogeneous coordinates in the destination image.
 * - vx,vy,vw are the homogeneous coordinates in the destination image.
 * - a11,a12,a21,a22 are the2x2 non-singular matrix with the linear component of the transform.
 * - tx,ty is the translation component of the transform.
 * - p0,p1,p are the projective components of the transform. p is usually 1.
 *
 * @ingroup VPI_PerspectiveWarp
 */
typedef float VPIPerspectiveTransform[3][3];

/**
 * Defines the states of the event.
 * @ingroup VPI_Types
 */
typedef enum
{

    /** Event is not signaled yet. */
    VPI_EVENT_STATE_NOT_SIGNALED,

    /** Event has been signaled. */
    VPI_EVENT_STATE_SIGNALED

} VPIEventState;

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_TYPES_H */
