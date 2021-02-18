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
 * @file Version.h
 *
 * Functions and structures for handling VPI library version.
 */


#ifndef NV_VPI_VERSION_HPP
#define NV_VPI_VERSION_HPP

#include "Export.h"

#include <stdint.h> // for uint32_t

/**
 * Declarations of entities to handle VPI versioning.
 *
 * These utilities allow querying the VPI header and library versions and
 * properly handle VPI forward- or backward-compatibility .
 *
 * @defgroup VPI_Version Versioning
 * @ingroup VPI_API_Misc
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** VPI library version. 
  * It's an integer value computed from `MAJOR*1000000 + MINOR*10000 + PATCH*100 + TWEAK`. 
  * Integer versions can be compared, recent versions are greater than older ones.
  */
#define NV_VPI_VERSION 1001200

/** Assemble an integer version from its components.
 * This makes it easy to conditionally compile code for different VPI versions, e.g:
 * \code
 * #if NV_VPI_VERSION < NV_VPI_MAKE_VERSION(1,0,0)
 *    // code that runs on versions prior 1.0.0
 * #else
 *    // code that runs on versions after that, including 1.0.0
 * #endif
 * \endcode
 */
#define NV_VPI_MAKE_VERSION(MAJOR, MINOR, PATCH) \
    ((MAJOR)*1000000 + (MINOR)*10000 + (PATCH)*100)

/** Major version number component.
 * This is incremented every time there's a incompatible ABI change.
 * In the special case of major version 0, compatibility between minor versions
 * is not guaranteed.
 */
#define NV_VPI_VERSION_MAJOR 1

/** Minor version number component.
 * This is incremented every time there's a new feature added to VPI that
 * doesn't break backward compatibility. This number is reset to zero when
 * major version changes.
 */
#define NV_VPI_VERSION_MINOR 0

/** Patch version number component.
 * This is incremented every time a bug is fixed, but no new functionality is added
 * to the library. This number is reset to zero when minor version changes.
 */
#define NV_VPI_VERSION_PATCH 12

/** Tweak version number component.
 * Incremented for packaging or documentation updates, etc. The library itself isn't updated.
 * Gets reset to zero when patch version changes.
 */
#define NV_VPI_VERSION_TWEAK 0

/** Version suffix.
 * String appended to version number to designate special builds.
 */
#define NV_VPI_VERSION_SUFFIX 

/** VPI library version number represented as a string. */
#define NV_VPI_VERSION_STRING "1.0.12"

/** For internal use only. */
#define NV_VPI_COMMIT "533d1156da56b122d687b721a694fcf99ac014ad"

/** Retrieves the library's version number.
 * The number is represented as a integer. It may differ from \ref NV_VPI_VERSION if
 * header doesn't correspond to VPI binary. This can be used by user's program
 * to handle semantic differences between library versions.
 */
VPI_PUBLIC int vpiGetVersion(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif // NV_VPI_VERSION_HPP
