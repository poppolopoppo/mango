/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2026 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <algorithm>
#include <mango/simd/simd.hpp>

namespace mango::math::detail
{

    template <int Bits, typename T512, typename T256, typename T128, typename T64>
    struct select_bits
    {
        using type = std::conditional_t<
            (Bits >= 512), T512,
            std::conditional_t<(Bits >= 256), T256,
            std::conditional_t<(Bits >= 128), T128, T64>>
        >;
    };

    template <int Bits, typename T512, typename T256, typename T128, typename T64>
    using select_bits_t = typename select_bits<Bits, T512, T256, T128, T64>::type;

} // namespace mango::math::detail

namespace mango::math
{

    constexpr int native_float_bits = simd::native_float_bits;
    constexpr int native_int_bits   = simd::native_int_bits;
    constexpr int kernel_bits       = simd::kernel_bits;

    // -----------------------------------------------------------------
    // Widest native hardware vectors (W = wide)
    // -----------------------------------------------------------------

    using float32xW = detail::select_bits_t<native_float_bits, float32x16, float32x8, float32x4, float32x2>;
    using int32xW   = detail::select_bits_t<native_int_bits, int32x16, int32x8, int32x4, int32x2>;
    using uint32xW  = detail::select_bits_t<native_int_bits, uint32x16, uint32x8, uint32x4, uint32x2>;
    using mask32xW  = detail::select_bits_t<native_float_bits, mask32x16, mask32x8, mask32x4, mask32x4>;

    // -----------------------------------------------------------------
    // Unified kernel vectors (N = natural width for mixed int/float loops)
    // Both int and float are native hardware at kernel_bits.
    // -----------------------------------------------------------------

    using float32xN = detail::select_bits_t<kernel_bits, float32x16, float32x8, float32x4, float32x2>;
    using int32xN   = detail::select_bits_t<kernel_bits, int32x16, int32x8, int32x4, int32x2>;
    using uint32xN  = detail::select_bits_t<kernel_bits, uint32x16, uint32x8, uint32x4, uint32x2>;
    using mask32xN  = detail::select_bits_t<kernel_bits, mask32x16, mask32x8, mask32x4, mask32x4>;

#if defined(MANGO_ENABLE_SIMD)
    static_assert(!float32xW::VectorType::is_composite);
    static_assert(!int32xW::VectorType::is_composite);
    static_assert(float32xW::VectorType::is_hardware);
    static_assert(int32xW::VectorType::is_hardware);
    static_assert(!float32xN::VectorType::is_composite);
    static_assert(!int32xN::VectorType::is_composite);
    static_assert(float32xN::VectorType::is_hardware);
    static_assert(int32xN::VectorType::is_hardware);
#endif

    static_assert(float32xW::VectorType::vector_bits == native_float_bits);
    static_assert(int32xW::VectorType::vector_bits == native_int_bits);
    static_assert(float32xN::VectorType::vector_bits == kernel_bits);
    static_assert(int32xN::VectorType::vector_bits == kernel_bits);

} // namespace mango::math
