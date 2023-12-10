// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/construct.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_ConstructTest = IRTestHelper;

TEST_F(IR_ConstructTest, Usage) {
    auto* arg1 = b.Constant(true);
    auto* arg2 = b.Constant(false);
    auto* c = b.Construct(mod.Types().f32(), arg1, arg2);

    EXPECT_THAT(arg1->Usages(), testing::UnorderedElementsAre(Usage{c, 0u}));
    EXPECT_THAT(arg2->Usages(), testing::UnorderedElementsAre(Usage{c, 1u}));
}

TEST_F(IR_ConstructTest, Result) {
    auto* arg1 = b.Constant(true);
    auto* arg2 = b.Constant(false);
    auto* c = b.Construct(mod.Types().f32(), arg1, arg2);

    EXPECT_TRUE(c->HasResults());
    EXPECT_FALSE(c->HasMultiResults());
    EXPECT_TRUE(c->Result()->Is<InstructionResult>());
    EXPECT_EQ(c, c->Result()->Source());
}

TEST_F(IR_ConstructTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Construct(nullptr);
        },
        "");
}

TEST_F(IR_ConstructTest, Clone) {
    auto* arg1 = b.Constant(true);
    auto* arg2 = b.Constant(false);
    auto* c = b.Construct(mod.Types().f32(), arg1, arg2);

    auto* new_c = clone_ctx.Clone(c);

    EXPECT_NE(c, new_c);
    EXPECT_NE(c->Result(), new_c->Result());
    EXPECT_EQ(mod.Types().f32(), new_c->Result()->Type());

    auto args = new_c->Args();
    EXPECT_EQ(2u, args.Length());

    auto* val0 = args[0]->As<Constant>()->Value();
    EXPECT_TRUE(val0->As<core::constant::Scalar<bool>>()->ValueAs<bool>());

    auto* val1 = args[1]->As<Constant>()->Value();
    EXPECT_FALSE(val1->As<core::constant::Scalar<bool>>()->ValueAs<bool>());
}

TEST_F(IR_ConstructTest, CloneEmpty) {
    auto* c = b.Construct(mod.Types().f32());

    auto* new_c = clone_ctx.Clone(c);
    EXPECT_NE(c->Result(), new_c->Result());
    EXPECT_EQ(mod.Types().f32(), new_c->Result()->Type());
    EXPECT_TRUE(new_c->Args().IsEmpty());
}

}  // namespace
}  // namespace tint::core::ir
