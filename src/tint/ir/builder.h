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

#ifndef SRC_TINT_IR_BUILDER_H_
#define SRC_TINT_IR_BUILDER_H_

#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/terminator.h"

// Forward Declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::ir {

/// Builds an ir::Module from a given Program
class Builder {
  public:
    /// Constructor
    /// @param prog the program this ir is associated with
    explicit Builder(const Program* prog);
    /// Constructor
    /// @param mod the ir::Module to wrap with this builder
    explicit Builder(Module&& mod);
    /// Destructor
    ~Builder();

    /// @returns a new block flow node
    Block* CreateBlock();

    /// @returns a new terminator flow node
    Terminator* CreateTerminator();

    /// Creates a function flow node for the given ast::Function
    /// @param func the ast::Function
    /// @returns the flow node
    Function* CreateFunction(const ast::Function* func);

    /// Flags used for creation of if flow nodes
    enum class IfFlags {
        /// Do not create a merge node, `merge_target` will be `nullptr`
        kSkipMerge,
        /// Create the `merge_target` block
        kCreateMerge,
    };

    /// Creates an if flow node for the given ast::IfStatement or ast::BreakIfStatement
    /// @param stmt the ast::IfStatement or ast::BreakIfStatement
    /// @param flags the if creation flags. By default the merge block will not be created, pass
    ///              IfFlags::kCreateMerge if creation is desired.
    /// @returns the flow node
    If* CreateIf(const ast::Statement* stmt, IfFlags flags = IfFlags::kSkipMerge);

    /// Creates a loop flow node for the given ast::LoopStatement
    /// @param stmt the ast::LoopStatement
    /// @returns the flow node
    Loop* CreateLoop(const ast::LoopStatement* stmt);

    /// Branches the given block to the given flow node.
    /// @param from the block to branch from
    /// @param to the node to branch too
    void Branch(Block* from, const FlowNode* to);

    /// The IR module.
    Module ir;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BUILDER_H_
