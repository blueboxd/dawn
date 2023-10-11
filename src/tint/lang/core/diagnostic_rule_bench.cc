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
//   src/tint/lang/core/diagnostic_rule_bench.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/diagnostic_rule.h"

#include <array>

#include "benchmark/benchmark.h"

namespace tint::core {
namespace {

void CoreDiagnosticRuleParser(::benchmark::State& state) {
    const char* kStrings[] = {
        "deriative_unccformity",   "dlivative_3iformiy",    "derivative_uniforVity",
        "derivative_uniformity",   "derivative_uniform1ty", "derivativeJunifqrmity",
        "derivative_unifllrmit77",
    };
    for (auto _ : state) {
        for (auto* str : kStrings) {
            auto result = ParseCoreDiagnosticRule(str);
            benchmark::DoNotOptimize(result);
        }
    }
}  // NOLINT(readability/fn_size)

BENCHMARK(CoreDiagnosticRuleParser);

void ChromiumDiagnosticRuleParser(::benchmark::State& state) {
    const char* kStrings[] = {
        "pqnreachableHHcode", "unrechcbe_cov",     "unreachGblecode",  "unreachable_code",
        "vnriiachable_code",  "unreac8ablWW_code", "unreMchablxxcode",
    };
    for (auto _ : state) {
        for (auto* str : kStrings) {
            auto result = ParseChromiumDiagnosticRule(str);
            benchmark::DoNotOptimize(result);
        }
    }
}  // NOLINT(readability/fn_size)

BENCHMARK(ChromiumDiagnosticRuleParser);

}  // namespace
}  // namespace tint::core
