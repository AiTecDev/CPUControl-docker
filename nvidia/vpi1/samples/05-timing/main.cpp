/*
* Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
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

#include <vpi/Event.h>
#include <vpi/Image.h>
#include <vpi/Pyramid.h>
#include <vpi/Status.h>
#include <vpi/Stream.h>
#include <vpi/algo/GaussianFilter.h>
#include <vpi/algo/GaussianPyramid.h>

#include <cstdio>
#include <iostream>
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

int main(int argc, char *argv[])
{
    VPIImage image    = NULL;
    VPIImage blurred  = NULL;
    VPIPyramid output = NULL;
    VPIStream stream  = NULL;

    VPIEvent evStart   = NULL;
    VPIEvent evBlur    = NULL;
    VPIEvent evPyramid = NULL;

    int retval = 0;

    try
    {
        // 1. Initialization stage ----------------------

        if (argc != 2)
        {
            throw std::runtime_error(std::string("Usage: ") + argv[0] + " <cpu|pva|cuda>");
        }

        std::string strBackend = argv[1];

        // Parse the backend
        VPIBackend backendType;

        if (strBackend == "cpu")
        {
            backendType = VPI_BACKEND_CPU;
        }
        else if (strBackend == "cuda")
        {
            backendType = VPI_BACKEND_CUDA;
        }
        else if (strBackend == "pva")
        {
            backendType = VPI_BACKEND_PVA;
        }
        else
        {
            throw std::runtime_error("Backend '" + strBackend +
                                     "' not recognized, it must be either cpu, cuda or pva.");
        }

        // Create the stream for the given backend.
        CHECK_STATUS(vpiStreamCreate(backendType, &stream));

        int width = 1920, height = 1080;
        VPIImageFormat imgFormat = VPI_IMAGE_FORMAT_U16;

        std::cout << "Input size: " << width << " x " << height << '\n';

        // Create image with zero content
        CHECK_STATUS(vpiImageCreate(width, height, imgFormat, 0, &image));

        // Now create the output image
        CHECK_STATUS(vpiPyramidCreate(width, height, imgFormat, 3, 0.5, 0, &output));

        // Create a temporary image convolved with a low-pass filter.
        CHECK_STATUS(vpiImageCreate(width, height, imgFormat, 0, &blurred));

        // Create the events we'll need to get timing info
        CHECK_STATUS(vpiEventCreate(0, &evStart));
        CHECK_STATUS(vpiEventCreate(0, &evBlur));
        CHECK_STATUS(vpiEventCreate(0, &evPyramid));

        // 2. Processing ----------------------

        // Record stream queue when we start processing
        CHECK_STATUS(vpiEventRecord(evStart, stream));

        // First we apply a low-pass filter using gaussian to avoid aliasing
        CHECK_STATUS(vpiSubmitGaussianFilter(stream, backendType, image, blurred, 5, 5, 1, 1, VPI_BORDER_ZERO));

        // Record stream queue just after blurring
        CHECK_STATUS(vpiEventRecord(evBlur, stream));

        // Now we create a gaussian pyramid out of it.
        CHECK_STATUS(vpiSubmitGaussianPyramidGenerator(stream, backendType, blurred, output));

        // Record stream queue just after pyramid generation, thus capturing the whole processing
        CHECK_STATUS(vpiEventRecord(evPyramid, stream));

        // Wait until the gaussian pyramid finishes processing
        CHECK_STATUS(vpiEventSync(evPyramid));

        // 3. Timing analysis ----------------------

        float elapsedBlurMS, elapsedPyramidMS, elapsedTotalMS;
        CHECK_STATUS(vpiEventElapsedTimeMillis(evStart, evBlur, &elapsedBlurMS));
        CHECK_STATUS(vpiEventElapsedTimeMillis(evBlur, evPyramid, &elapsedPyramidMS));
        CHECK_STATUS(vpiEventElapsedTimeMillis(evStart, evPyramid, &elapsedTotalMS));

        printf("Blurring elapsed time: %f ms\n", elapsedBlurMS);
        printf("Gaussian pyramid elapsed time: %f ms\n", elapsedPyramidMS);
        printf("Total elapsed time: %f ms\n", elapsedTotalMS);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retval = 1;
    }

    // 4. Clean up -----------------------------------

    // Make sure stream is synchronized before destroying the objects
    // that might still be in use.
    if (stream != NULL)
    {
        vpiStreamSync(stream);
    }

    vpiImageDestroy(image);
    vpiPyramidDestroy(output);
    vpiImageDestroy(blurred);
    vpiEventDestroy(evStart);
    vpiEventDestroy(evBlur);
    vpiEventDestroy(evPyramid);
    vpiStreamDestroy(stream);

    return retval;
}
