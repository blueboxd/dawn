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

#include "src/tint/writer/msl/generator_support.h"
#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

struct MslBuiltinData {
    builtin::BuiltinValue builtin;
    const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
    utils::StringStream str;
    str << data.builtin;
    out << str.str();
    return out;
}
using MslBuiltinConversionTest = TestParamHelper<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
    auto params = GetParam();
    EXPECT_EQ(BuiltinToAttribute(params.builtin), std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinConversionTest,
    testing::Values(
        MslBuiltinData{builtin::BuiltinValue::kPosition, "position"},
        MslBuiltinData{builtin::BuiltinValue::kVertexIndex, "vertex_id"},
        MslBuiltinData{builtin::BuiltinValue::kInstanceIndex, "instance_id"},
        MslBuiltinData{builtin::BuiltinValue::kFrontFacing, "front_facing"},
        MslBuiltinData{builtin::BuiltinValue::kFragDepth, "depth(any)"},
        MslBuiltinData{builtin::BuiltinValue::kLocalInvocationId, "thread_position_in_threadgroup"},
        MslBuiltinData{builtin::BuiltinValue::kLocalInvocationIndex, "thread_index_in_threadgroup"},
        MslBuiltinData{builtin::BuiltinValue::kGlobalInvocationId, "thread_position_in_grid"},
        MslBuiltinData{builtin::BuiltinValue::kWorkgroupId, "threadgroup_position_in_grid"},
        MslBuiltinData{builtin::BuiltinValue::kNumWorkgroups, "threadgroups_per_grid"},
        MslBuiltinData{builtin::BuiltinValue::kSampleIndex, "sample_id"},
        MslBuiltinData{builtin::BuiltinValue::kSampleMask, "sample_mask"},
        MslBuiltinData{builtin::BuiltinValue::kPointSize, "point_size"}));

}  // namespace
}  // namespace tint::writer::msl
