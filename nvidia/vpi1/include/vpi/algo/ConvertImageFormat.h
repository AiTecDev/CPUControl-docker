/*
 * Copyright 2020-2021 NVIDIA Corporation. All rights reserved.
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
 * @file ConvertImageFormat.h
 *
 * Declares functions that handle image format conversion.
 */

#ifndef NV_VPI_ALGORITHMS_CONVERT_IMAGE_FORMAT_H
#define NV_VPI_ALGORITHMS_CONVERT_IMAGE_FORMAT_H

#include "../AlgoFlags.h"
#include "../Export.h"
#include "../Status.h"
#include "../Types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup VPI_ConvertImageFormat Convert Image Format
 * @ingroup VPI_API_Algorithms
 * @{
 *
 * Converts the image contents to the desired format, with optional pixel value scaling and offset.
 */

/** Parameters for customizing image format conversion.
 * These parameters are used to customize how the conversion will be made.
 * Make sure to call \ref vpiInitConvertImageFormatParams to initialize this
 * structure before updating its attributes. This guarantees that new attributes
 * added in future versions will have a suitable default value assigned.
 */
typedef struct
{
    VPIConversionPolicy policy; /**< Conversion policy to be used. */
    float scale;                /**< Scaling factor. Pass 1 for no scaling. */
    float offset;               /**< Offset factor. Pass 0 for no offset. */

    /**< Control flags.
     * Valid values:
     * - 0: default, negation of all other flags.
     * - \ref VPI_PRECISE : precise, potentially slower implementation.
     */
    uint32_t flags;

    /** Interpolation to use for chroma upsampling.
     * @note VIC backend on Jetson Nano devices doesn't allow choosing of
     * chroma upsampling filter. It will only use \ref VPI_INTERP_NEAREST,
     * no matter what value is chosen here. */
    VPIInterpolationType chromaUpFilter;

    /** Interpolation to use for chroma downsampling.
     * @note Currently all backends support \ref VPI_INTERP_NEAREST only.
     * Different values will be silently ignored.
     */
    VPIInterpolationType chromaDownFilter;
} VPIConvertImageFormatParams;

/** Initialize \ref VPIConvertImageFormatParams with default values.
 *
 *  The following parameters are set:
 * - policy: \ref VPI_CONVERSION_CLAMP
 * - scale: 1
 * - offset: 0
 * - flags: 0
 * - chromaUpFilter: VPI_INTERP_LINEAR
 * - chromaDownFilter: VPI_INTERP_LINEAR
 *
 * @param[out] params Pointer to structure to be filled. Must not be NULL.
 * @returns An error code if params is NULL, \ref VPI_SUCCESS otherwise.
 */
VPI_PUBLIC VPIStatus vpiInitConvertImageFormatParams(VPIConvertImageFormatParams *params);

/**
 * Converts the image contents to the desired format, with optional scaling and offset.
 * The input and output types are inferred from the corresponding images passed as parameters.
 * When scaling and offset are fractional, input is converted to 32-bit floating point prior conversion takes place.
 * The formula that relates input and output pixels is:
 *
 * \f[
 *    out(x,y) = clamp_{[\mathsf{min_{out}},\mathsf{max_{out}}]}(in(x,y)*\alpha + \beta)
 * \f]
 *
 * where:
 * - \f$\mathsf{min_{out}}\f$ and \f$\mathsf{max_{out}}\f$ are the minimum and maximum representable value by output image format. Exception is for floating point types, where limits are \f$[-\infty,\infty]\f$, i.e., no clamping is done.
 * - \f$\alpha\f$ is the scaling.
 * - \f$\beta\f$ is the offset.
 *
 * float to integer conversion does returns the nearest integer number, rounding halfway cases away from zero.
 *
 * When passing NULL as params argument, it'll use default values as the ones set by
 * \ref vpiInitConvertImageFormatParams.
 *
 * @param[in] stream A stream handle where the operation will be queued into.
 * @param[in] backend Backend that will execute the algorithm.
 * @param[in] input Input image to be converted.
 * @param[out] output Output image where result will be written to, with the desired type.
 * @param[in] params Optional parameters. Pass NULL for default. 
 *
 * @returns an error code on failure else \ref VPI_SUCCESS
 */
VPI_PUBLIC VPIStatus vpiSubmitConvertImageFormat(VPIStream stream, uint32_t backend, VPIImage input, VPIImage output,
                                                 const VPIConvertImageFormatParams *params);

/** @} end of VPI_ConvertImageFormat */

#ifdef __cplusplus
}
#endif

#endif /* NV_VPI_ALGORITHMS_CONVERT_IMAGE_FORMAT_H */
