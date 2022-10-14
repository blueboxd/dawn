// Copyright 2022 The Tint Authors.
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

#include "src/tint/resolver/const_eval_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

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
                         ast::AddressSpace::kStorage);
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
}  // namespace
}  // namespace tint::resolver
