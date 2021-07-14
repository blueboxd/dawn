// Copyright 2020 The Tint Authors.
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

#include "gtest/gtest.h"
#include "src/ast/call_statement.h"
#include "src/ast/disable_validation_decoration.h"
#include "src/ast/override_decoration.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/inspector/test_inspector_builder.h"
#include "src/program_builder.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/variable.h"
#include "tint/tint.h"

namespace tint {
namespace inspector {
namespace {

// All the tests that descend from InspectorBuilder are expected to define their
// test state via building up the AST through InspectorBuilder and then generate
// the program with ::Build().
// The returned Inspector from ::Build() can then be used to test expecations.
class InspectorGetEntryPointTest : public InspectorBuilder,
                                   public testing::Test {};

typedef std::tuple<inspector::ComponentType, inspector::CompositionType>
    InspectorGetEntryPointComponentAndCompositionTestParams;
class InspectorGetEntryPointComponentAndCompositionTest
    : public InspectorBuilder,
      public testing::TestWithParam<
          InspectorGetEntryPointComponentAndCompositionTestParams> {};
struct InspectorGetEntryPointInterpolateTestParams {
  ast::InterpolationType in_type;
  ast::InterpolationSampling in_sampling;
  inspector::InterpolationType out_type;
  inspector::InterpolationSampling out_sampling;
};
class InspectorGetEntryPointInterpolateTest
    : public InspectorBuilder,
      public testing::TestWithParam<
          InspectorGetEntryPointInterpolateTestParams> {};
class InspectorGetRemappedNameForEntryPointTest : public InspectorBuilder,
                                                  public testing::Test {};
class InspectorGetConstantIDsTest : public InspectorBuilder,
                                    public testing::Test {};
class InspectorGetConstantNameToIdMapTest : public InspectorBuilder,
                                            public testing::Test {};
class InspectorGetStorageSizeTest : public InspectorBuilder,
                                    public testing::Test {};
class InspectorGetResourceBindingsTest : public InspectorBuilder,
                                         public testing::Test {};
class InspectorGetUniformBufferResourceBindingsTest : public InspectorBuilder,
                                                      public testing::Test {};
class InspectorGetStorageBufferResourceBindingsTest : public InspectorBuilder,
                                                      public testing::Test {};
class InspectorGetReadOnlyStorageBufferResourceBindingsTest
    : public InspectorBuilder,
      public testing::Test {};
class InspectorGetSamplerResourceBindingsTest : public InspectorBuilder,
                                                public testing::Test {};
class InspectorGetComparisonSamplerResourceBindingsTest
    : public InspectorBuilder,
      public testing::Test {};
class InspectorGetSampledTextureResourceBindingsTest : public InspectorBuilder,
                                                       public testing::Test {};
class InspectorGetSampledArrayTextureResourceBindingsTest
    : public InspectorBuilder,
      public testing::Test {};
struct GetSampledTextureTestParams {
  ast::TextureDimension type_dim;
  inspector::ResourceBinding::TextureDimension inspector_dim;
  inspector::ResourceBinding::SampledKind sampled_kind;
};
class InspectorGetSampledTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetSampledTextureTestParams> {};
class InspectorGetSampledArrayTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetSampledTextureTestParams> {};
class InspectorGetMultisampledTextureResourceBindingsTest
    : public InspectorBuilder,
      public testing::Test {};
class InspectorGetMultisampledArrayTextureResourceBindingsTest
    : public InspectorBuilder,
      public testing::Test {};
typedef GetSampledTextureTestParams GetMultisampledTextureTestParams;
class InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetMultisampledTextureTestParams> {};
class InspectorGetMultisampledTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetMultisampledTextureTestParams> {};
class InspectorGetStorageTextureResourceBindingsTest : public InspectorBuilder,
                                                       public testing::Test {};
struct GetDepthTextureTestParams {
  ast::TextureDimension type_dim;
  inspector::ResourceBinding::TextureDimension inspector_dim;
};
class InspectorGetDepthTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetDepthTextureTestParams> {};

typedef std::tuple<ast::TextureDimension, ResourceBinding::TextureDimension>
    DimensionParams;
typedef std::tuple<ast::ImageFormat,
                   ResourceBinding::ImageFormat,
                   ResourceBinding::SampledKind>
    ImageFormatParams;
typedef std::tuple<bool, DimensionParams, ImageFormatParams>
    GetStorageTextureTestParams;
class InspectorGetStorageTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetStorageTextureTestParams> {};

class InspectorGetExternalTextureResourceBindingsTest : public InspectorBuilder,
                                                        public testing::Test {};

class InspectorGetSamplerTextureUsesTest : public InspectorBuilder,
                                           public testing::Test {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
  MakeEmptyBodyFunction("foo", ast::DecorationList{
                                   Stage(ast::PipelineStage::kFragment),
                               });

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kFragment, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
  MakeEmptyBodyFunction("foo", ast::DecorationList{
                                   Stage(ast::PipelineStage::kFragment),
                               });

  MakeEmptyBodyFunction("bar",
                        ast::DecorationList{Stage(ast::PipelineStage::kCompute),
                                            WorkgroupSize(1)});

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kFragment, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
  MakeEmptyBodyFunction("func", {});

  MakeCallerBodyFunction(
      "foo", {"func"},
      ast::DecorationList{Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(1)});

  MakeCallerBodyFunction("bar", {"func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  // TODO(dsinclair): Update to run the namer transform when available.

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  EXPECT_FALSE(inspector.has_error());

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ("foo", result[0].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ("bar", result[1].remapped_name);
  EXPECT_EQ(ast::PipelineStage::kFragment, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, DefaultWorkgroupSize) {
  MakeEmptyBodyFunction("foo",
                        ast::DecorationList{Stage(ast::PipelineStage::kCompute),
                                            WorkgroupSize(8, 2, 1)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(8u, x);
  EXPECT_EQ(2u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NonDefaultWorkgroupSize) {
  MakeEmptyBodyFunction(
      "foo", {Stage(ast::PipelineStage::kCompute), WorkgroupSize(8, 2, 1)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(8u, x);
  EXPECT_EQ(2u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NoInOutVariables) {
  MakeEmptyBodyFunction("func", {});

  MakeCallerBodyFunction("foo", {"func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].input_variables.size());
  EXPECT_EQ(0u, result[0].output_variables.size());
}

TEST_P(InspectorGetEntryPointComponentAndCompositionTest, Test) {
  ComponentType component;
  CompositionType composition;
  std::tie(component, composition) = GetParam();
  std::function<ast::Type*()> tint_type =
      GetTypeFunction(component, composition);

  auto* in_var = Param("in_var", tint_type(), {Location(0u)});
  Func("foo", {in_var}, tint_type(), {Return("in_var")},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(component, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("<retval>", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(component, result[0].output_variables[0].component_type);
}
INSTANTIATE_TEST_SUITE_P(
    InspectorGetEntryPointTest,
    InspectorGetEntryPointComponentAndCompositionTest,
    testing::Combine(testing::Values(ComponentType::kFloat,
                                     ComponentType::kSInt,
                                     ComponentType::kUInt),
                     testing::Values(CompositionType::kScalar,
                                     CompositionType::kVec2,
                                     CompositionType::kVec3,
                                     CompositionType::kVec4)));

TEST_F(InspectorGetEntryPointTest, MultipleInOutVariables) {
  auto* in_var0 = Param("in_var0", ty.u32(), {Location(0u)});
  auto* in_var1 = Param("in_var1", ty.u32(), {Location(1u)});
  auto* in_var4 = Param("in_var4", ty.u32(), {Location(4u)});
  Func("foo", {in_var0, in_var1, in_var4}, ty.u32(), {Return("in_var0")},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(3u, result[0].input_variables.size());
  EXPECT_EQ("in_var0", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_EQ("in_var1", result[0].input_variables[1].name);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);
  EXPECT_EQ("in_var4", result[0].input_variables[2].name);
  EXPECT_TRUE(result[0].input_variables[2].has_location_decoration);
  EXPECT_EQ(4u, result[0].input_variables[2].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[2].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("<retval>", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables) {
  auto* in_var_foo = Param("in_var_foo", ty.u32(), {Location(0u)});
  Func("foo", {in_var_foo}, ty.u32(), {Return("in_var_foo")},
       {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});

  auto* in_var_bar = Param("in_var_bar", ty.u32(), {Location(0u)});
  Func("bar", {in_var_bar}, ty.u32(), {Return("in_var_bar")},
       {Stage(ast::PipelineStage::kFragment)}, {Location(1u)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var_foo", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("<retval>", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);

  ASSERT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in_var_bar", result[1].input_variables[0].name);
  EXPECT_TRUE(result[1].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[1].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);

  ASSERT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("<retval>", result[1].output_variables[0].name);
  EXPECT_TRUE(result[1].output_variables[0].has_location_decoration);
  EXPECT_EQ(1u, result[1].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, BuiltInsNotStageVariables) {
  auto* in_var0 =
      Param("in_var0", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)});
  auto* in_var1 = Param("in_var1", ty.f32(), {Location(0u)});
  Func("foo", {in_var0, in_var1}, ty.f32(), {Return("in_var1")},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kFragDepth)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var1", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[0].component_type);

  ASSERT_EQ(0u, result[0].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, InOutStruct) {
  auto* interface = MakeInOutStruct("interface", {{"a", 0u}, {"b", 1u}});
  Func("foo", {Param("param", ty.Of(interface))}, ty.Of(interface),
       {Return("param")}, {Stage(ast::PipelineStage::kFragment)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_EQ("param.a", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_EQ("param.b", result[0].input_variables[1].name);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutSharedStruct) {
  auto* interface = MakeInOutStruct("interface", {{"a", 0u}, {"b", 1u}});
  Func("foo", {}, ty.Of(interface), {Return(Construct(ty.Of(interface)))},
       {Stage(ast::PipelineStage::kFragment)});
  Func("bar", {Param("param", ty.Of(interface))}, ty.void_(), {},
       {Stage(ast::PipelineStage::kFragment)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ(0u, result[0].input_variables.size());

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);

  ASSERT_EQ(2u, result[1].input_variables.size());
  EXPECT_EQ("param.a", result[1].input_variables[0].name);
  EXPECT_TRUE(result[1].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[1].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);
  EXPECT_EQ("param.b", result[1].input_variables[1].name);
  EXPECT_TRUE(result[1].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[1].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[1].component_type);

  ASSERT_EQ(0u, result[1].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, MixInOutVariablesAndStruct) {
  auto* struct_a = MakeInOutStruct("struct_a", {{"a", 0u}, {"b", 1u}});
  auto* struct_b = MakeInOutStruct("struct_b", {{"a", 2u}});
  Func("foo",
       {Param("param_a", ty.Of(struct_a)), Param("param_b", ty.Of(struct_b)),
        Param("param_c", ty.f32(), {Location(3u)}),
        Param("param_d", ty.f32(), {Location(4u)})},
       ty.Of(struct_a), {Return("param_a")},
       {Stage(ast::PipelineStage::kFragment)});
  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(5u, result[0].input_variables.size());
  EXPECT_EQ("param_a.a", result[0].input_variables[0].name);
  EXPECT_TRUE(result[0].input_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].input_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
  EXPECT_EQ("param_a.b", result[0].input_variables[1].name);
  EXPECT_TRUE(result[0].input_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].input_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);
  EXPECT_EQ("param_b.a", result[0].input_variables[2].name);
  EXPECT_TRUE(result[0].input_variables[2].has_location_decoration);
  EXPECT_EQ(2u, result[0].input_variables[2].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[2].component_type);
  EXPECT_EQ("param_c", result[0].input_variables[3].name);
  EXPECT_TRUE(result[0].input_variables[3].has_location_decoration);
  EXPECT_EQ(3u, result[0].input_variables[3].location_decoration);
  EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[3].component_type);
  EXPECT_EQ("param_d", result[0].input_variables[4].name);
  EXPECT_TRUE(result[0].input_variables[4].has_location_decoration);
  EXPECT_EQ(4u, result[0].input_variables[4].location_decoration);
  EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[4].component_type);

  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
  EXPECT_TRUE(result[0].output_variables[0].has_location_decoration);
  EXPECT_EQ(0u, result[0].output_variables[0].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
  EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
  EXPECT_TRUE(result[0].output_variables[1].has_location_decoration);
  EXPECT_EQ(1u, result[0].output_variables[1].location_decoration);
  EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantUnreferenced) {
  AddOverridableConstantWithoutID<float>("foo", ty.f32(), nullptr);
  MakeEmptyBodyFunction(
      "ep_func", {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].overridable_constants.size());
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantReferencedByEntryPoint) {
  AddOverridableConstantWithoutID<float>("foo", ty.f32(), nullptr);
  MakeConstReferenceBodyFunction(
      "ep_func", "foo", ty.f32(),
      {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(1u, result[0].overridable_constants.size());
  EXPECT_EQ("foo", result[0].overridable_constants[0].name);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantReferencedByCallee) {
  AddOverridableConstantWithoutID<float>("foo", ty.f32(), nullptr);
  MakeConstReferenceBodyFunction("callee_func", "foo", ty.f32(), {});
  MakeCallerBodyFunction(
      "ep_func", {"callee_func"},
      {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(1u, result[0].overridable_constants.size());
  EXPECT_EQ("foo", result[0].overridable_constants[0].name);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantSomeReferenced) {
  AddOverridableConstantWithID<float>("foo", 1, ty.f32(), nullptr);
  AddOverridableConstantWithID<float>("bar", 2, ty.f32(), nullptr);
  MakeConstReferenceBodyFunction("callee_func", "foo", ty.f32(), {});
  MakeCallerBodyFunction(
      "ep_func", {"callee_func"},
      {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(1u, result[0].overridable_constants.size());
  EXPECT_EQ("foo", result[0].overridable_constants[0].name);
}

TEST_F(InspectorGetEntryPointTest, NonOverridableConstantSkipped) {
  ast::Struct* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
  AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);
  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});
  MakeCallerBodyFunction("ep_func", {"ub_func"},
                         {Stage(ast::PipelineStage::kFragment)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].overridable_constants.size());
}

TEST_F(InspectorGetEntryPointTest, SampleMaskNotReferenced) {
  MakeEmptyBodyFunction("ep_func", {Stage(ast::PipelineStage::kFragment)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  EXPECT_FALSE(result[0].sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, SampleMaskSimpleReferenced) {
  auto* in_var =
      Param("in_var", ty.u32(), {Builtin(ast::Builtin::kSampleMask)});
  Func("ep_func", {in_var}, ty.u32(), {Return("in_var")},
       {Stage(ast::PipelineStage::kFragment)},
       {Builtin(ast::Builtin::kSampleMask)});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  EXPECT_TRUE(result[0].sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, SampleMaskStructReferenced) {
  ast::StructMemberList members;
  members.push_back(Member("inner_sample_mask", ty.u32(),
                           {Builtin(ast::Builtin::kSampleMask)}));
  Structure("out_struct", members, {});

  Func("ep_func", {}, ty.type_name("out_struct"),
       {Decl(Var("out_var", ty.type_name("out_struct"))), Return("out_var")},
       {Stage(ast::PipelineStage::kFragment)}, {});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  EXPECT_TRUE(result[0].sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, ImplicitInterpolate) {
  ast::StructMemberList members;
  members.push_back(Member("struct_inner", ty.f32(), {Location(0)}));
  Structure("in_struct", members, {});
  auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

  Func("ep_func", {in_var}, ty.void_(), {Return()},
       {Stage(ast::PipelineStage::kFragment)}, {});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ(InterpolationType::kPerspective,
            result[0].input_variables[0].interpolation_type);
  EXPECT_EQ(InterpolationSampling::kCenter,
            result[0].input_variables[0].interpolation_sampling);
}

TEST_P(InspectorGetEntryPointInterpolateTest, Test) {
  auto& params = GetParam();
  ast::StructMemberList members;
  members.push_back(
      Member("struct_inner", ty.f32(),
             {Interpolate(params.in_type, params.in_sampling), Location(0)}));
  Structure("in_struct", members, {});
  auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

  Func("ep_func", {in_var}, ty.void_(), {Return()},
       {Stage(ast::PipelineStage::kFragment)}, {});

  Inspector& inspector = Build();

  auto result = inspector.GetEntryPoints();

  ASSERT_EQ(1u, result.size());
  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ(params.out_type, result[0].input_variables[0].interpolation_type);
  EXPECT_EQ(params.out_sampling,
            result[0].input_variables[0].interpolation_sampling);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetEntryPointTest,
    InspectorGetEntryPointInterpolateTest,
    testing::Values(
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective,
            ast::InterpolationSampling::kCenter,
            InterpolationType::kPerspective, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective,
            ast::InterpolationSampling::kCentroid,
            InterpolationType::kPerspective, InterpolationSampling::kCentroid},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective,
            ast::InterpolationSampling::kSample,
            InterpolationType::kPerspective, InterpolationSampling::kSample},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective,
            ast::InterpolationSampling::kNone, InterpolationType::kPerspective,
            InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear,
            ast::InterpolationSampling::kCenter, InterpolationType::kLinear,
            InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear,
            ast::InterpolationSampling::kCentroid, InterpolationType::kLinear,
            InterpolationSampling::kCentroid},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear,
            ast::InterpolationSampling::kSample, InterpolationType::kLinear,
            InterpolationSampling::kSample},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear, ast::InterpolationSampling::kNone,
            InterpolationType::kLinear, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kFlat, ast::InterpolationSampling::kNone,
            InterpolationType::kFlat, InterpolationSampling::kNone}));

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_NoFunctions) {
  Inspector& inspector = Build();

  auto result = inspector.GetRemappedNameForEntryPoint("foo");
  ASSERT_TRUE(inspector.has_error());

  EXPECT_EQ("", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_NoEntryPoints) {
  Inspector& inspector = Build();

  auto result = inspector.GetRemappedNameForEntryPoint("foo");
  ASSERT_TRUE(inspector.has_error());

  EXPECT_EQ("", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest, DISABLED_OneEntryPoint) {
  MakeEmptyBodyFunction("foo", ast::DecorationList{
                                   Stage(ast::PipelineStage::kVertex),
                               });

  // TODO(dsinclair): Update to run the namer transform when
  // available.

  Inspector& inspector = Build();

  auto result = inspector.GetRemappedNameForEntryPoint("foo");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ("foo", result);
}

// TODO(rharrison): Reenable once GetRemappedNameForEntryPoint isn't a pass
// through
TEST_F(InspectorGetRemappedNameForEntryPointTest,
       DISABLED_MultipleEntryPoints) {
  MakeEmptyBodyFunction("foo", ast::DecorationList{
                                   Stage(ast::PipelineStage::kVertex),
                               });

  // TODO(dsinclair): Update to run the namer transform when
  // available.

  MakeEmptyBodyFunction("bar",
                        ast::DecorationList{Stage(ast::PipelineStage::kCompute),
                                            WorkgroupSize(1)});

  Inspector& inspector = Build();

  {
    auto result = inspector.GetRemappedNameForEntryPoint("foo");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ("foo", result);
  }
  {
    auto result = inspector.GetRemappedNameForEntryPoint("bar");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ("bar", result);
  }
}

TEST_F(InspectorGetConstantIDsTest, Bool) {
  bool val_true = true;
  bool val_false = false;
  AddOverridableConstantWithID<bool>("foo", 1, ty.bool_(), nullptr);
  AddOverridableConstantWithID<bool>("bar", 20, ty.bool_(), &val_true);
  AddOverridableConstantWithID<bool>("baz", 300, ty.bool_(), &val_false);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
  ASSERT_EQ(3u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsBool());
  EXPECT_TRUE(result[20].AsBool());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsBool());
  EXPECT_FALSE(result[300].AsBool());
}

TEST_F(InspectorGetConstantIDsTest, U32) {
  uint32_t val = 42;
  AddOverridableConstantWithID<uint32_t>("foo", 1, ty.u32(), nullptr);
  AddOverridableConstantWithID<uint32_t>("bar", 20, ty.u32(), &val);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
  ASSERT_EQ(2u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsU32());
  EXPECT_EQ(42u, result[20].AsU32());
}

TEST_F(InspectorGetConstantIDsTest, I32) {
  int32_t val_neg = -42;
  int32_t val_pos = 42;
  AddOverridableConstantWithID<int32_t>("foo", 1, ty.i32(), nullptr);
  AddOverridableConstantWithID<int32_t>("bar", 20, ty.i32(), &val_neg);
  AddOverridableConstantWithID<int32_t>("baz", 300, ty.i32(), &val_pos);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
  ASSERT_EQ(3u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsI32());
  EXPECT_EQ(-42, result[20].AsI32());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsI32());
  EXPECT_EQ(42, result[300].AsI32());
}

TEST_F(InspectorGetConstantIDsTest, Float) {
  float val_zero = 0.0f;
  float val_neg = -10.0f;
  float val_pos = 15.0f;
  AddOverridableConstantWithID<float>("foo", 1, ty.f32(), nullptr);
  AddOverridableConstantWithID<float>("bar", 20, ty.f32(), &val_zero);
  AddOverridableConstantWithID<float>("baz", 300, ty.f32(), &val_neg);
  AddOverridableConstantWithID<float>("x", 4000, ty.f32(), &val_pos);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantIDs();
  ASSERT_EQ(4u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsFloat());
  EXPECT_FLOAT_EQ(0.0, result[20].AsFloat());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsFloat());
  EXPECT_FLOAT_EQ(-10.0, result[300].AsFloat());

  ASSERT_TRUE(result.find(4000) != result.end());
  EXPECT_TRUE(result[4000].IsFloat());
  EXPECT_FLOAT_EQ(15.0, result[4000].AsFloat());
}

TEST_F(InspectorGetConstantNameToIdMapTest, WithAndWithoutIds) {
  AddOverridableConstantWithID<float>("v1", 1, ty.f32(), nullptr);
  AddOverridableConstantWithID<float>("v20", 20, ty.f32(), nullptr);
  AddOverridableConstantWithID<float>("v300", 300, ty.f32(), nullptr);
  auto* a = AddOverridableConstantWithoutID<float>("a", ty.f32(), nullptr);
  auto* b = AddOverridableConstantWithoutID<float>("b", ty.f32(), nullptr);
  auto* c = AddOverridableConstantWithoutID<float>("c", ty.f32(), nullptr);

  Inspector& inspector = Build();

  auto result = inspector.GetConstantNameToIdMap();
  ASSERT_EQ(6u, result.size());

  ASSERT_TRUE(result.count("v1"));
  EXPECT_EQ(result["v1"], 1u);

  ASSERT_TRUE(result.count("v20"));
  EXPECT_EQ(result["v20"], 20u);

  ASSERT_TRUE(result.count("v300"));
  EXPECT_EQ(result["v300"], 300u);

  ASSERT_TRUE(result.count("a"));
  ASSERT_TRUE(program_->Sem().Get(a));
  EXPECT_EQ(result["a"], program_->Sem().Get(a)->ConstantId());

  ASSERT_TRUE(result.count("b"));
  ASSERT_TRUE(program_->Sem().Get(b));
  EXPECT_EQ(result["b"], program_->Sem().Get(b)->ConstantId());

  ASSERT_TRUE(result.count("c"));
  ASSERT_TRUE(program_->Sem().Get(c));
  EXPECT_EQ(result["c"], program_->Sem().Get(c)->ConstantId());
}

TEST_F(InspectorGetStorageSizeTest, Empty) {
  MakeEmptyBodyFunction("ep_func",
                        ast::DecorationList{Stage(ast::PipelineStage::kCompute),
                                            WorkgroupSize(1)});
  Inspector& inspector = Build();
  EXPECT_EQ(0u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetStorageSizeTest, Simple) {
  ast::Struct* ub_struct_type =
      MakeUniformBufferType("ub_type", {ty.i32(), ty.i32()});
  AddUniformBuffer("ub_var", ty.Of(ub_struct_type), 0, 0);
  MakeStructVariableReferenceBodyFunction("ub_func", "ub_var", {{0, ty.i32()}});

  auto sb = MakeStorageBufferTypes("sb_type", {ty.i32()});
  AddStorageBuffer("sb_var", sb(), ast::Access::kReadWrite, 1, 0);
  MakeStructVariableReferenceBodyFunction("sb_func", "sb_var", {{0, ty.i32()}});

  auto ro_sb = MakeStorageBufferTypes("rosb_type", {ty.i32()});
  AddStorageBuffer("rosb_var", ro_sb(), ast::Access::kRead, 1, 1);
  MakeStructVariableReferenceBodyFunction("rosb_func", "rosb_var",
                                          {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"ub_func", "sb_func", "rosb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kCompute),
                             WorkgroupSize(1),
                         });

  Inspector& inspector = Build();

  EXPECT_EQ(16u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetResourceBindingsTest, Empty) {
  MakeCallerBodyFunction("ep_func", {},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetResourceBindingsTest, Simple) {
  ast::Struct* ub_struct_type = MakeUniformBufferType("ub_type", {ty.i32()});
  AddUniformBuffer("ub_var", ty.Of(ub_struct_type), 0, 0);
  MakeStructVariableReferenceBodyFunction("ub_func", "ub_var", {{0, ty.i32()}});

  auto sb = MakeStorageBufferTypes("sb_type", {ty.i32()});
  AddStorageBuffer("sb_var", sb(), ast::Access::kReadWrite, 1, 0);
  MakeStructVariableReferenceBodyFunction("sb_func", "sb_var", {{0, ty.i32()}});

  auto ro_sb = MakeStorageBufferTypes("rosb_type", {ty.i32()});
  AddStorageBuffer("rosb_var", ro_sb(), ast::Access::kRead, 1, 1);
  MakeStructVariableReferenceBodyFunction("rosb_func", "rosb_var",
                                          {{0, ty.i32()}});

  auto* s_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("s_texture", s_texture_type, 2, 0);
  AddSampler("s_var", 3, 0);
  AddGlobalVariable("s_coords", ty.f32());
  MakeSamplerReferenceBodyFunction("s_func", "s_texture", "s_var", "s_coords",
                                   ty.f32(), {});

  auto* cs_depth_texture_type =
      MakeDepthTextureType(ast::TextureDimension::k2d);
  AddDepthTexture("cs_texture", cs_depth_texture_type, 3, 1);
  AddComparisonSampler("cs_var", 3, 2);
  AddGlobalVariable("cs_coords", ty.vec2<f32>());
  AddGlobalVariable("cs_depth", ty.f32());
  MakeComparisonSamplerReferenceBodyFunction(
      "cs_func", "cs_texture", "cs_var", "cs_coords", "cs_depth", ty.f32(), {});

  auto* st_type = MakeStorageTextureTypes(ast::TextureDimension::k2d,
                                          ast::ImageFormat::kR32Uint, false);
  AddStorageTexture("st_var", st_type, 4, 0);
  MakeStorageTextureBodyFunction("st_func", "st_var", ty.vec2<i32>(), {});

  auto* rost_type = MakeStorageTextureTypes(ast::TextureDimension::k2d,
                                            ast::ImageFormat::kR32Uint, true);
  AddStorageTexture("rost_var", rost_type, 4, 1);
  MakeStorageTextureBodyFunction("rost_func", "rost_var", ty.vec2<i32>(), {});

  MakeCallerBodyFunction("ep_func",
                         {"ub_func", "sb_func", "rosb_func", "s_func",
                          "cs_func", "st_func", "rost_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(9u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[1].resource_type);
  EXPECT_EQ(1u, result[1].bind_group);
  EXPECT_EQ(0u, result[1].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[2].resource_type);
  EXPECT_EQ(1u, result[2].bind_group);
  EXPECT_EQ(1u, result[2].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[3].resource_type);
  EXPECT_EQ(3u, result[3].bind_group);
  EXPECT_EQ(0u, result[3].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler,
            result[4].resource_type);
  EXPECT_EQ(3u, result[4].bind_group);
  EXPECT_EQ(2u, result[4].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture,
            result[5].resource_type);
  EXPECT_EQ(2u, result[5].bind_group);
  EXPECT_EQ(0u, result[5].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageTexture,
            result[6].resource_type);
  EXPECT_EQ(4u, result[6].bind_group);
  EXPECT_EQ(1u, result[6].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kWriteOnlyStorageTexture,
            result[7].resource_type);
  EXPECT_EQ(4u, result[7].bind_group);
  EXPECT_EQ(0u, result[7].binding);

  EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture,
            result[8].resource_type);
  EXPECT_EQ(3u, result[8].bind_group);
  EXPECT_EQ(1u, result[8].binding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MissingEntryPoint) {
  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_TRUE(inspector.has_error());
  std::string error = inspector.error();
  EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, NonEntryPointFunc) {
  ast::Struct* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
  AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"ub_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ub_func");
  std::string error = inspector.error();
  EXPECT_TRUE(error.find("not an entry point") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, Simple) {
  ast::Struct* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
  AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"ub_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].size);
  EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleMembers) {
  ast::Struct* foo_struct_type =
      MakeUniformBufferType("foo_type", {ty.i32(), ty.u32(), ty.f32()});
  AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

  MakeStructVariableReferenceBodyFunction(
      "ub_func", "foo_ub", {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});

  MakeCallerBodyFunction("ep_func", {"ub_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingPadding) {
  ast::Struct* foo_struct_type =
      MakeUniformBufferType("foo_type", {ty.vec3<f32>()});
  AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                          {{0, ty.vec3<f32>()}});

  MakeCallerBodyFunction("ep_func", {"ub_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleUniformBuffers) {
  ast::Struct* ub_struct_type =
      MakeUniformBufferType("ub_type", {ty.i32(), ty.u32(), ty.f32()});
  AddUniformBuffer("ub_foo", ty.Of(ub_struct_type), 0, 0);
  AddUniformBuffer("ub_bar", ty.Of(ub_struct_type), 0, 1);
  AddUniformBuffer("ub_baz", ty.Of(ub_struct_type), 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    MakeStructVariableReferenceBodyFunction(
        func_name, var_name, {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
  };
  AddReferenceFunc("ub_foo_func", "ub_foo");
  AddReferenceFunc("ub_bar_func", "ub_bar");
  AddReferenceFunc("ub_baz_func", "ub_baz");

  auto FuncCall = [&](const std::string& callee) {
    return create<ast::CallStatement>(Call(callee));
  };

  Func("ep_func", ast::VariableList(), ty.void_(),
       ast::StatementList{FuncCall("ub_foo_func"), FuncCall("ub_bar_func"),
                          FuncCall("ub_baz_func"), Return()},
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[1].resource_type);
  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].size);
  EXPECT_EQ(12u, result[1].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[2].resource_type);
  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].size);
  EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingArray) {
  // TODO(bclayton) - This is not a legal structure layout for uniform buffer
  // usage. Once crbug.com/tint/628 is implemented, this will fail validation
  // and will need to be fixed.
  ast::Struct* foo_struct_type =
      MakeUniformBufferType("foo_type", {ty.i32(), ty.array<u32, 4>()});
  AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

  MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"ub_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].size);
  EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, Simple) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].size);
  EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleMembers) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                ty.i32(),
                                                                ty.u32(),
                                                                ty.f32(),
                                                            });
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

  MakeStructVariableReferenceBodyFunction(
      "sb_func", "foo_sb", {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleStorageBuffers) {
  auto sb_struct_type = MakeStorageBufferTypes("sb_type", {
                                                              ty.i32(),
                                                              ty.u32(),
                                                              ty.f32(),
                                                          });
  AddStorageBuffer("sb_foo", sb_struct_type(), ast::Access::kReadWrite, 0, 0);
  AddStorageBuffer("sb_bar", sb_struct_type(), ast::Access::kReadWrite, 0, 1);
  AddStorageBuffer("sb_baz", sb_struct_type(), ast::Access::kReadWrite, 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    MakeStructVariableReferenceBodyFunction(
        func_name, var_name, {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
  };
  AddReferenceFunc("sb_foo_func", "sb_foo");
  AddReferenceFunc("sb_bar_func", "sb_bar");
  AddReferenceFunc("sb_baz_func", "sb_baz");

  auto FuncCall = [&](const std::string& callee) {
    return create<ast::CallStatement>(Call(callee));
  };

  Func("ep_func", ast::VariableList(), ty.void_(),
       ast::StatementList{
           FuncCall("sb_foo_func"),
           FuncCall("sb_bar_func"),
           FuncCall("sb_baz_func"),
           Return(),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[1].resource_type);
  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].size);
  EXPECT_EQ(12u, result[1].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[2].resource_type);
  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].size);
  EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingArray) {
  auto foo_struct_type =
      MakeStorageBufferTypes("foo_type", {ty.i32(), ty.array<u32, 4>()});
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].size);
  EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingRuntimeArray) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                ty.i32(),
                                                                ty.array<u32>(),
                                                            });
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(8u, result[0].size);
  EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingPadding) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.vec3<f32>()});
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                          {{0, ty.vec3<f32>()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(16u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, SkipReadOnly) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, Simple) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].size);
  EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest,
       MultipleStorageBuffers) {
  auto sb_struct_type = MakeStorageBufferTypes("sb_type", {
                                                              ty.i32(),
                                                              ty.u32(),
                                                              ty.f32(),
                                                          });
  AddStorageBuffer("sb_foo", sb_struct_type(), ast::Access::kRead, 0, 0);
  AddStorageBuffer("sb_bar", sb_struct_type(), ast::Access::kRead, 0, 1);
  AddStorageBuffer("sb_baz", sb_struct_type(), ast::Access::kRead, 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    MakeStructVariableReferenceBodyFunction(
        func_name, var_name, {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
  };
  AddReferenceFunc("sb_foo_func", "sb_foo");
  AddReferenceFunc("sb_bar_func", "sb_bar");
  AddReferenceFunc("sb_baz_func", "sb_baz");

  auto FuncCall = [&](const std::string& callee) {
    return create<ast::CallStatement>(Call(callee));
  };

  Func("ep_func", ast::VariableList(), ty.void_(),
       ast::StatementList{
           FuncCall("sb_foo_func"),
           FuncCall("sb_bar_func"),
           FuncCall("sb_baz_func"),
           Return(),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].size);
  EXPECT_EQ(12u, result[0].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[1].resource_type);
  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].size);
  EXPECT_EQ(12u, result[1].size_no_padding);

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[2].resource_type);
  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].size);
  EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, ContainingArray) {
  auto foo_struct_type =
      MakeStorageBufferTypes("foo_type", {
                                             ty.i32(),
                                             ty.array<u32, 4>(),
                                         });
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].size);
  EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest,
       ContainingRuntimeArray) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                ty.i32(),
                                                                ty.array<u32>(),
                                                            });
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(8u, result[0].size);
  EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, SkipNonReadOnly) {
  auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
  AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

  MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

  MakeCallerBodyFunction("ep_func", {"sb_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, Simple) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, NoSampler) {
  MakeEmptyBodyFunction("ep_func", ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, InFunction) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("foo_func", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(), {});

  MakeCallerBodyFunction("ep_func", {"foo_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, UnknownEntryPoint) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("foo");
  ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetSamplerResourceBindingsTest, SkipsComparisonSamplers) {
  auto* depth_texture_type = MakeDepthTextureType(ast::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", ty.f32(),
      ast::DecorationList{
          Stage(ast::PipelineStage::kFragment),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, Simple) {
  auto* depth_texture_type = MakeDepthTextureType(ast::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", ty.f32(),
      ast::DecorationList{
          Stage(ast::PipelineStage::kFragment),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, NoSampler) {
  MakeEmptyBodyFunction("ep_func", ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, InFunction) {
  auto* depth_texture_type = MakeDepthTextureType(ast::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction("foo_func", "foo_texture",
                                             "foo_sampler", "foo_coords",
                                             "foo_depth", ty.f32(), {});

  MakeCallerBodyFunction("ep_func", {"foo_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, UnknownEntryPoint) {
  auto* depth_texture_type = MakeDepthTextureType(ast::TextureDimension::k2d);
  AddDepthTexture("foo_texture", depth_texture_type, 0, 0);
  AddComparisonSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.vec2<f32>());
  AddGlobalVariable("foo_depth", ty.f32());

  MakeComparisonSamplerReferenceBodyFunction(
      "ep", "foo_texture", "foo_sampler", "foo_coords", "foo_depth", ty.f32(),
      ast::DecorationList{
          Stage(ast::PipelineStage::kFragment),
      });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerResourceBindings("foo");
  ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, SkipsSamplers) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetComparisonSamplerResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSampledTextureResourceBindingsTest, Empty) {
  MakeEmptyBodyFunction("foo", ast::DecorationList{
                                   Stage(ast::PipelineStage::kFragment),
                               });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("foo");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetSampledTextureResourceBindingsTestWithParam, textureSample) {
  auto* sampled_texture_type = MakeSampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
  AddGlobalVariable("foo_coords", coord_type);

  MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler",
                                   "foo_coords",
                                   GetBaseType(GetParam().sampled_kind),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);

  // Prove that sampled and multi-sampled bindings are accounted
  // for separately.
  auto multisampled_result =
      inspector.GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_TRUE(multisampled_result.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledTextureResourceBindingsTest,
    InspectorGetSampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetSampledTextureTestParams{
            ast::TextureDimension::k1d,
            inspector::ResourceBinding::TextureDimension::k1d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::TextureDimension::k3d,
            inspector::ResourceBinding::TextureDimension::k3d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube,
            inspector::ResourceBinding::SampledKind::kFloat}));

TEST_P(InspectorGetSampledArrayTextureResourceBindingsTestWithParam,
       textureSample) {
  auto* sampled_texture_type = MakeSampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_array_index", ty.i32());

  MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler",
                                   "foo_coords", "foo_array_index",
                                   GetBaseType(GetParam().sampled_kind),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledArrayTextureResourceBindingsTest,
    InspectorGetSampledArrayTextureResourceBindingsTestWithParam,
    testing::Values(
        GetSampledTextureTestParams{
            ast::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{
            ast::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray,
            inspector::ResourceBinding::SampledKind::kFloat}));

TEST_P(InspectorGetMultisampledTextureResourceBindingsTestWithParam,
       textureLoad) {
  auto* multisampled_texture_type = MakeMultisampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddMultisampledTexture("foo_texture", multisampled_texture_type, 0, 0);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.i32());
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_sample_index", ty.i32());

  Func("ep", ast::VariableList(), ty.void_(),
       ast::StatementList{
           Ignore(Call("textureLoad", "foo_texture", "foo_coords",
                       "foo_sample_index")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);

  // Prove that sampled and multi-sampled bindings are accounted
  // for separately.
  auto single_sampled_result =
      inspector.GetSampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_TRUE(single_sampled_result.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledTextureResourceBindingsTest,
    InspectorGetMultisampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{
            ast::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            ast::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            ast::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetMultisampledArrayTextureResourceBindingsTest, Empty) {
  MakeEmptyBodyFunction("foo", ast::DecorationList{
                                   Stage(ast::PipelineStage::kFragment),
                               });

  Inspector& inspector = Build();

  auto result = inspector.GetSampledTextureResourceBindings("foo");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam,
       DISABLED_textureSample) {
  auto* multisampled_texture_type = MakeMultisampledTextureType(
      GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
  AddMultisampledTexture("foo_texture", multisampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
  AddGlobalVariable("foo_coords", coord_type);
  AddGlobalVariable("foo_array_index", ty.i32());

  MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler",
                                   "foo_coords", "foo_array_index",
                                   GetBaseType(GetParam().sampled_kind),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetMultisampledTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
  EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledArrayTextureResourceBindingsTest,
    InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{
            ast::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{
            ast::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{
            ast::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray,
            inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetStorageTextureResourceBindingsTest, Empty) {
  MakeEmptyBodyFunction("ep", ast::DecorationList{
                                  Stage(ast::PipelineStage::kFragment),
                              });

  Inspector& inspector = Build();

  auto result = inspector.GetReadOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  EXPECT_EQ(0u, result.size());

  result = inspector.GetWriteOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetStorageTextureResourceBindingsTestWithParam, Simple) {
  bool read_only;
  DimensionParams dim_params;
  ImageFormatParams format_params;
  std::tie(read_only, dim_params, format_params) = GetParam();

  ast::TextureDimension dim;
  ResourceBinding::TextureDimension expected_dim;
  std::tie(dim, expected_dim) = dim_params;

  ast::ImageFormat format;
  ResourceBinding::ImageFormat expected_format;
  ResourceBinding::SampledKind expected_kind;
  std::tie(format, expected_format, expected_kind) = format_params;

  auto* st_type = MakeStorageTextureTypes(dim, format, read_only);
  AddStorageTexture("st_var", st_type, 0, 0);

  ast::Type* dim_type = nullptr;
  switch (dim) {
    case ast::TextureDimension::k1d:
      dim_type = ty.i32();
      break;
    case ast::TextureDimension::k2d:
    case ast::TextureDimension::k2dArray:
      dim_type = ty.vec2<i32>();
      break;
    case ast::TextureDimension::k3d:
      dim_type = ty.vec3<i32>();
      break;
    default:
      break;
  }

  ASSERT_FALSE(dim_type == nullptr);

  MakeStorageTextureBodyFunction(
      "ep", "st_var", dim_type,
      ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  Inspector& inspector = Build();

  auto result =
      read_only ? inspector.GetReadOnlyStorageTextureResourceBindings("ep")
                : inspector.GetWriteOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(read_only ? ResourceBinding::ResourceType::kReadOnlyStorageTexture
                      : ResourceBinding::ResourceType::kWriteOnlyStorageTexture,
            result[0].resource_type);
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(expected_dim, result[0].dim);
  EXPECT_EQ(expected_format, result[0].image_format);
  EXPECT_EQ(expected_kind, result[0].sampled_kind);

  result = read_only
               ? inspector.GetWriteOnlyStorageTextureResourceBindings("ep")
               : inspector.GetReadOnlyStorageTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  ASSERT_EQ(0u, result.size());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetStorageTextureResourceBindingsTest,
    InspectorGetStorageTextureResourceBindingsTestWithParam,
    testing::Combine(
        testing::Bool(),
        testing::Values(
            std::make_tuple(ast::TextureDimension::k1d,
                            ResourceBinding::TextureDimension::k1d),
            std::make_tuple(ast::TextureDimension::k2d,
                            ResourceBinding::TextureDimension::k2d),
            std::make_tuple(ast::TextureDimension::k2dArray,
                            ResourceBinding::TextureDimension::k2dArray),
            std::make_tuple(ast::TextureDimension::k3d,
                            ResourceBinding::TextureDimension::k3d)),
        testing::Values(
            std::make_tuple(ast::ImageFormat::kR32Float,
                            ResourceBinding::ImageFormat::kR32Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(ast::ImageFormat::kR32Sint,
                            ResourceBinding::ImageFormat::kR32Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(ast::ImageFormat::kR32Uint,
                            ResourceBinding::ImageFormat::kR32Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(ast::ImageFormat::kRg32Float,
                            ResourceBinding::ImageFormat::kRg32Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(ast::ImageFormat::kRg32Sint,
                            ResourceBinding::ImageFormat::kRg32Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(ast::ImageFormat::kRg32Uint,
                            ResourceBinding::ImageFormat::kRg32Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(ast::ImageFormat::kRgba16Float,
                            ResourceBinding::ImageFormat::kRgba16Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(ast::ImageFormat::kRgba16Sint,
                            ResourceBinding::ImageFormat::kRgba16Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(ast::ImageFormat::kRgba16Uint,
                            ResourceBinding::ImageFormat::kRgba16Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(ast::ImageFormat::kRgba32Float,
                            ResourceBinding::ImageFormat::kRgba32Float,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(ast::ImageFormat::kRgba32Sint,
                            ResourceBinding::ImageFormat::kRgba32Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(ast::ImageFormat::kRgba32Uint,
                            ResourceBinding::ImageFormat::kRgba32Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(ast::ImageFormat::kRgba8Sint,
                            ResourceBinding::ImageFormat::kRgba8Sint,
                            ResourceBinding::SampledKind::kSInt),
            std::make_tuple(ast::ImageFormat::kRgba8Snorm,
                            ResourceBinding::ImageFormat::kRgba8Snorm,
                            ResourceBinding::SampledKind::kFloat),
            std::make_tuple(ast::ImageFormat::kRgba8Uint,
                            ResourceBinding::ImageFormat::kRgba8Uint,
                            ResourceBinding::SampledKind::kUInt),
            std::make_tuple(ast::ImageFormat::kRgba8Unorm,
                            ResourceBinding::ImageFormat::kRgba8Unorm,
                            ResourceBinding::SampledKind::kFloat))));

TEST_P(InspectorGetDepthTextureResourceBindingsTestWithParam,
       textureDimensions) {
  auto* depth_texture_type = MakeDepthTextureType(GetParam().type_dim);
  AddDepthTexture("dt", depth_texture_type, 0, 0);
  AddGlobalVariable("dt_level", ty.i32());

  Func("ep", ast::VariableList(), ty.void_(),
       ast::StatementList{
           Ignore(Call("textureDimensions", "dt", "dt_level")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetDepthTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture,
            result[0].resource_type);
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetDepthTextureResourceBindingsTest,
    InspectorGetDepthTextureResourceBindingsTestWithParam,
    testing::Values(
        GetDepthTextureTestParams{
            ast::TextureDimension::k2d,
            inspector::ResourceBinding::TextureDimension::k2d},
        GetDepthTextureTestParams{
            ast::TextureDimension::k2dArray,
            inspector::ResourceBinding::TextureDimension::k2dArray},
        GetDepthTextureTestParams{
            ast::TextureDimension::kCube,
            inspector::ResourceBinding::TextureDimension::kCube},
        GetDepthTextureTestParams{
            ast::TextureDimension::kCubeArray,
            inspector::ResourceBinding::TextureDimension::kCubeArray}));

TEST_F(InspectorGetExternalTextureResourceBindingsTest, Simple) {
  auto* external_texture_type = MakeExternalTextureType();
  AddExternalTexture("et", external_texture_type, 0, 0);

  Func("ep", ast::VariableList(), ty.void_(),
       ast::StatementList{
           Ignore(Call("textureDimensions", "et")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  Inspector& inspector = Build();

  auto result = inspector.GetExternalTextureResourceBindings("ep");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();
  EXPECT_EQ(ResourceBinding::ResourceType::kExternalTexture,
            result[0].resource_type);

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, None) {
  MakeEmptyBodyFunction("ep_func", ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerTextureUses("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerTextureUsesTest, Simple) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 10);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("ep_func", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerTextureUses("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].sampler_binding_point.group);
  EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
  EXPECT_EQ(0u, result[0].texture_binding_point.group);
  EXPECT_EQ(10u, result[0].texture_binding_point.binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, MultipleCalls) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 10);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("ep_func", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(),
                                   ast::DecorationList{
                                       Stage(ast::PipelineStage::kFragment),
                                   });

  Inspector& inspector = Build();

  auto result_0 = inspector.GetSamplerTextureUses("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  auto result_1 = inspector.GetSamplerTextureUses("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  EXPECT_EQ(result_0, result_1);
}

TEST_F(InspectorGetSamplerTextureUsesTest, InFunction) {
  auto* sampled_texture_type =
      MakeSampledTextureType(ast::TextureDimension::k1d, ty.f32());
  AddSampledTexture("foo_texture", sampled_texture_type, 0, 0);
  AddSampler("foo_sampler", 0, 1);
  AddGlobalVariable("foo_coords", ty.f32());

  MakeSamplerReferenceBodyFunction("foo_func", "foo_texture", "foo_sampler",
                                   "foo_coords", ty.f32(), {});

  MakeCallerBodyFunction("ep_func", {"foo_func"},
                         ast::DecorationList{
                             Stage(ast::PipelineStage::kFragment),
                         });

  Inspector& inspector = Build();

  auto result = inspector.GetSamplerTextureUses("ep_func");
  ASSERT_FALSE(inspector.has_error()) << inspector.error();

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].sampler_binding_point.group);
  EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
  EXPECT_EQ(0u, result[0].texture_binding_point.group);
  EXPECT_EQ(0u, result[0].texture_binding_point.binding);
}

}  // namespace
}  // namespace inspector
}  // namespace tint
