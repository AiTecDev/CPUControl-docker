/*
* Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*  * Neither the name of NVIDIA CORPORATION nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <opencv2/core/version.hpp>
#if CV_MAJOR_VERSION >= 3
#    include <opencv2/imgcodecs.hpp>
#    include <opencv2/videoio.hpp>
#else
#    include <opencv2/highgui/highgui.hpp>
#endif

#include <opencv2/imgproc/imgproc.hpp>
#include <vpi/OpenCVInterop.hpp>

#include <vpi/Context.h>
#include <vpi/Image.h>
#include <vpi/Status.h>
#include <vpi/Stream.h>
#include <vpi/algo/ConvertImageFormat.h>
#include <vpi/algo/PerspectiveWarp.h>

#include <algorithm>
#include <cstring> // for memset
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

#define CHECK_STATUS(STMT)                                    \
    do                                                        \
    {                                                         \
        VPIStatus status = (STMT);                            \
        if (status != VPI_SUCCESS)                            \
        {                                                     \
            char buffer[VPI_MAX_STATUS_MESSAGE_LENGTH];       \
            vpiGetLastStatusMessage(buffer, sizeof(buffer));  \
            std::ostringstream ss;                            \
            ss << vpiStatusGetName(status) << ": " << buffer; \
            throw std::runtime_error(ss.str());               \
        }                                                     \
    } while (0);

static void MatrixMultiply(VPIPerspectiveTransform &r, const VPIPerspectiveTransform &a,
                           const VPIPerspectiveTransform &b)
{
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            r[i][j] = a[i][0] * b[0][j];
            for (int k = 1; k < 3; ++k)
            {
                r[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// Utility function to wrap a cv::Mat into a VPIImage
static VPIImage ToVPIImage(VPIImage image, const cv::Mat &frame)
{
    if (image == nullptr)
    {
        // Ceate a VPIImage that wraps the frame
        CHECK_STATUS(vpiImageCreateOpenCVMatWrapper(frame, 0, &image));
    }
    else
    {
        // reuse existing VPIImage wrapper to wrap the new frame.
        CHECK_STATUS(vpiImageSetWrappedOpenCVMat(image, frame));
    }
    return image;
}

int main(int argc, char *argv[])
{
    // We'll create all our objects under this context, so that
    // we don't have to track what objects to destroy. Just destroying
    // the context will destroy all objects.
    VPIContext ctx = nullptr;

    int retval = 0;

    try
    {
        if (argc != 4)
        {
            throw std::runtime_error(std::string("Usage: ") + argv[0] + " <cpu|vic|cuda> <input_video> <output>");
        }

        std::string strBackend     = argv[1];
        std::string strInputVideo  = argv[2];
        std::string strOutputVideo = argv[3];

        // Load the input video
        cv::VideoCapture invid;
        if (!invid.open(strInputVideo))
        {
            throw std::runtime_error("Can't open '" + strInputVideo + "'");
        }

        // Create our context.
        CHECK_STATUS(vpiContextCreate(0, &ctx));

        // Activate it. From now on all created objects will be owned by it.
        CHECK_STATUS(vpiContextSetCurrent(ctx));

        // Now parse the backend
        VPIBackend backend;

        if (strBackend == "cpu")
        {
            backend = VPI_BACKEND_CPU;
        }
        else if (strBackend == "cuda")
        {
            backend = VPI_BACKEND_CUDA;
        }
        else if (strBackend == "vic")
        {
            backend = VPI_BACKEND_VIC;
        }
        else
        {
            throw std::runtime_error("Backend '" + strBackend +
                                     "' not recognized, it must be either cpu, cuda or vic.");
        }

        // Create the stream for the given backend. We'll be using CUDA for image format conversion.
        VPIStream stream;
        CHECK_STATUS(vpiStreamCreate(backend | VPI_BACKEND_CUDA, &stream));

#if CV_MAJOR_VERSION >= 3
        int w      = invid.get(cv::CAP_PROP_FRAME_WIDTH);
        int h      = invid.get(cv::CAP_PROP_FRAME_HEIGHT);
        int fourcc = invid.get(cv::CAP_PROP_FOURCC);
        double fps = invid.get(cv::CAP_PROP_FPS);
#else
        int w      = invid.get(CV_CAP_PROP_FRAME_WIDTH);
        int h      = invid.get(CV_CAP_PROP_FRAME_HEIGHT);
        int fourcc = invid.get(CV_FOURCC('M', 'J', 'P', 'G'));
        double fps = invid.get(CV_CAP_PROP_FPS);
#endif

        cv::VideoWriter outVideo(strOutputVideo, fourcc, fps, cv::Size(w, h));
        if (!outVideo.isOpened())
        {
            throw std::runtime_error("Can't create output video");
        }

        VPIImage imgInput, imgOutput;
        CHECK_STATUS(vpiImageCreate(w, h, VPI_IMAGE_FORMAT_NV12_ER, 0, &imgInput));
        CHECK_STATUS(vpiImageCreate(w, h, VPI_IMAGE_FORMAT_NV12_ER, 0, &imgOutput));

        // Create a Perspective Warp payload.
        VPIPayload warp;
        CHECK_STATUS(vpiCreatePerspectiveWarp(backend, &warp));

        VPIPerspectiveTransform xform;
        memset(&xform, 0, sizeof(xform));

        VPIImage frameBGR = nullptr;

        int curFrame = 1;
        cv::Mat cvFrame;
        while (invid.read(cvFrame))
        {
            printf("Frame: %d\n", curFrame++);

            frameBGR = ToVPIImage(frameBGR, cvFrame);

            // First convert it to NV12 using CUDA
            CHECK_STATUS(vpiSubmitConvertImageFormat(stream, VPI_BACKEND_CUDA, frameBGR, imgInput, NULL));

            // move image's center to origin of coordinate system
            VPIPerspectiveTransform t1 = {{1, 0, -w / 2.0f}, {0, 1, -h / 2.0f}, {0, 0, 1}};

            // Apply some time-dependent perspective transform
            float v1                  = sin(curFrame / 30.0 * 2 * M_PI / 2) * 0.0005f;
            float v2                  = cos(curFrame / 30.0 * 2 * M_PI / 3) * 0.0005f;
            VPIPerspectiveTransform P = {{0.66, 0, 0}, {0, 0.66, 0}, {v1, v2, 1}};

            // move image's center back to where it was.
            VPIPerspectiveTransform t2 = {{1, 0, w / 2.0f}, {0, 1, h / 2.0f}, {0, 0, 1}};

            // Apply the transforms defined above.
            VPIPerspectiveTransform tmp;
            MatrixMultiply(tmp, P, t1);
            MatrixMultiply(xform, t2, tmp);

            // Do perspective warp using the backend passed in the command line.
            CHECK_STATUS(vpiSubmitPerspectiveWarp(stream, 0, warp, imgInput, xform, imgOutput, VPI_INTERP_LINEAR,
                                                  VPI_BORDER_ZERO, 0));

            // Convert output back to BGR using CUDA
            CHECK_STATUS(vpiSubmitConvertImageFormat(stream, VPI_BACKEND_CUDA, imgOutput, frameBGR, NULL));
            CHECK_STATUS(vpiStreamSync(stream));

            // Now add it to the output video stream
            VPIImageData imgdata;
            CHECK_STATUS(vpiImageLock(frameBGR, VPI_LOCK_READ, &imgdata));

            cv::Mat outFrame;
            CHECK_STATUS(vpiImageDataExportOpenCVMat(imgdata, &outFrame));
            outVideo << outFrame;

            CHECK_STATUS(vpiImageUnlock(frameBGR));
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retval = 1;
    }

    // Clean up
    vpiContextDestroy(ctx);

    return retval;
}
