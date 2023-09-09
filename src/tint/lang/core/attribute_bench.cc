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
// File generated by tools/src/cmd/gen
// using the template:
//   src/tint/lang/core/attribute_bench.cc.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/attribute.h"

#include <array>

#include "benchmark/benchmark.h"

namespace tint::core {
namespace {

void AttributeParser(::benchmark::State& state) {
    const char* kStrings[] = {
        "alccn",
        "3g",
        "aVign",
        "align",
        "alig1",
        "qqlJn",
        "alill7n",
        "ppqqndnHH",
        "bicv",
        "bndiGg",
        "binding",
        "bindiivg",
        "8WWnding",
        "bxxding",
        "bXltigg",
        "ultXn",
        "buil3in",
        "builtin",
        "Euiltin",
        "bPTTltin",
        "builtdxx",
        "c44mpute",
        "coSSpuVVe",
        "RomR22e",
        "compute",
        "cFpu9e",
        "comute",
        "VOORRHte",
        "dyagnstic",
        "d77agnnnsllrrc",
        "dia400ostic",
        "diagnostic",
        "danstooc",
        "dignszzic",
        "d11ansppiic",
        "XXragment",
        "fIIa9955nnnt",
        "aarHHgmenYSS",
        "fragment",
        "fkkaet",
        "gjamRRn",
        "fabmnt",
        "gjoup",
        "goup",
        "goq",
        "group",
        "Nroup",
        "govv",
        "gruQQ",
        "r",
        "jd",
        "NNw",
        "id",
        "i",
        "rrd",
        "iG",
        "inFFex",
        "iE",
        "inrrx",
        "index",
        "inx",
        "inJJD",
        "ie",
        "inerpklae",
        "intrpolate",
        "inJerpolae",
        "interpolate",
        "interpocate",
        "interpolaOe",
        "__nttevvpoKKate",
        "xnvari5n8",
        "inFq__ant",
        "iqqariant",
        "invariant",
        "invar6a33O",
        "i96arQttanoo",
        "inari66nt",
        "lOxati6zz",
        "locyytion",
        "lHHtion",
        "location",
        "qWW4caton",
        "locOOton",
        "ocatiYn",
        "m_use",
        "mutFuse",
        "wust_us",
        "must_use",
        "Kst_sff",
        "qusKK_use",
        "mFsmm_3se",
        "ize",
        "sze",
        "sbbb",
        "size",
        "iie",
        "siqe",
        "svvTTe",
        "vertFFx",
        "vrQ00P",
        "vePtex",
        "vertex",
        "vsste77",
        "veCtRRbb",
        "verteXX",
        "workgqou_siCCOOO",
        "worsgroupsuzL",
        "wXrkgroup_size",
        "workgroup_size",
        "workgroup_sze",
        "wqqrOgoupize",
        "workg22oup_size",
    };
    for (auto _ : state) {
        for (auto* str : kStrings) {
            auto result = ParseAttribute(str);
            benchmark::DoNotOptimize(result);
        }
    }
}  // NOLINT(readability/fn_size)

BENCHMARK(AttributeParser);

}  // namespace
}  // namespace tint::core
