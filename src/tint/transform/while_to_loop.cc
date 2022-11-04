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

#include "src/tint/transform/while_to_loop.h"

#include <utility>

#include "src/tint/ast/break_statement.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::WhileToLoop);

namespace tint::transform {
namespace {

bool ShouldRun(const Program* program) {
    for (auto* node : program->ASTNodes().Objects()) {
        if (node->Is<ast::WhileStatement>()) {
            return true;
        }
    }
    return false;
}

}  // namespace

WhileToLoop::WhileToLoop() = default;

WhileToLoop::~WhileToLoop() = default;

Transform::ApplyResult WhileToLoop::Apply(const Program* src, const DataMap&, DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    ctx.ReplaceAll([&](const ast::WhileStatement* w) -> const ast::Statement* {
        utils::Vector<const ast::Statement*, 16> stmts;
        auto* cond = w->condition;

        // !condition
        auto* not_cond = b.Not(ctx.Clone(cond));

        // { break; }
        auto* break_body = b.Block(b.Break());

        // if (!condition) { break; }
        stmts.Push(b.If(not_cond, break_body));

        for (auto* stmt : w->body->statements) {
            stmts.Push(ctx.Clone(stmt));
        }

        const ast::BlockStatement* continuing = nullptr;

        auto* body = b.Block(stmts);
        auto* loop = b.Loop(body, continuing);

        return loop;
    });

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
