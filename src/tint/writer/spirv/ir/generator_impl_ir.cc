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

#include "src/tint/writer/spirv/ir/generator_impl_ir.h"

#include <utility>

#include "spirv/unified1/GLSL.std.450.h"
#include "spirv/unified1/spirv.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/access.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/block_param.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/builtin_call.h"
#include "src/tint/ir/construct.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/let.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/multi_in_block.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/terminate_invocation.h"
#include "src/tint/ir/terminator.h"
#include "src/tint/ir/transform/add_empty_entry_point.h"
#include "src/tint/ir/transform/block_decorated_structs.h"
#include "src/tint/ir/transform/builtin_polyfill_spirv.h"
#include "src/tint/ir/transform/demote_to_helper.h"
#include "src/tint/ir/transform/merge_return.h"
#include "src/tint/ir/transform/shader_io_spirv.h"
#include "src/tint/ir/transform/var_for_dynamic_index.h"
#include "src/tint/ir/unreachable.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/validate.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/transform/manager.h"
#include "src/tint/type/array.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/sampler.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/struct.h"
#include "src/tint/type/texture.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/type/void.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/spirv/generator.h"
#include "src/tint/writer/spirv/module.h"

namespace tint::writer::spirv {

namespace {

using namespace tint::number_suffixes;  // NOLINT

constexpr uint32_t kGeneratorVersion = 1;

void Sanitize(ir::Module* module) {
    transform::Manager manager;
    transform::DataMap data;

    manager.Add<ir::transform::AddEmptyEntryPoint>();
    manager.Add<ir::transform::BlockDecoratedStructs>();
    manager.Add<ir::transform::BuiltinPolyfillSpirv>();
    manager.Add<ir::transform::DemoteToHelper>();
    manager.Add<ir::transform::MergeReturn>();
    manager.Add<ir::transform::ShaderIOSpirv>();
    manager.Add<ir::transform::VarForDynamicIndex>();

    data.Add<ir::transform::ShaderIO::Config>(ir::transform::ShaderIO::Config());

    transform::DataMap outputs;
    manager.Run(module, data, outputs);
}

SpvStorageClass StorageClass(builtin::AddressSpace addrspace) {
    switch (addrspace) {
        case builtin::AddressSpace::kHandle:
            return SpvStorageClassUniformConstant;
        case builtin::AddressSpace::kFunction:
            return SpvStorageClassFunction;
        case builtin::AddressSpace::kIn:
            return SpvStorageClassInput;
        case builtin::AddressSpace::kPrivate:
            return SpvStorageClassPrivate;
        case builtin::AddressSpace::kOut:
            return SpvStorageClassOutput;
        case builtin::AddressSpace::kStorage:
            return SpvStorageClassStorageBuffer;
        case builtin::AddressSpace::kUniform:
            return SpvStorageClassUniform;
        case builtin::AddressSpace::kWorkgroup:
            return SpvStorageClassWorkgroup;
        default:
            return SpvStorageClassMax;
    }
}

}  // namespace

GeneratorImplIr::GeneratorImplIr(ir::Module* module, bool zero_init_workgroup_mem)
    : ir_(module), zero_init_workgroup_memory_(zero_init_workgroup_mem) {}

bool GeneratorImplIr::Generate() {
    auto valid = ir::Validate(*ir_);
    if (!valid) {
        diagnostics_ = valid.Failure();
        return false;
    }

    // Run the IR transformations to prepare for SPIR-V emission.
    Sanitize(ir_);

    // TODO(crbug.com/tint/1906): Check supported extensions.

    module_.PushCapability(SpvCapabilityShader);
    module_.PushMemoryModel(spv::Op::OpMemoryModel, {U32Operand(SpvAddressingModelLogical),
                                                     U32Operand(SpvMemoryModelGLSL450)});

    // TODO(crbug.com/tint/1906): Emit extensions.

    // Emit module-scope declarations.
    if (ir_->root_block) {
        EmitRootBlock(ir_->root_block);
    }

    // Emit functions.
    for (auto* func : ir_->functions) {
        EmitFunction(func);
    }

    if (diagnostics_.contains_errors()) {
        return false;
    }

    // Serialize the module into binary SPIR-V.
    writer_.WriteHeader(module_.IdBound(), kGeneratorVersion);
    writer_.WriteModule(&module_);

    return true;
}

uint32_t GeneratorImplIr::Builtin(builtin::BuiltinValue builtin, builtin::AddressSpace addrspace) {
    switch (builtin) {
        case builtin::BuiltinValue::kPointSize:
            return SpvBuiltInPointSize;
        case builtin::BuiltinValue::kFragDepth:
            return SpvBuiltInFragDepth;
        case builtin::BuiltinValue::kFrontFacing:
            return SpvBuiltInFrontFacing;
        case builtin::BuiltinValue::kGlobalInvocationId:
            return SpvBuiltInGlobalInvocationId;
        case builtin::BuiltinValue::kInstanceIndex:
            return SpvBuiltInInstanceIndex;
        case builtin::BuiltinValue::kLocalInvocationId:
            return SpvBuiltInLocalInvocationId;
        case builtin::BuiltinValue::kLocalInvocationIndex:
            return SpvBuiltInLocalInvocationIndex;
        case builtin::BuiltinValue::kNumWorkgroups:
            return SpvBuiltInNumWorkgroups;
        case builtin::BuiltinValue::kPosition:
            if (addrspace == builtin::AddressSpace::kOut) {
                // Vertex output.
                return SpvBuiltInPosition;
            } else {
                // Fragment input.
                return SpvBuiltInFragCoord;
            }
        case builtin::BuiltinValue::kSampleIndex:
            module_.PushCapability(SpvCapabilitySampleRateShading);
            return SpvBuiltInSampleId;
        case builtin::BuiltinValue::kSampleMask:
            return SpvBuiltInSampleMask;
        case builtin::BuiltinValue::kVertexIndex:
            return SpvBuiltInVertexIndex;
        case builtin::BuiltinValue::kWorkgroupId:
            return SpvBuiltInWorkgroupId;
        case builtin::BuiltinValue::kUndefined:
            return SpvBuiltInMax;
    }
    return SpvBuiltInMax;
}

uint32_t GeneratorImplIr::Constant(ir::Constant* constant) {
    auto id = Constant(constant->Value());

    // Set the name for the SPIR-V result ID if provided in the module.
    if (auto name = ir_->NameOf(constant)) {
        module_.PushDebug(spv::Op::OpName, {id, Operand(name.Name())});
    }

    return id;
}

uint32_t GeneratorImplIr::Constant(const constant::Value* constant) {
    return constants_.GetOrCreate(constant, [&] {
        auto id = module_.NextId();
        auto* ty = constant->Type();
        Switch(
            ty,  //
            [&](const type::Bool*) {
                module_.PushType(
                    constant->ValueAs<bool>() ? spv::Op::OpConstantTrue : spv::Op::OpConstantFalse,
                    {Type(ty), id});
            },
            [&](const type::I32*) {
                module_.PushType(spv::Op::OpConstant, {Type(ty), id, constant->ValueAs<u32>()});
            },
            [&](const type::U32*) {
                module_.PushType(spv::Op::OpConstant,
                                 {Type(ty), id, U32Operand(constant->ValueAs<i32>())});
            },
            [&](const type::F32*) {
                module_.PushType(spv::Op::OpConstant, {Type(ty), id, constant->ValueAs<f32>()});
            },
            [&](const type::F16*) {
                module_.PushType(
                    spv::Op::OpConstant,
                    {Type(ty), id, U32Operand(constant->ValueAs<f16>().BitsRepresentation())});
            },
            [&](const type::Vector* vec) {
                OperandList operands = {Type(ty), id};
                for (uint32_t i = 0; i < vec->Width(); i++) {
                    operands.push_back(Constant(constant->Index(i)));
                }
                module_.PushType(spv::Op::OpConstantComposite, operands);
            },
            [&](const type::Matrix* mat) {
                OperandList operands = {Type(ty), id};
                for (uint32_t i = 0; i < mat->columns(); i++) {
                    operands.push_back(Constant(constant->Index(i)));
                }
                module_.PushType(spv::Op::OpConstantComposite, operands);
            },
            [&](const type::Array* arr) {
                TINT_ASSERT(Writer, arr->ConstantCount());
                OperandList operands = {Type(ty), id};
                for (uint32_t i = 0; i < arr->ConstantCount(); i++) {
                    operands.push_back(Constant(constant->Index(i)));
                }
                module_.PushType(spv::Op::OpConstantComposite, operands);
            },
            [&](const type::Struct* str) {
                OperandList operands = {Type(ty), id};
                for (uint32_t i = 0; i < str->Members().Length(); i++) {
                    operands.push_back(Constant(constant->Index(i)));
                }
                module_.PushType(spv::Op::OpConstantComposite, operands);
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled constant type: " << ty->FriendlyName();
            });
        return id;
    });
}

uint32_t GeneratorImplIr::ConstantNull(const type::Type* type) {
    return constant_nulls_.GetOrCreate(type, [&] {
        auto id = module_.NextId();
        module_.PushType(spv::Op::OpConstantNull, {Type(type), id});
        return id;
    });
}

uint32_t GeneratorImplIr::Undef(const type::Type* type) {
    return undef_values_.GetOrCreate(type, [&] {
        auto id = module_.NextId();
        module_.PushType(spv::Op::OpUndef, {Type(type), id});
        return id;
    });
}

uint32_t GeneratorImplIr::Type(const type::Type* ty,
                               builtin::AddressSpace addrspace /* = kUndefined */) {
    return types_.GetOrCreate(ty, [&] {
        auto id = module_.NextId();
        Switch(
            ty,  //
            [&](const type::Void*) { module_.PushType(spv::Op::OpTypeVoid, {id}); },
            [&](const type::Bool*) { module_.PushType(spv::Op::OpTypeBool, {id}); },
            [&](const type::I32*) {
                module_.PushType(spv::Op::OpTypeInt, {id, 32u, 1u});
            },
            [&](const type::U32*) {
                module_.PushType(spv::Op::OpTypeInt, {id, 32u, 0u});
            },
            [&](const type::F32*) {
                module_.PushType(spv::Op::OpTypeFloat, {id, 32u});
            },
            [&](const type::F16*) {
                module_.PushCapability(SpvCapabilityFloat16);
                module_.PushCapability(SpvCapabilityUniformAndStorageBuffer16BitAccess);
                module_.PushCapability(SpvCapabilityStorageBuffer16BitAccess);
                module_.PushCapability(SpvCapabilityStorageInputOutput16);
                module_.PushType(spv::Op::OpTypeFloat, {id, 16u});
            },
            [&](const type::Vector* vec) {
                module_.PushType(spv::Op::OpTypeVector, {id, Type(vec->type()), vec->Width()});
            },
            [&](const type::Matrix* mat) {
                module_.PushType(spv::Op::OpTypeMatrix,
                                 {id, Type(mat->ColumnType()), mat->columns()});
            },
            [&](const type::Array* arr) {
                if (arr->ConstantCount()) {
                    auto* count = ir_->constant_values.Get(u32(arr->ConstantCount().value()));
                    module_.PushType(spv::Op::OpTypeArray,
                                     {id, Type(arr->ElemType()), Constant(count)});
                } else {
                    TINT_ASSERT(Writer, arr->Count()->Is<type::RuntimeArrayCount>());
                    module_.PushType(spv::Op::OpTypeRuntimeArray, {id, Type(arr->ElemType())});
                }
                module_.PushAnnot(spv::Op::OpDecorate,
                                  {id, U32Operand(SpvDecorationArrayStride), arr->Stride()});
            },
            [&](const type::Pointer* ptr) {
                module_.PushType(spv::Op::OpTypePointer,
                                 {id, U32Operand(StorageClass(ptr->AddressSpace())),
                                  Type(ptr->StoreType(), ptr->AddressSpace())});
            },
            [&](const type::Struct* str) { EmitStructType(id, str, addrspace); },
            [&](const type::Texture* tex) { EmitTextureType(id, tex); },
            [&](const type::Sampler* s) {
                module_.PushType(spv::Op::OpTypeSampler, {id});

                // Register both of the sampler types, as they're the same in SPIR-V.
                if (s->kind() == type::SamplerKind::kSampler) {
                    types_.Add(
                        ir_->Types().Get<type::Sampler>(type::SamplerKind::kComparisonSampler), id);
                } else {
                    types_.Add(ir_->Types().Get<type::Sampler>(type::SamplerKind::kSampler), id);
                }
            },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_) << "unhandled type: " << ty->FriendlyName();
            });
        return id;
    });
}

uint32_t GeneratorImplIr::Value(ir::Instruction* inst) {
    return Value(inst->Result());
}

uint32_t GeneratorImplIr::Value(ir::Value* value) {
    return Switch(
        value,  //
        [&](ir::Constant* constant) { return Constant(constant); },
        [&](ir::Value*) { return values_.GetOrCreate(value, [&] { return module_.NextId(); }); });
}

uint32_t GeneratorImplIr::Label(ir::Block* block) {
    return block_labels_.GetOrCreate(block, [&] { return module_.NextId(); });
}

void GeneratorImplIr::EmitStructType(uint32_t id,
                                     const type::Struct* str,
                                     builtin::AddressSpace addrspace /* = kUndefined */) {
    // Helper to return `type` or a potentially nested array element type within `type` as a matrix
    // type, or nullptr if no such matrix type is present.
    auto get_nested_matrix_type = [&](const type::Type* type) {
        while (auto* arr = type->As<type::Array>()) {
            type = arr->ElemType();
        }
        return type->As<type::Matrix>();
    };

    OperandList operands = {id};
    for (auto* member : str->Members()) {
        operands.push_back(Type(member->Type()));

        // Generate struct member offset decoration.
        module_.PushAnnot(
            spv::Op::OpMemberDecorate,
            {operands[0], member->Index(), U32Operand(SpvDecorationOffset), member->Offset()});

        // Generate shader IO decorations.
        const auto& attrs = member->Attributes();
        if (attrs.location) {
            module_.PushAnnot(
                spv::Op::OpMemberDecorate,
                {operands[0], member->Index(), U32Operand(SpvDecorationLocation), *attrs.location});
            if (attrs.interpolation) {
                switch (attrs.interpolation->type) {
                    case builtin::InterpolationType::kLinear:
                        module_.PushAnnot(
                            spv::Op::OpMemberDecorate,
                            {operands[0], member->Index(), U32Operand(SpvDecorationNoPerspective)});
                        break;
                    case builtin::InterpolationType::kFlat:
                        module_.PushAnnot(
                            spv::Op::OpMemberDecorate,
                            {operands[0], member->Index(), U32Operand(SpvDecorationFlat)});
                        break;
                    case builtin::InterpolationType::kPerspective:
                    case builtin::InterpolationType::kUndefined:
                        break;
                }
                switch (attrs.interpolation->sampling) {
                    case builtin::InterpolationSampling::kCentroid:
                        module_.PushAnnot(
                            spv::Op::OpMemberDecorate,
                            {operands[0], member->Index(), U32Operand(SpvDecorationCentroid)});
                        break;
                    case builtin::InterpolationSampling::kSample:
                        module_.PushCapability(SpvCapabilitySampleRateShading);
                        module_.PushAnnot(
                            spv::Op::OpMemberDecorate,
                            {operands[0], member->Index(), U32Operand(SpvDecorationSample)});
                        break;
                    case builtin::InterpolationSampling::kCenter:
                    case builtin::InterpolationSampling::kUndefined:
                        break;
                }
            }
        }
        if (attrs.builtin) {
            module_.PushAnnot(spv::Op::OpMemberDecorate,
                              {operands[0], member->Index(), U32Operand(SpvDecorationBuiltIn),
                               Builtin(*attrs.builtin, addrspace)});
        }
        if (attrs.invariant) {
            module_.PushAnnot(spv::Op::OpMemberDecorate,
                              {operands[0], member->Index(), U32Operand(SpvDecorationInvariant)});
        }

        // Emit matrix layout decorations if necessary.
        if (auto* matrix_type = get_nested_matrix_type(member->Type())) {
            const uint32_t effective_row_count = (matrix_type->rows() == 2) ? 2 : 4;
            module_.PushAnnot(spv::Op::OpMemberDecorate,
                              {id, member->Index(), U32Operand(SpvDecorationColMajor)});
            module_.PushAnnot(spv::Op::OpMemberDecorate,
                              {id, member->Index(), U32Operand(SpvDecorationMatrixStride),
                               Operand(effective_row_count * matrix_type->type()->Size())});
        }

        if (member->Name().IsValid()) {
            module_.PushDebug(spv::Op::OpMemberName,
                              {operands[0], member->Index(), Operand(member->Name().Name())});
        }
    }
    module_.PushType(spv::Op::OpTypeStruct, std::move(operands));

    // Add a Block decoration if necessary.
    if (str->StructFlags().Contains(type::StructFlag::kBlock)) {
        module_.PushAnnot(spv::Op::OpDecorate, {id, U32Operand(SpvDecorationBlock)});
    }

    if (str->Name().IsValid()) {
        module_.PushDebug(spv::Op::OpName, {operands[0], Operand(str->Name().Name())});
    }
}

void GeneratorImplIr::EmitTextureType(uint32_t id, const type::Texture* texture) {
    uint32_t sampled_type = Switch(
        texture,  //
        [&](const type::DepthTexture*) { return Type(ir_->Types().f32()); },
        [&](const type::DepthMultisampledTexture*) { return Type(ir_->Types().f32()); },
        [&](const type::SampledTexture* t) { return Type(t->type()); },
        [&](const type::MultisampledTexture* t) { return Type(t->type()); },
        [&](const type::StorageTexture* t) { return Type(t->type()); });

    uint32_t dim = SpvDimMax;
    uint32_t array = 0u;
    switch (texture->dim()) {
        case type::TextureDimension::kNone: {
            break;
        }
        case type::TextureDimension::k1d: {
            dim = SpvDim1D;
            if (texture->Is<type::SampledTexture>()) {
                module_.PushCapability(SpvCapabilitySampled1D);
            } else if (texture->Is<type::StorageTexture>()) {
                module_.PushCapability(SpvCapabilityImage1D);
            }
            break;
        }
        case type::TextureDimension::k2d: {
            dim = SpvDim2D;
            break;
        }
        case type::TextureDimension::k2dArray: {
            dim = SpvDim2D;
            array = 1u;
            break;
        }
        case type::TextureDimension::k3d: {
            dim = SpvDim3D;
            break;
        }
        case type::TextureDimension::kCube: {
            dim = SpvDimCube;
            break;
        }
        case type::TextureDimension::kCubeArray: {
            dim = SpvDimCube;
            array = 1u;
            if (texture->IsAnyOf<type::SampledTexture, type::DepthTexture>()) {
                module_.PushCapability(SpvCapabilitySampledCubeArray);
            }
            break;
        }
    }

    // The Vulkan spec says: The "Depth" operand of OpTypeImage is ignored.
    // In SPIRV, 0 means not depth, 1 means depth, and 2 means unknown.
    // Using anything other than 0 is problematic on various Vulkan drivers.
    uint32_t depth = 0u;

    uint32_t ms = 0u;
    if (texture->IsAnyOf<type::MultisampledTexture, type::DepthMultisampledTexture>()) {
        ms = 1u;
    }

    uint32_t sampled = 2u;
    if (texture->IsAnyOf<type::MultisampledTexture, type::SampledTexture, type::DepthTexture,
                         type::DepthMultisampledTexture>()) {
        sampled = 1u;
    }

    uint32_t format = SpvImageFormat_::SpvImageFormatUnknown;
    if (auto* st = texture->As<type::StorageTexture>()) {
        format = TexelFormat(st->texel_format());
    }

    module_.PushType(spv::Op::OpTypeImage,
                     {id, sampled_type, dim, depth, array, ms, sampled, format});
}

void GeneratorImplIr::EmitFunction(ir::Function* func) {
    auto id = Value(func);

    // Emit the function name.
    module_.PushDebug(spv::Op::OpName, {id, Operand(ir_->NameOf(func).Name())});

    // Emit OpEntryPoint and OpExecutionMode declarations if needed.
    if (func->Stage() != ir::Function::PipelineStage::kUndefined) {
        EmitEntryPoint(func, id);
    }

    // Get the ID for the return type.
    auto return_type_id = Type(func->ReturnType());

    FunctionType function_type{return_type_id, {}};
    InstructionList params;

    // Generate function parameter declarations and add their type IDs to the function signature.
    for (auto* param : func->Params()) {
        auto param_type_id = Type(param->Type());
        auto param_id = Value(param);
        params.push_back(Instruction(spv::Op::OpFunctionParameter, {param_type_id, param_id}));
        function_type.param_type_ids.Push(param_type_id);
        if (auto name = ir_->NameOf(param)) {
            module_.PushDebug(spv::Op::OpName, {param_id, Operand(name.Name())});
        }
    }

    // Get the ID for the function type (creating it if needed).
    auto function_type_id = function_types_.GetOrCreate(function_type, [&] {
        auto func_ty_id = module_.NextId();
        OperandList operands = {func_ty_id, return_type_id};
        operands.insert(operands.end(), function_type.param_type_ids.begin(),
                        function_type.param_type_ids.end());
        module_.PushType(spv::Op::OpTypeFunction, operands);
        return func_ty_id;
    });

    // Declare the function.
    auto decl =
        Instruction{spv::Op::OpFunction,
                    {return_type_id, id, U32Operand(SpvFunctionControlMaskNone), function_type_id}};

    // Create a function that we will add instructions to.
    auto entry_block = module_.NextId();
    current_function_ = Function(decl, entry_block, std::move(params));
    TINT_DEFER(current_function_ = Function());

    // Emit the body of the function.
    EmitBlock(func->Block());

    // Add the function to the module.
    module_.PushFunction(current_function_);
}

void GeneratorImplIr::EmitEntryPoint(ir::Function* func, uint32_t id) {
    SpvExecutionModel stage = SpvExecutionModelMax;
    switch (func->Stage()) {
        case ir::Function::PipelineStage::kCompute: {
            stage = SpvExecutionModelGLCompute;
            module_.PushExecutionMode(
                spv::Op::OpExecutionMode,
                {id, U32Operand(SpvExecutionModeLocalSize), func->WorkgroupSize()->at(0),
                 func->WorkgroupSize()->at(1), func->WorkgroupSize()->at(2)});
            break;
        }
        case ir::Function::PipelineStage::kFragment: {
            stage = SpvExecutionModelFragment;
            module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                      {id, U32Operand(SpvExecutionModeOriginUpperLeft)});
            break;
        }
        case ir::Function::PipelineStage::kVertex: {
            stage = SpvExecutionModelVertex;
            break;
        }
        case ir::Function::PipelineStage::kUndefined:
            TINT_ICE(Writer, diagnostics_) << "undefined pipeline stage for entry point";
            return;
    }

    OperandList operands = {U32Operand(stage), id, ir_->NameOf(func).Name()};

    // Add the list of all referenced shader IO variables.
    if (ir_->root_block) {
        for (auto* global : *ir_->root_block) {
            auto* var = global->As<ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result()->Type()->As<type::Pointer>();
            if (!(ptr->AddressSpace() == builtin::AddressSpace::kIn ||
                  ptr->AddressSpace() == builtin::AddressSpace::kOut)) {
                continue;
            }

            // Determine if this IO variable is used by the entry point.
            bool used = false;
            for (const auto& use : var->Result()->Usages()) {
                auto* block = use.instruction->Block();
                while (block->Parent()) {
                    block = block->Parent()->Block();
                }
                if (block == func->Block()) {
                    used = true;
                    break;
                }
            }
            if (!used) {
                continue;
            }
            operands.push_back(Value(var));

            // Add the `DepthReplacing` execution mode if `frag_depth` is used.
            if (auto* str = ptr->StoreType()->As<type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin == builtin::BuiltinValue::kFragDepth) {
                        module_.PushExecutionMode(spv::Op::OpExecutionMode,
                                                  {id, U32Operand(SpvExecutionModeDepthReplacing)});
                    }
                }
            }
        }
    }

    module_.PushEntryPoint(spv::Op::OpEntryPoint, operands);
}

void GeneratorImplIr::EmitRootBlock(ir::Block* root_block) {
    for (auto* inst : *root_block) {
        Switch(
            inst,  //
            [&](ir::Var* v) { return EmitVar(v); },
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unimplemented root block instruction: " << inst->TypeInfo().name;
            });
    }
}

void GeneratorImplIr::EmitBlock(ir::Block* block) {
    // Emit the label.
    // Skip if this is the function's entry block, as it will be emitted by the function object.
    if (!current_function_.instructions().empty()) {
        current_function_.push_inst(spv::Op::OpLabel, {Label(block)});
    }

    // If there are no instructions in the block, it's a dead end, so we shouldn't be able to get
    // here to begin with.
    if (block->IsEmpty()) {
        current_function_.push_inst(spv::Op::OpUnreachable, {});
        return;
    }

    if (auto* mib = block->As<ir::MultiInBlock>()) {
        // Emit all OpPhi nodes for incoming branches to block.
        EmitIncomingPhis(mib);
    }

    // Emit the block's statements.
    EmitBlockInstructions(block);
}

void GeneratorImplIr::EmitIncomingPhis(ir::MultiInBlock* block) {
    // Emit Phi nodes for all the incoming block parameters
    for (size_t param_idx = 0; param_idx < block->Params().Length(); param_idx++) {
        auto* param = block->Params()[param_idx];
        OperandList ops{Type(param->Type()), Value(param)};

        for (auto* incoming : block->InboundSiblingBranches()) {
            auto* arg = incoming->Args()[param_idx];
            ops.push_back(Value(arg));
            ops.push_back(Label(incoming->Block()));
        }

        current_function_.push_inst(spv::Op::OpPhi, std::move(ops));
    }
}

void GeneratorImplIr::EmitBlockInstructions(ir::Block* block) {
    for (auto* inst : *block) {
        Switch(
            inst,                                                           //
            [&](ir::Access* a) { EmitAccess(a); },                          //
            [&](ir::Binary* b) { EmitBinary(b); },                          //
            [&](ir::Bitcast* b) { EmitBitcast(b); },                        //
            [&](ir::BuiltinCall* b) { EmitBuiltinCall(b); },                //
            [&](ir::Construct* c) { EmitConstruct(c); },                    //
            [&](ir::Convert* c) { EmitConvert(c); },                        //
            [&](ir::IntrinsicCall* i) { EmitIntrinsicCall(i); },            //
            [&](ir::Load* l) { EmitLoad(l); },                              //
            [&](ir::LoadVectorElement* l) { EmitLoadVectorElement(l); },    //
            [&](ir::Loop* l) { EmitLoop(l); },                              //
            [&](ir::Switch* sw) { EmitSwitch(sw); },                        //
            [&](ir::Swizzle* s) { EmitSwizzle(s); },                        //
            [&](ir::Store* s) { EmitStore(s); },                            //
            [&](ir::StoreVectorElement* s) { EmitStoreVectorElement(s); },  //
            [&](ir::UserCall* c) { EmitUserCall(c); },                      //
            [&](ir::Unary* u) { EmitUnary(u); },                            //
            [&](ir::Var* v) { EmitVar(v); },                                //
            [&](ir::Let* l) { EmitLet(l); },                                //
            [&](ir::If* i) { EmitIf(i); },                                  //
            [&](ir::Terminator* t) { EmitTerminator(t); },                  //
            [&](Default) {
                TINT_ICE(Writer, diagnostics_)
                    << "unimplemented instruction: " << inst->TypeInfo().name;
            });

        // Set the name for the SPIR-V result ID if provided in the module.
        if (inst->Result() && !inst->Is<ir::Var>()) {
            if (auto name = ir_->NameOf(inst)) {
                module_.PushDebug(spv::Op::OpName, {Value(inst), Operand(name.Name())});
            }
        }
    }

    if (block->IsEmpty()) {
        // If the last emitted instruction is not a branch, then this should be unreachable.
        current_function_.push_inst(spv::Op::OpUnreachable, {});
    }
}

void GeneratorImplIr::EmitTerminator(ir::Terminator* t) {
    tint::Switch(  //
        t,         //
        [&](ir::Return*) {
            if (!t->Args().IsEmpty()) {
                TINT_ASSERT(Writer, t->Args().Length() == 1u);
                OperandList operands;
                operands.push_back(Value(t->Args()[0]));
                current_function_.push_inst(spv::Op::OpReturnValue, operands);
            } else {
                current_function_.push_inst(spv::Op::OpReturn, {});
            }
            return;
        },
        [&](ir::BreakIf* breakif) {
            current_function_.push_inst(spv::Op::OpBranchConditional,
                                        {
                                            Value(breakif->Condition()),
                                            loop_merge_label_,
                                            loop_header_label_,
                                        });
        },
        [&](ir::Continue* cont) {
            current_function_.push_inst(spv::Op::OpBranch, {Label(cont->Loop()->Continuing())});
        },
        [&](ir::ExitIf*) { current_function_.push_inst(spv::Op::OpBranch, {if_merge_label_}); },
        [&](ir::ExitLoop*) { current_function_.push_inst(spv::Op::OpBranch, {loop_merge_label_}); },
        [&](ir::ExitSwitch*) {
            current_function_.push_inst(spv::Op::OpBranch, {switch_merge_label_});
        },
        [&](ir::NextIteration*) {
            current_function_.push_inst(spv::Op::OpBranch, {loop_header_label_});
        },
        [&](ir::TerminateInvocation*) { current_function_.push_inst(spv::Op::OpKill, {}); },
        [&](ir::Unreachable*) { current_function_.push_inst(spv::Op::OpUnreachable, {}); },

        [&](Default) {
            TINT_ICE(Writer, diagnostics_) << "unimplemented branch: " << t->TypeInfo().name;
        });
}

void GeneratorImplIr::EmitIf(ir::If* i) {
    auto* true_block = i->True();
    auto* false_block = i->False();

    // Generate labels for the blocks. We emit the true or false block if it:
    // 1. contains instructions other then the branch, or
    // 2. branches somewhere instead of exiting the loop (e.g. return or break), or
    // 3. the if returns a value
    // Otherwise we skip them and branch straight to the merge block.
    uint32_t merge_label = module_.NextId();
    TINT_SCOPED_ASSIGNMENT(if_merge_label_, merge_label);

    uint32_t true_label = merge_label;
    uint32_t false_label = merge_label;
    if (true_block->Length() > 1 || i->HasResults() ||
        (true_block->HasTerminator() && !true_block->Terminator()->Is<ir::ExitIf>())) {
        true_label = Label(true_block);
    }
    if (false_block->Length() > 1 || i->HasResults() ||
        (false_block->HasTerminator() && !false_block->Terminator()->Is<ir::ExitIf>())) {
        false_label = Label(false_block);
    }

    // Emit the OpSelectionMerge and OpBranchConditional instructions.
    current_function_.push_inst(spv::Op::OpSelectionMerge,
                                {merge_label, U32Operand(SpvSelectionControlMaskNone)});
    current_function_.push_inst(spv::Op::OpBranchConditional,
                                {Value(i->Condition()), true_label, false_label});

    // Emit the `true` and `false` blocks, if they're not being skipped.
    if (true_label != merge_label) {
        EmitBlock(true_block);
    }
    if (false_label != merge_label) {
        EmitBlock(false_block);
    }

    current_function_.push_inst(spv::Op::OpLabel, {merge_label});

    // Emit the OpPhis for the ExitIfs
    EmitExitPhis(i);
}

void GeneratorImplIr::EmitAccess(ir::Access* access) {
    auto* ty = access->Result()->Type();

    auto id = Value(access);
    OperandList operands = {Type(ty), id, Value(access->Object())};

    if (ty->Is<type::Pointer>()) {
        // Use OpAccessChain for accesses into pointer types.
        for (auto* idx : access->Indices()) {
            operands.push_back(Value(idx));
        }
        current_function_.push_inst(spv::Op::OpAccessChain, std::move(operands));
        return;
    }

    // For non-pointer types, we assume that the indices are constants and use OpCompositeExtract.
    // If we hit a non-constant index into a vector type, use OpVectorExtractDynamic for it.
    auto* source_ty = access->Object()->Type();
    for (auto* idx : access->Indices()) {
        if (auto* constant = idx->As<ir::Constant>()) {
            // Push the index to the chain and update the current type.
            auto i = constant->Value()->ValueAs<u32>();
            operands.push_back(i);
            source_ty = source_ty->Element(i);
        } else {
            // The VarForDynamicIndex transform ensures that only value types that are vectors
            // will be dynamically indexed, as we can use OpVectorExtractDynamic for this case.
            TINT_ASSERT(Writer, source_ty->Is<type::Vector>());

            // If this wasn't the first access in the chain then emit the chain so far as an
            // OpCompositeExtract, creating a new result ID for the resulting vector.
            auto vec_id = Value(access->Object());
            if (operands.size() > 3) {
                vec_id = module_.NextId();
                operands[0] = Type(source_ty);
                operands[1] = vec_id;
                current_function_.push_inst(spv::Op::OpCompositeExtract, std::move(operands));
            }

            // Now emit the OpVectorExtractDynamic instruction.
            operands = {Type(ty), id, vec_id, Value(idx)};
            current_function_.push_inst(spv::Op::OpVectorExtractDynamic, std::move(operands));
            return;
        }
    }
    current_function_.push_inst(spv::Op::OpCompositeExtract, std::move(operands));
}

void GeneratorImplIr::EmitBinary(ir::Binary* binary) {
    auto id = Value(binary);
    auto lhs = Value(binary->LHS());
    auto rhs = Value(binary->RHS());
    auto* ty = binary->Result()->Type();
    auto* lhs_ty = binary->LHS()->Type();
    auto* rhs_ty = binary->RHS()->Type();

    // Determine the opcode.
    spv::Op op = spv::Op::Max;
    switch (binary->Kind()) {
        case ir::Binary::Kind::kAdd: {
            op = ty->is_integer_scalar_or_vector() ? spv::Op::OpIAdd : spv::Op::OpFAdd;
            break;
        }
        case ir::Binary::Kind::kDivide: {
            if (ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSDiv;
            } else if (ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpUDiv;
            } else if (ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFDiv;
            }
            break;
        }
        case ir::Binary::Kind::kMultiply: {
            if (ty->is_integer_scalar_or_vector()) {
                // If the result is an integer then we can only use OpIMul.
                op = spv::Op::OpIMul;
            } else if (lhs_ty->is_float_scalar() && rhs_ty->is_float_scalar()) {
                // Two float scalars multiply with OpFMul.
                op = spv::Op::OpFMul;
            } else if (lhs_ty->is_float_vector() && rhs_ty->is_float_vector()) {
                // Two float vectors multiply with OpFMul.
                op = spv::Op::OpFMul;
            } else if (lhs_ty->is_float_scalar() && rhs_ty->is_float_vector()) {
                // Use OpVectorTimesScalar for scalar * vector, and swap the operand order.
                std::swap(lhs, rhs);
                op = spv::Op::OpVectorTimesScalar;
            } else if (lhs_ty->is_float_vector() && rhs_ty->is_float_scalar()) {
                // Use OpVectorTimesScalar for scalar * vector.
                op = spv::Op::OpVectorTimesScalar;
            } else if (lhs_ty->is_float_scalar() && rhs_ty->is_float_matrix()) {
                // Use OpMatrixTimesScalar for scalar * matrix, and swap the operand order.
                std::swap(lhs, rhs);
                op = spv::Op::OpMatrixTimesScalar;
            } else if (lhs_ty->is_float_matrix() && rhs_ty->is_float_scalar()) {
                // Use OpMatrixTimesScalar for scalar * matrix.
                op = spv::Op::OpMatrixTimesScalar;
            } else if (lhs_ty->is_float_vector() && rhs_ty->is_float_matrix()) {
                // Use OpVectorTimesMatrix for vector * matrix.
                op = spv::Op::OpVectorTimesMatrix;
            } else if (lhs_ty->is_float_matrix() && rhs_ty->is_float_vector()) {
                // Use OpMatrixTimesVector for matrix * vector.
                op = spv::Op::OpMatrixTimesVector;
            } else if (lhs_ty->is_float_matrix() && rhs_ty->is_float_matrix()) {
                // Use OpMatrixTimesMatrix for matrix * vector.
                op = spv::Op::OpMatrixTimesMatrix;
            }
            break;
        }
        case ir::Binary::Kind::kSubtract: {
            op = ty->is_integer_scalar_or_vector() ? spv::Op::OpISub : spv::Op::OpFSub;
            break;
        }
        case ir::Binary::Kind::kModulo: {
            if (ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSRem;
            } else if (ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpUMod;
            } else if (ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFRem;
            }
            break;
        }

        case ir::Binary::Kind::kAnd: {
            op = spv::Op::OpBitwiseAnd;
            break;
        }
        case ir::Binary::Kind::kOr: {
            op = spv::Op::OpBitwiseOr;
            break;
        }
        case ir::Binary::Kind::kXor: {
            op = spv::Op::OpBitwiseXor;
            break;
        }

        case ir::Binary::Kind::kShiftLeft: {
            op = spv::Op::OpShiftLeftLogical;
            break;
        }
        case ir::Binary::Kind::kShiftRight: {
            if (ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpShiftRightArithmetic;
            } else if (ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpShiftRightLogical;
            }
            break;
        }

        case ir::Binary::Kind::kEqual: {
            if (lhs_ty->is_bool_scalar_or_vector()) {
                op = spv::Op::OpLogicalEqual;
            } else if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdEqual;
            } else if (lhs_ty->is_integer_scalar_or_vector()) {
                op = spv::Op::OpIEqual;
            }
            break;
        }
        case ir::Binary::Kind::kNotEqual: {
            if (lhs_ty->is_bool_scalar_or_vector()) {
                op = spv::Op::OpLogicalNotEqual;
            } else if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdNotEqual;
            } else if (lhs_ty->is_integer_scalar_or_vector()) {
                op = spv::Op::OpINotEqual;
            }
            break;
        }
        case ir::Binary::Kind::kGreaterThan: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdGreaterThan;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSGreaterThan;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpUGreaterThan;
            }
            break;
        }
        case ir::Binary::Kind::kGreaterThanEqual: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdGreaterThanEqual;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSGreaterThanEqual;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpUGreaterThanEqual;
            }
            break;
        }
        case ir::Binary::Kind::kLessThan: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdLessThan;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSLessThan;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpULessThan;
            }
            break;
        }
        case ir::Binary::Kind::kLessThanEqual: {
            if (lhs_ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFOrdLessThanEqual;
            } else if (lhs_ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSLessThanEqual;
            } else if (lhs_ty->is_unsigned_integer_scalar_or_vector()) {
                op = spv::Op::OpULessThanEqual;
            }
            break;
        }
    }

    // Emit the instruction.
    current_function_.push_inst(op, {Type(ty), id, lhs, rhs});
}

void GeneratorImplIr::EmitBitcast(ir::Bitcast* bitcast) {
    auto* ty = bitcast->Result()->Type();
    if (ty == bitcast->Val()->Type()) {
        values_.Add(bitcast->Result(), Value(bitcast->Val()));
        return;
    }
    current_function_.push_inst(spv::Op::OpBitcast,
                                {Type(ty), Value(bitcast), Value(bitcast->Val())});
}

void GeneratorImplIr::EmitBuiltinCall(ir::BuiltinCall* builtin) {
    auto* result_ty = builtin->Result()->Type();

    if (builtin->Func() == builtin::Function::kAbs &&
        result_ty->is_unsigned_integer_scalar_or_vector()) {
        // abs() is a no-op for unsigned integers.
        values_.Add(builtin->Result(), Value(builtin->Args()[0]));
        return;
    }
    if (builtin->Func() == builtin::Function::kAny &&
        builtin->Args()[0]->Type()->Is<type::Bool>()) {
        // any() is a passthrough for a scalar argument.
        values_.Add(builtin->Result(), Value(builtin->Args()[0]));
        return;
    }

    auto id = Value(builtin);

    spv::Op op = spv::Op::Max;
    OperandList operands = {Type(result_ty), id};

    // Helper to set up the opcode and operand list for a GLSL extended instruction.
    auto glsl_ext_inst = [&](enum GLSLstd450 inst) {
        constexpr const char* kGLSLstd450 = "GLSL.std.450";
        op = spv::Op::OpExtInst;
        operands.push_back(imports_.GetOrCreate(kGLSLstd450, [&] {
            // Import the instruction set the first time it is requested.
            auto import = module_.NextId();
            module_.PushExtImport(spv::Op::OpExtInstImport, {import, Operand(kGLSLstd450)});
            return import;
        }));
        operands.push_back(U32Operand(inst));
    };

    // Determine the opcode.
    switch (builtin->Func()) {
        case builtin::Function::kAbs:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450FAbs);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SAbs);
            }
            break;
        case builtin::Function::kAny:
            op = spv::Op::OpAny;
            break;
        case builtin::Function::kAcos:
            glsl_ext_inst(GLSLstd450Acos);
            break;
        case builtin::Function::kAcosh:
            glsl_ext_inst(GLSLstd450Acosh);
            break;
        case builtin::Function::kAsin:
            glsl_ext_inst(GLSLstd450Asin);
            break;
        case builtin::Function::kAsinh:
            glsl_ext_inst(GLSLstd450Asinh);
            break;
        case builtin::Function::kAtan:
            glsl_ext_inst(GLSLstd450Atan);
            break;
        case builtin::Function::kAtan2:
            glsl_ext_inst(GLSLstd450Atan2);
            break;
        case builtin::Function::kAtanh:
            glsl_ext_inst(GLSLstd450Atanh);
            break;
        case builtin::Function::kClamp:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450NClamp);
            } else if (result_ty->is_unsigned_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450UClamp);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SClamp);
            }
            break;
        case builtin::Function::kCeil:
            glsl_ext_inst(GLSLstd450Ceil);
            break;
        case builtin::Function::kCos:
            glsl_ext_inst(GLSLstd450Cos);
            break;
        case builtin::Function::kCosh:
            glsl_ext_inst(GLSLstd450Cosh);
            break;
        case builtin::Function::kCross:
            glsl_ext_inst(GLSLstd450Cross);
            break;
        case builtin::Function::kDistance:
            glsl_ext_inst(GLSLstd450Distance);
            break;
        case builtin::Function::kDpdx:
            module_.PushCapability(SpvCapabilityDerivativeControl);
            op = spv::Op::OpDPdx;
            break;
        case builtin::Function::kDpdxCoarse:
            module_.PushCapability(SpvCapabilityDerivativeControl);
            op = spv::Op::OpDPdxCoarse;
            break;
        case builtin::Function::kDpdxFine:
            module_.PushCapability(SpvCapabilityDerivativeControl);
            op = spv::Op::OpDPdxFine;
            break;
        case builtin::Function::kDpdy:
            module_.PushCapability(SpvCapabilityDerivativeControl);
            op = spv::Op::OpDPdy;
            break;
        case builtin::Function::kDpdyCoarse:
            module_.PushCapability(SpvCapabilityDerivativeControl);
            op = spv::Op::OpDPdyCoarse;
            break;
        case builtin::Function::kDpdyFine:
            module_.PushCapability(SpvCapabilityDerivativeControl);
            op = spv::Op::OpDPdyFine;
            break;
        case builtin::Function::kExp:
            glsl_ext_inst(GLSLstd450Exp);
            break;
        case builtin::Function::kExp2:
            glsl_ext_inst(GLSLstd450Exp2);
            break;
        case builtin::Function::kFloor:
            glsl_ext_inst(GLSLstd450Floor);
            break;
        case builtin::Function::kFract:
            glsl_ext_inst(GLSLstd450Fract);
            break;
        case builtin::Function::kFrexp:
            glsl_ext_inst(GLSLstd450FrexpStruct);
            break;
        case builtin::Function::kInverseSqrt:
            glsl_ext_inst(GLSLstd450InverseSqrt);
            break;
        case builtin::Function::kLength:
            glsl_ext_inst(GLSLstd450Length);
            break;
        case builtin::Function::kLog:
            glsl_ext_inst(GLSLstd450Log);
            break;
        case builtin::Function::kLog2:
            glsl_ext_inst(GLSLstd450Log2);
            break;
        case builtin::Function::kMax:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450FMax);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SMax);
            } else if (result_ty->is_unsigned_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450UMax);
            }
            break;
        case builtin::Function::kMin:
            if (result_ty->is_float_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450FMin);
            } else if (result_ty->is_signed_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450SMin);
            } else if (result_ty->is_unsigned_integer_scalar_or_vector()) {
                glsl_ext_inst(GLSLstd450UMin);
            }
            break;
        case builtin::Function::kModf:
            glsl_ext_inst(GLSLstd450ModfStruct);
            break;
        case builtin::Function::kNormalize:
            glsl_ext_inst(GLSLstd450Normalize);
            break;
        case builtin::Function::kSin:
            glsl_ext_inst(GLSLstd450Sin);
            break;
        case builtin::Function::kSinh:
            glsl_ext_inst(GLSLstd450Sinh);
            break;
        case builtin::Function::kSqrt:
            glsl_ext_inst(GLSLstd450Sqrt);
            break;
        case builtin::Function::kTan:
            glsl_ext_inst(GLSLstd450Tan);
            break;
        case builtin::Function::kTanh:
            glsl_ext_inst(GLSLstd450Tanh);
            break;
        case builtin::Function::kTrunc:
            glsl_ext_inst(GLSLstd450Trunc);
            break;
        default:
            TINT_ICE(Writer, diagnostics_) << "unimplemented builtin function: " << builtin->Func();
    }
    TINT_ASSERT(Writer, op != spv::Op::Max);

    // Add the arguments to the builtin call.
    for (auto* arg : builtin->Args()) {
        operands.push_back(Value(arg));
    }

    // Emit the instruction.
    current_function_.push_inst(op, operands);
}

void GeneratorImplIr::EmitConstruct(ir::Construct* construct) {
    OperandList operands = {Type(construct->Result()->Type()), Value(construct)};
    for (auto* arg : construct->Args()) {
        operands.push_back(Value(arg));
    }
    current_function_.push_inst(spv::Op::OpCompositeConstruct, std::move(operands));
}

void GeneratorImplIr::EmitConvert(ir::Convert* convert) {
    auto* res_ty = convert->Result()->Type();
    auto* arg_ty = convert->Args()[0]->Type();

    OperandList operands = {Type(convert->Result()->Type()), Value(convert)};
    for (auto* arg : convert->Args()) {
        operands.push_back(Value(arg));
    }

    spv::Op op = spv::Op::Max;
    if (res_ty->is_signed_integer_scalar_or_vector() && arg_ty->is_float_scalar_or_vector()) {
        // float to signed int.
        op = spv::Op::OpConvertFToS;
    } else if (res_ty->is_unsigned_integer_scalar_or_vector() &&
               arg_ty->is_float_scalar_or_vector()) {
        // float to unsigned int.
        op = spv::Op::OpConvertFToU;
    } else if (res_ty->is_float_scalar_or_vector() &&
               arg_ty->is_signed_integer_scalar_or_vector()) {
        // signed int to float.
        op = spv::Op::OpConvertSToF;
    } else if (res_ty->is_float_scalar_or_vector() &&
               arg_ty->is_unsigned_integer_scalar_or_vector()) {
        // unsigned int to float.
        op = spv::Op::OpConvertUToF;
    } else if (res_ty->is_float_scalar_or_vector() && arg_ty->is_float_scalar_or_vector() &&
               res_ty->Size() != arg_ty->Size()) {
        // float to float (different bitwidth).
        op = spv::Op::OpFConvert;
    } else if (res_ty->is_integer_scalar_or_vector() && arg_ty->is_integer_scalar_or_vector() &&
               res_ty->Size() == arg_ty->Size()) {
        // int to int (same bitwidth, different signedness).
        op = spv::Op::OpBitcast;
    } else if (res_ty->is_bool_scalar_or_vector()) {
        if (arg_ty->is_integer_scalar_or_vector()) {
            // int to bool.
            op = spv::Op::OpINotEqual;
        } else {
            // float to bool.
            op = spv::Op::OpFUnordNotEqual;
        }
        operands.push_back(ConstantNull(arg_ty));
    } else if (arg_ty->is_bool_scalar_or_vector()) {
        // Select between constant one and zero, splatting them to vectors if necessary.
        const constant::Value* one = nullptr;
        const constant::Value* zero = nullptr;
        Switch(
            res_ty->DeepestElement(),  //
            [&](const type::F32*) {
                one = ir_->constant_values.Get(1_f);
                zero = ir_->constant_values.Get(0_f);
            },
            [&](const type::F16*) {
                one = ir_->constant_values.Get(1_h);
                zero = ir_->constant_values.Get(0_h);
            },
            [&](const type::I32*) {
                one = ir_->constant_values.Get(1_i);
                zero = ir_->constant_values.Get(0_i);
            },
            [&](const type::U32*) {
                one = ir_->constant_values.Get(1_u);
                zero = ir_->constant_values.Get(0_u);
            });
        TINT_ASSERT_OR_RETURN(Writer, one && zero);

        if (auto* vec = res_ty->As<type::Vector>()) {
            // Splat the scalars into vectors.
            one = ir_->constant_values.Splat(vec, one, vec->Width());
            zero = ir_->constant_values.Splat(vec, zero, vec->Width());
        }

        op = spv::Op::OpSelect;
        operands.push_back(Constant(one));
        operands.push_back(Constant(zero));
    } else {
        TINT_ICE(Writer, diagnostics_) << "unhandled convert instruction";
    }

    current_function_.push_inst(op, std::move(operands));
}

void GeneratorImplIr::EmitIntrinsicCall(ir::IntrinsicCall* call) {
    auto id = Value(call);

    spv::Op op = spv::Op::Max;
    switch (call->Kind()) {
        case ir::IntrinsicCall::Kind::kSpirvDot:
            op = spv::Op::OpDot;
            break;
        case ir::IntrinsicCall::Kind::kSpirvSelect:
            op = spv::Op::OpSelect;
            break;
    }

    OperandList operands = {Type(call->Result()->Type()), id};
    for (auto* arg : call->Args()) {
        operands.push_back(Value(arg));
    }
    current_function_.push_inst(op, operands);
}

void GeneratorImplIr::EmitLoad(ir::Load* load) {
    current_function_.push_inst(spv::Op::OpLoad,
                                {Type(load->Result()->Type()), Value(load), Value(load->From())});
}

void GeneratorImplIr::EmitLoadVectorElement(ir::LoadVectorElement* load) {
    auto* vec_ptr_ty = load->From()->Type()->As<type::Pointer>();
    auto* el_ty = load->Result()->Type();
    auto* el_ptr_ty = ir_->Types().ptr(vec_ptr_ty->AddressSpace(), el_ty, vec_ptr_ty->Access());
    auto el_ptr_id = module_.NextId();
    current_function_.push_inst(
        spv::Op::OpAccessChain,
        {Type(el_ptr_ty), el_ptr_id, Value(load->From()), Value(load->Index())});
    current_function_.push_inst(spv::Op::OpLoad,
                                {Type(load->Result()->Type()), Value(load), el_ptr_id});
}

void GeneratorImplIr::EmitLoop(ir::Loop* loop) {
    auto init_label = loop->HasInitializer() ? Label(loop->Initializer()) : 0;
    auto body_label = Label(loop->Body());
    auto continuing_label = Label(loop->Continuing());

    auto header_label = module_.NextId();
    TINT_SCOPED_ASSIGNMENT(loop_header_label_, header_label);

    auto merge_label = module_.NextId();
    TINT_SCOPED_ASSIGNMENT(loop_merge_label_, merge_label);

    if (init_label != 0) {
        // Emit the loop initializer.
        current_function_.push_inst(spv::Op::OpBranch, {init_label});
        EmitBlock(loop->Initializer());
    } else {
        // No initializer. Branch to body.
        current_function_.push_inst(spv::Op::OpBranch, {header_label});
    }

    // Emit the loop body header, which contains the OpLoopMerge and OpPhis.
    // This then unconditionally branches to body_label
    current_function_.push_inst(spv::Op::OpLabel, {header_label});
    EmitIncomingPhis(loop->Body());
    current_function_.push_inst(
        spv::Op::OpLoopMerge, {merge_label, continuing_label, U32Operand(SpvLoopControlMaskNone)});
    current_function_.push_inst(spv::Op::OpBranch, {body_label});

    // Emit the loop body
    current_function_.push_inst(spv::Op::OpLabel, {body_label});
    EmitBlockInstructions(loop->Body());

    // Emit the loop continuing block.
    if (loop->Continuing()->HasTerminator()) {
        EmitBlock(loop->Continuing());
    } else {
        // We still need to emit a continuing block with a back-edge, even if it is unreachable.
        current_function_.push_inst(spv::Op::OpLabel, {continuing_label});
        current_function_.push_inst(spv::Op::OpBranch, {header_label});
    }

    // Emit the loop merge block.
    current_function_.push_inst(spv::Op::OpLabel, {merge_label});

    // Emit the OpPhis for the ExitLoops
    EmitExitPhis(loop);
}

void GeneratorImplIr::EmitSwitch(ir::Switch* swtch) {
    // Find the default selector. There must be exactly one.
    uint32_t default_label = 0u;
    for (auto& c : swtch->Cases()) {
        for (auto& sel : c.selectors) {
            if (sel.IsDefault()) {
                default_label = Label(c.Block());
            }
        }
    }
    TINT_ASSERT(Writer, default_label != 0u);

    // Build the operands to the OpSwitch instruction.
    OperandList switch_operands = {Value(swtch->Condition()), default_label};
    for (auto& c : swtch->Cases()) {
        auto label = Label(c.Block());
        for (auto& sel : c.selectors) {
            if (sel.IsDefault()) {
                continue;
            }
            switch_operands.push_back(sel.val->Value()->ValueAs<uint32_t>());
            switch_operands.push_back(label);
        }
    }

    uint32_t merge_label = module_.NextId();
    TINT_SCOPED_ASSIGNMENT(switch_merge_label_, merge_label);

    // Emit the OpSelectionMerge and OpSwitch instructions.
    current_function_.push_inst(spv::Op::OpSelectionMerge,
                                {merge_label, U32Operand(SpvSelectionControlMaskNone)});
    current_function_.push_inst(spv::Op::OpSwitch, switch_operands);

    // Emit the cases.
    for (auto& c : swtch->Cases()) {
        EmitBlock(c.Block());
    }

    // Emit the switch merge block.
    current_function_.push_inst(spv::Op::OpLabel, {merge_label});

    // Emit the OpPhis for the ExitSwitches
    EmitExitPhis(swtch);
}

void GeneratorImplIr::EmitSwizzle(ir::Swizzle* swizzle) {
    auto id = Value(swizzle);
    auto obj = Value(swizzle->Object());
    OperandList operands = {Type(swizzle->Result()->Type()), id, obj, obj};
    for (auto idx : swizzle->Indices()) {
        operands.push_back(idx);
    }
    current_function_.push_inst(spv::Op::OpVectorShuffle, operands);
}

void GeneratorImplIr::EmitStore(ir::Store* store) {
    current_function_.push_inst(spv::Op::OpStore, {Value(store->To()), Value(store->From())});
}

void GeneratorImplIr::EmitStoreVectorElement(ir::StoreVectorElement* store) {
    auto* vec_ptr_ty = store->To()->Type()->As<type::Pointer>();
    auto* el_ty = store->Value()->Type();
    auto* el_ptr_ty = ir_->Types().ptr(vec_ptr_ty->AddressSpace(), el_ty, vec_ptr_ty->Access());
    auto el_ptr_id = module_.NextId();
    current_function_.push_inst(
        spv::Op::OpAccessChain,
        {Type(el_ptr_ty), el_ptr_id, Value(store->To()), Value(store->Index())});
    current_function_.push_inst(spv::Op::OpStore, {el_ptr_id, Value(store->Value())});
}

void GeneratorImplIr::EmitUnary(ir::Unary* unary) {
    auto id = Value(unary);
    auto* ty = unary->Result()->Type();
    spv::Op op = spv::Op::Max;
    switch (unary->Kind()) {
        case ir::Unary::Kind::kComplement:
            op = spv::Op::OpNot;
            break;
        case ir::Unary::Kind::kNegation:
            if (ty->is_float_scalar_or_vector()) {
                op = spv::Op::OpFNegate;
            } else if (ty->is_signed_integer_scalar_or_vector()) {
                op = spv::Op::OpSNegate;
            }
            break;
    }
    current_function_.push_inst(op, {Type(ty), id, Value(unary->Val())});
}

void GeneratorImplIr::EmitUserCall(ir::UserCall* call) {
    auto id = Value(call);
    OperandList operands = {Type(call->Result()->Type()), id, Value(call->Func())};
    for (auto* arg : call->Args()) {
        operands.push_back(Value(arg));
    }
    current_function_.push_inst(spv::Op::OpFunctionCall, operands);
}

void GeneratorImplIr::EmitVar(ir::Var* var) {
    auto id = Value(var);
    auto* ptr = var->Result()->Type()->As<type::Pointer>();
    auto ty = Type(ptr);

    switch (ptr->AddressSpace()) {
        case builtin::AddressSpace::kFunction: {
            TINT_ASSERT(Writer, current_function_);
            current_function_.push_var({ty, id, U32Operand(SpvStorageClassFunction)});
            if (var->Initializer()) {
                current_function_.push_inst(spv::Op::OpStore, {id, Value(var->Initializer())});
            }
            break;
        }
        case builtin::AddressSpace::kIn: {
            TINT_ASSERT(Writer, !current_function_);
            module_.PushType(spv::Op::OpVariable, {ty, id, U32Operand(SpvStorageClassInput)});
            break;
        }
        case builtin::AddressSpace::kPrivate: {
            TINT_ASSERT(Writer, !current_function_);
            OperandList operands = {ty, id, U32Operand(SpvStorageClassPrivate)};
            if (var->Initializer()) {
                TINT_ASSERT(Writer, var->Initializer()->Is<ir::Constant>());
                operands.push_back(Value(var->Initializer()));
            }
            module_.PushType(spv::Op::OpVariable, operands);
            break;
        }
        case builtin::AddressSpace::kOut: {
            TINT_ASSERT(Writer, !current_function_);
            module_.PushType(spv::Op::OpVariable, {ty, id, U32Operand(SpvStorageClassOutput)});
            break;
        }
        case builtin::AddressSpace::kHandle:
        case builtin::AddressSpace::kStorage:
        case builtin::AddressSpace::kUniform: {
            TINT_ASSERT(Writer, !current_function_);
            module_.PushType(spv::Op::OpVariable,
                             {ty, id, U32Operand(StorageClass(ptr->AddressSpace()))});
            auto bp = var->BindingPoint().value();
            module_.PushAnnot(spv::Op::OpDecorate,
                              {id, U32Operand(SpvDecorationDescriptorSet), bp.group});
            module_.PushAnnot(spv::Op::OpDecorate,
                              {id, U32Operand(SpvDecorationBinding), bp.binding});
            break;
        }
        case builtin::AddressSpace::kWorkgroup: {
            TINT_ASSERT(Writer, !current_function_);
            OperandList operands = {ty, id, U32Operand(SpvStorageClassWorkgroup)};
            if (zero_init_workgroup_memory_) {
                // If requested, use the VK_KHR_zero_initialize_workgroup_memory to zero-initialize
                // the workgroup variable using an null constant initializer.
                operands.push_back(ConstantNull(ptr->StoreType()));
            }
            module_.PushType(spv::Op::OpVariable, operands);
            break;
        }
        default: {
            TINT_ICE(Writer, diagnostics_)
                << "unimplemented variable address space " << ptr->AddressSpace();
        }
    }

    // Set the name if present.
    if (auto name = ir_->NameOf(var)) {
        module_.PushDebug(spv::Op::OpName, {id, Operand(name.Name())});
    }
}

void GeneratorImplIr::EmitLet(ir::Let* let) {
    auto id = Value(let->Value());
    values_.Add(let->Result(), id);
}

void GeneratorImplIr::EmitExitPhis(ir::ControlInstruction* inst) {
    struct Branch {
        uint32_t label = 0;
        ir::Value* value = nullptr;
        bool operator<(const Branch& other) const { return label < other.label; }
    };

    auto results = inst->Results();
    for (size_t index = 0; index < results.Length(); index++) {
        auto* result = results[index];
        auto* ty = result->Type();

        utils::Vector<Branch, 8> branches;
        branches.Reserve(inst->Exits().Count());
        for (auto& exit : inst->Exits()) {
            branches.Push(Branch{Label(exit->Block()), exit->Args()[index]});
        }
        branches.Sort();  // Sort the branches by label to ensure deterministic output

        OperandList ops{Type(ty), Value(result)};
        for (auto& branch : branches) {
            if (branch.value == nullptr) {
                ops.push_back(Undef(ty));
            } else {
                ops.push_back(Value(branch.value));
            }
            ops.push_back(branch.label);
        }
        current_function_.push_inst(spv::Op::OpPhi, std::move(ops));
    }
}

uint32_t GeneratorImplIr::TexelFormat(const builtin::TexelFormat format) {
    switch (format) {
        case builtin::TexelFormat::kBgra8Unorm:
            TINT_ICE(Writer, diagnostics_)
                << "bgra8unorm should have been polyfilled to rgba8unorm";
            return SpvImageFormatUnknown;
        case builtin::TexelFormat::kR32Uint:
            return SpvImageFormatR32ui;
        case builtin::TexelFormat::kR32Sint:
            return SpvImageFormatR32i;
        case builtin::TexelFormat::kR32Float:
            return SpvImageFormatR32f;
        case builtin::TexelFormat::kRgba8Unorm:
            return SpvImageFormatRgba8;
        case builtin::TexelFormat::kRgba8Snorm:
            return SpvImageFormatRgba8Snorm;
        case builtin::TexelFormat::kRgba8Uint:
            return SpvImageFormatRgba8ui;
        case builtin::TexelFormat::kRgba8Sint:
            return SpvImageFormatRgba8i;
        case builtin::TexelFormat::kRg32Uint:
            module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
            return SpvImageFormatRg32ui;
        case builtin::TexelFormat::kRg32Sint:
            module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
            return SpvImageFormatRg32i;
        case builtin::TexelFormat::kRg32Float:
            module_.PushCapability(SpvCapabilityStorageImageExtendedFormats);
            return SpvImageFormatRg32f;
        case builtin::TexelFormat::kRgba16Uint:
            return SpvImageFormatRgba16ui;
        case builtin::TexelFormat::kRgba16Sint:
            return SpvImageFormatRgba16i;
        case builtin::TexelFormat::kRgba16Float:
            return SpvImageFormatRgba16f;
        case builtin::TexelFormat::kRgba32Uint:
            return SpvImageFormatRgba32ui;
        case builtin::TexelFormat::kRgba32Sint:
            return SpvImageFormatRgba32i;
        case builtin::TexelFormat::kRgba32Float:
            return SpvImageFormatRgba32f;
        case builtin::TexelFormat::kUndefined:
            return SpvImageFormatUnknown;
    }
    return SpvImageFormatUnknown;
}

}  // namespace tint::writer::spirv
