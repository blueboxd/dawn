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

################################################################################
# Target:    tint_utils_diagnostic
# Kind:      lib
################################################################################
tint_add_target(tint_utils_diagnostic lib
  utils/diagnostic/diagnostic.cc
  utils/diagnostic/diagnostic.h
  utils/diagnostic/formatter.cc
  utils/diagnostic/formatter.h
  utils/diagnostic/printer.cc
  utils/diagnostic/printer.h
  utils/diagnostic/source.cc
  utils/diagnostic/source.h
)

tint_target_add_dependencies(tint_utils_diagnostic lib
  tint_utils_containers
  tint_utils_ice
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_rtti
  tint_utils_text
  tint_utils_traits
)

if((NOT IS_LINUX) AND (NOT IS_MAC) AND (NOT IS_WIN))
  tint_target_add_sources(tint_utils_diagnostic lib
    "utils/diagnostic/printer_other.cc"
  )
endif((NOT IS_LINUX) AND (NOT IS_MAC) AND (NOT IS_WIN))

if(IS_LINUX OR IS_MAC)
  tint_target_add_sources(tint_utils_diagnostic lib
    "utils/diagnostic/printer_posix.cc"
  )
endif(IS_LINUX OR IS_MAC)

if(IS_WIN)
  tint_target_add_sources(tint_utils_diagnostic lib
    "utils/diagnostic/printer_windows.cc"
  )
endif(IS_WIN)

################################################################################
# Target:    tint_utils_diagnostic_test
# Kind:      test
################################################################################
tint_add_target(tint_utils_diagnostic_test test
  utils/diagnostic/diagnostic_test.cc
  utils/diagnostic/formatter_test.cc
  utils/diagnostic/printer_test.cc
  utils/diagnostic/source_test.cc
)

tint_target_add_dependencies(tint_utils_diagnostic_test test
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_rtti
  tint_utils_text
  tint_utils_traits
)

tint_target_add_external_dependencies(tint_utils_diagnostic_test test
  "gtest"
)
