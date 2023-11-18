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

#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, FunctionVar_NoInit) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("v", ty.ptr<function, i32>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
}

TEST_F(SpirvWriterTest, FunctionVar_WithInit) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        v->SetInitializer(b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpirvWriterTest, FunctionVar_DeclInsideBlock) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        b.Append(i->True(), [&] {
            auto* v = b.Var("v", ty.ptr<function, i32>());
            v->SetInitializer(b.Constant(42_i));
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
          %4 = OpLabel
          %v = OpVariable %_ptr_Function_int Function
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
               OpStore %v %int_42
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, FunctionVar_Load) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        auto* result = b.Load(v);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("%result = OpLoad %int %v");
}

TEST_F(SpirvWriterTest, FunctionVar_Store) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        b.Store(v, 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpirvWriterTest, PrivateVar_NoInit) {
    mod.root_block->Append(b.Var("v", ty.ptr<private_, i32>()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private");
}

TEST_F(SpirvWriterTest, PrivateVar_WithInit) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
}

TEST_F(SpirvWriterTest, PrivateVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpStore %v %add");
}

TEST_F(SpirvWriterTest, WorkgroupVar) {
    mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
}

TEST_F(SpirvWriterTest, WorkgroupVar_LoadAndStore) {
    auto* v = mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpStore %v %add");
}

TEST_F(SpirvWriterTest, WorkgroupVar_ZeroInitializeWithExtension) {
    mod.root_block->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    // Create a writer with the zero_init_workgroup_memory flag set to `true`.
    ASSERT_TRUE(Generate({}, /* zero_init_workgroup_memory */ true)) << Error() << output_;
    EXPECT_INST("%4 = OpConstantNull %int");
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup %4");
}

TEST_F(SpirvWriterTest, StorageVar_ReadOnly) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %tint_symbol_1 Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
               OpDecorate %1 NonWritable
)");
    EXPECT_INST(R"(
%tint_symbol_1 = OpTypeStruct %int
%_ptr_StorageBuffer_tint_symbol_1 = OpTypePointer StorageBuffer %tint_symbol_1
          %1 = OpVariable %_ptr_StorageBuffer_tint_symbol_1 StorageBuffer
)");
}

TEST_F(SpirvWriterTest, StorageVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<storage, i32, read_write>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
       %load = OpLoad %int %9
        %add = OpIAdd %int %load %int_1
         %16 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %16 %add
)");
}

TEST_F(SpirvWriterTest, StorageVar_WriteOnly) {
    auto* v = b.Var("v", ty.ptr<storage, i32, write>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.Append(func->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %tint_symbol_1 Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
               OpDecorate %1 NonReadable
)");
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %9 %int_42
)");
}

TEST_F(SpirvWriterTest, UniformVar) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %tint_symbol_1 Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
)");
    EXPECT_INST(R"(
%tint_symbol_1 = OpTypeStruct %int
%_ptr_Uniform_tint_symbol_1 = OpTypePointer Uniform %tint_symbol_1
          %1 = OpVariable %_ptr_Uniform_tint_symbol_1 Uniform
)");
}

TEST_F(SpirvWriterTest, UniformVar_Load) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_Uniform_int %1 %uint_0
       %load = OpLoad %int %9
)");
}

TEST_F(SpirvWriterTest, PushConstantVar) {
    auto* v = b.Var("v", ty.ptr<push_constant, i32>());
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %tint_symbol_1 Block
)");
    EXPECT_INST(R"(
%tint_symbol_1 = OpTypeStruct %int
%_ptr_PushConstant_tint_symbol_1 = OpTypePointer PushConstant %tint_symbol_1
          %1 = OpVariable %_ptr_PushConstant_tint_symbol_1 PushConstant
)");
}

TEST_F(SpirvWriterTest, PushConstantVar_Load) {
    auto* v = b.Var("v", ty.ptr<push_constant, i32>());
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func, load);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %8 = OpAccessChain %_ptr_PushConstant_int %1 %uint_0
       %load = OpLoad %int %8
               OpReturnValue %load
)");
}

TEST_F(SpirvWriterTest, SamplerVar) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle, ty.sampler(), core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeSampler
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

TEST_F(SpirvWriterTest, SamplerVar_Load) {
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle, ty.sampler(), core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpirvWriterTest, TextureVar) {
    auto* v = b.Var(
        "v", ty.ptr(core::AddressSpace::kHandle,
                    ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
                    core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

TEST_F(SpirvWriterTest, TextureVar_Load) {
    auto* v = b.Var(
        "v", ty.ptr(core::AddressSpace::kHandle,
                    ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
                    core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpirvWriterTest, ReadOnlyStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.Get<core::type::StorageTexture>(
                                    core::type::TextureDimension::k2d, format, read,
                                    core::type::StorageTexture::SubtypeFor(format, ty)),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v NonWritable
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

TEST_F(SpirvWriterTest, ReadWriteStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.Get<core::type::StorageTexture>(
                                    core::type::TextureDimension::k2d, format, read_write,
                                    core::type::StorageTexture::SubtypeFor(format, ty)),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

TEST_F(SpirvWriterTest, WriteOnlyStorageTextureVar) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* v = b.Var("v", ty.ptr(core::AddressSpace::kHandle,
                                ty.Get<core::type::StorageTexture>(
                                    core::type::TextureDimension::k2d, format, write,
                                    core::type::StorageTexture::SubtypeFor(format, ty)),
                                core::Access::kRead));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
               OpDecorate %v NonReadable
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

}  // namespace
}  // namespace tint::spirv::writer
