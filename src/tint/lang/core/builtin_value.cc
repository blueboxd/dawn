// Copyright 2020 The Dawn & Tint Authors
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
//   src/tint/lang/core/builtin_value.cc.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/lang/core/builtin_value.h"

namespace tint::core {

/// ParseBuiltinValue parses a BuiltinValue from a string.
/// @param str the string to parse
/// @returns the parsed enum, or BuiltinValue::kUndefined if the string could not be parsed.
BuiltinValue ParseBuiltinValue(std::string_view str) {
    if (str == "__point_size") {
        return BuiltinValue::kPointSize;
    }
    if (str == "frag_depth") {
        return BuiltinValue::kFragDepth;
    }
    if (str == "front_facing") {
        return BuiltinValue::kFrontFacing;
    }
    if (str == "global_invocation_id") {
        return BuiltinValue::kGlobalInvocationId;
    }
    if (str == "instance_index") {
        return BuiltinValue::kInstanceIndex;
    }
    if (str == "local_invocation_id") {
        return BuiltinValue::kLocalInvocationId;
    }
    if (str == "local_invocation_index") {
        return BuiltinValue::kLocalInvocationIndex;
    }
    if (str == "num_workgroups") {
        return BuiltinValue::kNumWorkgroups;
    }
    if (str == "position") {
        return BuiltinValue::kPosition;
    }
    if (str == "sample_index") {
        return BuiltinValue::kSampleIndex;
    }
    if (str == "sample_mask") {
        return BuiltinValue::kSampleMask;
    }
    if (str == "subgroup_invocation_id") {
        return BuiltinValue::kSubgroupInvocationId;
    }
    if (str == "subgroup_size") {
        return BuiltinValue::kSubgroupSize;
    }
    if (str == "vertex_index") {
        return BuiltinValue::kVertexIndex;
    }
    if (str == "workgroup_id") {
        return BuiltinValue::kWorkgroupId;
    }
    return BuiltinValue::kUndefined;
}

std::string_view ToString(BuiltinValue value) {
    switch (value) {
        case BuiltinValue::kUndefined:
            return "undefined";
        case BuiltinValue::kPointSize:
            return "__point_size";
        case BuiltinValue::kFragDepth:
            return "frag_depth";
        case BuiltinValue::kFrontFacing:
            return "front_facing";
        case BuiltinValue::kGlobalInvocationId:
            return "global_invocation_id";
        case BuiltinValue::kInstanceIndex:
            return "instance_index";
        case BuiltinValue::kLocalInvocationId:
            return "local_invocation_id";
        case BuiltinValue::kLocalInvocationIndex:
            return "local_invocation_index";
        case BuiltinValue::kNumWorkgroups:
            return "num_workgroups";
        case BuiltinValue::kPosition:
            return "position";
        case BuiltinValue::kSampleIndex:
            return "sample_index";
        case BuiltinValue::kSampleMask:
            return "sample_mask";
        case BuiltinValue::kSubgroupInvocationId:
            return "subgroup_invocation_id";
        case BuiltinValue::kSubgroupSize:
            return "subgroup_size";
        case BuiltinValue::kVertexIndex:
            return "vertex_index";
        case BuiltinValue::kWorkgroupId:
            return "workgroup_id";
    }
    return "<unknown>";
}

}  // namespace tint::core