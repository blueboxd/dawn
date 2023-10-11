# Copyright 2023 The Tint Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

################################################################################
# File generated by 'tools/src/cmd/gen' using the template:
#   tools/src/cmd/gen/build/BUILD.cmake.tmpl
#
# To regenerate run: './tools/run gen'
#
#                       Do not modify this file directly
################################################################################

################################################################################
# Target:    tint_lang_wgsl_helpers
# Kind:      lib
################################################################################
tint_add_target(tint_lang_wgsl_helpers lib
  lang/wgsl/helpers/append_vector.cc
  lang/wgsl/helpers/append_vector.h
  lang/wgsl/helpers/check_supported_extensions.cc
  lang/wgsl/helpers/check_supported_extensions.h
  lang/wgsl/helpers/flatten_bindings.cc
  lang/wgsl/helpers/flatten_bindings.h
)

tint_target_add_dependencies(tint_lang_wgsl_helpers lib
  tint_api_common
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_type
  tint_lang_wgsl_ast
  tint_lang_wgsl_ast_transform
  tint_lang_wgsl_inspector
  tint_lang_wgsl_program
  tint_lang_wgsl_sem
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_id
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_reflection
  tint_utils_result
  tint_utils_rtti
  tint_utils_symbol
  tint_utils_text
  tint_utils_traits
)

################################################################################
# Target:    tint_lang_wgsl_helpers_test
# Kind:      test
################################################################################
tint_add_target(tint_lang_wgsl_helpers_test test
  lang/wgsl/helpers/append_vector_test.cc
  lang/wgsl/helpers/check_supported_extensions_test.cc
  lang/wgsl/helpers/flatten_bindings_test.cc
  lang/wgsl/helpers/ir_program_test.h
)

tint_target_add_dependencies(tint_lang_wgsl_helpers_test test
  tint_api_common
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_intrinsic
  tint_lang_core_type
  tint_lang_wgsl_ast
  tint_lang_wgsl_ast_test
  tint_lang_wgsl_helpers
  tint_lang_wgsl_program
  tint_lang_wgsl_reader
  tint_lang_wgsl_resolver
  tint_lang_wgsl_sem
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_id
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_reflection
  tint_utils_result
  tint_utils_rtti
  tint_utils_symbol
  tint_utils_text
  tint_utils_traits
)

tint_target_add_external_dependencies(tint_lang_wgsl_helpers_test test
  "gtest"
)

if(TINT_BUILD_IR)
  tint_target_add_dependencies(tint_lang_wgsl_helpers_test test
    tint_lang_core_ir
    tint_lang_wgsl_reader_program_to_ir
  )
endif(TINT_BUILD_IR)
