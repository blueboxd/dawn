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
  tint_utils_diagnostic
  tint_utils_text
  tint_utils_traits
)

tint_target_add_external_dependencies(tint_utils_diagnostic_test test
  "gtest"
)
