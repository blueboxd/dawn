# Copyright 2023 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

################################################################################
# File generated by 'tools/src/cmd/gen' using the template:
#   tools/src/cmd/gen/build/BUILD.cmake.tmpl
#
# To regenerate run: './tools/run gen'
#
#                       Do not modify this file directly
################################################################################

include(lang/core/ir/binary/BUILD.cmake)
include(lang/core/ir/transform/BUILD.cmake)

################################################################################
# Target:    tint_lang_core_ir
# Kind:      lib
################################################################################
tint_add_target(tint_lang_core_ir lib
  lang/core/ir/access.cc
  lang/core/ir/access.h
  lang/core/ir/binary.cc
  lang/core/ir/binary.h
  lang/core/ir/bitcast.cc
  lang/core/ir/bitcast.h
  lang/core/ir/block.cc
  lang/core/ir/block.h
  lang/core/ir/block_param.cc
  lang/core/ir/block_param.h
  lang/core/ir/break_if.cc
  lang/core/ir/break_if.h
  lang/core/ir/builder.cc
  lang/core/ir/builder.h
  lang/core/ir/builtin_call.cc
  lang/core/ir/builtin_call.h
  lang/core/ir/call.cc
  lang/core/ir/call.h
  lang/core/ir/clone_context.cc
  lang/core/ir/clone_context.h
  lang/core/ir/constant.cc
  lang/core/ir/constant.h
  lang/core/ir/construct.cc
  lang/core/ir/construct.h
  lang/core/ir/continue.cc
  lang/core/ir/continue.h
  lang/core/ir/control_instruction.cc
  lang/core/ir/control_instruction.h
  lang/core/ir/convert.cc
  lang/core/ir/convert.h
  lang/core/ir/core_binary.cc
  lang/core/ir/core_binary.h
  lang/core/ir/core_builtin_call.cc
  lang/core/ir/core_builtin_call.h
  lang/core/ir/core_unary.cc
  lang/core/ir/core_unary.h
  lang/core/ir/disassembler.cc
  lang/core/ir/disassembler.h
  lang/core/ir/discard.cc
  lang/core/ir/discard.h
  lang/core/ir/exit.cc
  lang/core/ir/exit.h
  lang/core/ir/exit_if.cc
  lang/core/ir/exit_if.h
  lang/core/ir/exit_loop.cc
  lang/core/ir/exit_loop.h
  lang/core/ir/exit_switch.cc
  lang/core/ir/exit_switch.h
  lang/core/ir/function.cc
  lang/core/ir/function.h
  lang/core/ir/function_param.cc
  lang/core/ir/function_param.h
  lang/core/ir/ice.h
  lang/core/ir/if.cc
  lang/core/ir/if.h
  lang/core/ir/instruction.cc
  lang/core/ir/instruction.h
  lang/core/ir/instruction_result.cc
  lang/core/ir/instruction_result.h
  lang/core/ir/let.cc
  lang/core/ir/let.h
  lang/core/ir/load.cc
  lang/core/ir/load.h
  lang/core/ir/load_vector_element.cc
  lang/core/ir/load_vector_element.h
  lang/core/ir/location.h
  lang/core/ir/loop.cc
  lang/core/ir/loop.h
  lang/core/ir/module.cc
  lang/core/ir/module.h
  lang/core/ir/multi_in_block.cc
  lang/core/ir/multi_in_block.h
  lang/core/ir/next_iteration.cc
  lang/core/ir/next_iteration.h
  lang/core/ir/operand_instruction.cc
  lang/core/ir/operand_instruction.h
  lang/core/ir/return.cc
  lang/core/ir/return.h
  lang/core/ir/store.cc
  lang/core/ir/store.h
  lang/core/ir/store_vector_element.cc
  lang/core/ir/store_vector_element.h
  lang/core/ir/switch.cc
  lang/core/ir/switch.h
  lang/core/ir/swizzle.cc
  lang/core/ir/swizzle.h
  lang/core/ir/terminate_invocation.cc
  lang/core/ir/terminate_invocation.h
  lang/core/ir/terminator.cc
  lang/core/ir/terminator.h
  lang/core/ir/traverse.h
  lang/core/ir/unary.cc
  lang/core/ir/unary.h
  lang/core/ir/unreachable.cc
  lang/core/ir/unreachable.h
  lang/core/ir/user_call.cc
  lang/core/ir/user_call.h
  lang/core/ir/validator.cc
  lang/core/ir/validator.h
  lang/core/ir/value.cc
  lang/core/ir/value.h
  lang/core/ir/var.cc
  lang/core/ir/var.h
)

tint_target_add_dependencies(tint_lang_core_ir lib
  tint_api_common
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_intrinsic
  tint_lang_core_type
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
# Target:    tint_lang_core_ir_test
# Kind:      test
################################################################################
tint_add_target(tint_lang_core_ir_test test
  lang/core/ir/access_test.cc
  lang/core/ir/bitcast_test.cc
  lang/core/ir/block_param_test.cc
  lang/core/ir/block_test.cc
  lang/core/ir/break_if_test.cc
  lang/core/ir/constant_test.cc
  lang/core/ir/construct_test.cc
  lang/core/ir/continue_test.cc
  lang/core/ir/convert_test.cc
  lang/core/ir/core_binary_test.cc
  lang/core/ir/core_builtin_call_test.cc
  lang/core/ir/core_unary_test.cc
  lang/core/ir/discard_test.cc
  lang/core/ir/exit_if_test.cc
  lang/core/ir/exit_loop_test.cc
  lang/core/ir/exit_switch_test.cc
  lang/core/ir/function_param_test.cc
  lang/core/ir/function_test.cc
  lang/core/ir/if_test.cc
  lang/core/ir/instruction_result_test.cc
  lang/core/ir/instruction_test.cc
  lang/core/ir/ir_helper_test.h
  lang/core/ir/let_test.cc
  lang/core/ir/load_test.cc
  lang/core/ir/load_vector_element_test.cc
  lang/core/ir/loop_test.cc
  lang/core/ir/module_test.cc
  lang/core/ir/multi_in_block_test.cc
  lang/core/ir/next_iteration_test.cc
  lang/core/ir/operand_instruction_test.cc
  lang/core/ir/return_test.cc
  lang/core/ir/store_test.cc
  lang/core/ir/store_vector_element_test.cc
  lang/core/ir/switch_test.cc
  lang/core/ir/swizzle_test.cc
  lang/core/ir/terminate_invocation_test.cc
  lang/core/ir/traverse_test.cc
  lang/core/ir/unreachable_test.cc
  lang/core/ir/user_call_test.cc
  lang/core/ir/validator_test.cc
  lang/core/ir/value_test.cc
  lang/core/ir/var_test.cc
)

tint_target_add_dependencies(tint_lang_core_ir_test test
  tint_api_common
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_intrinsic
  tint_lang_core_ir
  tint_lang_core_type
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

tint_target_add_external_dependencies(tint_lang_core_ir_test test
  "gtest"
)