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

#include "src/tint/ir/transform/shader_io_spirv.h"

#include <memory>
#include <utility>

#include "src/tint/ir/builder.h"
#include "src/tint/ir/module.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/struct.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::ShaderIOSpirv);

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

namespace tint::ir::transform {

ShaderIOSpirv::ShaderIOSpirv() = default;

ShaderIOSpirv::~ShaderIOSpirv() = default;

namespace {

/// PIMPL state for the parts of the shader IO transform specific to SPIR-V.
/// For SPIR-V, we split builtins and locations into two separate structures each for input and
/// output, and declare global variables for them. The wrapper entry point then loads from and
/// stores to these variables.
/// We also modify the type of the SampleMask builtin to be an array, as required by Vulkan.
struct StateImpl : ShaderIO::BackendState {
    /// The global variable for input builtins.
    Var* builtin_input_var = nullptr;
    /// The global variable for input locations.
    Var* location_input_var = nullptr;
    /// The global variable for output builtins.
    Var* builtin_output_var = nullptr;
    /// The global variable for output locations.
    Var* location_output_var = nullptr;
    /// The member indices for inputs.
    utils::Vector<uint32_t, 4> input_indices;
    /// The member indices for outputs.
    utils::Vector<uint32_t, 4> output_indices;

    /// Constructor
    /// @copydoc ShaderIO::BackendState::BackendState
    using ShaderIO::BackendState::BackendState;
    /// Destructor
    ~StateImpl() override {}

    /// Split the members listed in @p entries into two separate structures for builtins and
    /// locations, and make global variables for them. Record the new member indices in @p indices.
    /// @param builtin_var the generated global variable for builtins
    /// @param location_var the generated global variable for locations
    /// @param indices the new member indices
    /// @param entries the entries to split
    /// @param addrspace the address to use for the global variables
    /// @param access the access mode to use for the global variables
    /// @param name_suffix the suffix to add to struct and variable names
    void MakeStructs(Var*& builtin_var,
                     Var*& location_var,
                     utils::Vector<uint32_t, 4>* indices,
                     utils::Vector<type::Manager::StructMemberDesc, 4>& entries,
                     builtin::AddressSpace addrspace,
                     builtin::Access access,
                     const char* name_suffix) {
        // Build separate lists of builtin and location entries and record their new indices.
        uint32_t next_builtin_idx = 0;
        uint32_t next_location_idx = 0;
        utils::Vector<type::Manager::StructMemberDesc, 4> builtin_members;
        utils::Vector<type::Manager::StructMemberDesc, 4> location_members;
        for (auto io : entries) {
            if (io.attributes.builtin) {
                // SampleMask must be an array for Vulkan.
                if (io.attributes.builtin.value() == builtin::BuiltinValue::kSampleMask) {
                    io.type = ty.array<u32, 1>();
                }
                builtin_members.Push(io);
                indices->Push(next_builtin_idx++);
            } else {
                location_members.Push(io);
                indices->Push(next_location_idx++);
            }
        }

        // Declare the structs and variables if needed.
        auto make_struct = [&](auto& members, const char* iotype) {
            auto name = ir->NameOf(func).Name() + iotype + name_suffix;
            auto* str = ty.Struct(ir->symbols.New(name + "Struct"), std::move(members));
            auto* var = b.Var(name, ty.ptr(addrspace, str, access));
            str->SetStructFlag(type::kBlock);
            b.RootBlock()->Append(var);
            return var;
        };
        if (!builtin_members.IsEmpty()) {
            builtin_var = make_struct(builtin_members, "_Builtin");
        }
        if (!location_members.IsEmpty()) {
            location_var = make_struct(location_members, "_Location");
        }
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    utils::Vector<FunctionParam*, 4> FinalizeInputs() override {
        MakeStructs(builtin_input_var, location_input_var, &input_indices, inputs,
                    builtin::AddressSpace::kIn, builtin::Access::kRead, "Inputs");
        return utils::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    Value* FinalizeOutputs() override {
        MakeStructs(builtin_output_var, location_output_var, &output_indices, outputs,
                    builtin::AddressSpace::kOut, builtin::Access::kWrite, "Outputs");
        return nullptr;
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    Value* GetInput(Builder& builder, uint32_t idx) override {
        // Load the input from the global variable declared earlier.
        auto* ptr = ty.ptr(builtin::AddressSpace::kIn, inputs[idx].type, builtin::Access::kRead);
        Access* from = nullptr;
        if (inputs[idx].attributes.builtin) {
            if (inputs[idx].attributes.builtin.value() == builtin::BuiltinValue::kSampleMask) {
                // SampleMask becomes an array for SPIR-V, so load from the first element.
                from = builder.Access(ptr, builtin_input_var, u32(input_indices[idx]), 0_u);
            } else {
                from = builder.Access(ptr, builtin_input_var, u32(input_indices[idx]));
            }
        } else {
            from = builder.Access(ptr, location_input_var, u32(input_indices[idx]));
        }
        return builder.Load(from)->Result();
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(Builder& builder, uint32_t idx, Value* value) override {
        // Store the output to the global variable declared earlier.
        auto* ptr = ty.ptr(builtin::AddressSpace::kOut, outputs[idx].type, builtin::Access::kWrite);
        Access* to = nullptr;
        if (outputs[idx].attributes.builtin) {
            if (outputs[idx].attributes.builtin.value() == builtin::BuiltinValue::kSampleMask) {
                // SampleMask becomes an array for SPIR-V, so store to the first element.
                to = builder.Access(ptr, builtin_output_var, u32(output_indices[idx]), 0_u);
            } else {
                to = builder.Access(ptr, builtin_output_var, u32(output_indices[idx]));
            }
        } else {
            to = builder.Access(ptr, location_output_var, u32(output_indices[idx]));
        }
        builder.Store(to, value);
    }
};
}  // namespace

std::unique_ptr<ShaderIO::BackendState> ShaderIOSpirv::MakeBackendState(Module* mod,
                                                                        Function* func) const {
    return std::make_unique<StateImpl>(mod, func);
}

}  // namespace tint::ir::transform
