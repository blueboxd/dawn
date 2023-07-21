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

#ifndef INCLUDE_TINT_TINT_H_
#define INCLUDE_TINT_TINT_H_

// Guard for accidental includes to private headers
#define CURRENTLY_IN_TINT_PUBLIC_HEADER

// TODO(tint:88): When implementing support for an install target, all of these
//                headers will need to be moved to include/tint/.

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/wgsl/ast/transform/first_index_offset.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/lang/wgsl/ast/transform/single_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/substitute_override.h"
#include "src/tint/lang/wgsl/ast/transform/vertex_pulling.h"
#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/reader/reader.h"
#include "src/tint/transform/manager.h"
#include "src/tint/utils/diagnostic/printer.h"
#include "src/tint/utils/text/unicode.h"
#include "src/tint/writer/array_length_from_uniform_options.h"
#include "src/tint/writer/binding_point.h"
#include "src/tint/writer/binding_remapper_options.h"
#include "src/tint/writer/external_texture_options.h"
#include "src/tint/writer/writer.h"

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/parser.h"
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/parser.h"
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/ast_writer/generator.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/ast_writer/generator.h"
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/ast_writer/generator.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/ast_writer/generator.h"
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/ast_writer/generator.h"
#endif  // TINT_BUILD_GLSL_WRITER

namespace tint {

/// Initialize initializes the Tint library. Call before using the Tint API.
void Initialize();

/// Shutdown uninitializes the Tint library. Call after using the Tint API.
void Shutdown();

}  // namespace tint

#undef CURRENTLY_IN_TINT_PUBLIC_HEADER

#endif  // INCLUDE_TINT_TINT_H_
