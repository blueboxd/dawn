// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_AST_TRANSFORM_TRANSFORM_H_
#define SRC_TINT_AST_TRANSFORM_TRANSFORM_H_

#include "src/tint/transform/transform.h"

#include <utility>

#include "src/tint/program.h"
#include "src/tint/utils/castable.h"

namespace tint::ast::transform {

/// The return type of Run()
class Output {
  public:
    /// Constructor
    Output();

    /// Constructor
    /// @param program the program to move into this Output
    explicit Output(Program&& program);

    /// Constructor
    /// @param program_ the program to move into this Output
    /// @param data_ a variadic list of additional data unique_ptrs produced by
    /// the transform
    template <typename... DATA>
    Output(Program&& program_, DATA... data_)
        : program(std::move(program_)), data(std::forward<DATA>(data_)...) {}

    /// The transformed program. May be empty on error.
    Program program;

    /// Extra output generated by the transforms.
    tint::transform::DataMap data;
};

/// Interface for Program transforms
class Transform : public utils::Castable<Transform, tint::transform::Transform> {
  public:
    /// Constructor
    Transform();
    /// Destructor
    ~Transform() override;

    /// Runs the transform on @p program, returning the transformation result or a clone of
    /// @p program.
    /// @param program the source program to transform
    /// @param data optional extra transform-specific input data
    /// @returns the transformation result
    Output Run(const Program* program, const DataMap& data = {}) const;

    /// The return value of Apply().
    /// If SkipTransform (std::nullopt), then the transform is not needed to be run.
    using ApplyResult = std::optional<Program>;

    /// Value returned from Apply() to indicate that the transform does not need to be run
    static inline constexpr std::nullopt_t SkipTransform = std::nullopt;

    /// Runs the transform on `program`, return.
    /// @param program the input program
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    /// @returns a transformed program, or std::nullopt if the transform didn't need to run.
    virtual ApplyResult Apply(const Program* program,
                              const DataMap& inputs,
                              DataMap& outputs) const = 0;

    /// CreateASTTypeFor constructs new Type that reconstructs the semantic type `ty`.
    /// @param ctx the clone context
    /// @param ty the semantic type to reconstruct
    /// @returns an Type that when resolved, will produce the semantic type `ty`.
    static Type CreateASTTypeFor(CloneContext& ctx, const type::Type* ty);

  protected:
    /// Removes the statement `stmt` from the transformed program.
    /// RemoveStatement handles edge cases, like statements in the initializer and
    /// continuing of for-loops.
    /// @param ctx the clone context
    /// @param stmt the statement to remove when the program is cloned
    static void RemoveStatement(CloneContext& ctx, const Statement* stmt);
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_AST_TRANSFORM_TRANSFORM_H_
