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

////////////////////////////////////////////////////////////////////////////////
// File generated by 'tools/src/cmd/gen' using the template:
//   src/tint/lang/core/attribute_test.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/attribute.h"

#include <gtest/gtest.h>

#include <string>

#include "src/tint/utils/text/string.h"

namespace tint::core {
namespace {

namespace parse_print_tests {

struct Case {
    const char* string;
    Attribute value;
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "'" << std::string(c.string) << "'";
}

static constexpr Case kValidCases[] = {
    {"align", Attribute::kAlign},           {"binding", Attribute::kBinding},
    {"builtin", Attribute::kBuiltin},       {"compute", Attribute::kCompute},
    {"diagnostic", Attribute::kDiagnostic}, {"fragment", Attribute::kFragment},
    {"group", Attribute::kGroup},           {"id", Attribute::kId},
    {"index", Attribute::kIndex},           {"interpolate", Attribute::kInterpolate},
    {"invariant", Attribute::kInvariant},   {"location", Attribute::kLocation},
    {"must_use", Attribute::kMustUse},      {"size", Attribute::kSize},
    {"vertex", Attribute::kVertex},         {"workgroup_size", Attribute::kWorkgroupSize},
};

static constexpr Case kInvalidCases[] = {
    {"alccn", Attribute::kUndefined},
    {"3g", Attribute::kUndefined},
    {"aVign", Attribute::kUndefined},
    {"bind1ng", Attribute::kUndefined},
    {"bqnJing", Attribute::kUndefined},
    {"bindin7ll", Attribute::kUndefined},
    {"ppqqiliHH", Attribute::kUndefined},
    {"bucv", Attribute::kUndefined},
    {"biltGn", Attribute::kUndefined},
    {"compiive", Attribute::kUndefined},
    {"8WWmpute", Attribute::kUndefined},
    {"cxxpute", Attribute::kUndefined},
    {"dXagnosigg", Attribute::kUndefined},
    {"dagnXuVc", Attribute::kUndefined},
    {"diagnosti3", Attribute::kUndefined},
    {"fraEment", Attribute::kUndefined},
    {"PPagTTent", Attribute::kUndefined},
    {"xxragddnt", Attribute::kUndefined},
    {"g44oup", Attribute::kUndefined},
    {"grVVSSp", Attribute::kUndefined},
    {"22RRp", Attribute::kUndefined},
    {"d", Attribute::kUndefined},
    {"i", Attribute::kUndefined},
    {"OVd", Attribute::kUndefined},
    {"ndyx", Attribute::kUndefined},
    {"n77rrldGx", Attribute::kUndefined},
    {"inde40", Attribute::kUndefined},
    {"itooolate", Attribute::kUndefined},
    {"intezplate", Attribute::kUndefined},
    {"ppnerii1olat", Attribute::kUndefined},
    {"invarianXX", Attribute::kUndefined},
    {"inv55ria99nII", Attribute::kUndefined},
    {"irrvariaSSaHH", Attribute::kUndefined},
    {"lkkcin", Attribute::kUndefined},
    {"gjctRRo", Attribute::kUndefined},
    {"lcbton", Attribute::kUndefined},
    {"mustjuse", Attribute::kUndefined},
    {"must_se", Attribute::kUndefined},
    {"muquse", Attribute::kUndefined},
    {"szNN", Attribute::kUndefined},
    {"zvv", Attribute::kUndefined},
    {"QQze", Attribute::kUndefined},
    {"eterf", Attribute::kUndefined},
    {"vertjx", Attribute::kUndefined},
    {"v82wNNx", Attribute::kUndefined},
    {"worgroup_size", Attribute::kUndefined},
    {"workgrourr_size", Attribute::kUndefined},
    {"workgroGp_size", Attribute::kUndefined},
};

using AttributeParseTest = testing::TestWithParam<Case>;

TEST_P(AttributeParseTest, Parse) {
    const char* string = GetParam().string;
    Attribute expect = GetParam().value;
    EXPECT_EQ(expect, ParseAttribute(string));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AttributeParseTest, testing::ValuesIn(kValidCases));
INSTANTIATE_TEST_SUITE_P(InvalidCases, AttributeParseTest, testing::ValuesIn(kInvalidCases));

using AttributePrintTest = testing::TestWithParam<Case>;

TEST_P(AttributePrintTest, Print) {
    Attribute value = GetParam().value;
    const char* expect = GetParam().string;
    EXPECT_EQ(expect, tint::ToString(value));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AttributePrintTest, testing::ValuesIn(kValidCases));

}  // namespace parse_print_tests

}  // namespace
}  // namespace tint::core
