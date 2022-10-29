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

#ifndef SRC_TINT_IR_IF_H_
#define SRC_TINT_IR_IF_H_

#include "src/tint/ast/if_statement.h"
#include "src/tint/ir/flow_node.h"

// Forward declarations
namespace tint::ir {
class Block;
}  // namespace tint::ir

namespace tint::ir {

/// A flow node representing an if statement. The node always contains a true and a false block. It
/// may contain a merge block where the true/false blocks will merge too unless they return.
class If : public Castable<If, FlowNode> {
  public:
    /// Constructor
    /// @param stmt the ast::IfStatement or ast::BreakIfStatement
    explicit If(const ast::Statement* stmt);
    ~If() override;

    /// The ast::IfStatement or ast::BreakIfStatement source for this flow node.
    const ast::Statement* source;

    /// The true branch block
    Block* true_target = nullptr;
    /// The false branch block
    Block* false_target = nullptr;
    /// An optional block where the true/false blocks will branch too if needed.
    Block* merge_target = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_IF_H_
