/*
 * Copyright 2020 NVIDIA Corporation. All rights reserved.
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

#ifndef NV_VPI_OPENCV_INTEROP_HPP
#define NV_VPI_OPENCV_INTEROP_HPP

#include "detail/OpenCVUtils.hpp"

#include "Image.h"

#include <opencv2/core/core.hpp>

#include <vector>

// Wraps a cv::Mat in an VPIImage with the given image format.
// It also wraps YUV images in 4:2:2 (packed and semi-planar) and 4:2:0 (semi-planar) formats
// that are understood by cv::cvtColor.
inline VPIStatus vpiImageCreateOpenCVMatWrapper(const cv::Mat &mat, VPIImageFormat fmt, uint32_t flags, VPIImage *out)
{
    VPIImageData imgData = {};
    VPIStatus status     = ::nv::vpi::detail::FillImageData(mat, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageCreateHostMemWrapper(&imgData, flags, out);
}

// Wraps a cv::Mat in an VPIImage and infers the image format from its type.
// unsigned and float 3-channel images are taken to be BGR
// unsigned and float 4-channel images are taken to be BGRA
// other types are mapped to non-color vpi image formats with the same pixel layout.
inline VPIStatus vpiImageCreateOpenCVMatWrapper(const cv::Mat &mat, uint32_t flags, VPIImage *out)
{
    VPIImageData imgData = {};
    VPIStatus status     = ::nv::vpi::detail::FillImageData(mat, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageCreateHostMemWrapper(&imgData, flags, out);
}

// Replaces the wrapped cv::Mat with a new one. Must have same dimensions and type as the wrapped one.
inline VPIStatus vpiImageSetWrappedOpenCVMat(VPIImage img, const cv::Mat &mat)
{
    VPIImageFormat fmt;
    VPIStatus status = vpiImageGetFormat(img, &fmt);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    VPIImageData imgData = {};
    status               = ::nv::vpi::detail::FillImageData(mat, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageSetWrappedHostMem(img, &imgData);
}

// Wraps a list of cv::Mats in a VPIImage, each one in one plane.
inline VPIStatus vpiImageCreateOpenCVMatPlanesWrapper(const std::vector<cv::Mat> &matPlanes, VPIImageFormat fmt,
                                                      uint32_t flags, VPIImage *out)
{
    VPIImageData imgData = {};
    VPIStatus status     = ::nv::vpi::detail::FillImageData(matPlanes, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageCreateHostMemWrapper(&imgData, flags, out);
}

// Replaces the wrapped cv::Mat planes with new ones. Must have same dimensions, type and number of planes as the wrapped one.
inline VPIStatus vpiImageSetWrappedOpenCVMatPlanes(VPIImage img, const std::vector<cv::Mat> &matPlanes)
{
    VPIImageFormat fmt;
    VPIStatus status = vpiImageGetFormat(img, &fmt);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    VPIImageData imgData = {};
    status               = ::nv::vpi::detail::FillImageData(matPlanes, fmt, &imgData);
    if (status != VPI_SUCCESS)
    {
        return status;
    }

    return vpiImageSetWrappedHostMem(img, &imgData);
}

// Fills a cv::Mat with data from VPIImageData, no image contents will be copied, they're just referenced.
inline VPIStatus vpiImageDataExportOpenCVMat(const VPIImageData &imgData, cv::Mat *mat)
{
    if (mat == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::ExportOpenCVMat(imgData, *mat);
}

// Fills a vector of cv::Mat with all planes from VPIImageData. No image contents will be copied, they're just referenced.
inline VPIStatus vpiImageDataExportOpenCVMatPlanes(const VPIImageData &imgData, std::vector<cv::Mat> *matPlanes)
{
    if (matPlanes == NULL)
    {
        return VPI_ERROR_INVALID_ARGUMENT;
    }

    return ::nv::vpi::detail::ExportOpenCVMatPlanes(imgData, *matPlanes);
}

#endif // NV_VPI_OPENCV_INTEROP_HPP
