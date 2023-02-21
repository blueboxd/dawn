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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, Attribute_Id) {
    auto p = parser("id(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Id_Expression) {
    auto p = parser("id(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::BinaryExpression>());
    auto* expr = loc->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Id_TrailingComma) {
    auto p = parser("id(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::IdAttribute>());

    auto* loc = var_attr->As<ast::IdAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Id_MissingLeftParen) {
    auto p = parser("id 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected '(' for id attribute");
}

TEST_F(ParserImplTest, Attribute_Id_MissingRightParen) {
    auto p = parser("id(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: expected ')' for id attribute");
}

TEST_F(ParserImplTest, Attribute_Id_MissingValue) {
    auto p = parser("id()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected id expression");
}

TEST_F(ParserImplTest, Attribute_Id_MissingInvalid) {
    auto p = parser("id(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected id expression");
}

TEST_F(ParserImplTest, Attribute_Location) {
    auto p = parser("location(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Location_Expression) {
    auto p = parser("location(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::BinaryExpression>());
    auto* expr = loc->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Location_TrailingComma) {
    auto p = parser("location(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Location_MissingLeftParen) {
    auto p = parser("location 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '(' for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingRightParen) {
    auto p = parser("location(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected ')' for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingValue) {
    auto p = parser("location()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected location expression");
}

TEST_F(ParserImplTest, Attribute_Location_MissingInvalid) {
    auto p = parser("location(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected location expression");
}

struct BuiltinData {
    const char* input;
    builtin::BuiltinValue result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << std::string(data.input);
    return out;
}

class BuiltinTest : public ParserImplTestWithParam<BuiltinData> {};

TEST_P(BuiltinTest, Attribute_Builtin) {
    auto params = GetParam();
    auto p = parser(std::string("builtin(") + params.input + ")");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    EXPECT_EQ(builtin->builtin, params.result);
}
TEST_P(BuiltinTest, Attribute_Builtin_TrailingComma) {
    auto params = GetParam();
    auto p = parser(std::string("builtin(") + params.input + ",)");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    EXPECT_EQ(builtin->builtin, params.result);
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    BuiltinTest,
    testing::Values(BuiltinData{"position", builtin::BuiltinValue::kPosition},
                    BuiltinData{"vertex_index", builtin::BuiltinValue::kVertexIndex},
                    BuiltinData{"instance_index", builtin::BuiltinValue::kInstanceIndex},
                    BuiltinData{"front_facing", builtin::BuiltinValue::kFrontFacing},
                    BuiltinData{"frag_depth", builtin::BuiltinValue::kFragDepth},
                    BuiltinData{"local_invocation_id", builtin::BuiltinValue::kLocalInvocationId},
                    BuiltinData{"local_invocation_index",
                                builtin::BuiltinValue::kLocalInvocationIndex},
                    BuiltinData{"global_invocation_id", builtin::BuiltinValue::kGlobalInvocationId},
                    BuiltinData{"workgroup_id", builtin::BuiltinValue::kWorkgroupId},
                    BuiltinData{"num_workgroups", builtin::BuiltinValue::kNumWorkgroups},
                    BuiltinData{"sample_index", builtin::BuiltinValue::kSampleIndex},
                    BuiltinData{"sample_mask", builtin::BuiltinValue::kSampleMask}));

TEST_F(ParserImplTest, Attribute_Builtin_MissingLeftParen) {
    auto p = parser("builtin position)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingRightParen) {
    auto p = parser("builtin(position");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingValue) {
    auto p = parser("builtin()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:9: expected builtin
Possible values: 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'vertex_index', 'workgroup_id')");
}

TEST_F(ParserImplTest, Attribute_Builtin_InvalidValue) {
    auto p = parser("builtin(other_thingy)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:9: expected builtin
Possible values: 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'vertex_index', 'workgroup_id')");
}

TEST_F(ParserImplTest, Attribute_Builtin_InvalidValueSuggest) {
    auto p = parser("builtin(front_face)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:9: expected builtin
Did you mean 'front_facing'?
Possible values: 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'vertex_index', 'workgroup_id')");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingInvalid) {
    auto p = parser("builtin(3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:9: expected builtin
Possible values: 'frag_depth', 'front_facing', 'global_invocation_id', 'instance_index', 'local_invocation_id', 'local_invocation_index', 'num_workgroups', 'position', 'sample_index', 'sample_mask', 'vertex_index', 'workgroup_id')");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Flat) {
    auto p = parser("interpolate(flat)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, builtin::InterpolationType::kFlat);
    EXPECT_EQ(interp->sampling, builtin::InterpolationSampling::kUndefined);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Single_TrailingComma) {
    auto p = parser("interpolate(flat,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, builtin::InterpolationType::kFlat);
    EXPECT_EQ(interp->sampling, builtin::InterpolationSampling::kUndefined);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Single_DoubleTrailingComma) {
    auto p = parser("interpolate(flat,,)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:18: expected interpolation sampling
Possible values: 'center', 'centroid', 'sample')");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Perspective_Center) {
    auto p = parser("interpolate(perspective, center)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, builtin::InterpolationType::kPerspective);
    EXPECT_EQ(interp->sampling, builtin::InterpolationSampling::kCenter);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Double_TrailingComma) {
    auto p = parser("interpolate(perspective, center,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, builtin::InterpolationType::kPerspective);
    EXPECT_EQ(interp->sampling, builtin::InterpolationSampling::kCenter);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Perspective_Centroid) {
    auto p = parser("interpolate(perspective, centroid)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, builtin::InterpolationType::kPerspective);
    EXPECT_EQ(interp->sampling, builtin::InterpolationSampling::kCentroid);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Linear_Sample) {
    auto p = parser("interpolate(linear, sample)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, builtin::InterpolationType::kLinear);
    EXPECT_EQ(interp->sampling, builtin::InterpolationSampling::kSample);
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingLeftParen) {
    auto p = parser("interpolate flat)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '(' for interpolate attribute");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingRightParen) {
    auto p = parser("interpolate(flat");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for interpolate attribute");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingFirstValue) {
    auto p = parser("interpolate()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:13: expected interpolation type
Possible values: 'flat', 'linear', 'perspective')");
}

TEST_F(ParserImplTest, Attribute_Interpolate_InvalidFirstValue) {
    auto p = parser("interpolate(other_thingy)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:13: expected interpolation type
Possible values: 'flat', 'linear', 'perspective')");
}

TEST_F(ParserImplTest, Attribute_Interpolate_InvalidSecondValue) {
    auto p = parser("interpolate(perspective, nope)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:26: expected interpolation sampling
Did you mean 'sample'?
Possible values: 'center', 'centroid', 'sample')");
}

TEST_F(ParserImplTest, Attribute_Binding) {
    auto p = parser("binding(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::IntLiteralExpression>());
    auto* expr = binding->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Binding_Expression) {
    auto p = parser("binding(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::BinaryExpression>());
    auto* expr = binding->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_Binding_TrailingComma) {
    auto p = parser("binding(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    ASSERT_TRUE(binding->expr->Is<ast::IntLiteralExpression>());
    auto* expr = binding->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Binding_MissingLeftParen) {
    auto p = parser("binding 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingRightParen) {
    auto p = parser("binding(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected ')' for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingValue) {
    auto p = parser("binding()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected binding expression");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingInvalid) {
    auto p = parser("binding(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected binding expression");
}

TEST_F(ParserImplTest, Attribute_group) {
    auto p = parser("group(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_group_expression) {
    auto p = parser("group(4 + 5)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::BinaryExpression>());
    auto* expr = group->expr->As<ast::BinaryExpression>();

    EXPECT_EQ(ast::BinaryOp::kAdd, expr->op);
    auto* v = expr->lhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 4u);

    v = expr->rhs->As<ast::IntLiteralExpression>();
    ASSERT_NE(nullptr, v);
    EXPECT_EQ(v->value, 5u);
}

TEST_F(ParserImplTest, Attribute_group_TrailingComma) {
    auto p = parser("group(4,)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    ASSERT_TRUE(group->expr->Is<ast::IntLiteralExpression>());
    auto* expr = group->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(expr->value, 4);
    EXPECT_EQ(expr->suffix, ast::IntLiteralExpression::Suffix::kNone);
}

TEST_F(ParserImplTest, Attribute_Group_MissingLeftParen) {
    auto p = parser("group 2)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingRightParen) {
    auto p = parser("group(2");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')' for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingValue) {
    auto p = parser("group()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected group expression");
}

TEST_F(ParserImplTest, Attribute_Group_MissingInvalid) {
    auto p = parser("group(if)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected group expression");
}

}  // namespace
}  // namespace tint::reader::wgsl
