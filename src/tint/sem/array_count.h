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

#ifndef SRC_TINT_SEM_ARRAY_COUNT_H_
#define SRC_TINT_SEM_ARRAY_COUNT_H_

#include <string>

#include "src/tint/sem/expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/array_count.h"

namespace tint::sem {

/// The variant of an ArrayCount when the count is a named override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N>
/// ```
class NamedOverrideArrayCount final : public Castable<NamedOverrideArrayCount, type::ArrayCount> {
  public:
    /// Constructor
    /// @param var the `override` variable
    explicit NamedOverrideArrayCount(const GlobalVariable* var);
    ~NamedOverrideArrayCount() override;

    /// @returns a hash of the array count.
    size_t Hash() const override;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    bool Equals(const type::ArrayCount& t) const override;

    /// @param symbols the symbol table
    /// @returns the friendly name for this array count
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// The `override` variable.
    const GlobalVariable* variable;
};

/// The variant of an ArrayCount when the count is an unnamed override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N*2>
/// ```
class UnnamedOverrideArrayCount final
    : public Castable<UnnamedOverrideArrayCount, type::ArrayCount> {
  public:
    /// Constructor
    /// @param e the override expression
    explicit UnnamedOverrideArrayCount(const Expression* e);
    ~UnnamedOverrideArrayCount() override;

    /// @returns a hash of the array count.
    size_t Hash() const override;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    bool Equals(const type::ArrayCount& t) const override;

    /// @param symbols the symbol table
    /// @returns the friendly name for this array count
    std::string FriendlyName(const SymbolTable& symbols) const override;

    /// The unnamed override expression.
    /// Note: Each AST expression gets a unique semantic expression node, so two equivalent AST
    /// expressions will not result in the same `expr` pointer. This property is important to ensure
    /// that two array declarations with equivalent AST expressions do not compare equal.
    /// For example, consider:
    /// ```
    /// override size : u32;
    /// var<workgroup> a : array<f32, size * 2>;
    /// var<workgroup> b : array<f32, size * 2>;
    /// ```
    // The array count for `a` and `b` have equivalent AST expressions, but the types for `a` and
    // `b` must not compare equal.
    const Expression* expr;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_ARRAY_COUNT_H_
