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
# Target:    tint_utils_containers
# Kind:      lib
################################################################################
tint_add_target(tint_utils_containers lib
  utils/containers/bitset.h
  utils/containers/containers.cc
  utils/containers/enum_set.h
  utils/containers/hashmap.h
  utils/containers/hashmap_base.h
  utils/containers/hashset.h
  utils/containers/map.h
  utils/containers/predicates.h
  utils/containers/reverse.h
  utils/containers/scope_stack.h
  utils/containers/slice.h
  utils/containers/transform.h
  utils/containers/unique_allocator.h
  utils/containers/unique_vector.h
  utils/containers/vector.h
)

tint_target_add_dependencies(tint_utils_containers lib
  tint_utils_ice
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_rtti
  tint_utils_traits
)

################################################################################
# Target:    tint_utils_containers_test
# Kind:      test
################################################################################
tint_add_target(tint_utils_containers_test test
  utils/containers/bitset_test.cc
  utils/containers/enum_set_test.cc
  utils/containers/hashmap_test.cc
  utils/containers/hashset_test.cc
  utils/containers/map_test.cc
  utils/containers/predicates_test.cc
  utils/containers/reverse_test.cc
  utils/containers/scope_stack_test.cc
  utils/containers/slice_test.cc
  utils/containers/transform_test.cc
  utils/containers/unique_allocator_test.cc
  utils/containers/unique_vector_test.cc
  utils/containers/vector_test.cc
)

tint_target_add_dependencies(tint_utils_containers_test test
  tint_api_common
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_type
  tint_lang_wgsl_ast
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

tint_target_add_external_dependencies(tint_utils_containers_test test
  "gtest"
)
