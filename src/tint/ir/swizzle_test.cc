// Copyright 2023 The Tint Authors.
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

#include "src/tint/ir/swizzle.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ir/ir_test_helper.h"

namespace tint::ir {
namespace {

using IR_SwizzleTest = IRTestHelper;

TEST_F(IR_SwizzleTest, SetsUsage) {
    auto* var = b.Var(ty.ptr<function, i32>());
    auto* a = b.Swizzle(mod.Types().i32(), var, {1u});

    EXPECT_THAT(var->Usages(), testing::UnorderedElementsAre(Usage{a, 0u}));
}

TEST_F(IR_SwizzleTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(nullptr, var, {1u});
        },
        "");
}

TEST_F(IR_SwizzleTest, Fail_NullObject) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Swizzle(mod.Types().i32(), nullptr, {1u});
        },
        "");
}

TEST_F(IR_SwizzleTest, Fail_EmptyIndices) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(mod.Types().i32(), var, utils::Empty);
        },
        "");
}

TEST_F(IR_SwizzleTest, Fail_TooManyIndices) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(mod.Types().i32(), var, {1u, 1u, 1u, 1u, 1u});
        },
        "");
}

TEST_F(IR_SwizzleTest, Fail_IndexOutOfRange) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* var = b.Var(mod.Types().ptr<function, i32>());
            b.Swizzle(mod.Types().i32(), var, {4u});
        },
        "");
}

}  // namespace
}  // namespace tint::ir
