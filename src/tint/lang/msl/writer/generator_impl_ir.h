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

#ifndef SRC_TINT_LANG_MSL_WRITER_GENERATOR_IMPL_IR_H_
#define SRC_TINT_LANG_MSL_WRITER_GENERATOR_IMPL_IR_H_

#include <string>
#include <unordered_set>

#include "src/tint/ir/module.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/writer/ir_text_generator.h"

namespace tint::writer::msl {

/// Implementation class for the MSL generator
class GeneratorImplIr : public IRTextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit GeneratorImplIr(ir::Module* module);
    ~GeneratorImplIr() override;

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(ir::Function* func);

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(utils::StringStream& out, const type::Type* ty);

    /// Handles generating an array declaration
    /// @param out the output stream
    /// @param arr the array to emit
    void EmitArrayType(utils::StringStream& out, const type::Array* arr);
    /// Handles generating an atomic declaration
    /// @param out the output stream
    /// @param atomic the atomic to emit
    void EmitAtomicType(utils::StringStream& out, const type::Atomic* atomic);
    /// Handles generating a pointer declaration
    /// @param out the output stream
    /// @param ptr the pointer to emit
    void EmitPointerType(utils::StringStream& out, const type::Pointer* ptr);
    /// Handles generating a vector declaration
    /// @param out the output stream
    /// @param vec the vector to emit
    void EmitVectorType(utils::StringStream& out, const type::Vector* vec);
    /// Handles generating a matrix declaration
    /// @param out the output stream
    /// @param mat the matrix to emit
    void EmitMatrixType(utils::StringStream& out, const type::Matrix* mat);
    /// Handles generating a texture declaration
    /// @param out the output stream
    /// @param tex the texture to emit
    void EmitTextureType(utils::StringStream& out, const type::Texture* tex);
    /// Handles generating a struct declaration. If the structure has already been emitted, then
    /// this function will simply return without emitting anything.
    /// @param str the struct to generate
    void EmitStructType(const type::Struct* str);

    /// Handles generating a address space
    /// @param out the output of the type stream
    /// @param sc the address space to generate
    void EmitAddressSpace(utils::StringStream& out, builtin::AddressSpace sc);

    /// Handles ir::Constant values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(utils::StringStream& out, ir::Constant* c);
    /// Handles constant::Value values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(utils::StringStream& out, const constant::Value* c);

    /// @returns the name of the templated `tint_array` helper type, generating it if needed
    const std::string& ArrayTemplateName();

    /// Unique name of the tint_array<T, N> template.
    /// Non-empty only if the template has been generated.
    std::string array_template_name_;

  private:
    /// Unique name of the 'TINT_INVARIANT' preprocessor define.
    /// Non-empty only if an invariant attribute has been generated.
    std::string invariant_define_name_;

    std::unordered_set<const type::Struct*> emitted_structs_;
};

}  // namespace tint::writer::msl

#endif  // SRC_TINT_LANG_MSL_WRITER_GENERATOR_IMPL_IR_H_
