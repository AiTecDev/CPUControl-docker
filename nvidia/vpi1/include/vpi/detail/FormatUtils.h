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

#ifndef NV_VPI_DETAIL_FORMATUTILS_H
#define NV_VPI_DETAIL_FORMATUTILS_H

#include <stdint.h>

#define VPI_DETAIL_SET_BITFIELD(value, offset, size) (((uint64_t)(value) & ((1ULL << (size)) - 1)) << (offset))
#define VPI_DETAIL_GET_BITFIELD(value, offset, size) (((uint64_t)(value) >> (offset)) & ((1ULL << (size)) - 1))

#define VPI_DETAIL_ENCODE_BPP(bpp)             \
    ((bpp) <= 8 ? 0                            \
                : ((bpp) <= 32 ? (bpp) / 8 - 1 \
                               : ((bpp) <= 64 ? (bpp) / 16 + 1 : ((bpp) <= 128 ? (bpp) / 32 + 3 : (bpp) / 64 + 5))))

#define VPI_DETAIL_BPP_NCH(bpp, chcount)                                                                      \
    (VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_ENCODE_BPP(bpp), 6, 4) | VPI_DETAIL_SET_BITFIELD((chcount)-1, 4, 2) | \
     VPI_DETAIL_SET_BITFIELD((bpp) <= 2 ? (bpp) : ((bpp) == 4 ? 3 : ((bpp) == 8 ? 4 : 0)), 0, 4))

#define VPI_DETAIL_MAKE_SWIZZLE(x, y, z, w)                                                                   \
    (VPI_DETAIL_SET_BITFIELD(x, 0, 3) | VPI_DETAIL_SET_BITFIELD(y, 3, 3) | VPI_DETAIL_SET_BITFIELD(z, 6, 3) | \
     VPI_DETAIL_SET_BITFIELD(w, 9, 3))

#define VPI_DETAIL_MAKE_SWZL(x, y, z, w) \
    VPI_DETAIL_MAKE_SWIZZLE(VPI_CHANNEL_##x, VPI_CHANNEL_##y, VPI_CHANNEL_##z, VPI_CHANNEL_##w)

#define VPI_DETAIL_DEF_SWIZZLE_ENUM(x, y, z, w) VPI_SWIZZLE_##x##y##z##w = VPI_DETAIL_MAKE_SWZL(x, y, z, w)

#define VPI_DETAIL_ADJUST_BPP_ENCODING(PACK, BPP, PACKLEN) \
    ((PACKLEN) == 0 && (BPP) == 0 && (PACK) == 4 ? (uint64_t)-1 : (BPP))

#define VPI_DETAIL_ENCODE_PACKING(P, CHLEN, PACKLEN, BPPLEN)                                                          \
    (VPI_DETAIL_SET_BITFIELD(                                                                                         \
         VPI_DETAIL_ADJUST_BPP_ENCODING(VPI_DETAIL_GET_BITFIELD(P, 0, 4), VPI_DETAIL_GET_BITFIELD(P, 6, 4), PACKLEN), \
         (PACKLEN) + (CHLEN), BPPLEN) |                                                                               \
     VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_GET_BITFIELD(P, 4, 2), PACKLEN, CHLEN) |                                      \
     VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_GET_BITFIELD(P, 0, 4), 0, PACKLEN))

#define VPI_DETAIL_HANDLE_DATATYPE_SIGNEDNESS(DataType, rest) \
    (VPI_DETAIL_GET_BITFIELD(DataType, 2, 1) ? -(1 + (int64_t)((~(uint64_t)(rest)) & ~(1ULL << 63))) : (int64_t)(rest))

/* clang-format off */
#define VPI_DETAIL_MAKE_FMT_HELPER(ColorModel, ColorSpecOrRawPattern, Subsampling, MemLayout, DataType, Swizzle, \
                                   Packing0, Packing1, Packing2, Packing3)                                        \
    ((int64_t)VPI_DETAIL_HANDLE_DATATYPE_SIGNEDNESS(                                                              \
        DataType,                                                                                                 \
        VPI_DETAIL_SET_BITFIELD(((DataType)&0x3), 61, 2) | VPI_DETAIL_SET_BITFIELD(Swizzle, 0, 4 * 3) |           \
        VPI_DETAIL_SET_BITFIELD(MemLayout, 12, 3) | \
        ((ColorModel) == VPI_COLOR_MODEL_YCbCr \
            ? VPI_DETAIL_SET_BITFIELD(ColorSpecOrRawPattern, 20, 15) | VPI_DETAIL_SET_BITFIELD(Subsampling, 17, 3) \
            : ((ColorModel) == VPI_COLOR_MODEL_UNDEFINED \
                ? VPI_DETAIL_SET_BITFIELD((1U<<19)-1, 16, 19) \
                : (VPI_DETAIL_SET_BITFIELD(1,16,1) | \
                     ((ColorModel)-2 < 0x7 \
                       ? VPI_DETAIL_SET_BITFIELD(ColorSpecOrRawPattern, 20, 15) \
                            | VPI_DETAIL_SET_BITFIELD((ColorModel)-2, 17, 3) \
                       : (VPI_DETAIL_SET_BITFIELD(0x7, 17, 3) | \
                            ((ColorModel) == VPI_COLOR_MODEL_RAW \
                              ? VPI_DETAIL_SET_BITFIELD(ColorSpecOrRawPattern, 21, 6) \
                              : (VPI_DETAIL_SET_BITFIELD(1, 20, 1) | VPI_DETAIL_SET_BITFIELD((ColorModel)-(7+2+1), 21, 6)) \
                            ) \
                         ) \
                     ) \
                  ) \
              ) \
        ) | \
        VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_ENCODE_PACKING(Packing0, 2, 3, 4), 35, 9) |                           \
        VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_ENCODE_PACKING(Packing1, 1, 3, 3), 44, 7) |                           \
        VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_ENCODE_PACKING(Packing2, 1, 3, 3), 51, 7) |                           \
        VPI_DETAIL_SET_BITFIELD(VPI_DETAIL_ENCODE_PACKING(Packing3, 0, 0, 3), 58, 3)))
/* clang-format on */

#define VPI_DETAIL_MAKE_COLOR_FMT_HELPER VPI_DETAIL_MAKE_FMT_HELPER

#define VPI_DETAIL_MAKE_COLOR_FMT_HELPER_ABBREV(ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle, \
                                                Packing0, Packing1, Packing2, Packing3)                           \
    VPI_DETAIL_MAKE_FMT_HELPER(VPI_COLOR_MODEL_##ColorModel, VPI_COLOR_SPEC_##ColorSpec, VPI_CSS_##Subsampling,   \
                               VPI_MEM_LAYOUT_##MemLayout, VPI_DATA_TYPE_##DataType, VPI_SWIZZLE_##Swizzle,       \
                               VPI_PACKING_##Packing0, VPI_PACKING_##Packing1, VPI_PACKING_##Packing2,            \
                               VPI_PACKING_##Packing3)

#define VPI_DETAIL_MAKE_COLOR_FMT_HELPER_4(TYPE, ...) VPI_DETAIL_MAKE_COLOR_FMT_HELPER##TYPE(__VA_ARGS__)

#define VPI_DETAIL_MAKE_COLOR_FMT_HELPER_3(TYPE, ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle, \
                                           Packing0, Packing1, Packing2)                                           \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_4(TYPE, ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle,     \
                                       Packing0, Packing1, Packing2, 0)

#define VPI_DETAIL_MAKE_COLOR_FMT_HELPER_2(TYPE, ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle, \
                                           Packing0, Packing1)                                                     \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_3(TYPE, ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle,     \
                                       Packing0, Packing1, 0)

#define VPI_DETAIL_MAKE_COLOR_FMT_HELPER_1(TYPE, ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle, \
                                           Packing0)                                                               \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_2(TYPE, ColorModel, ColorSpec, Subsampling, MemLayout, DataType, Swizzle,     \
                                       Packing0, 0)

#define VPI_DETAIL_EXTRACT_PACKING_CHANNELS(Packing) (VPI_DETAIL_GET_BITFIELD(Packing, 4, 2) + 1)

#define VPI_DETAIL_MAKE_PIXEL_TYPE(MemLayout, DataType, Packing)                                                      \
    VPI_DETAIL_MAKE_FMT_HELPER(                                                                                       \
        VPI_COLOR_MODEL_UNDEFINED, VPI_COLOR_SPEC_UNDEFINED, VPI_CSS_NONE, MemLayout, DataType,                       \
        VPI_DETAIL_MAKE_SWIZZLE(VPI_CHANNEL_X, VPI_DETAIL_EXTRACT_PACKING_CHANNELS(Packing) >= 2 ? VPI_CHANNEL_Y : 0, \
                                VPI_DETAIL_EXTRACT_PACKING_CHANNELS(Packing) >= 3 ? VPI_CHANNEL_Z : 0,                \
                                VPI_DETAIL_EXTRACT_PACKING_CHANNELS(Packing) >= 4 ? VPI_CHANNEL_W : 0),               \
        Packing, 0, 0, 0)

#define VPI_DETAIL_MAKE_PIXEL_FMT(MemLayout, DataType, Packing) \
    VPI_DETAIL_MAKE_PIXEL_TYPE(VPI_MEM_LAYOUT_##MemLayout, VPI_DATA_TYPE_##DataType, VPI_PACKING_##Packing)

#define VPI_DETAIL_MAKE_COLOR_FMT(ColorModel, ColorSpec, MemLayout, DataType, Swizzle, PlaneCount, ...)               \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_##PlaneCount(_ABBREV, ColorModel, ColorSpec, NONE, MemLayout, DataType, Swizzle, \
                                                  __VA_ARGS__)

#define VPI_DETAIL_MAKE_COLOR_FORMAT(ColorModel, ColorSpec, MemLayout, DataType, Swizzle, PlaneCount, ...)             \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_##PlaneCount(, ColorModel, ColorSpec, VPI_CSS_NONE, MemLayout, DataType, Swizzle, \
                                                  __VA_ARGS__)

#define VPI_DETAIL_MAKE_NONCOLOR_FMT(MemLayout, DataType, Swizzle, PlaneCount, ...)                                  \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_##PlaneCount(_ABBREV, UNDEFINED, UNDEFINED, NONE, MemLayout, DataType, Swizzle, \
                                                  __VA_ARGS__)

#define VPI_DETAIL_MAKE_NONCOLOR_FORMAT(MemLayout, DataType, Swizzle, PlaneCount, ...)                                 \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_##PlaneCount(, VPI_COLOR_MODEL_UNDEFINED, VPI_COLOR_SPEC_UNDEFINED, VPI_CSS_NONE, \
                                                  MemLayout, DataType, Swizzle, __VA_ARGS__)

#define VPI_DETAIL_MAKE_RAW_FMT_HELPER(RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, Packing2,        \
                                       Packing3)                                                                      \
    VPI_DETAIL_MAKE_FMT_HELPER(VPI_COLOR_MODEL_RAW, RawPattern, VPI_CSS_NONE, MemLayout, DataType, Swizzle, Packing0, \
                               Packing1, Packing2, Packing3)

#define VPI_DETAIL_MAKE_RAW_FMT_HELPER_ABBREV(RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, Packing2, \
                                              Packing3)                                                               \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER(VPI_RAW_##RawPattern, VPI_MEM_LAYOUT_##MemLayout, VPI_DATA_TYPE_##DataType,        \
                                   VPI_SWIZZLE_##Swizzle, VPI_PACKING_##Packing0, VPI_PACKING_##Packing1,             \
                                   VPI_PACKING_##Packing2, VPI_PACKING_##Packing3)

#define VPI_DETAIL_MAKE_RAW_FMT_HELPER_4(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, Packing2, \
                                         Packing3)                                                                     \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER##TYPE(RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, Packing2,       \
                                         Packing3)

#define VPI_DETAIL_MAKE_RAW_FMT_HELPER_3(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, Packing2) \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER_4(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, Packing2, 0)

#define VPI_DETAIL_MAKE_RAW_FMT_HELPER_2(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1) \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER_3(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0, Packing1, 0)

#define VPI_DETAIL_MAKE_RAW_FMT_HELPER_1(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0) \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER_2(TYPE, RawPattern, MemLayout, DataType, Swizzle, Packing0, 0)

#define VPI_DETAIL_MAKE_RAW_FMT(RawPattern, MemLayout, DataType, Swizzle, PlaneCount, ...) \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER_##PlaneCount(_ABBREV, RawPattern, MemLayout, DataType, Swizzle, __VA_ARGS__)

#define VPI_DETAIL_MAKE_RAW_FORMAT(RawPattern, MemLayout, DataType, Swizzle, PlaneCount, ...) \
    VPI_DETAIL_MAKE_RAW_FMT_HELPER_##PlaneCount(, RawPattern, MemLayout, DataType, Swizzle, __VA_ARGS__)

#define VPI_DETAIL_MAKE_YCbCr_FORMAT(ColorSpec, ChromaSubsamp, MemLayout, DataType, Swizzle, PlaneCount, ...)   \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_##PlaneCount(, VPI_COLOR_MODEL_YCbCr, ColorSpec, ChromaSubsamp, MemLayout, \
                                                  DataType, Swizzle, __VA_ARGS__)

#define VPI_DETAIL_MAKE_YCbCr_FMT(ColorSpec, ChromaSubsamp, MemLayout, DataType, Swizzle, PlaneCount, ...)       \
    VPI_DETAIL_MAKE_COLOR_FMT_HELPER_##PlaneCount(_ABBREV, YCbCr, ColorSpec, ChromaSubsamp, MemLayout, DataType, \
                                                  Swizzle, __VA_ARGS__)

#define VPI_DETAIL_MAKE_COLOR_SPEC(CSpace, Encoding, XferFunc, Range, LocHoriz, LocVert)  \
    (VPI_DETAIL_SET_BITFIELD((CSpace), 0, 3) | VPI_DETAIL_SET_BITFIELD(XferFunc, 3, 4) |  \
     VPI_DETAIL_SET_BITFIELD(Encoding, 7, 3) | VPI_DETAIL_SET_BITFIELD(LocHoriz, 10, 2) | \
     VPI_DETAIL_SET_BITFIELD(LocVert, 12, 2) | VPI_DETAIL_SET_BITFIELD(Range, 14, 1))

#define VPI_DETAIL_MAKE_CSPC(CSpace, Encoding, XferFunc, Range, LocHoriz, LocVert)                                \
    VPI_DETAIL_MAKE_COLOR_SPEC(VPI_COLOR_##CSpace, VPI_YCbCr_##Encoding, VPI_COLOR_##XferFunc, VPI_COLOR_##Range, \
                               VPI_CHROMA_##LocHoriz, VPI_CHROMA_##LocVert)

#endif /* NV_VPI_DETAIL_FORMATUTILS_H */
