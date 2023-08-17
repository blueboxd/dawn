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

#ifdef TINT_BUILD_SYNTAX_TREE_WRITER

#include "src/tint/lang/wgsl/writer/options.h"

namespace tint::wgsl::writer {

Options::Options() = default;

Options::~Options() = default;

Options::Options(const Options&) = default;

Options& Options::operator=(const Options&) = default;

}  // namespace tint::wgsl::writer

#endif  // TINT_BUILD_SYNTAX_TREE_WRITER
