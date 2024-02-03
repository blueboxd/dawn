// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_RENAMER_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_RENAMER_H_

#include <string>
#include <unordered_map>

#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::ast::transform {

/// Renamer is a Transform that renames all the symbols in a program.
class Renamer final : public Castable<Renamer, Transform> {
  public:
    /// Remappings is a map of old symbol name to new symbol name
    using Remappings = std::unordered_map<std::string, std::string>;

    /// Data is outputted by the Renamer transform.
    /// Data holds information about shader usage and constant buffer offsets.
    struct Data final : public Castable<Data, transform::Data> {
        /// Constructor
        /// @param remappings the symbol remappings
        explicit Data(Remappings&& remappings);

        /// Copy constructor
        Data(const Data&);

        /// Destructor
        ~Data() override;

        /// A map of old symbol name to new symbol name
        const Remappings remappings;
    };

    /// Target is an enumerator of rename targets that can be used
    enum class Target {
        /// Rename every symbol.
        kAll,
        /// Only rename symbols that are reserved keywords in GLSL.
        kGlslKeywords,
        /// Only rename symbols that are reserved keywords in HLSL.
        kHlslKeywords,
        /// Only rename symbols that are reserved keywords in MSL.
        kMslKeywords,
    };

    /// Optional configuration options for the transform.
    /// If omitted, then the renamer will use Target::kAll.
    struct Config final : public Castable<Config, transform::Data> {
        /// Constructor
        /// @param tgt the targets to rename
        /// @param keep_unicode if false, symbols with non-ascii code-points are
        /// renamed
        explicit Config(Target tgt, bool keep_unicode = false);

        /// Constructor
        /// @param tgt the targets to rename
        /// @param keep_unicode if false, symbols with non-ascii code-points are renamed
        /// @param remappings requested old to new name map
        explicit Config(Target tgt, bool keep_unicode, Remappings&& remappings);

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// The targets to rename
        Target const target = Target::kAll;

        /// If false, symbols with non-ascii code-points are renamed.
        bool preserve_unicode = false;

        /// Requested renaming rules
        const Remappings requested_names = {};
    };

    /// Constructor using a the configuration provided in the input Data
    Renamer();

    /// Destructor
    ~Renamer() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_RENAMER_H_
