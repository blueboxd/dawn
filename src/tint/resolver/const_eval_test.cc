// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cmath>
#include <type_traits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/builtin_type.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/test_helper.h"
#include "src/tint/utils/transform.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

template <typename T>
const auto kPi = T(UnwrapNumber<T>(3.14159265358979323846));

template <typename T>
const auto kPiOver2 = T(UnwrapNumber<T>(1.57079632679489661923));

template <typename T>
const auto kPiOver4 = T(UnwrapNumber<T>(0.785398163397448309616));

template <typename T>
const auto k3PiOver4 = T(UnwrapNumber<T>(2.356194490192344928846));

template <typename T>
constexpr auto Negate(const Number<T>& v) {
    if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_signed_v<T>) {
            // For signed integrals, avoid C++ UB by not negating the smallest negative number. In
            // WGSL, this operation is well defined to return the same value, see:
            // https://gpuweb.github.io/gpuweb/wgsl/#arithmetic-expr.
            if (v == std::numeric_limits<T>::min()) {
                return v;
            }
            return -v;

        } else {
            // Allow negating unsigned values
            using ST = std::make_signed_t<T>;
            auto as_signed = Number<ST>{static_cast<ST>(v)};
            return Number<T>{static_cast<T>(Negate(as_signed))};
        }
    } else {
        // float case
        return -v;
    }
}

template <typename T>
auto Abs(const Number<T>& v) {
    if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        return v;
    } else {
        return Number<T>(std::abs(v));
    }
}

TINT_BEGIN_DISABLE_WARNING(CONSTANT_OVERFLOW);
template <typename T>
constexpr Number<T> Mul(Number<T> v1, Number<T> v2) {
    if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        // For signed integrals, avoid C++ UB by multiplying as unsigned
        using UT = std::make_unsigned_t<T>;
        return static_cast<Number<T>>(static_cast<UT>(v1) * static_cast<UT>(v2));
    } else {
        return static_cast<Number<T>>(v1 * v2);
    }
}
TINT_END_DISABLE_WARNING(CONSTANT_OVERFLOW);

// Concats any number of std::vectors
template <typename Vec, typename... Vecs>
[[nodiscard]] auto Concat(Vec&& v1, Vecs&&... vs) {
    auto total_size = v1.size() + (vs.size() + ...);
    v1.reserve(total_size);
    (std::move(vs.begin(), vs.end(), std::back_inserter(v1)), ...);
    return std::move(v1);
}

// Concats vectors `vs` into `v1`
template <typename Vec, typename... Vecs>
void ConcatInto(Vec& v1, Vecs&&... vs) {
    auto total_size = v1.size() + (vs.size() + ...);
    v1.reserve(total_size);
    (std::move(vs.begin(), vs.end(), std::back_inserter(v1)), ...);
}

// Concats vectors `vs` into `v1` iff `condition` is true
template <bool condition, typename Vec, typename... Vecs>
void ConcatIntoIf([[maybe_unused]] Vec& v1, [[maybe_unused]] Vecs&&... vs) {
    if constexpr (condition) {
        ConcatInto(v1, std::forward<Vecs>(vs)...);
    }
}

using ResolverConstEvalTest = ResolverTest;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(ResolverConstEvalTest, Scalar_i32) {
    auto* expr = Expr(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 99);
}

TEST_F(ResolverConstEvalTest, Scalar_u32) {
    auto* expr = Expr(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::U32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 99u);
}

TEST_F(ResolverConstEvalTest, Scalar_f32) {
    auto* expr = Expr(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::F32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<AFloat>().value, 9.9f);
}

TEST_F(ResolverConstEvalTest, Scalar_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = Expr(9.9_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::F16>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    // 9.9 is not exactly representable by f16, and should be quantized to 9.8984375
    EXPECT_EQ(sem->ConstantValue()->As<AFloat>(), 9.8984375f);
}

TEST_F(ResolverConstEvalTest, Scalar_bool) {
    auto* expr = Expr(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::Bool>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_i32) {
    auto* expr = vec3<i32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 0);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 0);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 0);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_u32) {
    auto* expr = vec3<u32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 0u);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 0u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 0u);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_f32) {
    auto* expr = vec3<f32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 0._a);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 0._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 0._a);
}

TEST_F(ResolverConstEvalTest, Vec3_ZeroInit_bool) {
    auto* expr = vec3<bool>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_i32) {
    auto* expr = vec3<i32>(99_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 99);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 99);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 99);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_u32) {
    auto* expr = vec3<u32>(99_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 99u);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 99u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 99u);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_f32) {
    auto* expr = vec3<f32>(9.9_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 9.9f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 9.9f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 9.9f);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(9.9_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    // 9.9 is not exactly representable by f16, and should be quantized to 9.8984375

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 9.8984375f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 9.8984375f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 9.8984375f);
}

TEST_F(ResolverConstEvalTest, Vec3_Splat_bool) {
    auto* expr = vec3<bool>(true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_i32) {
    auto* expr = vec3<i32>(1_i, 2_i, 3_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_u32) {
    auto* expr = vec3<u32>(1_u, 2_u, 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_f32) {
    auto* expr = vec3<f32>(1_f, 2_f, 3_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(1_h, 2_h, 3_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_FullConstruct_bool) {
    auto* expr = vec3<bool>(true, false, true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_i32) {
    auto* expr = vec3<i32>(1_i, vec2<i32>(2_i, 3_i));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_u32) {
    auto* expr = vec3<u32>(vec2<u32>(1_u, 2_u), 3_u);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32) {
    auto* expr = vec3<f32>(1_f, vec2<f32>(2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_all_10) {
    auto* expr = vec3<f32>(10_f, vec2<f32>(10_f, 10_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 10_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 10_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 10_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_all_positive_0) {
    auto* expr = vec3<f32>(0_f, vec2<f32>(0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_all_negative_0) {
    auto* expr = vec3<f32>(vec2<f32>(-0_f, -0_f), -0_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), -0_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f32_mixed_sign_0) {
    auto* expr = vec3<f32>(0_f, vec2<f32>(-0_f, 0_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), -0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(1_h, vec2<f16>(2_h, 3_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 1.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 2.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 3.f);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_all_10) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(10_h, vec2<f16>(10_h, 10_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), 10_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), 10_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), 10_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_all_positive_0) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(0_h, vec2<f16>(0_h, 0_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), 0_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_all_negative_0) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec2<f16>(-0_h, -0_h), -0_h);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), -0_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_f16_mixed_sign_0) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(0_h, vec2<f16>(-0_h, 0_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f16>(), 0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f16>(), -0_h);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f16>(), 0_h);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_bool) {
    auto* expr = vec3<bool>(vec2<bool>(true, false), true);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_all_true) {
    auto* expr = vec3<bool>(true, vec2<bool>(true, true));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), true);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), true);
}

TEST_F(ResolverConstEvalTest, Vec3_MixConstruct_all_false) {
    auto* expr = vec3<bool>(false, vec2<bool>(false, false));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::Bool>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<bool>(), false);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_f32_to_i32) {
    auto* expr = vec3<i32>(vec3<f32>(1.1_f, 2.2_f, 3.3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_u32_to_f32) {
    auto* expr = vec3<f32>(vec3<u32>(10_u, 20_u, 30_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 10.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 20.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 30.f);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_f16_to_i32) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<i32>(vec3<f16>(1.1_h, 2.2_h, 3.3_h));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), 1_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), 2_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), 3_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_u32_to_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec3<u32>(10_u, 20_u, 30_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 10.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), 20.f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 30.f);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Large_f32_to_i32) {
    auto* expr = vec3<i32>(vec3<f32>(1e10_f, -1e20_f, 1e30_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), i32::Highest());

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), i32::Lowest());

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), i32::Highest());
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Large_f32_to_u32) {
    auto* expr = vec3<u32>(vec3<f32>(1e10_f, -1e20_f, 1e30_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::U32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AInt>(), u32::Highest());

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AInt>(), u32::Lowest());

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AInt>(), u32::Highest());
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Large_f32_to_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec3<f32>(1e10_f, -1e20_f, 1e30_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    constexpr auto kInfinity = std::numeric_limits<double>::infinity();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), kInfinity);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), -kInfinity);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), kInfinity);
}

TEST_F(ResolverConstEvalTest, Vec3_Convert_Small_f32_to_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = vec3<f16>(vec3<f32>(1e-20_f, -2e-30_f, 3e-40_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F16>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<AFloat>(), 0.0);
    EXPECT_FALSE(std::signbit(sem->ConstantValue()->Index(0)->As<AFloat>().value));

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<AFloat>(), -0.0);
    EXPECT_TRUE(std::signbit(sem->ConstantValue()->Index(1)->As<AFloat>().value));

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<AFloat>(), 0.0);
    EXPECT_FALSE(std::signbit(sem->ConstantValue()->Index(2)->As<AFloat>().value));
}

TEST_F(ResolverConstEvalTest, Mat2x3_ZeroInit_f32) {
    auto* expr = mat2x3<f32>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F32>());
    EXPECT_EQ(mat->columns(), 2u);
    EXPECT_EQ(mat->rows(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0._f);
}

TEST_F(ResolverConstEvalTest, Mat2x3_ZeroInit_f16) {
    Enable(ast::Extension::kF16);

    auto* expr = mat2x3<f16>();
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    EXPECT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F16>());
    EXPECT_EQ(mat->columns(), 2u);
    EXPECT_EQ(mat->rows(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f16>(), 0._h);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Construct_Scalars_af) {
    auto* expr = Construct(ty.mat(nullptr, 3, 2), 1.0_a, 2.0_a, 3.0_a, 4.0_a, 5.0_a, 6.0_a);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F32>());
    EXPECT_EQ(mat->columns(), 3u);
    EXPECT_EQ(mat->rows(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<AFloat>(), 1._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<AFloat>(), 2._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<AFloat>(), 3._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<AFloat>(), 4._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<AFloat>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<AFloat>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Construct_Columns_af) {
    auto* expr = Construct(ty.mat(nullptr, 3, 2),           //
                           vec(nullptr, 2u, 1.0_a, 2.0_a),  //
                           vec(nullptr, 2u, 3.0_a, 4.0_a),  //
                           vec(nullptr, 2u, 5.0_a, 6.0_a));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* mat = sem->Type()->As<sem::Matrix>();
    ASSERT_NE(mat, nullptr);
    EXPECT_TRUE(mat->type()->Is<sem::F32>());
    EXPECT_EQ(mat->columns(), 3u);
    EXPECT_EQ(mat->rows(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<AFloat>(), 1._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<AFloat>(), 2._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<AFloat>(), 3._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<AFloat>(), 4._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<AFloat>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<AFloat>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Array_i32_Zero) {
    auto* expr = Construct(ty.array<i32, 4>());
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::I32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<i32>(), 0_i);
}

TEST_F(ResolverConstEvalTest, Array_f32_Zero) {
    auto* expr = Construct(ty.array<f32, 4>());
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::F32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Zero) {
    auto* expr = Construct(ty.array(ty.vec3<f32>(), 2_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Vector>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Array_Struct_f32_Zero) {
    Structure("S", utils::Vector{
                       Member("m1", ty.f32()),
                       Member("m2", ty.f32()),
                   });
    auto* expr = Construct(ty.array(ty.type_name("S"), 2_u));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Struct>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Array_i32_Elements) {
    auto* expr = Construct(ty.array<i32, 4>(), 10_i, 20_i, 30_i, 40_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::I32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 10_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<i32>(), 20_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<i32>(), 30_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<i32>(), 40_i);
}

TEST_F(ResolverConstEvalTest, Array_f32_Elements) {
    auto* expr = Construct(ty.array<f32, 4>(), 10_f, 20_f, 30_f, 40_f);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::F32>());
    EXPECT_EQ(arr->Count(), 4u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 10_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 20_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 30_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f32>(), 40_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Elements) {
    auto* expr = Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                           vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Vector>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 1_f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 2_f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 3_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 4_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 5_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Array_Struct_f32_Elements) {
    Structure("S", utils::Vector{
                       Member("m1", ty.f32()),
                       Member("m2", ty.f32()),
                   });
    auto* expr = Construct(ty.array(ty.type_name("S"), 2_u),        //
                           Construct(ty.type_name("S"), 1_f, 2_f),  //
                           Construct(ty.type_name("S"), 3_f, 4_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<sem::Array>();
    ASSERT_NE(arr, nullptr);
    EXPECT_TRUE(arr->ElemType()->Is<sem::Struct>());
    EXPECT_EQ(arr->Count(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 1_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 2_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 3_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 4_f);
}

TEST_F(ResolverConstEvalTest, Struct_I32s_ZeroInit) {
    Structure(
        "S", utils::Vector{Member("m1", ty.i32()), Member("m2", ty.i32()), Member("m3", ty.i32())});
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 3u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<i32>(), 0_i);
}

TEST_F(ResolverConstEvalTest, Struct_MixedScalars_ZeroInit) {
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{
                       Member("m1", ty.i32()),
                       Member("m2", ty.u32()),
                       Member("m3", ty.f32()),
                       Member("m4", ty.f16()),
                       Member("m5", ty.bool_()),
                   });
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<u32>(), 0_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_VectorF32s_ZeroInit) {
    Structure("S", utils::Vector{
                       Member("m1", ty.vec3<f32>()),
                       Member("m2", ty.vec3<f32>()),
                       Member("m3", ty.vec3<f32>()),
                   });
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 3u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->As<f32>(), 0._f);
}

TEST_F(ResolverConstEvalTest, Struct_MixedVectors_ZeroInit) {
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{
                       Member("m1", ty.vec2<i32>()),
                       Member("m2", ty.vec3<u32>()),
                       Member("m3", ty.vec4<f32>()),
                       Member("m4", ty.vec3<f16>()),
                       Member("m5", ty.vec2<bool>()),
                   });
    auto* expr = Construct(ty.type_name("S"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<i32>(), 0_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<u32>(), 0_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->As<f32>(), 0._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(3)->As<f32>(), 0._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->As<sem::Vector>()->type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(0)->As<f16>(), 0._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(1)->As<f16>(), 0._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(2)->As<f16>(), 0._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(0)->As<bool>(), false);
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(1)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_Struct_ZeroInit) {
    Structure("Inner", utils::Vector{
                           Member("m1", ty.i32()),
                           Member("m2", ty.u32()),
                           Member("m3", ty.f32()),
                       });

    Structure("Outer", utils::Vector{
                           Member("m1", ty.type_name("Inner")),
                           Member("m2", ty.type_name("Inner")),
                       });
    auto* expr = Construct(ty.type_name("Outer"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 0_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<i32>(), 0_i);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 0_f);
}

TEST_F(ResolverConstEvalTest, Struct_MixedScalars_Construct) {
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{
                       Member("m1", ty.i32()),
                       Member("m2", ty.u32()),
                       Member("m3", ty.f32()),
                       Member("m4", ty.f16()),
                       Member("m5", ty.bool_()),
                   });
    auto* expr = Construct(ty.type_name("S"), 1_i, 2_u, 3_f, 4_h, false);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<i32>(), 1_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<u32>(), 2_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 3._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->As<f16>(), 4._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_MixedVectors_Construct) {
    Enable(ast::Extension::kF16);

    Structure("S", utils::Vector{
                       Member("m1", ty.vec2<i32>()),
                       Member("m2", ty.vec3<u32>()),
                       Member("m3", ty.vec4<f32>()),
                       Member("m4", ty.vec3<f16>()),
                       Member("m5", ty.vec2<bool>()),
                   });
    auto* expr = Construct(ty.type_name("S"), vec2<i32>(1_i), vec3<u32>(2_u), vec4<f32>(3_f),
                           vec3<f16>(4_h), vec2<bool>(false));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 5u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<i32>(), 1_i);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->As<sem::Vector>()->type()->Is<sem::U32>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<u32>(), 2_u);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(2)->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(0)->As<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(1)->As<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(2)->As<f32>(), 3._f);
    EXPECT_EQ(sem->ConstantValue()->Index(2)->Index(3)->As<f32>(), 3._f);

    EXPECT_TRUE(sem->ConstantValue()->Index(3)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(3)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(3)->Type()->As<sem::Vector>()->type()->Is<sem::F16>());
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(0)->As<f16>(), 4._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(1)->As<f16>(), 4._h);
    EXPECT_EQ(sem->ConstantValue()->Index(3)->Index(2)->As<f16>(), 4._h);

    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AnyZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(sem->ConstantValue()->Index(4)->Type()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(0)->As<bool>(), false);
    EXPECT_EQ(sem->ConstantValue()->Index(4)->Index(1)->As<bool>(), false);
}

TEST_F(ResolverConstEvalTest, Struct_Struct_Construct) {
    Structure("Inner", utils::Vector{
                           Member("m1", ty.i32()),
                           Member("m2", ty.u32()),
                           Member("m3", ty.f32()),
                       });

    Structure("Outer", utils::Vector{
                           Member("m1", ty.type_name("Inner")),
                           Member("m2", ty.type_name("Inner")),
                       });
    auto* expr = Construct(ty.type_name("Outer"),  //
                           Construct(ty.type_name("Inner"), 1_i, 2_u, 3_f),
                           Construct(ty.type_name("Inner"), 4_i, 0_u, 6_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<u32>(), 2_u);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(2)->As<f32>(), 3_f);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Struct>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<i32>(), 4_i);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 0_u);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Struct_Array_Construct) {
    Structure("S", utils::Vector{
                       Member("m1", ty.array<i32, 2>()),
                       Member("m2", ty.array<f32, 3>()),
                   });
    auto* expr = Construct(ty.type_name("S"),  //
                           Construct(ty.array<i32, 2>(), 1_i, 2_i),
                           Construct(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* str = sem->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(sem->ConstantValue(), nullptr);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_FALSE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());

    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(0)->Type()->Is<sem::Array>());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(sem->ConstantValue()->Index(0)->Index(1)->As<u32>(), 2_i);

    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_TRUE(sem->ConstantValue()->Index(1)->Type()->Is<sem::Array>());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(0)->As<i32>(), 1_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(1)->As<u32>(), 2_f);
    EXPECT_EQ(sem->ConstantValue()->Index(1)->Index(2)->As<f32>(), 3_f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Indexing
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(ResolverConstEvalTest, Vec3_Index) {
    auto* expr = IndexAccessor(vec3<i32>(1_i, 2_i, 3_i), 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 3_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Index_OOB_High) {
    auto* expr = IndexAccessor(vec3<i32>(1_i, 2_i, 3_i), Expr(Source{{12, 34}}, 3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 3 out of bounds [0..2]");
}

TEST_F(ResolverConstEvalTest, Vec3_Index_OOB_Low) {
    auto* expr = IndexAccessor(vec3<i32>(1_i, 2_i, 3_i), Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -3 out of bounds [0..2]");
}

TEST_F(ResolverConstEvalTest, Vec3_Swizzle_Scalar) {
    auto* expr = MemberAccessor(vec3<i32>(1_i, 2_i, 3_i), "y");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 2_i);
}

TEST_F(ResolverConstEvalTest, Vec3_Swizzle_Vector) {
    auto* expr = MemberAccessor(vec3<i32>(1_i, 2_i, 3_i), "zx");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 3._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 1._a);
}

TEST_F(ResolverConstEvalTest, Vec3_Swizzle_Chain) {
    auto* expr =  // (1, 2, 3) -> (2, 3, 1) -> (3, 2) -> 2
        MemberAccessor(MemberAccessor(MemberAccessor(vec3<i32>(1_i, 2_i, 3_i), "gbr"), "yx"), "y");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    ASSERT_TRUE(sem->Type()->Is<sem::I32>());
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    EXPECT_TRUE(sem->ConstantValue()->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->AllZero());
    EXPECT_EQ(sem->ConstantValue()->As<i32>(), 2_i);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Index) {
    auto* expr = IndexAccessor(
        mat3x2<f32>(vec2<f32>(1._a, 2._a), vec2<f32>(3._a, 4._a), vec2<f32>(5._a, 6._a)), 2_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 5._a);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 6._a);
}

TEST_F(ResolverConstEvalTest, Mat3x2_Index_OOB_High) {
    auto* expr = IndexAccessor(
        mat3x2<f32>(vec2<f32>(1._a, 2._a), vec2<f32>(3._a, 4._a), vec2<f32>(5._a, 6._a)),
        Expr(Source{{12, 34}}, 3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 3 out of bounds [0..2]");
}

TEST_F(ResolverConstEvalTest, Mat3x2_Index_OOB_Low) {
    auto* expr = IndexAccessor(
        mat3x2<f32>(vec2<f32>(1._a, 2._a), vec2<f32>(3._a, 4._a), vec2<f32>(5._a, 6._a)),
        Expr(Source{{12, 34}}, -3_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -3 out of bounds [0..2]");
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Index) {
    auto* expr = IndexAccessor(Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                                         vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f)),
                               1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<sem::Vector>();
    ASSERT_NE(vec, nullptr);
    EXPECT_TRUE(vec->type()->Is<sem::F32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());

    EXPECT_TRUE(sem->ConstantValue()->Index(0)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(0)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(0)->As<f32>(), 4_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(1)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(1)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(1)->As<f32>(), 5_f);

    EXPECT_TRUE(sem->ConstantValue()->Index(2)->AllEqual());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AnyZero());
    EXPECT_FALSE(sem->ConstantValue()->Index(2)->AllZero());
    EXPECT_EQ(sem->ConstantValue()->Index(2)->As<f32>(), 6_f);
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Index_OOB_High) {
    auto* expr = IndexAccessor(Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                                         vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f)),
                               Expr(Source{{12, 34}}, 2_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index 2 out of bounds [0..1]");
}

TEST_F(ResolverConstEvalTest, Array_vec3_f32_Index_OOB_Low) {
    auto* expr = IndexAccessor(Construct(ty.array(ty.vec3<f32>(), 2_u),  //
                                         vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(4_f, 5_f, 6_f)),
                               Expr(Source{{12, 34}}, -2_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -2 out of bounds [0..1]");
}

TEST_F(ResolverConstEvalTest, RuntimeArray_vec3_f32_Index_OOB_Low) {
    auto* sb = GlobalVar("sb", ty.array(ty.vec3<f32>()), Group(0_a), Binding(0_a),
                         ast::StorageClass::kStorage);
    auto* expr = IndexAccessor(sb, Expr(Source{{12, 34}}, -2_i));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 error: index -2 out of bounds");
}

TEST_F(ResolverConstEvalTest, ChainedIndex) {
    auto* arr_expr = Construct(ty.array(ty.mat2x3<f32>(), 2_u),        // array<mat2x3<f32>, 2u>
                               mat2x3<f32>(vec3<f32>(1_f, 2_f, 3_f),   //
                                           vec3<f32>(4_f, 5_f, 6_f)),  //
                               mat2x3<f32>(vec3<f32>(7_f, 0_f, 9_f),   //
                                           vec3<f32>(10_f, 11_f, 12_f)));

    auto* mat_expr = IndexAccessor(arr_expr, 1_i);  // arr[1]
    auto* vec_expr = IndexAccessor(mat_expr, 0_i);  // arr[1][0]
    auto* f32_expr = IndexAccessor(vec_expr, 2_i);  // arr[1][0][2]
    WrapInFunction(f32_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* mat = Sem().Get(mat_expr);
        EXPECT_NE(mat, nullptr);
        auto* ty = mat->Type()->As<sem::Matrix>();
        ASSERT_NE(mat->Type(), nullptr);
        EXPECT_TRUE(ty->ColumnType()->Is<sem::Vector>());
        EXPECT_EQ(ty->columns(), 2u);
        EXPECT_EQ(ty->rows(), 3u);
        EXPECT_EQ(mat->ConstantValue()->Type(), mat->Type());
        EXPECT_FALSE(mat->ConstantValue()->AllEqual());
        EXPECT_TRUE(mat->ConstantValue()->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->AllZero());

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(0)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(0)->As<f32>(), 7_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AllEqual());
        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AnyZero());
        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(1)->As<f32>(), 0_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(0)->Index(2)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(0)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(0)->Index(2)->As<f32>(), 9_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(0)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(0)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(0)->As<f32>(), 10_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(1)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(1)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(1)->As<f32>(), 11_f);

        EXPECT_TRUE(mat->ConstantValue()->Index(1)->Index(2)->AllEqual());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AnyZero());
        EXPECT_FALSE(mat->ConstantValue()->Index(1)->Index(2)->AllZero());
        EXPECT_EQ(mat->ConstantValue()->Index(1)->Index(2)->As<f32>(), 12_f);
    }
    {
        auto* vec = Sem().Get(vec_expr);
        EXPECT_NE(vec, nullptr);
        auto* ty = vec->Type()->As<sem::Vector>();
        ASSERT_NE(vec->Type(), nullptr);
        EXPECT_TRUE(ty->type()->Is<sem::F32>());
        EXPECT_EQ(ty->Width(), 3u);
        EXPECT_EQ(vec->ConstantValue()->Type(), vec->Type());
        EXPECT_FALSE(vec->ConstantValue()->AllEqual());
        EXPECT_TRUE(vec->ConstantValue()->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->AllZero());

        EXPECT_TRUE(vec->ConstantValue()->Index(0)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(0)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(0)->As<f32>(), 7_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AllEqual());
        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AnyZero());
        EXPECT_TRUE(vec->ConstantValue()->Index(1)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(1)->As<f32>(), 0_f);

        EXPECT_TRUE(vec->ConstantValue()->Index(2)->AllEqual());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AnyZero());
        EXPECT_FALSE(vec->ConstantValue()->Index(2)->AllZero());
        EXPECT_EQ(vec->ConstantValue()->Index(2)->As<f32>(), 9_f);
    }
    {
        auto* f = Sem().Get(f32_expr);
        EXPECT_NE(f, nullptr);
        EXPECT_TRUE(f->Type()->Is<sem::F32>());
        EXPECT_EQ(f->ConstantValue()->Type(), f->Type());
        EXPECT_TRUE(f->ConstantValue()->AllEqual());
        EXPECT_FALSE(f->ConstantValue()->AnyZero());
        EXPECT_FALSE(f->ConstantValue()->AllZero());
        EXPECT_EQ(f->ConstantValue()->As<f32>(), 9_f);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member accessing
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_F(ResolverConstEvalTest, MemberAccess) {
    Structure("Inner", utils::Vector{
                           Member("i1", ty.i32()),
                           Member("i2", ty.u32()),
                           Member("i3", ty.f32()),
                       });

    Structure("Outer", utils::Vector{
                           Member("o1", ty.type_name("Inner")),
                           Member("o2", ty.type_name("Inner")),
                       });
    auto* outer_expr = Construct(ty.type_name("Outer"),  //
                                 Construct(ty.type_name("Inner"), 1_i, 2_u, 3_f),
                                 Construct(ty.type_name("Inner")));
    auto* o1_expr = MemberAccessor(outer_expr, "o1");
    auto* i2_expr = MemberAccessor(o1_expr, "i2");
    WrapInFunction(i2_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* outer = Sem().Get(outer_expr);
    ASSERT_NE(outer, nullptr);
    auto* str = outer->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().size(), 2u);
    ASSERT_NE(outer->ConstantValue(), nullptr);
    EXPECT_TYPE(outer->ConstantValue()->Type(), outer->Type());
    EXPECT_FALSE(outer->ConstantValue()->AllEqual());
    EXPECT_TRUE(outer->ConstantValue()->AnyZero());
    EXPECT_FALSE(outer->ConstantValue()->AllZero());

    auto* o1 = Sem().Get(o1_expr);
    ASSERT_NE(o1->ConstantValue(), nullptr);
    EXPECT_FALSE(o1->ConstantValue()->AllEqual());
    EXPECT_FALSE(o1->ConstantValue()->AnyZero());
    EXPECT_FALSE(o1->ConstantValue()->AllZero());
    EXPECT_TRUE(o1->ConstantValue()->Type()->Is<sem::Struct>());
    EXPECT_EQ(o1->ConstantValue()->Index(0)->As<i32>(), 1_i);
    EXPECT_EQ(o1->ConstantValue()->Index(1)->As<u32>(), 2_u);
    EXPECT_EQ(o1->ConstantValue()->Index(2)->As<f32>(), 3_f);

    auto* i2 = Sem().Get(i2_expr);
    ASSERT_NE(i2->ConstantValue(), nullptr);
    EXPECT_TRUE(i2->ConstantValue()->AllEqual());
    EXPECT_FALSE(i2->ConstantValue()->AnyZero());
    EXPECT_FALSE(i2->ConstantValue()->AllZero());
    EXPECT_TRUE(i2->ConstantValue()->Type()->Is<sem::U32>());
    EXPECT_EQ(i2->ConstantValue()->As<u32>(), 2_u);
}

TEST_F(ResolverConstEvalTest, Matrix_AFloat_Construct_From_AInt_Vectors) {
    auto* c = Const("a", Construct(ty.mat(nullptr, 2, 2),  //
                                   Construct(ty.vec(nullptr, 2), Expr(1_a), Expr(2_a)),
                                   Construct(ty.vec(nullptr, 2), Expr(3_a), Expr(4_a))));
    WrapInFunction(c);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(c);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<sem::Matrix>());
    auto* cv = sem->ConstantValue();
    EXPECT_TYPE(cv->Type(), sem->Type());
    EXPECT_TRUE(cv->Index(0)->Type()->Is<sem::Vector>());
    EXPECT_TRUE(cv->Index(0)->Index(0)->Type()->Is<sem::AbstractFloat>());
    EXPECT_FALSE(cv->AllEqual());
    EXPECT_FALSE(cv->AnyZero());
    EXPECT_FALSE(cv->AllZero());
    auto* c0 = cv->Index(0);
    auto* c1 = cv->Index(1);
    EXPECT_EQ(std::get<AFloat>(c0->Index(0)->Value()), 1.0);
    EXPECT_EQ(std::get<AFloat>(c0->Index(1)->Value()), 2.0);
    EXPECT_EQ(std::get<AFloat>(c1->Index(0)->Value()), 3.0);
    EXPECT_EQ(std::get<AFloat>(c1->Index(1)->Value()), 4.0);
}

using builder::IsValue;
using builder::Mat;
using builder::Val;
using builder::Value;
using builder::Vec;

using Types = std::variant<  //
    Value<AInt>,
    Value<AFloat>,
    Value<u32>,
    Value<i32>,
    Value<f32>,
    Value<f16>,
    Value<bool>,

    Value<builder::vec2<AInt>>,
    Value<builder::vec2<AFloat>>,
    Value<builder::vec2<u32>>,
    Value<builder::vec2<i32>>,
    Value<builder::vec2<f32>>,
    Value<builder::vec2<f16>>,
    Value<builder::vec2<bool>>,

    Value<builder::vec3<AInt>>,
    Value<builder::vec3<AFloat>>,
    Value<builder::vec3<u32>>,
    Value<builder::vec3<i32>>,
    Value<builder::vec3<f32>>,
    Value<builder::vec3<f16>>,

    Value<builder::vec4<AInt>>,
    Value<builder::vec4<AFloat>>,
    Value<builder::vec4<u32>>,
    Value<builder::vec4<i32>>,
    Value<builder::vec4<f32>>,
    Value<builder::vec4<f16>>,

    Value<builder::mat2x2<AInt>>,
    Value<builder::mat2x2<AFloat>>,
    Value<builder::mat2x2<f32>>,
    Value<builder::mat2x2<f16>>,

    Value<builder::mat2x3<AInt>>,
    Value<builder::mat2x3<AFloat>>,
    Value<builder::mat2x3<f32>>,
    Value<builder::mat2x3<f16>>,

    Value<builder::mat3x2<AInt>>,
    Value<builder::mat3x2<AFloat>>,
    Value<builder::mat3x2<f32>>,
    Value<builder::mat3x2<f16>>
    //
    >;

std::ostream& operator<<(std::ostream& o, const Types& types) {
    std::visit(
        [&](auto&& v) {
            using ValueType = std::decay_t<decltype(v)>;
            o << ValueType::DataType::Name() << "(";
            for (auto& a : v.args.values) {
                o << std::get<typename ValueType::ElementType>(a);
                if (&a != &v.args.values.Back()) {
                    o << ", ";
                }
            }
            o << ")";
        },
        types);
    return o;
}

// Calls `f` on deepest elements of both `a` and `b`. If function returns Action::kStop, it stops
// traversing, and return Action::kStop; if the function returns Action::kContinue, it continues and
// returns Action::kContinue when done.
// TODO(amaiorano): Move to Constant.h?
enum class Action { kStop, kContinue };
template <typename Func>
Action ForEachElemPair(const sem::Constant* a, const sem::Constant* b, Func&& f) {
    EXPECT_EQ(a->Type(), b->Type());
    size_t i = 0;
    while (true) {
        auto* a_elem = a->Index(i);
        if (!a_elem) {
            break;
        }
        auto* b_elem = b->Index(i);
        if (ForEachElemPair(a_elem, b_elem, f) == Action::kStop) {
            return Action::kStop;
        }
        i++;
    }
    if (i == 0) {
        return f(a, b);
    }
    return Action::kContinue;
}

template <typename T>
struct BitValues {
    using UT = UnwrapNumber<T>;
    static constexpr size_t NumBits = sizeof(UT) * 8;
    static inline const T All = T{~T{0}};
    static inline const T LeftMost = T{T{1} << (NumBits - 1u)};
    static inline const T AllButLeftMost = T{~LeftMost};
    static inline const T RightMost = T{1};
    static inline const T AllButRightMost = T{~RightMost};
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unary op
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace unary_op {
using resolver::operator<<;

struct Case {
    Types input;
    Types expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "input: " << c.input << ", expected: " << c.expected;
    return o;
}

/// Creates a Case with Values of any type
template <typename T, typename U>
Case C(Value<T> input, Value<U> expected) {
    return Case{std::move(input), std::move(expected)};
}

/// Convenience overload to creates a Case with just scalars
template <typename T, typename U, typename = std::enable_if_t<!IsValue<T>>>
Case C(T input, U expected) {
    return Case{Val(input), Val(expected)};
}

using ResolverConstEvalUnaryOpTest = ResolverTestWithParam<std::tuple<ast::UnaryOp, Case>>;

TEST_P(ResolverConstEvalUnaryOpTest, Test) {
    Enable(ast::Extension::kF16);

    auto op = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());
    std::visit(
        [&](auto&& expected) {
            using T = typename std::decay_t<decltype(expected)>::ElementType;

            auto* input_expr = std::visit([&](auto&& value) { return value.Expr(*this); }, c.input);
            auto* expected_expr = create<ast::UnaryOpExpression>(op, input_expr);
            GlobalConst("C", expected_expr);

            EXPECT_TRUE(r()->Resolve()) << r()->error();

            auto* sem = Sem().Get(expected_expr);
            const sem::Constant* value = sem->ConstantValue();
            ASSERT_NE(value, nullptr);
            EXPECT_TYPE(value->Type(), sem->Type());

            auto* expected_sem = Sem().Get(expected_expr);
            const sem::Constant* expected_value = expected_sem->ConstantValue();
            ASSERT_NE(expected_value, nullptr);
            EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

            ForEachElemPair(value, expected_value,
                            [&](const sem::Constant* a, const sem::Constant* b) {
                                EXPECT_EQ(a->As<T>(), b->As<T>());
                                if constexpr (IsIntegral<UnwrapNumber<T>>) {
                                    // Check that the constant's integer doesn't contain unexpected
                                    // data in the MSBs that are outside of the bit-width of T.
                                    EXPECT_EQ(a->As<AInt>(), b->As<AInt>());
                                }
                                return HasFailure() ? Action::kStop : Action::kContinue;
                            });
        },
        c.expected);
}
INSTANTIATE_TEST_SUITE_P(Complement,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kComplement),
                                          testing::ValuesIn({
                                              // AInt
                                              C(0_a, 0xffffffffffffffff_a),
                                              C(0xffffffffffffffff_a, 0_a),
                                              C(0xf0f0f0f0f0f0f0f0_a, 0x0f0f0f0f0f0f0f0f_a),
                                              C(0xaaaaaaaaaaaaaaaa_a, 0x5555555555555555_a),
                                              C(0x5555555555555555_a, 0xaaaaaaaaaaaaaaaa_a),
                                              // u32
                                              C(0_u, 0xffffffff_u),
                                              C(0xffffffff_u, 0_u),
                                              C(0xf0f0f0f0_u, 0x0f0f0f0f_u),
                                              C(0xaaaaaaaa_u, 0x55555555_u),
                                              C(0x55555555_u, 0xaaaaaaaa_u),
                                              // i32
                                              C(0_i, -1_i),
                                              C(-1_i, 0_i),
                                              C(1_i, -2_i),
                                              C(-2_i, 1_i),
                                              C(2_i, -3_i),
                                              C(-3_i, 2_i),
                                          })));

INSTANTIATE_TEST_SUITE_P(Negation,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kNegation),
                                          testing::ValuesIn({
                                              // AInt
                                              C(0_a, -0_a),
                                              C(-0_a, 0_a),
                                              C(1_a, -1_a),
                                              C(-1_a, 1_a),
                                              C(AInt::Highest(), -AInt::Highest()),
                                              C(-AInt::Highest(), AInt::Highest()),
                                              C(AInt::Lowest(), Negate(AInt::Lowest())),
                                              C(Negate(AInt::Lowest()), AInt::Lowest()),
                                              // i32
                                              C(0_i, -0_i),
                                              C(-0_i, 0_i),
                                              C(1_i, -1_i),
                                              C(-1_i, 1_i),
                                              C(i32::Highest(), -i32::Highest()),
                                              C(-i32::Highest(), i32::Highest()),
                                              C(i32::Lowest(), Negate(i32::Lowest())),
                                              C(Negate(i32::Lowest()), i32::Lowest()),
                                              // AFloat
                                              C(0.0_a, -0.0_a),
                                              C(-0.0_a, 0.0_a),
                                              C(1.0_a, -1.0_a),
                                              C(-1.0_a, 1.0_a),
                                              C(AFloat::Highest(), -AFloat::Highest()),
                                              C(-AFloat::Highest(), AFloat::Highest()),
                                              C(AFloat::Lowest(), Negate(AFloat::Lowest())),
                                              C(Negate(AFloat::Lowest()), AFloat::Lowest()),
                                              // f32
                                              C(0.0_f, -0.0_f),
                                              C(-0.0_f, 0.0_f),
                                              C(1.0_f, -1.0_f),
                                              C(-1.0_f, 1.0_f),
                                              C(f32::Highest(), -f32::Highest()),
                                              C(-f32::Highest(), f32::Highest()),
                                              C(f32::Lowest(), Negate(f32::Lowest())),
                                              C(Negate(f32::Lowest()), f32::Lowest()),
                                              // f16
                                              C(0.0_h, -0.0_h),
                                              C(-0.0_h, 0.0_h),
                                              C(1.0_h, -1.0_h),
                                              C(-1.0_h, 1.0_h),
                                              C(f16::Highest(), -f16::Highest()),
                                              C(-f16::Highest(), f16::Highest()),
                                              C(f16::Lowest(), Negate(f16::Lowest())),
                                              C(Negate(f16::Lowest()), f16::Lowest()),
                                          })));

// Make sure UBSan doesn't trip on C++'s undefined behaviour of negating the smallest negative
// number.
TEST_F(ResolverConstEvalTest, UnaryNegateLowestAbstract) {
    // const break_me = -(-9223372036854775808);
    auto* c = GlobalConst("break_me", Negation(Negation(Expr(9223372036854775808_a))));
    (void)c;
    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(c);
    EXPECT_EQ(sem->ConstantValue()->As<AInt>(), 9223372036854775808_a);
}

INSTANTIATE_TEST_SUITE_P(Not,
                         ResolverConstEvalUnaryOpTest,
                         testing::Combine(testing::Values(ast::UnaryOp::kNot),
                                          testing::ValuesIn({
                                              C(true, false),
                                              C(false, true),
                                              C(Vec(true, true), Vec(false, false)),
                                              C(Vec(true, false), Vec(false, true)),
                                              C(Vec(false, true), Vec(true, false)),
                                              C(Vec(false, false), Vec(true, true)),
                                          })));

}  // namespace unary_op

////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary op
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace binary_op {
using resolver::operator<<;

struct Case {
    Types lhs;
    Types rhs;
    Types expected;
    bool overflow;
};

/// Creates a Case with Values of any type
template <typename T, typename U, typename V>
Case C(Value<T> lhs, Value<U> rhs, Value<V> expected, bool overflow = false) {
    return Case{std::move(lhs), std::move(rhs), std::move(expected), overflow};
}

/// Convenience overload to creates a Case with just scalars
template <typename T, typename U, typename V, typename = std::enable_if_t<!IsValue<T>>>
Case C(T lhs, U rhs, V expected, bool overflow = false) {
    return Case{Val(lhs), Val(rhs), Val(expected), overflow};
}

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "lhs: " << c.lhs << ", rhs: " << c.rhs << ", expected: " << c.expected
      << ", overflow: " << c.overflow;
    return o;
}

using ResolverConstEvalBinaryOpTest = ResolverTestWithParam<std::tuple<ast::BinaryOp, Case>>;
TEST_P(ResolverConstEvalBinaryOpTest, Test) {
    Enable(ast::Extension::kF16);
    auto op = std::get<0>(GetParam());
    auto& c = std::get<1>(GetParam());

    std::visit(
        [&](auto&& expected) {
            using T = typename std::decay_t<decltype(expected)>::ElementType;
            if constexpr (std::is_same_v<T, AInt> || std::is_same_v<T, AFloat>) {
                if (c.overflow) {
                    // Overflow is not allowed for abstract types. This is tested separately.
                    return;
                }
            }

            auto* lhs_expr = std::visit([&](auto&& value) { return value.Expr(*this); }, c.lhs);
            auto* rhs_expr = std::visit([&](auto&& value) { return value.Expr(*this); }, c.rhs);
            auto* expr = create<ast::BinaryExpression>(op, lhs_expr, rhs_expr);

            GlobalConst("C", expr);
            auto* expected_expr = expected.Expr(*this);
            GlobalConst("E", expected_expr);
            EXPECT_TRUE(r()->Resolve()) << r()->error();

            auto* sem = Sem().Get(expr);
            const sem::Constant* value = sem->ConstantValue();
            ASSERT_NE(value, nullptr);
            EXPECT_TYPE(value->Type(), sem->Type());

            auto* expected_sem = Sem().Get(expected_expr);
            const sem::Constant* expected_value = expected_sem->ConstantValue();
            ASSERT_NE(expected_value, nullptr);
            EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

            ForEachElemPair(value, expected_value,
                            [&](const sem::Constant* a, const sem::Constant* b) {
                                EXPECT_EQ(a->As<T>(), b->As<T>());
                                if constexpr (IsIntegral<UnwrapNumber<T>>) {
                                    // Check that the constant's integer doesn't contain unexpected
                                    // data in the MSBs that are outside of the bit-width of T.
                                    EXPECT_EQ(a->As<AInt>(), b->As<AInt>());
                                }
                                return HasFailure() ? Action::kStop : Action::kContinue;
                            });
        },
        c.expected);
}

INSTANTIATE_TEST_SUITE_P(MixedAbstractArgs,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(ast::BinaryOp::kAdd),
                                          testing::ValuesIn(std::vector{
                                              // Mixed abstract type args
                                              C(1_a, 2.3_a, 3.3_a),
                                              C(2.3_a, 1_a, 3.3_a),
                                          })));

template <typename T>
std::vector<Case> OpAddIntCases() {
    static_assert(IsIntegral<UnwrapNumber<T>>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{3}),
        C(T::Lowest(), T{1}, T{T::Lowest() + 1}),
        C(T::Highest(), Negate(T{1}), T{T::Highest() - 1}),
        C(T::Lowest(), T::Highest(), Negate(T{1})),
        C(T::Highest(), T{1}, T::Lowest(), true),
        C(T::Lowest(), Negate(T{1}), T::Highest(), true),
    };
}
template <typename T>
std::vector<Case> OpAddFloatCases() {
    static_assert(IsFloatingPoint<UnwrapNumber<T>>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{3}),
        C(T::Lowest(), T{1}, T{T::Lowest() + 1}),
        C(T::Highest(), Negate(T{1}), T{T::Highest() - 1}),
        C(T::Lowest(), T::Highest(), T{0}),
        C(T::Highest(), T::Highest(), T::Inf(), true),
        C(T::Lowest(), Negate(T::Highest()), -T::Inf(), true),
    };
}
INSTANTIATE_TEST_SUITE_P(Add,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(ast::BinaryOp::kAdd),
                                          testing::ValuesIn(Concat(  //
                                              OpAddIntCases<AInt>(),
                                              OpAddIntCases<i32>(),
                                              OpAddIntCases<u32>(),
                                              OpAddFloatCases<AFloat>(),
                                              OpAddFloatCases<f32>(),
                                              OpAddFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpSubIntCases() {
    static_assert(IsIntegral<UnwrapNumber<T>>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{3}, T{2}, T{1}),
        C(T{T::Lowest() + 1}, T{1}, T::Lowest()),
        C(T{T::Highest() - 1}, Negate(T{1}), T::Highest()),
        C(Negate(T{1}), T::Highest(), T::Lowest()),
        C(T::Lowest(), T{1}, T::Highest(), true),
        C(T::Highest(), Negate(T{1}), T::Lowest(), true),
    };
}
template <typename T>
std::vector<Case> OpSubFloatCases() {
    static_assert(IsFloatingPoint<UnwrapNumber<T>>);
    return {
        C(T{0}, T{0}, T{0}),
        C(T{3}, T{2}, T{1}),
        C(T::Highest(), T{1}, T{T::Highest() - 1}),
        C(T::Lowest(), Negate(T{1}), T{T::Lowest() + 1}),
        C(T{0}, T::Highest(), T::Lowest()),
        C(T::Highest(), Negate(T::Highest()), T::Inf(), true),
        C(T::Lowest(), T::Highest(), -T::Inf(), true),
    };
}
INSTANTIATE_TEST_SUITE_P(Sub,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(testing::Values(ast::BinaryOp::kSubtract),
                                          testing::ValuesIn(Concat(  //
                                              OpSubIntCases<AInt>(),
                                              OpSubIntCases<i32>(),
                                              OpSubIntCases<u32>(),
                                              OpSubFloatCases<AFloat>(),
                                              OpSubFloatCases<f32>(),
                                              OpSubFloatCases<f16>()))));

template <typename T>
std::vector<Case> OpMulScalarCases() {
    return {
        C(T{0}, T{0}, T{0}),
        C(T{1}, T{2}, T{2}),
        C(T{2}, T{3}, T{6}),
        C(Negate(T{2}), T{3}, Negate(T{6})),
        C(T::Highest(), T{1}, T::Highest()),
        C(T::Lowest(), T{1}, T::Lowest()),
        C(T::Highest(), T::Highest(), Mul(T::Highest(), T::Highest()), true),
        C(T::Lowest(), T::Lowest(), Mul(T::Lowest(), T::Lowest()), true),
    };
}

template <typename T>
std::vector<Case> OpMulVecCases() {
    return {
        // s * vec3 = vec3
        C(Val(T{2.0}), Vec(T{1.25}, T{2.25}, T{3.25}), Vec(T{2.5}, T{4.5}, T{6.5})),
        // vec3 * s = vec3
        C(Vec(T{1.25}, T{2.25}, T{3.25}), Val(T{2.0}), Vec(T{2.5}, T{4.5}, T{6.5})),
        // vec3 * vec3 = vec3
        C(Vec(T{1.25}, T{2.25}, T{3.25}), Vec(T{2.0}, T{2.0}, T{2.0}), Vec(T{2.5}, T{4.5}, T{6.5})),
    };
}

template <typename T>
std::vector<Case> OpMulMatCases() {
    return {
        // s * mat3x2 = mat3x2
        C(Val(T{2.25}),
          Mat({T{1.0}, T{4.0}},  //
              {T{2.0}, T{5.0}},  //
              {T{3.0}, T{6.0}}),
          Mat({T{2.25}, T{9.0}},   //
              {T{4.5}, T{11.25}},  //
              {T{6.75}, T{13.5}})),
        // mat3x2 * s = mat3x2
        C(Mat({T{1.0}, T{4.0}},  //
              {T{2.0}, T{5.0}},  //
              {T{3.0}, T{6.0}}),
          Val(T{2.25}),
          Mat({T{2.25}, T{9.0}},   //
              {T{4.5}, T{11.25}},  //
              {T{6.75}, T{13.5}})),
        // vec3 * mat2x3 = vec2
        C(Vec(T{1.25}, T{2.25}, T{3.25}),  //
          Mat({T{1.0}, T{2.0}, T{3.0}},    //
              {T{4.0}, T{5.0}, T{6.0}}),   //
          Vec(T{15.5}, T{35.75})),
        // mat2x3 * vec2 = vec3
        C(Mat({T{1.0}, T{2.0}, T{3.0}},   //
              {T{4.0}, T{5.0}, T{6.0}}),  //
          Vec(T{1.25}, T{2.25}),          //
          Vec(T{10.25}, T{13.75}, T{17.25})),
        // mat3x2 * mat2x3 = mat2x2
        C(Mat({T{1.0}, T{2.0}},              //
              {T{3.0}, T{4.0}},              //
              {T{5.0}, T{6.0}}),             //
          Mat({T{1.25}, T{2.25}, T{3.25}},   //
              {T{4.25}, T{5.25}, T{6.25}}),  //
          Mat({T{24.25}, T{31.0}},           //
              {T{51.25}, T{67.0}})),         //
    };
}

INSTANTIATE_TEST_SUITE_P(Mul,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kMultiply),
                             testing::ValuesIn(Concat(  //
                                 OpMulScalarCases<AInt>(),
                                 OpMulScalarCases<i32>(),
                                 OpMulScalarCases<u32>(),
                                 OpMulScalarCases<AFloat>(),
                                 OpMulScalarCases<f32>(),
                                 OpMulScalarCases<f16>(),
                                 OpMulVecCases<AInt>(),
                                 OpMulVecCases<i32>(),
                                 OpMulVecCases<u32>(),
                                 OpMulVecCases<AFloat>(),
                                 OpMulVecCases<f32>(),
                                 OpMulVecCases<f16>(),
                                 OpMulMatCases<AFloat>(),
                                 OpMulMatCases<f32>(),
                                 OpMulMatCases<f16>()))));

template <typename T>
std::vector<Case> OpDivIntCases() {
    std::vector<Case> r = {
        C(Val(T{0}), Val(T{1}), Val(T{0})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{2}), Val(T{1}), Val(T{2})),
        C(Val(T{4}), Val(T{2}), Val(T{2})),
        C(Val(T::Highest()), Val(T{1}), Val(T::Highest())),
        C(Val(T::Lowest()), Val(T{1}), Val(T::Lowest())),
        C(Val(T::Highest()), Val(T::Highest()), Val(T{1})),
        C(Val(T{0}), Val(T::Highest()), Val(T{0})),
        C(Val(T{0}), Val(T::Lowest()), Val(T{0})),
    };
    ConcatIntoIf<IsIntegral<T>>(  //
        r, std::vector<Case>{
               // e1, when e2 is zero.
               C(T{123}, T{0}, T{123}, true),
           });
    ConcatIntoIf<IsSignedIntegral<T>>(  //
        r, std::vector<Case>{
               // e1, when e1 is the most negative value in T, and e2 is -1.
               C(T::Smallest(), T{-1}, T::Smallest(), true),
           });
    return r;
}

template <typename T>
std::vector<Case> OpDivFloatCases() {
    return {
        C(Val(T{0}), Val(T{1}), Val(T{0})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{1}), Val(T{1}), Val(T{1})),
        C(Val(T{2}), Val(T{1}), Val(T{2})),
        C(Val(T{4}), Val(T{2}), Val(T{2})),
        C(Val(T::Highest()), Val(T{1}), Val(T::Highest())),
        C(Val(T::Lowest()), Val(T{1}), Val(T::Lowest())),
        C(Val(T::Highest()), Val(T::Highest()), Val(T{1})),
        C(Val(T{0}), Val(T::Highest()), Val(T{0})),
        C(Val(T{0}), Val(T::Lowest()), Val(-T{0})),
        C(T{123}, T{0}, T::Inf(), true),
        C(T{-123}, -T{0}, T::Inf(), true),
        C(T{-123}, T{0}, -T::Inf(), true),
        C(T{123}, -T{0}, -T::Inf(), true),
    };
}
INSTANTIATE_TEST_SUITE_P(Div,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kDivide),
                             testing::ValuesIn(Concat(  //
                                 OpDivIntCases<AInt>(),
                                 OpDivIntCases<i32>(),
                                 OpDivIntCases<u32>(),
                                 OpDivFloatCases<AFloat>(),
                                 OpDivFloatCases<f32>(),
                                 OpDivFloatCases<f16>()))));

template <typename T, bool equals>
std::vector<Case> OpEqualCases() {
    return {
        C(Val(T{0}), Val(T{0}), Val(true == equals)),
        C(Val(T{0}), Val(T{1}), Val(false == equals)),
        C(Val(T{1}), Val(T{0}), Val(false == equals)),
        C(Val(T{1}), Val(T{1}), Val(true == equals)),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(true == equals, true == equals)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(false == equals, false == equals)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{1}), Vec(false == equals, true == equals)),
    };
}
INSTANTIATE_TEST_SUITE_P(Equal,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kEqual),
                             testing::ValuesIn(Concat(  //
                                 OpEqualCases<AInt, true>(),
                                 OpEqualCases<i32, true>(),
                                 OpEqualCases<u32, true>(),
                                 OpEqualCases<AFloat, true>(),
                                 OpEqualCases<f32, true>(),
                                 OpEqualCases<f16, true>(),
                                 OpEqualCases<bool, true>()))));
INSTANTIATE_TEST_SUITE_P(NotEqual,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kNotEqual),
                             testing::ValuesIn(Concat(  //
                                 OpEqualCases<AInt, false>(),
                                 OpEqualCases<i32, false>(),
                                 OpEqualCases<u32, false>(),
                                 OpEqualCases<AFloat, false>(),
                                 OpEqualCases<f32, false>(),
                                 OpEqualCases<f16, false>(),
                                 OpEqualCases<bool, false>()))));

template <typename T, bool less_than>
std::vector<Case> OpLessThanCases() {
    return {
        C(Val(T{0}), Val(T{0}), Val(false == less_than)),
        C(Val(T{0}), Val(T{1}), Val(true == less_than)),
        C(Val(T{1}), Val(T{0}), Val(false == less_than)),
        C(Val(T{1}), Val(T{1}), Val(false == less_than)),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(false == less_than, false == less_than)),
        C(Vec(T{0}, T{0}), Vec(T{1}, T{1}), Vec(true == less_than, true == less_than)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{0}), Vec(false == less_than, false == less_than)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(false == less_than, true == less_than)),
    };
}
INSTANTIATE_TEST_SUITE_P(LessThan,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kLessThan),
                             testing::ValuesIn(Concat(  //
                                 OpLessThanCases<AInt, true>(),
                                 OpLessThanCases<i32, true>(),
                                 OpLessThanCases<u32, true>(),
                                 OpLessThanCases<AFloat, true>(),
                                 OpLessThanCases<f32, true>(),
                                 OpLessThanCases<f16, true>()))));
INSTANTIATE_TEST_SUITE_P(GreaterThanEqual,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kGreaterThanEqual),
                             testing::ValuesIn(Concat(  //
                                 OpLessThanCases<AInt, false>(),
                                 OpLessThanCases<i32, false>(),
                                 OpLessThanCases<u32, false>(),
                                 OpLessThanCases<AFloat, false>(),
                                 OpLessThanCases<f32, false>(),
                                 OpLessThanCases<f16, false>()))));

template <typename T, bool greater_than>
std::vector<Case> OpGreaterThanCases() {
    return {
        C(Val(T{0}), Val(T{0}), Val(false == greater_than)),
        C(Val(T{0}), Val(T{1}), Val(false == greater_than)),
        C(Val(T{1}), Val(T{0}), Val(true == greater_than)),
        C(Val(T{1}), Val(T{1}), Val(false == greater_than)),
        C(Vec(T{0}, T{0}), Vec(T{0}, T{0}), Vec(false == greater_than, false == greater_than)),
        C(Vec(T{1}, T{1}), Vec(T{0}, T{0}), Vec(true == greater_than, true == greater_than)),
        C(Vec(T{0}, T{0}), Vec(T{1}, T{1}), Vec(false == greater_than, false == greater_than)),
        C(Vec(T{1}, T{0}), Vec(T{0}, T{1}), Vec(true == greater_than, false == greater_than)),
    };
}
INSTANTIATE_TEST_SUITE_P(GreaterThan,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kGreaterThan),
                             testing::ValuesIn(Concat(  //
                                 OpGreaterThanCases<AInt, true>(),
                                 OpGreaterThanCases<i32, true>(),
                                 OpGreaterThanCases<u32, true>(),
                                 OpGreaterThanCases<AFloat, true>(),
                                 OpGreaterThanCases<f32, true>(),
                                 OpGreaterThanCases<f16, true>()))));
INSTANTIATE_TEST_SUITE_P(LessThanEqual,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kLessThanEqual),
                             testing::ValuesIn(Concat(  //
                                 OpGreaterThanCases<AInt, false>(),
                                 OpGreaterThanCases<i32, false>(),
                                 OpGreaterThanCases<u32, false>(),
                                 OpGreaterThanCases<AFloat, false>(),
                                 OpGreaterThanCases<f32, false>(),
                                 OpGreaterThanCases<f16, false>()))));

static std::vector<Case> OpAndBoolCases() {
    return {
        C(true, true, true),
        C(true, false, false),
        C(false, true, false),
        C(false, false, false),
        C(Vec(true, true), Vec(true, false), Vec(true, false)),
        C(Vec(true, true), Vec(false, true), Vec(false, true)),
        C(Vec(true, false), Vec(true, false), Vec(true, false)),
        C(Vec(false, true), Vec(true, false), Vec(false, false)),
        C(Vec(false, false), Vec(true, false), Vec(false, false)),
    };
}
template <typename T>
std::vector<Case> OpAndIntCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b1010}),
        C(T{0b1010}, T{0b0000}, T{0b0000}),
        C(T{0b1010}, T{0b0011}, T{0b0010}),
        C(T{0b1010}, T{0b1100}, T{0b1000}),
        C(T{0b1010}, T{0b0101}, T{0b0000}),
        C(B::All, B::All, B::All),
        C(B::LeftMost, B::LeftMost, B::LeftMost),
        C(B::RightMost, B::RightMost, B::RightMost),
        C(B::All, T{0}, T{0}),
        C(T{0}, B::All, T{0}),
        C(B::LeftMost, B::AllButLeftMost, T{0}),
        C(B::AllButLeftMost, B::LeftMost, T{0}),
        C(B::RightMost, B::AllButRightMost, T{0}),
        C(B::AllButRightMost, B::RightMost, T{0}),
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(B::All, B::All, B::All),                 //
          Vec(B::All, B::LeftMost, B::RightMost)),     //
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(T{0}, T{0}, T{0}),                       //
          Vec(T{0}, T{0}, T{0})),                      //
        C(Vec(B::LeftMost, B::RightMost),              //
          Vec(B::AllButLeftMost, B::AllButRightMost),  //
          Vec(T{0}, T{0})),
    };
}
INSTANTIATE_TEST_SUITE_P(And,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kAnd),
                             testing::ValuesIn(            //
                                 Concat(OpAndBoolCases(),  //
                                        OpAndIntCases<AInt>(),
                                        OpAndIntCases<i32>(),
                                        OpAndIntCases<u32>()))));

static std::vector<Case> OpOrBoolCases() {
    return {
        C(true, true, true),
        C(true, false, true),
        C(false, true, true),
        C(false, false, false),
        C(Vec(true, true), Vec(true, false), Vec(true, true)),
        C(Vec(true, true), Vec(false, true), Vec(true, true)),
        C(Vec(true, false), Vec(true, false), Vec(true, false)),
        C(Vec(false, true), Vec(true, false), Vec(true, true)),
        C(Vec(false, false), Vec(true, false), Vec(true, false)),
    };
}
template <typename T>
std::vector<Case> OpOrIntCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b1111}),
        C(T{0b1010}, T{0b0000}, T{0b1010}),
        C(T{0b1010}, T{0b0011}, T{0b1011}),
        C(T{0b1010}, T{0b1100}, T{0b1110}),
        C(T{0b1010}, T{0b0101}, T{0b1111}),
        C(B::All, B::All, B::All),
        C(B::LeftMost, B::LeftMost, B::LeftMost),
        C(B::RightMost, B::RightMost, B::RightMost),
        C(B::All, T{0}, B::All),
        C(T{0}, B::All, B::All),
        C(B::LeftMost, B::AllButLeftMost, B::All),
        C(B::AllButLeftMost, B::LeftMost, B::All),
        C(B::RightMost, B::AllButRightMost, B::All),
        C(B::AllButRightMost, B::RightMost, B::All),
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(B::All, B::All, B::All),                 //
          Vec(B::All, B::All, B::All)),                //
        C(Vec(B::All, B::LeftMost, B::RightMost),      //
          Vec(T{0}, T{0}, T{0}),                       //
          Vec(B::All, B::LeftMost, B::RightMost)),     //
        C(Vec(B::LeftMost, B::RightMost),              //
          Vec(B::AllButLeftMost, B::AllButRightMost),  //
          Vec(B::All, B::All)),
    };
}
INSTANTIATE_TEST_SUITE_P(Or,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kOr),
                             testing::ValuesIn(Concat(OpOrBoolCases(),
                                                      OpOrIntCases<AInt>(),
                                                      OpOrIntCases<i32>(),
                                                      OpOrIntCases<u32>()))));

TEST_F(ResolverConstEvalTest, NotAndOrOfVecs) {
    // const C = !((vec2(true, true) & vec2(true, false)) | vec2(false, true));
    auto v1 = Vec(true, true).Expr(*this);
    auto v2 = Vec(true, false).Expr(*this);
    auto v3 = Vec(false, true).Expr(*this);
    auto expr = Not(Or(And(v1, v2), v3));
    GlobalConst("C", expr);
    auto expected_expr = Vec(false, false).Expr(*this);
    GlobalConst("E", expected_expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(expr);
    const sem::Constant* value = sem->ConstantValue();
    ASSERT_NE(value, nullptr);
    EXPECT_TYPE(value->Type(), sem->Type());

    auto* expected_sem = Sem().Get(expected_expr);
    const sem::Constant* expected_value = expected_sem->ConstantValue();
    ASSERT_NE(expected_value, nullptr);
    EXPECT_TYPE(expected_value->Type(), expected_sem->Type());

    ForEachElemPair(value, expected_value, [&](const sem::Constant* a, const sem::Constant* b) {
        EXPECT_EQ(a->As<bool>(), b->As<bool>());
        return HasFailure() ? Action::kStop : Action::kContinue;
    });
}

template <typename T>
std::vector<Case> XorCases() {
    using B = BitValues<T>;
    return {
        C(T{0b1010}, T{0b1111}, T{0b0101}),
        C(T{0b1010}, T{0b0000}, T{0b1010}),
        C(T{0b1010}, T{0b0011}, T{0b1001}),
        C(T{0b1010}, T{0b1100}, T{0b0110}),
        C(T{0b1010}, T{0b0101}, T{0b1111}),
        C(B::All, B::All, T{0}),
        C(B::LeftMost, B::LeftMost, T{0}),
        C(B::RightMost, B::RightMost, T{0}),
        C(B::All, T{0}, B::All),
        C(T{0}, B::All, B::All),
        C(B::LeftMost, B::AllButLeftMost, B::All),
        C(B::AllButLeftMost, B::LeftMost, B::All),
        C(B::RightMost, B::AllButRightMost, B::All),
        C(B::AllButRightMost, B::RightMost, B::All),
        C(Vec(B::All, B::LeftMost, B::RightMost),             //
          Vec(B::All, B::All, B::All),                        //
          Vec(T{0}, B::AllButLeftMost, B::AllButRightMost)),  //
        C(Vec(B::All, B::LeftMost, B::RightMost),             //
          Vec(T{0}, T{0}, T{0}),                              //
          Vec(B::All, B::LeftMost, B::RightMost)),            //
        C(Vec(B::LeftMost, B::RightMost),                     //
          Vec(B::AllButLeftMost, B::AllButRightMost),         //
          Vec(B::All, B::All)),
    };
}
INSTANTIATE_TEST_SUITE_P(Xor,
                         ResolverConstEvalBinaryOpTest,
                         testing::Combine(  //
                             testing::Values(ast::BinaryOp::kXor),
                             testing::ValuesIn(Concat(XorCases<AInt>(),  //
                                                      XorCases<i32>(),   //
                                                      XorCases<u32>()))));

// Tests for errors on overflow/underflow of binary operations with abstract numbers
struct OverflowCase {
    ast::BinaryOp op;
    Types lhs;
    Types rhs;
};

static std::ostream& operator<<(std::ostream& o, const OverflowCase& c) {
    o << ast::FriendlyName(c.op) << ", lhs: " << c.lhs << ", rhs: " << c.rhs;
    return o;
}
using ResolverConstEvalBinaryOpTest_Overflow = ResolverTestWithParam<OverflowCase>;
TEST_P(ResolverConstEvalBinaryOpTest_Overflow, Test) {
    Enable(ast::Extension::kF16);
    auto& c = GetParam();
    auto* lhs_expr = std::visit([&](auto&& value) { return value.Expr(*this); }, c.lhs);
    auto* rhs_expr = std::visit([&](auto&& value) { return value.Expr(*this); }, c.rhs);
    auto* expr = create<ast::BinaryExpression>(Source{{1, 1}}, c.op, lhs_expr, rhs_expr);
    GlobalConst("C", expr);
    ASSERT_FALSE(r()->Resolve());

    std::string type_name = std::visit(
        [&](auto&& value) {
            using ValueType = std::decay_t<decltype(value)>;
            return builder::FriendlyName<ValueType>();
        },
        c.lhs);

    EXPECT_THAT(r()->error(), HasSubstr("1:1 error: '"));
    EXPECT_THAT(r()->error(), HasSubstr("' cannot be represented as '" + type_name + "'"));
}
INSTANTIATE_TEST_SUITE_P(
    Test,
    ResolverConstEvalBinaryOpTest_Overflow,
    testing::Values(

        // scalar-scalar add
        OverflowCase{ast::BinaryOp::kAdd, Val(AInt::Highest()), Val(1_a)},
        OverflowCase{ast::BinaryOp::kAdd, Val(AInt::Lowest()), Val(-1_a)},
        OverflowCase{ast::BinaryOp::kAdd, Val(AFloat::Highest()), Val(AFloat::Highest())},
        OverflowCase{ast::BinaryOp::kAdd, Val(AFloat::Lowest()), Val(AFloat::Lowest())},
        // scalar-scalar subtract
        OverflowCase{ast::BinaryOp::kSubtract, Val(AInt::Lowest()), Val(1_a)},
        OverflowCase{ast::BinaryOp::kSubtract, Val(AInt::Highest()), Val(-1_a)},
        OverflowCase{ast::BinaryOp::kSubtract, Val(AFloat::Highest()), Val(AFloat::Lowest())},
        OverflowCase{ast::BinaryOp::kSubtract, Val(AFloat::Lowest()), Val(AFloat::Highest())},

        // scalar-scalar multiply
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Highest()), Val(2_a)},
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Lowest()), Val(-2_a)},

        // scalar-vector multiply
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Highest()), Vec(2_a, 1_a)},
        OverflowCase{ast::BinaryOp::kMultiply, Val(AInt::Lowest()), Vec(-2_a, 1_a)},

        // vector-matrix multiply

        // Overflow from first multiplication of dot product of vector and matrix column 0
        // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
        //            ^
        OverflowCase{ast::BinaryOp::kMultiply,       //
                     Vec(AFloat::Highest(), 1.0_a),  //
                     Mat({2.0_a, 1.0_a},             //
                         {1.0_a, 1.0_a})},

        // Overflow from second multiplication of dot product of vector and matrix column 0
        // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
        //                             ^
        OverflowCase{ast::BinaryOp::kMultiply,       //
                     Vec(1.0_a, AFloat::Highest()),  //
                     Mat({1.0_a, 2.0_a},             //
                         {1.0_a, 1.0_a})},

        // Overflow from addition of dot product of vector and matrix column 0
        // i.e. (v[0] * m[0][0] + v[1] * m[0][1])
        //                      ^
        OverflowCase{ast::BinaryOp::kMultiply,                   //
                     Vec(AFloat::Highest(), AFloat::Highest()),  //
                     Mat({1.0_a, 1.0_a},                         //
                         {1.0_a, 1.0_a})},

        // matrix-matrix multiply

        // Overflow from first multiplication of dot product of lhs row 0 and rhs column 0
        // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
        //               ^
        OverflowCase{ast::BinaryOp::kMultiply,        //
                     Mat({AFloat::Highest(), 1.0_a},  //
                         {1.0_a, 1.0_a}),             //
                     Mat({2.0_a, 1.0_a},              //
                         {1.0_a, 1.0_a})},

        // Overflow from second multiplication of dot product of lhs row 0 and rhs column 0
        // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
        //                                     ^
        OverflowCase{ast::BinaryOp::kMultiply,        //
                     Mat({1.0_a, AFloat::Highest()},  //
                         {1.0_a, 1.0_a}),             //
                     Mat({1.0_a, 1.0_a},              //
                         {2.0_a, 1.0_a})},

        // Overflow from addition of dot product of lhs row 0 and rhs column 0
        // i.e. m1[0][0] * m2[0][0] + m1[0][1] * m[1][0]
        //                          ^
        OverflowCase{ast::BinaryOp::kMultiply,         //
                     Mat({AFloat::Highest(), 1.0_a},   //
                         {AFloat::Highest(), 1.0_a}),  //
                     Mat({1.0_a, 1.0_a},               //
                         {1.0_a, 1.0_a})},

        // Divide by zero
        OverflowCase{ast::BinaryOp::kDivide, Val(123_a), Val(0_a)},
        OverflowCase{ast::BinaryOp::kDivide, Val(-123_a), Val(-0_a)},
        OverflowCase{ast::BinaryOp::kDivide, Val(-123_a), Val(0_a)},
        OverflowCase{ast::BinaryOp::kDivide, Val(123_a), Val(-0_a)},

        // Most negative value divided by -1
        OverflowCase{ast::BinaryOp::kDivide, Val(AInt::Lowest()), Val(-1_a)}

        ));

TEST_F(ResolverConstEvalTest, BinaryAbstractAddOverflow_AInt) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AInt::Highest()), 1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '9223372036854775807 + 1' cannot be represented as 'abstract-int'");
}

TEST_F(ResolverConstEvalTest, BinaryAbstractAddUnderflow_AInt) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AInt::Lowest()), -1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '-9223372036854775808 + -1' cannot be represented as 'abstract-int'");
}

TEST_F(ResolverConstEvalTest, BinaryAbstractAddOverflow_AFloat) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AFloat::Highest()), AFloat::Highest()));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:1 error: '1.79769e+308 + 1.79769e+308' cannot be represented as 'abstract-float'");
}

TEST_F(ResolverConstEvalTest, BinaryAbstractAddUnderflow_AFloat) {
    GlobalConst("c", Add(Source{{1, 1}}, Expr(AFloat::Lowest()), AFloat::Lowest()));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "1:1 error: '-1.79769e+308 + -1.79769e+308' cannot be represented as 'abstract-float'");
}

// Mixed AInt and AFloat args to test implicit conversion to AFloat
INSTANTIATE_TEST_SUITE_P(
    AbstractMixed,
    ResolverConstEvalBinaryOpTest,
    testing::Combine(
        testing::Values(ast::BinaryOp::kAdd),
        testing::Values(C(Val(1_a), Val(2.3_a), Val(3.3_a)),
                        C(Val(2.3_a), Val(1_a), Val(3.3_a)),
                        C(Val(1_a), Vec(2.3_a, 2.3_a, 2.3_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Vec(2.3_a, 2.3_a, 2.3_a), Val(1_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Vec(2.3_a, 2.3_a, 2.3_a), Val(1_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Val(1_a), Vec(2.3_a, 2.3_a, 2.3_a), Vec(3.3_a, 3.3_a, 3.3_a)),
                        C(Mat({1_a, 2_a},        //
                              {1_a, 2_a},        //
                              {1_a, 2_a}),       //
                          Mat({1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a}),   //
                          Mat({2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a})),  //
                        C(Mat({1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a},    //
                              {1.2_a, 2.3_a}),   //
                          Mat({1_a, 2_a},        //
                              {1_a, 2_a},        //
                              {1_a, 2_a}),       //
                          Mat({2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a},    //
                              {2.2_a, 4.3_a}))   //
                        )));
}  // namespace binary_op

////////////////////////////////////////////////////////////////////////////////////////////////////
// Builtin
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace builtin {

using Types = std::variant<AInt, AFloat, u32, i32, f32, f16>;

struct Case {
    utils::Vector<Types, 8> args;
    Types result;
    bool result_pos_or_neg;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    for (auto& a : c.args) {
        std::visit([&](auto&& v) { o << v << ((&a != &c.args.Back()) ? " " : ""); }, a);
    }
    return o;
}

template <typename T>
Case C(std::initializer_list<Types> args, T result, bool result_pos_or_neg = false) {
    return Case{std::move(args), std::move(result), result_pos_or_neg};
}

using ResolverConstEvalBuiltinTest = ResolverTestWithParam<std::tuple<sem::BuiltinType, Case>>;

TEST_P(ResolverConstEvalBuiltinTest, Test) {
    Enable(ast::Extension::kF16);

    auto builtin = std::get<0>(GetParam());
    auto c = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> args;
    for (auto& a : c.args) {
        std::visit([&](auto&& v) { args.Push(Expr(v)); }, a);
    }

    std::visit(
        [&](auto&& result) {
            using T = std::decay_t<decltype(result)>;
            auto* expr = Call(sem::str(builtin), std::move(args));

            GlobalConst("C", expr);

            EXPECT_TRUE(r()->Resolve()) << r()->error();

            auto* sem = Sem().Get(expr);
            const sem::Constant* value = sem->ConstantValue();
            ASSERT_NE(value, nullptr);
            EXPECT_TYPE(value->Type(), sem->Type());

            auto actual = value->As<T>();

            if constexpr (IsFloatingPoint<UnwrapNumber<T>>) {
                if (std::isnan(result)) {
                    EXPECT_TRUE(std::isnan(actual));
                } else {
                    EXPECT_FLOAT_EQ(c.result_pos_or_neg ? Abs(actual) : actual, result);
                }
            } else {
                EXPECT_EQ(c.result_pos_or_neg ? Abs(actual) : actual, result);
            }

            if constexpr (IsIntegral<UnwrapNumber<T>>) {
                // Check that the constant's integer doesn't contain unexpected data in the MSBs
                // that are outside of the bit-width of T.
                EXPECT_EQ(value->As<AInt>(), AInt(result));
            }
        },
        c.result);
}

template <typename T, bool finite_only>
std::vector<Case> Atan2Cases() {
    std::vector<Case> cases = {
        // If y is +/-0 and x is negative or -0, +/-PI is returned
        C({T(0.0), -T(0.0)}, kPi<T>, true),

        // If y is +/-0 and x is positive or +0, +/-0 is returned
        C({T(0.0), T(0.0)}, T(0.0), true),

        // If x is +/-0 and y is negative, -PI/2 is returned
        C({-T(1.0), T(0.0)}, -kPiOver2<T>),
        C({-T(1.0), -T(0.0)}, -kPiOver2<T>),

        // If x is +/-0 and y is positive, +PI/2 is returned
        C({T(1.0), T(0.0)}, kPiOver2<T>),
        C({T(1.0), -T(0.0)}, kPiOver2<T>),
    };

    if constexpr (!finite_only) {
        std::vector<Case> non_finite_cases = {
            // If y is +/-INF and x is finite, +/-PI/2 is returned
            C({T::Inf(), T(0.0)}, kPiOver2<T>, true),
            C({-T::Inf(), T(0.0)}, kPiOver2<T>, true),

            // If y is +/-INF and x is -INF, +/-3PI/4 is returned
            C({T::Inf(), -T::Inf()}, k3PiOver4<T>, true),
            C({-T::Inf(), -T::Inf()}, k3PiOver4<T>, true),

            // If y is +/-INF and x is +INF, +/-PI/4 is returned
            C({T::Inf(), T::Inf()}, kPiOver4<T>, true),
            C({-T::Inf(), T::Inf()}, kPiOver4<T>, true),

            // If x is -INF and y is finite and positive, +PI is returned
            C({T(0.0), -T::Inf()}, kPi<T>),

            // If x is -INF and y is finite and negative, -PI is returned
            C({-T(0.0), -T::Inf()}, -kPi<T>),

            // If x is +INF and y is finite and positive, +0 is returned
            C({T(0.0), T::Inf()}, T(0.0)),

            // If x is +INF and y is finite and negative, -0 is returned
            C({-T(0.0), T::Inf()}, -T(0.0)),

            // If either x is NaN or y is NaN, NaN is returned
            C({T::NaN(), T(0.0)}, T::NaN()),
            C({T(0.0), T::NaN()}, T::NaN()),
            C({T::NaN(), T::NaN()}, T::NaN()),
        };

        cases = Concat(cases, non_finite_cases);
    }

    return cases;
}

INSTANTIATE_TEST_SUITE_P(  //
    MixedAbstractArgs,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(std::vector{
                         C({1_a, 1.0_a}, 0.78539819_a),
                         C({1.0_a, 1_a}, 0.78539819_a),
                     })));

INSTANTIATE_TEST_SUITE_P(  //
    Atan2,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kAtan2),
                     testing::ValuesIn(Concat(Atan2Cases<AFloat, true>(),  //
                                              Atan2Cases<f32, false>(),
                                              Atan2Cases<f16, false>()))));

template <typename T>
std::vector<Case> ClampCases() {
    return {
        C({T(0), T(0), T(0)}, T(0)),
        C({T(0), T(42), T::Highest()}, T(42)),
        C({T::Lowest(), T(0), T(42)}, T(0)),
        C({T(0), T::Lowest(), T::Highest()}, T(0)),
        C({T(0), T::Highest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Highest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Lowest()}, T::Lowest()),
        C({T::Highest(), T::Lowest(), T::Highest()}, T::Highest()),
        C({T::Lowest(), T::Lowest(), T::Highest()}, T::Lowest()),
    };
}

INSTANTIATE_TEST_SUITE_P(  //
    Clamp,
    ResolverConstEvalBuiltinTest,
    testing::Combine(testing::Values(sem::BuiltinType::kClamp),
                     testing::ValuesIn(Concat(ClampCases<AInt>(),  //
                                              ClampCases<i32>(),
                                              ClampCases<u32>(),
                                              ClampCases<AFloat>(),
                                              ClampCases<f32>(),
                                              ClampCases<f16>()))));

}  // namespace builtin

}  // namespace
}  // namespace tint::resolver
