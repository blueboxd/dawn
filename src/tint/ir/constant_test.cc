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

#include "src/tint/ir/builder.h"
#include "src/tint/ir/test_helper.h"
#include "src/tint/ir/value.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_ConstantTest = TestHelper;

TEST_F(IR_ConstantTest, f32) {
    Module mod;
    Builder b{mod};

    utils::StringStream str;

    auto* c = b.Constant(1.2_f);
    EXPECT_EQ(1.2_f, c->Value()->As<constant::Scalar<f32>>()->ValueAs<f32>());

    EXPECT_TRUE(c->Value()->Is<constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, f16) {
    Module mod;
    Builder b{mod};

    utils::StringStream str;

    auto* c = b.Constant(1.1_h);
    EXPECT_EQ(1.1_h, c->Value()->As<constant::Scalar<f16>>()->ValueAs<f16>());

    EXPECT_FALSE(c->Value()->Is<constant::Scalar<f32>>());
    EXPECT_TRUE(c->Value()->Is<constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, i32) {
    Module mod;
    Builder b{mod};

    utils::StringStream str;

    auto* c = b.Constant(1_i);
    EXPECT_EQ(1_i, c->Value()->As<constant::Scalar<i32>>()->ValueAs<i32>());

    EXPECT_FALSE(c->Value()->Is<constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<f16>>());
    EXPECT_TRUE(c->Value()->Is<constant::Scalar<i32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, u32) {
    Module mod;
    Builder b{mod};

    utils::StringStream str;

    auto* c = b.Constant(2_u);
    EXPECT_EQ(2_u, c->Value()->As<constant::Scalar<u32>>()->ValueAs<u32>());

    EXPECT_FALSE(c->Value()->Is<constant::Scalar<f32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<f16>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<i32>>());
    EXPECT_TRUE(c->Value()->Is<constant::Scalar<u32>>());
    EXPECT_FALSE(c->Value()->Is<constant::Scalar<bool>>());
}

TEST_F(IR_ConstantTest, bool) {
    Module mod;
    Builder b{mod};

    {
        utils::StringStream str;

        auto* c = b.Constant(false);
        EXPECT_FALSE(c->Value()->As<constant::Scalar<bool>>()->ValueAs<bool>());
    }

    {
        utils::StringStream str;
        auto c = b.Constant(true);
        EXPECT_TRUE(c->Value()->As<constant::Scalar<bool>>()->ValueAs<bool>());

        EXPECT_FALSE(c->Value()->Is<constant::Scalar<f32>>());
        EXPECT_FALSE(c->Value()->Is<constant::Scalar<f16>>());
        EXPECT_FALSE(c->Value()->Is<constant::Scalar<i32>>());
        EXPECT_FALSE(c->Value()->Is<constant::Scalar<u32>>());
        EXPECT_TRUE(c->Value()->Is<constant::Scalar<bool>>());
    }
}

}  // namespace
}  // namespace tint::ir
