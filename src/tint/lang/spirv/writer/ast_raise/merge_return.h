// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_AST_RAISE_MERGE_RETURN_H_
#define SRC_TINT_LANG_SPIRV_WRITER_AST_RAISE_MERGE_RETURN_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::spirv::writer {

/// Merge return statements into a single return at the end of the function.
class MergeReturn final : public Castable<MergeReturn, ast::transform::Transform> {
  public:
    /// Constructor
    MergeReturn();
    /// Destructor
    ~MergeReturn() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_AST_RAISE_MERGE_RETURN_H_
