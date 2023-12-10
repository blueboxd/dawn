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
    {"align", Attribute::kAlign},
    {"binding", Attribute::kBinding},
    {"builtin", Attribute::kBuiltin},
    {"color", Attribute::kColor},
    {"compute", Attribute::kCompute},
    {"diagnostic", Attribute::kDiagnostic},
    {"fragment", Attribute::kFragment},
    {"group", Attribute::kGroup},
    {"id", Attribute::kId},
    {"index", Attribute::kIndex},
    {"interpolate", Attribute::kInterpolate},
    {"invariant", Attribute::kInvariant},
    {"location", Attribute::kLocation},
    {"must_use", Attribute::kMustUse},
    {"size", Attribute::kSize},
    {"vertex", Attribute::kVertex},
    {"workgroup_size", Attribute::kWorkgroupSize},
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
    {"covior", Attribute::kUndefined},
    {"co8WWr", Attribute::kUndefined},
    {"Mxxlo", Attribute::kUndefined},
    {"cXputgg", Attribute::kUndefined},
    {"opuXe", Attribute::kUndefined},
    {"comp3te", Attribute::kUndefined},
    {"diagnostiE", Attribute::kUndefined},
    {"TTiagnosPPi", Attribute::kUndefined},
    {"diagdoxxtic", Attribute::kUndefined},
    {"44ragment", Attribute::kUndefined},
    {"fSSagmenVV", Attribute::kUndefined},
    {"Rag2Rent", Attribute::kUndefined},
    {"gFup", Attribute::kUndefined},
    {"grop", Attribute::kUndefined},
    {"ROOHVp", Attribute::kUndefined},
    {"y", Attribute::kUndefined},
    {"Gn77rl", Attribute::kUndefined},
    {"04d", Attribute::kUndefined},
    {"oox", Attribute::kUndefined},
    {"inzz", Attribute::kUndefined},
    {"1ippex", Attribute::kUndefined},
    {"interpoXXate", Attribute::kUndefined},
    {"intII99r55olate", Attribute::kUndefined},
    {"intaarpoSSrHHYe", Attribute::kUndefined},
    {"kkvHant", Attribute::kUndefined},
    {"jgaianRR", Attribute::kUndefined},
    {"inaianb", Attribute::kUndefined},
    {"locajion", Attribute::kUndefined},
    {"locaton", Attribute::kUndefined},
    {"loqion", Attribute::kUndefined},
    {"mustusNN", Attribute::kUndefined},
    {"usvvuse", Attribute::kUndefined},
    {"mut_QQse", Attribute::kUndefined},
    {"srf", Attribute::kUndefined},
    {"sije", Attribute::kUndefined},
    {"NNz2w", Attribute::kUndefined},
    {"vrtex", Attribute::kUndefined},
    {"rrertex", Attribute::kUndefined},
    {"vGrtex", Attribute::kUndefined},
    {"workgroup_sizFF", Attribute::kUndefined},
    {"wErkrp_size", Attribute::kUndefined},
    {"worgrroup_size", Attribute::kUndefined},
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
    EXPECT_EQ(expect, ToString(value));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AttributePrintTest, testing::ValuesIn(kValidCases));

}  // namespace parse_print_tests

}  // namespace
}  // namespace tint::core
