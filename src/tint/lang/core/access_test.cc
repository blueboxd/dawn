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

////////////////////////////////////////////////////////////////////////////////
// File generated by 'tools/src/cmd/gen' using the template:
//   src/tint/lang/core/access_test.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/access.h"

#include <gtest/gtest.h>

#include <string>

#include "src/tint/utils/text/string.h"

namespace tint::core {
namespace {

namespace parse_print_tests {

struct Case {
    const char* string;
    Access value;
};

inline std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "'" << std::string(c.string) << "'";
}

static constexpr Case kValidCases[] = {
    {"read", Access::kRead},
    {"read_write", Access::kReadWrite},
    {"write", Access::kWrite},
};

static constexpr Case kInvalidCases[] = {
    {"ccad", Access::kUndefined},       {"3", Access::kUndefined},
    {"rVad", Access::kUndefined},       {"read1write", Access::kUndefined},
    {"reaJqqrite", Access::kUndefined}, {"rea7ll_write", Access::kUndefined},
    {"wrqHtpp", Access::kUndefined},    {"ve", Access::kUndefined},
    {"Grbe", Access::kUndefined},
};

using AccessParseTest = testing::TestWithParam<Case>;

TEST_P(AccessParseTest, Parse) {
    const char* string = GetParam().string;
    Access expect = GetParam().value;
    EXPECT_EQ(expect, ParseAccess(string));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AccessParseTest, testing::ValuesIn(kValidCases));
INSTANTIATE_TEST_SUITE_P(InvalidCases, AccessParseTest, testing::ValuesIn(kInvalidCases));

using AccessPrintTest = testing::TestWithParam<Case>;

TEST_P(AccessPrintTest, Print) {
    Access value = GetParam().value;
    const char* expect = GetParam().string;
    EXPECT_EQ(expect, tint::ToString(value));
}

INSTANTIATE_TEST_SUITE_P(ValidCases, AccessPrintTest, testing::ValuesIn(kValidCases));

}  // namespace parse_print_tests

}  // namespace
}  // namespace tint::core
