// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/ShaderModule.h"

#include <algorithm>
#include <sstream>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Constants.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/TintUtils.h"

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
#include "dawn/native/SpirvValidation.h"
#endif

#include "tint/tint.h"

namespace dawn::native {

namespace {

ResultOrError<SingleShaderStage> TintPipelineStageToShaderStage(
    tint::inspector::PipelineStage stage) {
    switch (stage) {
        case tint::inspector::PipelineStage::kVertex:
            return SingleShaderStage::Vertex;
        case tint::inspector::PipelineStage::kFragment:
            return SingleShaderStage::Fragment;
        case tint::inspector::PipelineStage::kCompute:
            return SingleShaderStage::Compute;
    }
    DAWN_UNREACHABLE();
}

BindingInfoType TintResourceTypeToBindingInfoType(
    tint::inspector::ResourceBinding::ResourceType type) {
    switch (type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return BindingInfoType::Buffer;
        case tint::inspector::ResourceBinding::ResourceType::kSampler:
        case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
            return BindingInfoType::Sampler;
        case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
        case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
        case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
        case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
            return BindingInfoType::Texture;
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
            return BindingInfoType::StorageTexture;
        case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
            return BindingInfoType::ExternalTexture;

        default:
            DAWN_UNREACHABLE();
            return BindingInfoType::Buffer;
    }
}

wgpu::TextureFormat TintImageFormatToTextureFormat(
    tint::inspector::ResourceBinding::TexelFormat format) {
    switch (format) {
        case tint::inspector::ResourceBinding::TexelFormat::kR32Uint:
            return wgpu::TextureFormat::R32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kR32Sint:
            return wgpu::TextureFormat::R32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kR32Float:
            return wgpu::TextureFormat::R32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kBgra8Unorm:
            return wgpu::TextureFormat::BGRA8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Unorm:
            return wgpu::TextureFormat::RGBA8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Snorm:
            return wgpu::TextureFormat::RGBA8Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Uint:
            return wgpu::TextureFormat::RGBA8Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Sint:
            return wgpu::TextureFormat::RGBA8Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Uint:
            return wgpu::TextureFormat::RG32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Sint:
            return wgpu::TextureFormat::RG32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Float:
            return wgpu::TextureFormat::RG32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Uint:
            return wgpu::TextureFormat::RGBA16Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Sint:
            return wgpu::TextureFormat::RGBA16Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Float:
            return wgpu::TextureFormat::RGBA16Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Uint:
            return wgpu::TextureFormat::RGBA32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Sint:
            return wgpu::TextureFormat::RGBA32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Float:
            return wgpu::TextureFormat::RGBA32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kNone:
            return wgpu::TextureFormat::Undefined;

        default:
            DAWN_UNREACHABLE();
            return wgpu::TextureFormat::Undefined;
    }
}

wgpu::TextureViewDimension TintTextureDimensionToTextureViewDimension(
    tint::inspector::ResourceBinding::TextureDimension dim) {
    switch (dim) {
        case tint::inspector::ResourceBinding::TextureDimension::k1d:
            return wgpu::TextureViewDimension::e1D;
        case tint::inspector::ResourceBinding::TextureDimension::k2d:
            return wgpu::TextureViewDimension::e2D;
        case tint::inspector::ResourceBinding::TextureDimension::k2dArray:
            return wgpu::TextureViewDimension::e2DArray;
        case tint::inspector::ResourceBinding::TextureDimension::k3d:
            return wgpu::TextureViewDimension::e3D;
        case tint::inspector::ResourceBinding::TextureDimension::kCube:
            return wgpu::TextureViewDimension::Cube;
        case tint::inspector::ResourceBinding::TextureDimension::kCubeArray:
            return wgpu::TextureViewDimension::CubeArray;
        case tint::inspector::ResourceBinding::TextureDimension::kNone:
            return wgpu::TextureViewDimension::Undefined;
    }
    DAWN_UNREACHABLE();
}

SampleTypeBit TintSampledKindToSampleTypeBit(tint::inspector::ResourceBinding::SampledKind s) {
    switch (s) {
        case tint::inspector::ResourceBinding::SampledKind::kSInt:
            return SampleTypeBit::Sint;
        case tint::inspector::ResourceBinding::SampledKind::kUInt:
            return SampleTypeBit::Uint;
        case tint::inspector::ResourceBinding::SampledKind::kFloat:
            return SampleTypeBit::Float | SampleTypeBit::UnfilterableFloat;
        case tint::inspector::ResourceBinding::SampledKind::kUnknown:
            return SampleTypeBit::None;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<TextureComponentType> TintComponentTypeToTextureComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
        case tint::inspector::ComponentType::kF16:
            return TextureComponentType::Float;
        case tint::inspector::ComponentType::kI32:
            return TextureComponentType::Sint;
        case tint::inspector::ComponentType::kU32:
            return TextureComponentType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<VertexFormatBaseType> TintComponentTypeToVertexFormatBaseType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
        case tint::inspector::ComponentType::kF16:
            return VertexFormatBaseType::Float;
        case tint::inspector::ComponentType::kI32:
            return VertexFormatBaseType::Sint;
        case tint::inspector::ComponentType::kU32:
            return VertexFormatBaseType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<wgpu::BufferBindingType> TintResourceTypeToBufferBindingType(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
            return wgpu::BufferBindingType::Uniform;
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
            return wgpu::BufferBindingType::Storage;
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return wgpu::BufferBindingType::ReadOnlyStorage;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-buffer resource type");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<wgpu::StorageTextureAccess> TintResourceTypeToStorageTextureAccess(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return wgpu::StorageTextureAccess::WriteOnly;
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
            return wgpu::StorageTextureAccess::ReadOnly;
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
            return wgpu::StorageTextureAccess::ReadWrite;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-storage texture resource type");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<InterStageComponentType> TintComponentTypeToInterStageComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
            return InterStageComponentType::F32;
        case tint::inspector::ComponentType::kI32:
            return InterStageComponentType::I32;
        case tint::inspector::ComponentType::kU32:
            return InterStageComponentType::U32;
        case tint::inspector::ComponentType::kF16:
            return InterStageComponentType::F16;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<uint32_t> TintCompositionTypeToInterStageComponentCount(
    tint::inspector::CompositionType type) {
    switch (type) {
        case tint::inspector::CompositionType::kScalar:
            return 1u;
        case tint::inspector::CompositionType::kVec2:
            return 2u;
        case tint::inspector::CompositionType::kVec3:
            return 3u;
        case tint::inspector::CompositionType::kVec4:
            return 4u;
        case tint::inspector::CompositionType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempt to convert 'Unknown' composition type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<InterpolationType> TintInterpolationTypeToInterpolationType(
    tint::inspector::InterpolationType type) {
    switch (type) {
        case tint::inspector::InterpolationType::kPerspective:
            return InterpolationType::Perspective;
        case tint::inspector::InterpolationType::kLinear:
            return InterpolationType::Linear;
        case tint::inspector::InterpolationType::kFlat:
            return InterpolationType::Flat;
        case tint::inspector::InterpolationType::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' interpolation type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<InterpolationSampling> TintInterpolationSamplingToInterpolationSamplingType(
    tint::inspector::InterpolationSampling type) {
    switch (type) {
        case tint::inspector::InterpolationSampling::kNone:
            return InterpolationSampling::None;
        case tint::inspector::InterpolationSampling::kCenter:
            return InterpolationSampling::Center;
        case tint::inspector::InterpolationSampling::kCentroid:
            return InterpolationSampling::Centroid;
        case tint::inspector::InterpolationSampling::kSample:
            return InterpolationSampling::Sample;
        case tint::inspector::InterpolationSampling::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' interpolation sampling type from Tint");
    }
    DAWN_UNREACHABLE();
}

EntryPointMetadata::Override::Type FromTintOverrideType(tint::inspector::Override::Type type) {
    switch (type) {
        case tint::inspector::Override::Type::kBool:
            return EntryPointMetadata::Override::Type::Boolean;
        case tint::inspector::Override::Type::kFloat32:
            return EntryPointMetadata::Override::Type::Float32;
        case tint::inspector::Override::Type::kFloat16:
            return EntryPointMetadata::Override::Type::Float16;
        case tint::inspector::Override::Type::kInt32:
            return EntryPointMetadata::Override::Type::Int32;
        case tint::inspector::Override::Type::kUint32:
            return EntryPointMetadata::Override::Type::Uint32;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<PixelLocalMemberType> FromTintPixelLocalMemberType(
    tint::inspector::PixelLocalMemberType type) {
    switch (type) {
        case tint::inspector::PixelLocalMemberType::kU32:
            return PixelLocalMemberType::U32;
        case tint::inspector::PixelLocalMemberType::kI32:
            return PixelLocalMemberType::I32;
        case tint::inspector::PixelLocalMemberType::kF32:
            return PixelLocalMemberType::F32;
        case tint::inspector::PixelLocalMemberType::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' pixel local member type from Tint");
    }
    DAWN_UNREACHABLE();
}

ResultOrError<tint::Program> ParseWGSL(const tint::Source::File* file,
                                       const tint::wgsl::AllowedFeatures& allowedFeatures,
                                       OwnedCompilationMessages* outMessages) {
    tint::wgsl::reader::Options options;
    options.allowed_features = allowedFeatures;
    tint::Program program = tint::wgsl::reader::Parse(file, options);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(program.Diagnostics()));
    }
    if (!program.IsValid()) {
        return DAWN_VALIDATION_ERROR("Error while parsing WGSL: %s\n", program.Diagnostics().str());
    }

    return std::move(program);
}

#if TINT_BUILD_SPV_READER
ResultOrError<tint::Program> ParseSPIRV(const std::vector<uint32_t>& spirv,
                                        const tint::wgsl::AllowedFeatures& allowedFeatures,
                                        OwnedCompilationMessages* outMessages,
                                        const DawnShaderModuleSPIRVOptionsDescriptor* optionsDesc) {
    tint::spirv::reader::Options options;
    if (optionsDesc) {
        options.allow_non_uniform_derivatives = optionsDesc->allowNonUniformDerivatives;
    }
    options.allowed_features = allowedFeatures;
    tint::Program program = tint::spirv::reader::Read(spirv, options);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(program.Diagnostics()));
    }
    if (!program.IsValid()) {
        return DAWN_VALIDATION_ERROR("Error while parsing SPIR-V: %s\n",
                                     program.Diagnostics().str());
    }

    return std::move(program);
}
#endif  // TINT_BUILD_SPV_READER

std::vector<uint64_t> GetBindGroupMinBufferSizes(const BindingGroupInfoMap& shaderBindings,
                                                 const BindGroupLayoutInternalBase* layout) {
    std::vector<uint64_t> requiredBufferSizes(layout->GetUnverifiedBufferCount());
    uint32_t packedIdx = 0;

    for (BindingIndex bindingIndex{0}; bindingIndex < layout->GetBufferCount(); ++bindingIndex) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        if (bindingInfo.buffer.minBindingSize != 0) {
            // Skip bindings that have minimum buffer size set in the layout
            continue;
        }

        DAWN_ASSERT(packedIdx < requiredBufferSizes.size());
        const auto& shaderInfo = shaderBindings.find(bindingInfo.binding);
        if (shaderInfo != shaderBindings.end()) {
            auto* shaderBufferInfo =
                std::get_if<BufferBindingInfo>(&shaderInfo->second.bindingInfo);
            if (shaderBufferInfo != nullptr) {
                requiredBufferSizes[packedIdx] = shaderBufferInfo->minBindingSize;
            } else {
                requiredBufferSizes[packedIdx] = 0;
            }
        } else {
            // We have to include buffers if they are included in the bind group's
            // packed vector. We don't actually need to check these at draw time, so
            // if this is a problem in the future we can optimize it further.
            requiredBufferSizes[packedIdx] = 0;
        }
        ++packedIdx;
    }

    return requiredBufferSizes;
}

bool IsShaderCompatibleWithPipelineLayoutOnStorageTextureAccess(
    const BindingInfo& bindingInfo,
    const StorageTextureBindingInfo& bindingLayout) {
    return bindingInfo.storageTexture.access == bindingLayout.access ||
           (bindingInfo.storageTexture.access == wgpu::StorageTextureAccess::ReadWrite &&
            bindingLayout.access == wgpu::StorageTextureAccess::WriteOnly);
}

BindingInfoType GetShaderBindingType(const ShaderBindingInfo& shaderInfo) {
    return MatchVariant(
        shaderInfo.bindingInfo, [&](const BufferBindingInfo&) { return BindingInfoType::Buffer; },
        [&](const StorageTextureBindingInfo&) { return BindingInfoType::StorageTexture; },
        [&](const SampledTextureBindingInfo&) { return BindingInfoType::Texture; },
        [&](const SamplerBindingInfo&) { return BindingInfoType::Sampler; },
        [&](const ExternalTextureBindingInfo&) { return BindingInfoType::ExternalTexture; });
}

MaybeError ValidateCompatibilityOfSingleBindingWithLayout(const DeviceBase* device,
                                                          const BindGroupLayoutInternalBase* layout,
                                                          SingleShaderStage entryPointStage,
                                                          BindingNumber bindingNumber,
                                                          const ShaderBindingInfo& shaderInfo) {
    const BindGroupLayoutInternalBase::BindingMap& layoutBindings = layout->GetBindingMap();

    // An external texture binding found in the shader will later be expanded into multiple
    // bindings at compile time. This expansion will have already happened in the bgl - so
    // the shader and bgl will always mismatch at this point. Expansion info is contained in
    // the bgl object, so we can still verify the bgl used to have an external texture in
    // the slot corresponding to the shader reflection.
    if (std::holds_alternative<ExternalTextureBindingInfo>(shaderInfo.bindingInfo)) {
        // If an external texture binding used to exist in the bgl, it will be found as a
        // key in the ExternalTextureBindingExpansions map.
        ExternalTextureBindingExpansionMap expansions =
            layout->GetExternalTextureBindingExpansionMap();
        std::map<BindingNumber, dawn::native::ExternalTextureBindingExpansion>::iterator it =
            expansions.find(bindingNumber);
        // TODO(dawn:563): Provide info about the binding types.
        DAWN_INVALID_IF(it == expansions.end(),
                        "Binding type in the shader (texture_external) doesn't match the "
                        "type in the layout.");

        return {};
    }

    const auto& bindingIt = layoutBindings.find(bindingNumber);
    DAWN_INVALID_IF(bindingIt == layoutBindings.end(), "Binding doesn't exist in %s.", layout);

    BindingIndex bindingIndex(bindingIt->second);
    const BindingInfo& layoutInfo = layout->GetBindingInfo(bindingIndex);

    BindingInfoType shaderBindingType = GetShaderBindingType(shaderInfo);
    DAWN_INVALID_IF(layoutInfo.bindingType != shaderBindingType,
                    "Binding type in the shader (%s) doesn't match the type in the layout (%s).",
                    shaderBindingType, layoutInfo.bindingType);

    ExternalTextureBindingExpansionMap expansions = layout->GetExternalTextureBindingExpansionMap();
    DAWN_INVALID_IF(expansions.find(bindingNumber) != expansions.end(),
                    "Binding type (buffer vs. texture vs. sampler vs. external) doesn't "
                    "match the type in the layout.");

    DAWN_INVALID_IF((layoutInfo.visibility & StageBit(entryPointStage)) == 0,
                    "Entry point's stage (%s) is not in the binding visibility in the layout (%s).",
                    StageBit(entryPointStage), layoutInfo.visibility);

    return MatchVariant(
        shaderInfo.bindingInfo,
        [&](const SampledTextureBindingInfo& bindingInfo) -> MaybeError {
            DAWN_INVALID_IF(
                layoutInfo.texture.multisampled != bindingInfo.multisampled,
                "Binding multisampled flag (%u) doesn't match the layout's multisampled "
                "flag (%u)",
                layoutInfo.texture.multisampled, bindingInfo.multisampled);

            // TODO(dawn:563): Provide info about the sample types.
            SampleTypeBit requiredType;
            if (layoutInfo.texture.sampleType == kInternalResolveAttachmentSampleType) {
                // If the layout's texture's sample type is
                // kInternalResolveAttachmentSampleType, then the shader's compatible sample
                // types must contain float.
                requiredType = SampleTypeBit::UnfilterableFloat;
            } else {
                requiredType = SampleTypeToSampleTypeBit(layoutInfo.texture.sampleType);
            }

            DAWN_INVALID_IF(!(bindingInfo.compatibleSampleTypes & requiredType),
                            "The sample type in the shader is not compatible with the "
                            "sample type of the layout.");

            DAWN_INVALID_IF(
                layoutInfo.texture.viewDimension != bindingInfo.viewDimension,
                "The shader's binding dimension (%s) doesn't match the shader's binding "
                "dimension (%s).",
                layoutInfo.texture.viewDimension, bindingInfo.viewDimension);
            return {};
        },
        [&](const StorageTextureBindingInfo& bindingInfo) -> MaybeError {
            DAWN_ASSERT(layoutInfo.storageTexture.format != wgpu::TextureFormat::Undefined);
            DAWN_ASSERT(bindingInfo.format != wgpu::TextureFormat::Undefined);

            DAWN_INVALID_IF(!IsShaderCompatibleWithPipelineLayoutOnStorageTextureAccess(
                                layoutInfo, bindingInfo),
                            "The layout's binding access (%s) isn't compatible with the shader's "
                            "binding access (%s).",
                            layoutInfo.storageTexture.access, bindingInfo.access);

            DAWN_INVALID_IF(layoutInfo.storageTexture.format != bindingInfo.format,
                            "The layout's binding format (%s) doesn't match the shader's binding "
                            "format (%s).",
                            layoutInfo.storageTexture.format, bindingInfo.format);

            DAWN_INVALID_IF(layoutInfo.storageTexture.viewDimension != bindingInfo.viewDimension,
                            "The layout's binding dimension (%s) doesn't match the "
                            "shader's binding dimension (%s).",
                            layoutInfo.storageTexture.viewDimension, bindingInfo.viewDimension);
            return {};
        },
        [&](const BufferBindingInfo& bindingInfo) -> MaybeError {
            // Binding mismatch between shader and bind group is invalid. For example, a
            // writable binding in the shader with a readonly storage buffer in the bind
            // group layout is invalid. For internal usage with internal shaders, a storage
            // binding in the shader with an internal storage buffer in the bind group
            // layout is also valid.
            bool validBindingConversion =
                (layoutInfo.buffer.type == kInternalStorageBufferBinding &&
                 bindingInfo.type == wgpu::BufferBindingType::Storage);

            DAWN_INVALID_IF(
                layoutInfo.buffer.type != bindingInfo.type && !validBindingConversion,
                "The buffer type in the shader (%s) is not compatible with the type in the "
                "layout (%s).",
                bindingInfo.type, layoutInfo.buffer.type);

            DAWN_INVALID_IF(layoutInfo.buffer.minBindingSize != 0 &&
                                bindingInfo.minBindingSize > layoutInfo.buffer.minBindingSize,
                            "The shader uses more bytes of the buffer (%u) than the layout's "
                            "minBindingSize (%u).",
                            bindingInfo.minBindingSize, layoutInfo.buffer.minBindingSize);
            return {};
        },
        [&](const SamplerBindingInfo& bindingInfo) -> MaybeError {
            DAWN_INVALID_IF(
                (layoutInfo.sampler.type == wgpu::SamplerBindingType::Comparison) !=
                    bindingInfo.isComparison,
                "The sampler type in the shader (comparison: %u) doesn't match the type in "
                "the layout (comparison: %u).",
                bindingInfo.isComparison,
                layoutInfo.sampler.type == wgpu::SamplerBindingType::Comparison);
            return {};
        },

        [&](const ExternalTextureBindingInfo&) -> MaybeError {
            DAWN_UNREACHABLE();
            return {};
        });
}

MaybeError ValidateCompatibilityWithBindGroupLayout(DeviceBase* device,
                                                    BindGroupIndex group,
                                                    const EntryPointMetadata& entryPoint,
                                                    const BindGroupLayoutInternalBase* layout) {
    // Iterate over all bindings used by this group in the shader, and find the
    // corresponding binding in the BindGroupLayout, if it exists.
    for (const auto& [bindingId, bindingInfo] : entryPoint.bindings[group]) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityOfSingleBindingWithLayout(
                             device, layout, entryPoint.stage, bindingId, bindingInfo),
                         "validating that the entry-point's declaration for @group(%u) "
                         "@binding(%u) matches %s",
                         group, bindingId, layout);
    }

    return {};
}

ResultOrError<std::unique_ptr<EntryPointMetadata>> ReflectEntryPointUsingTint(
    const DeviceBase* device,
    tint::inspector::Inspector* inspector,
    const tint::inspector::EntryPoint& entryPoint) {
    std::unique_ptr<EntryPointMetadata> metadata = std::make_unique<EntryPointMetadata>();

    // Returns the invalid argument, and if it is true additionally store the formatted
    // error in metadata.infringedLimits. This is to delay the emission of these validation
    // errors until the entry point is used.
#define DelayedInvalidIf(invalid, ...)                                              \
    ([&] {                                                                          \
        if (invalid) {                                                              \
            metadata->infringedLimitErrors.push_back(absl::StrFormat(__VA_ARGS__)); \
        }                                                                           \
        return invalid;                                                             \
    })()

    if (!entryPoint.overrides.empty()) {
        const auto& name2Id = inspector->GetNamedOverrideIds();

        for (auto& c : entryPoint.overrides) {
            auto id = name2Id.at(c.name);
            EntryPointMetadata::Override override = {id, FromTintOverrideType(c.type),
                                                     c.is_initialized};

            std::string identifier = c.is_id_specified ? std::to_string(override.id.value) : c.name;
            metadata->overrides[identifier] = override;

            if (!c.is_initialized) {
                auto [_, inserted] =
                    metadata->uninitializedOverrides.emplace(std::move(identifier));
                // The insertion should have taken place
                DAWN_ASSERT(inserted);
            } else {
                auto [_, inserted] = metadata->initializedOverrides.emplace(std::move(identifier));
                // The insertion should have taken place
                DAWN_ASSERT(inserted);
            }
        }
    }

    DAWN_TRY_ASSIGN(metadata->stage, TintPipelineStageToShaderStage(entryPoint.stage));

    if (metadata->stage == SingleShaderStage::Compute) {
        metadata->usesNumWorkgroups = entryPoint.num_workgroups_used;
    }

    const CombinedLimits& limits = device->GetLimits();
    const uint32_t maxVertexAttributes = limits.v1.maxVertexAttributes;
    const uint32_t maxInterStageShaderVariables = limits.v1.maxInterStageShaderVariables;
    const uint32_t maxInterStageShaderComponents = limits.v1.maxInterStageShaderComponents;

    metadata->usedInterStageVariables.resize(maxInterStageShaderVariables);
    metadata->interStageVariables.resize(maxInterStageShaderVariables);

    // Vertex shader specific reflection.
    if (metadata->stage == SingleShaderStage::Vertex) {
        // Vertex input reflection.
        for (const auto& inputVar : entryPoint.input_variables) {
            uint32_t unsanitizedLocation = inputVar.attributes.location.value();
            if (DelayedInvalidIf(unsanitizedLocation >= maxVertexAttributes,
                                 "Vertex input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u)",
                                 inputVar.name, unsanitizedLocation, maxVertexAttributes)) {
                continue;
            }

            VertexAttributeLocation location(static_cast<uint8_t>(unsanitizedLocation));
            DAWN_TRY_ASSIGN(metadata->vertexInputBaseTypes[location],
                            TintComponentTypeToVertexFormatBaseType(inputVar.component_type));
            metadata->usedVertexInputs.set(location);
        }

        // Vertex ouput (inter-stage variables) reflection.
        uint32_t totalInterStageShaderComponents = 0;
        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::InterStageVariableInfo variable;
            variable.name = outputVar.variable_name;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToInterStageComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            DAWN_TRY_ASSIGN(variable.interpolationType,
                            TintInterpolationTypeToInterpolationType(outputVar.interpolation_type));
            DAWN_TRY_ASSIGN(variable.interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                outputVar.interpolation_sampling));
            totalInterStageShaderComponents += variable.componentCount;

            uint32_t location = outputVar.attributes.location.value();
            if (DelayedInvalidIf(location >= maxInterStageShaderVariables,
                                 "Vertex output variable \"%s\" has a location (%u) that "
                                 "is greater than or equal to (%u).",
                                 outputVar.name, location, maxInterStageShaderVariables)) {
                continue;
            }

            metadata->usedInterStageVariables[location] = true;
            metadata->interStageVariables[location] = variable;
        }

        // Other vertex metadata.
        metadata->totalInterStageShaderComponents = totalInterStageShaderComponents;
        DelayedInvalidIf(totalInterStageShaderComponents > maxInterStageShaderComponents,
                         "Total vertex output components count (%u) exceeds the maximum (%u).",
                         totalInterStageShaderComponents, maxInterStageShaderComponents);

        metadata->usesVertexIndex = entryPoint.vertex_index_used;
        metadata->usesInstanceIndex = entryPoint.instance_index_used;
    }

    // Fragment shader specific reflection.
    if (metadata->stage == SingleShaderStage::Fragment) {
        uint32_t totalInterStageShaderComponents = 0;

        // Fragment input (inter-stage variables) reflection.
        for (const auto& inputVar : entryPoint.input_variables) {
            // Skip over @color framebuffer fetch, it is handled below.
            if (!inputVar.attributes.location.has_value()) {
                DAWN_ASSERT(inputVar.attributes.color.has_value());
                continue;
            }

            uint32_t location = inputVar.attributes.location.value();
            EntryPointMetadata::InterStageVariableInfo variable;
            variable.name = inputVar.variable_name;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToInterStageComponentType(inputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         inputVar.composition_type));
            DAWN_TRY_ASSIGN(variable.interpolationType,
                            TintInterpolationTypeToInterpolationType(inputVar.interpolation_type));
            DAWN_TRY_ASSIGN(variable.interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                inputVar.interpolation_sampling));
            totalInterStageShaderComponents += variable.componentCount;

            if (DelayedInvalidIf(location >= maxInterStageShaderVariables,
                                 "Fragment input variable \"%s\" has a location (%u) that "
                                 "is greater than or equal to (%u).",
                                 inputVar.name, location, maxInterStageShaderVariables)) {
                continue;
            }

            metadata->usedInterStageVariables[location] = true;
            metadata->interStageVariables[location] = variable;
        }

        // Other fragment metadata
        if (entryPoint.front_facing_used) {
            totalInterStageShaderComponents += 1;
        }
        if (entryPoint.input_sample_mask_used) {
            totalInterStageShaderComponents += 1;
        }
        metadata->usesSampleMaskOutput = entryPoint.output_sample_mask_used;
        metadata->usesSampleIndex = entryPoint.sample_index_used;
        if (entryPoint.sample_index_used) {
            totalInterStageShaderComponents += 1;
        }
        metadata->usesFragDepth = entryPoint.frag_depth_used;

        metadata->totalInterStageShaderComponents = totalInterStageShaderComponents;
        DelayedInvalidIf(totalInterStageShaderComponents > maxInterStageShaderComponents,
                         "Total fragment input components count (%u) exceeds the maximum (%u).",
                         totalInterStageShaderComponents, maxInterStageShaderComponents);

        // Fragment output reflection.
        uint32_t maxColorAttachments = limits.v1.maxColorAttachments;
        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::FragmentRenderAttachmentInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToTextureComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            DAWN_ASSERT(variable.componentCount <= 4);

            uint32_t unsanitizedAttachment = outputVar.attributes.location.value();
            if (DelayedInvalidIf(unsanitizedAttachment >= maxColorAttachments,
                                 "Fragment output variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 outputVar.name, unsanitizedAttachment, maxColorAttachments)) {
                continue;
            }

            ColorAttachmentIndex attachment(static_cast<uint8_t>(unsanitizedAttachment));
            metadata->fragmentOutputVariables[attachment] = variable;
            metadata->fragmentOutputMask.set(attachment);
        }

        // Fragment input reflection.
        for (const auto& inputVar : entryPoint.input_variables) {
            if (!inputVar.attributes.color.has_value()) {
                continue;
            }

            // Tint should disallow using @color(N) without the respective enable, which is gated
            // on the extension.
            DAWN_ASSERT(device->HasFeature(Feature::FramebufferFetch));

            EntryPointMetadata::FragmentRenderAttachmentInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToTextureComponentType(inputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         inputVar.composition_type));
            DAWN_ASSERT(variable.componentCount <= 4);

            uint32_t unsanitizedAttachment = inputVar.attributes.color.value();
            if (DelayedInvalidIf(unsanitizedAttachment >= maxColorAttachments,
                                 "Fragment input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 inputVar.name, unsanitizedAttachment, maxColorAttachments)) {
                continue;
            }

            ColorAttachmentIndex attachment(static_cast<uint8_t>(unsanitizedAttachment));
            metadata->fragmentInputVariables[attachment] = variable;
            metadata->fragmentInputMask.set(attachment);
        }

        // Fragment PLS reflection.
        if (!entryPoint.pixel_local_members.empty()) {
            metadata->usesPixelLocal = true;
            metadata->pixelLocalBlockSize =
                kPLSSlotByteSize * entryPoint.pixel_local_members.size();
            metadata->pixelLocalMembers.reserve(entryPoint.pixel_local_members.size());

            for (auto type : entryPoint.pixel_local_members) {
                PixelLocalMemberType metadataType;
                DAWN_TRY_ASSIGN(metadataType, FromTintPixelLocalMemberType(type));
                metadata->pixelLocalMembers.push_back(metadataType);
            }
        }
    }

    // Generic resource binding reflection.
    for (const tint::inspector::ResourceBinding& resource :
         inspector->GetResourceBindings(entryPoint.name)) {
        ShaderBindingInfo info;

        info.name = resource.variable_name;

        switch (TintResourceTypeToBindingInfoType(resource.resource_type)) {
            case BindingInfoType::Buffer: {
                BufferBindingInfo bindingInfo = {};
                bindingInfo.minBindingSize = resource.size;
                DAWN_TRY_ASSIGN(bindingInfo.type,
                                TintResourceTypeToBufferBindingType(resource.resource_type));
                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::Sampler: {
                SamplerBindingInfo bindingInfo = {};
                switch (resource.resource_type) {
                    case tint::inspector::ResourceBinding::ResourceType::kSampler:
                        bindingInfo.isComparison = false;
                        break;
                    case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
                        bindingInfo.isComparison = true;
                        break;
                    default:
                        DAWN_UNREACHABLE();
                }
                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::Texture: {
                SampledTextureBindingInfo bindingInfo = {};
                bindingInfo.viewDimension =
                    TintTextureDimensionToTextureViewDimension(resource.dim);
                if (resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthTexture ||
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture) {
                    bindingInfo.compatibleSampleTypes = SampleTypeBit::Depth;
                } else {
                    bindingInfo.compatibleSampleTypes =
                        TintSampledKindToSampleTypeBit(resource.sampled_kind);
                }
                bindingInfo.multisampled =
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture ||
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture;
                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::StorageTexture: {
                StorageTextureBindingInfo bindingInfo = {};
                DAWN_TRY_ASSIGN(bindingInfo.access,
                                TintResourceTypeToStorageTextureAccess(resource.resource_type));
                bindingInfo.format = TintImageFormatToTextureFormat(resource.image_format);
                bindingInfo.viewDimension =
                    TintTextureDimensionToTextureViewDimension(resource.dim);

                DAWN_INVALID_IF(bindingInfo.format == wgpu::TextureFormat::BGRA8Unorm &&
                                    !device->HasFeature(Feature::BGRA8UnormStorage),
                                "BGRA8Unorm storage textures are not supported if optional feature "
                                "bgra8unorm-storage is not supported.");

                info.bindingInfo = bindingInfo;
                break;
            }

            case BindingInfoType::ExternalTexture: {
                info.bindingInfo.emplace<ExternalTextureBindingInfo>();
                break;
            }
            default:
                return DAWN_VALIDATION_ERROR("Unknown binding type in Shader");
        }

        BindingNumber bindingNumber(resource.binding);
        BindGroupIndex bindGroupIndex(resource.bind_group);

        if (DelayedInvalidIf(bindGroupIndex >= kMaxBindGroupsTyped,
                             "The entry-point uses a binding with a group decoration (%u) "
                             "that exceeds the maximum (%u).",
                             resource.bind_group, kMaxBindGroups) ||
            DelayedInvalidIf(bindingNumber >= kMaxBindingsPerBindGroupTyped,
                             "Binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
                             uint32_t(bindingNumber), kMaxBindingsPerBindGroup)) {
            continue;
        }

        const auto& [binding, inserted] =
            metadata->bindings[bindGroupIndex].emplace(bindingNumber, info);
        DAWN_INVALID_IF(!inserted,
                        "Entry-point has a duplicate binding for (group:%u, binding:%u).",
                        resource.binding, resource.bind_group);
    }

    // Reflection of combined sampler and texture uses.
    auto samplerTextureUses = inspector->GetSamplerTextureUses(entryPoint.name);
    metadata->samplerTexturePairs.reserve(samplerTextureUses.Length());
    std::transform(samplerTextureUses.begin(), samplerTextureUses.end(),
                   std::back_inserter(metadata->samplerTexturePairs),
                   [](const tint::inspector::SamplerTexturePair& pair) {
                       EntryPointMetadata::SamplerTexturePair result;
                       result.sampler = {BindGroupIndex(pair.sampler_binding_point.group),
                                         BindingNumber(pair.sampler_binding_point.binding)};
                       result.texture = {BindGroupIndex(pair.texture_binding_point.group),
                                         BindingNumber(pair.texture_binding_point.binding)};
                       return result;
                   });

#undef DelayedInvalidIf
    return std::move(metadata);
}

MaybeError ReflectShaderUsingTint(const DeviceBase* device,
                                  const tint::Program* program,
                                  OwnedCompilationMessages* compilationMessages,
                                  EntryPointMetadataTable* entryPointMetadataTable) {
    DAWN_ASSERT(program->IsValid());

    tint::inspector::Inspector inspector(*program);

    std::vector<tint::inspector::EntryPoint> entryPoints = inspector.GetEntryPoints();
    DAWN_INVALID_IF(inspector.has_error(), "Tint Reflection failure: Inspector: %s\n",
                    inspector.error());

    for (const tint::inspector::EntryPoint& entryPoint : entryPoints) {
        std::unique_ptr<EntryPointMetadata> metadata;
        DAWN_TRY_ASSIGN_CONTEXT(metadata,
                                ReflectEntryPointUsingTint(device, &inspector, entryPoint),
                                "processing entry point \"%s\".", entryPoint.name);

        DAWN_ASSERT(!entryPointMetadataTable->contains(entryPoint.name));
        entryPointMetadataTable->emplace(entryPoint.name, std::move(metadata));
    }
    return {};
}
}  // anonymous namespace

ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    const tint::Program& program,
    const char* entryPointName,
    const LimitsForCompilationRequest& limits,
    std::optional<uint32_t> maxSubgroupSizeForFullSubgroups) {
    tint::inspector::Inspector inspector(program);
    // At this point the entry point must exist and must have workgroup size values.
    tint::inspector::EntryPoint entryPoint = inspector.GetEntryPoint(entryPointName);
    DAWN_ASSERT(entryPoint.workgroup_size.has_value());
    const tint::inspector::WorkgroupSize& workgroup_size = entryPoint.workgroup_size.value();

    DAWN_INVALID_IF(workgroup_size.x < 1 || workgroup_size.y < 1 || workgroup_size.z < 1,
                    "Entry-point uses workgroup_size(%u, %u, %u) that are below the "
                    "minimum allowed (1, 1, 1).",
                    workgroup_size.x, workgroup_size.y, workgroup_size.z);

    DAWN_INVALID_IF(workgroup_size.x > limits.maxComputeWorkgroupSizeX ||
                        workgroup_size.y > limits.maxComputeWorkgroupSizeY ||
                        workgroup_size.z > limits.maxComputeWorkgroupSizeZ,
                    "Entry-point uses workgroup_size(%u, %u, %u) that exceeds the "
                    "maximum allowed (%u, %u, %u).",
                    workgroup_size.x, workgroup_size.y, workgroup_size.z,
                    limits.maxComputeWorkgroupSizeX, limits.maxComputeWorkgroupSizeY,
                    limits.maxComputeWorkgroupSizeZ);

    uint64_t numInvocations =
        static_cast<uint64_t>(workgroup_size.x) * workgroup_size.y * workgroup_size.z;
    DAWN_INVALID_IF(numInvocations > limits.maxComputeInvocationsPerWorkgroup,
                    "The total number of workgroup invocations (%u) exceeds the "
                    "maximum allowed (%u).",
                    numInvocations, limits.maxComputeInvocationsPerWorkgroup);

    const size_t workgroupStorageSize = entryPoint.workgroup_storage_size;
    DAWN_INVALID_IF(workgroupStorageSize > limits.maxComputeWorkgroupStorageSize,
                    "The total use of workgroup storage (%u bytes) is larger than "
                    "the maximum allowed (%u bytes).",
                    workgroupStorageSize, limits.maxComputeWorkgroupStorageSize);

    // Validate workgroup_size.x is a multiple of maxSubgroupSizeForFullSubgroups if
    // it holds a value.
    DAWN_INVALID_IF(maxSubgroupSizeForFullSubgroups &&
                        (workgroup_size.x % *maxSubgroupSizeForFullSubgroups != 0),
                    "the X dimension of the workgroup size (%d) must be a multiple of "
                    "maxSubgroupSize (%d) if full subgroups required in compute pipeline",
                    workgroup_size.x, *maxSubgroupSizeForFullSubgroups);

    return Extent3D{workgroup_size.x, workgroup_size.y, workgroup_size.z};
}

ShaderModuleParseResult::ShaderModuleParseResult() = default;
ShaderModuleParseResult::~ShaderModuleParseResult() = default;

ShaderModuleParseResult::ShaderModuleParseResult(ShaderModuleParseResult&& rhs) = default;

ShaderModuleParseResult& ShaderModuleParseResult::operator=(ShaderModuleParseResult&& rhs) =
    default;

bool ShaderModuleParseResult::HasParsedShader() const {
    return tintProgram != nullptr;
}

MaybeError ValidateAndParseShaderModule(DeviceBase* device,
                                        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                        ShaderModuleParseResult* parseResult,
                                        OwnedCompilationMessages* outMessages) {
    DAWN_ASSERT(parseResult != nullptr);

    wgpu::SType moduleType;
    // A WGSL (or SPIR-V, if enabled) subdescriptor is required, and a Dawn-specific SPIR-V options
// descriptor is allowed when using SPIR-V.
#if TINT_BUILD_SPV_READER
    DAWN_TRY_ASSIGN(
        moduleType,
        (descriptor.ValidateBranches<
            Branch<ShaderModuleWGSLDescriptor>,
            Branch<ShaderModuleSPIRVDescriptor, DawnShaderModuleSPIRVOptionsDescriptor>>()));
#else
    DAWN_TRY_ASSIGN(moduleType,
                    (descriptor.ValidateBranches<Branch<ShaderModuleWGSLDescriptor>>()));
#endif
    DAWN_ASSERT(moduleType != wgpu::SType::Invalid);

    ScopedTintICEHandler scopedICEHandler(device);

    // Multiple paths may use a WGSL descriptor so declare it here now.
    const ShaderModuleWGSLDescriptor* wgslDesc = nullptr;
#if TINT_BUILD_WGSL_WRITER
    ShaderModuleWGSLDescriptor newWgslDesc = {};
    std::string newWgslCode;
#endif  // TINT_BUILD_WGSL_WRITER

    switch (moduleType) {
#if TINT_BUILD_SPV_READER
        case wgpu::SType::ShaderModuleSPIRVDescriptor: {
            DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisallowSpirv),
                            "SPIR-V is disallowed.");
            const auto* spirvDesc = descriptor.Get<ShaderModuleSPIRVDescriptor>();
            const auto* spirvOptions = descriptor.Get<DawnShaderModuleSPIRVOptionsDescriptor>();

            // TODO(dawn:2033): Avoid unnecessary copies of the SPIR-V code.
            std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
            const bool dumpSpirv = device->IsToggleEnabled(Toggle::DumpShaders);
            DAWN_TRY(ValidateSpirv(device, spirv.data(), spirv.size(), dumpSpirv));
#endif  // DAWN_ENABLE_SPIRV_VALIDATION
            tint::Program program;
            DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, device->GetWGSLAllowedFeatures(),
                                                outMessages, spirvOptions));
            parseResult->tintProgram = AcquireRef(new TintProgram(std::move(program), nullptr));

            return {};
        }
#endif  // TINT_BUILD_SPV_READER
        case wgpu::SType::ShaderModuleWGSLDescriptor: {
            wgslDesc = descriptor.Get<ShaderModuleWGSLDescriptor>();
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }
    DAWN_ASSERT(wgslDesc != nullptr);

    auto tintFile = std::make_unique<tint::Source::File>("", wgslDesc->code);

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "// Dumped WGSL:" << std::endl << wgslDesc->code << std::endl;
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    tint::Program program;
    DAWN_TRY_ASSIGN(program,
                    ParseWGSL(tintFile.get(), device->GetWGSLAllowedFeatures(), outMessages));

    parseResult->tintProgram = AcquireRef(new TintProgram(std::move(program), std::move(tintFile)));

    return {};
}

RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                        const PipelineLayoutBase* layout) {
    RequiredBufferSizes bufferSizes;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        bufferSizes[group] = GetBindGroupMinBufferSizes(entryPoint.bindings[group],
                                                        layout->GetBindGroupLayout(group));
    }

    return bufferSizes;
}

ResultOrError<tint::Program> RunTransforms(tint::ast::transform::Manager* transformManager,
                                           const tint::Program* program,
                                           const tint::ast::transform::DataMap& inputs,
                                           tint::ast::transform::DataMap* outputs,
                                           OwnedCompilationMessages* outMessages) {
    DAWN_ASSERT(program != nullptr);
    tint::ast::transform::DataMap transform_outputs;
    tint::Program result = transformManager->Run(*program, inputs, transform_outputs);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(result.Diagnostics()));
    }
    DAWN_INVALID_IF(!result.IsValid(), "Tint program failure: %s\n", result.Diagnostics().str());
    if (outputs != nullptr) {
        *outputs = std::move(transform_outputs);
    }
    return std::move(result);
}

MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                   const EntryPointMetadata& entryPoint,
                                                   const PipelineLayoutBase* layout) {
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityWithBindGroupLayout(
                             device, group, entryPoint, layout->GetBindGroupLayout(group)),
                         "validating the entry-point's compatibility for group %u with %s", group,
                         layout->GetBindGroupLayout(group));
    }

    for (BindGroupIndex group : IterateBitSet(~layout->GetBindGroupLayoutsMask())) {
        DAWN_INVALID_IF(entryPoint.bindings[group].size() > 0,
                        "The entry-point uses bindings in group %u but %s doesn't have a "
                        "BindGroupLayout for this index",
                        group, layout);
    }

    // Validate that filtering samplers are not used with unfilterable textures.
    for (const auto& pair : entryPoint.samplerTexturePairs) {
        const BindGroupLayoutInternalBase* samplerBGL =
            layout->GetBindGroupLayout(pair.sampler.group);
        const BindingInfo& samplerInfo =
            samplerBGL->GetBindingInfo(samplerBGL->GetBindingIndex(pair.sampler.binding));
        if (samplerInfo.sampler.type != wgpu::SamplerBindingType::Filtering) {
            continue;
        }
        const BindGroupLayoutInternalBase* textureBGL =
            layout->GetBindGroupLayout(pair.texture.group);
        const BindingInfo& textureInfo =
            textureBGL->GetBindingInfo(textureBGL->GetBindingIndex(pair.texture.binding));

        DAWN_ASSERT(textureInfo.bindingType != BindingInfoType::Buffer &&
                    textureInfo.bindingType != BindingInfoType::Sampler &&
                    textureInfo.bindingType != BindingInfoType::StorageTexture);

        if (textureInfo.bindingType != BindingInfoType::Texture) {
            continue;
        }

        // Uint/Sint can't be statically used with a sampler, so they any
        // texture bindings reflected must be float or depth textures. If
        // the shader uses a float/depth texture but the bind group layout
        // specifies a uint/sint texture binding,
        // |ValidateCompatibilityWithBindGroupLayout| will fail since the
        // sampleType does not match.
        DAWN_ASSERT(textureInfo.texture.sampleType != wgpu::TextureSampleType::Undefined &&
                    textureInfo.texture.sampleType != wgpu::TextureSampleType::Uint &&
                    textureInfo.texture.sampleType != wgpu::TextureSampleType::Sint);

        DAWN_INVALID_IF(
            textureInfo.texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat,
            "Texture binding (group:%u, binding:%u) is %s but used statically with a sampler "
            "(group:%u, binding:%u) that's %s",
            pair.texture.group, pair.texture.binding, wgpu::TextureSampleType::UnfilterableFloat,
            pair.sampler.group, pair.sampler.binding, wgpu::SamplerBindingType::Filtering);
    }

    // Validate compatibility of the pixel local storage.
    if (entryPoint.usesPixelLocal) {
        DAWN_INVALID_IF(!layout->HasPixelLocalStorage(),
                        "The entry-point uses `pixel_local` block but the pipeline layout doesn't "
                        "contain a pixel local storage.");

        // TODO(dawn:1704): Allow entryPoint.pixelLocalBlockSize < layoutPixelLocalSize.
        auto layoutStorageAttachments = layout->GetStorageAttachmentSlots();
        size_t layoutPixelLocalSize = layoutStorageAttachments.size() * kPLSSlotByteSize;
        DAWN_INVALID_IF(entryPoint.pixelLocalBlockSize != layoutPixelLocalSize,
                        "The entry-point's pixel local block size (%u) is different from the "
                        "layout's total pixel local size (%u).",
                        entryPoint.pixelLocalBlockSize, layoutPixelLocalSize);

        for (size_t i = 0; i < entryPoint.pixelLocalMembers.size(); i++) {
            wgpu::TextureFormat layoutFormat = layoutStorageAttachments[i];

            // TODO(dawn:1704): Allow format conversions by injecting them in the shader
            // automatically.
            PixelLocalMemberType expectedType;
            switch (layoutFormat) {
                case wgpu::TextureFormat::R32Sint:
                    expectedType = PixelLocalMemberType::I32;
                    break;
                case wgpu::TextureFormat::R32Float:
                    expectedType = PixelLocalMemberType::F32;
                    break;
                case wgpu::TextureFormat::R32Uint:
                case wgpu::TextureFormat::Undefined:
                    expectedType = PixelLocalMemberType::U32;
                    break;

                default:
                    DAWN_UNREACHABLE();
            }

            PixelLocalMemberType entryPointType = entryPoint.pixelLocalMembers[i];
            DAWN_INVALID_IF(
                expectedType != entryPointType,
                "The `pixel_local` block's member at index %u has a type (%s) that's not "
                "compatible with the layout's storage format (%s), the expected type is %s.",
                i, entryPointType, layoutFormat, expectedType);
        }
    } else {
        // TODO(dawn:1704): Allow a fragment entry-point without PLS to be used with a layout that
        // has PLS.
        DAWN_INVALID_IF(entryPoint.stage == SingleShaderStage::Fragment &&
                            !layout->GetStorageAttachmentSlots().empty(),
                        "The layout contains a (non-empty) pixel local storage but the entry-point "
                        "doesn't use a `pixel local` block.");
    }

    return {};
}

// ShaderModuleBase

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                                   ApiObjectBase::UntrackedByDeviceTag tag)
    : Base(device, descriptor->label), mType(Type::Undefined) {
    if (auto* spirvDesc = descriptor.Get<ShaderModuleSPIRVDescriptor>()) {
        mType = Type::Spirv;
        mOriginalSpirv.assign(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
    } else if (auto* wgslDesc = descriptor.Get<ShaderModuleWGSLDescriptor>()) {
        mType = Type::Wgsl;
        mWgsl = std::string(wgslDesc->code);
    } else {
        DAWN_ASSERT(false);
    }
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor)
    : ShaderModuleBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label)
    : Base(device, tag, label), mType(Type::Undefined) {}

ShaderModuleBase::~ShaderModuleBase() = default;

void ShaderModuleBase::DestroyImpl() {
    Uncache();
}

// static
Ref<ShaderModuleBase> ShaderModuleBase::MakeError(DeviceBase* device, const char* label) {
    return AcquireRef(new ShaderModuleBase(device, ObjectBase::kError, label));
}

ObjectType ShaderModuleBase::GetType() const {
    return ObjectType::ShaderModule;
}

bool ShaderModuleBase::HasEntryPoint(const std::string& entryPoint) const {
    return mEntryPoints.contains(entryPoint);
}

ShaderModuleEntryPoint ShaderModuleBase::ReifyEntryPointName(const char* entryPointName,
                                                             SingleShaderStage stage) const {
    ShaderModuleEntryPoint entryPoint;
    if (entryPointName) {
        entryPoint.defaulted = false;
        entryPoint.name = entryPointName;
    } else {
        entryPoint.defaulted = true;
        entryPoint.name = mDefaultEntryPointNames[stage];
    }
    return entryPoint;
}

const EntryPointMetadata& ShaderModuleBase::GetEntryPoint(const std::string& entryPoint) const {
    DAWN_ASSERT(HasEntryPoint(entryPoint));
    return *mEntryPoints.at(entryPoint);
}

size_t ShaderModuleBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mType);
    recorder.Record(mOriginalSpirv);
    recorder.Record(mWgsl);
    return recorder.GetContentHash();
}

bool ShaderModuleBase::EqualityFunc::operator()(const ShaderModuleBase* a,
                                                const ShaderModuleBase* b) const {
    return a->mType == b->mType && a->mOriginalSpirv == b->mOriginalSpirv && a->mWgsl == b->mWgsl;
}

ShaderModuleBase::ScopedUseTintProgram ShaderModuleBase::UseTintProgram() {
    return mTintData.Use(
        [&](auto tintData) {
            if (tintData->tintProgram) {
                return ScopedUseTintProgram(this);
            }

            // When the ShaderModuleBase is not referenced externally, and not used for initializing
            // any pipeline, the mTintProgram will be released. However the ShaderModuleBase itself
            // may still alive due to being referenced by some pipelines. In this case, when
            // DeviceBase::APICreateShaderModule() with the same shader source code, Dawn will look
            // up from the cache and return the same ShaderModuleBase. In this case, we have to
            // recreate mTintProgram, when the mTintProgram is required for initializing new
            // pipelines.
            ShaderModuleDescriptor descriptor;
            ShaderModuleWGSLDescriptor wgslDescriptor;
            ShaderModuleSPIRVDescriptor sprivDescriptor;

            switch (mType) {
                case Type::Spirv:
                    sprivDescriptor.codeSize = mOriginalSpirv.size();
                    sprivDescriptor.code = mOriginalSpirv.data();
                    descriptor.nextInChain = &sprivDescriptor;
                    break;
                case Type::Wgsl:
                    wgslDescriptor.code = mWgsl.c_str();
                    descriptor.nextInChain = &wgslDescriptor;
                    break;
                default:
                    DAWN_ASSERT(false);
            }

            ShaderModuleParseResult parseResult;
            ValidateAndParseShaderModule(GetDevice(), Unpack(&descriptor), &parseResult,
                                         /*compilationMessages=*/nullptr)
                .AcquireSuccess();
            DAWN_ASSERT(parseResult.tintProgram != nullptr);

            tintData->tintProgram = std::move(parseResult.tintProgram);
            tintData->tintProgramRecreateCount++;

            return ScopedUseTintProgram(this);
        });
}

Ref<TintProgram> ShaderModuleBase::GetTintProgram() const {
    return mTintData.Use([&](auto tintData) {
        DAWN_ASSERT(tintData->tintProgram != nullptr);
        return tintData->tintProgram;
    });
}

Ref<TintProgram> ShaderModuleBase::GetTintProgramForTesting() const {
    return mTintData.Use([&](auto tintData) { return tintData->tintProgram; });
}

int ShaderModuleBase::GetTintProgramRecreateCountForTesting() const {
    return mTintData.Use([&](auto tintData) { return tintData->tintProgramRecreateCount; });
}

void ShaderModuleBase::APIGetCompilationInfo(wgpu::CompilationInfoCallback callback,
                                             void* userdata) {
    if (callback == nullptr) {
        return;
    }

    callback(WGPUCompilationInfoRequestStatus_Success, mCompilationMessages->GetCompilationInfo(),
             userdata);
}

void ShaderModuleBase::InjectCompilationMessages(
    std::unique_ptr<OwnedCompilationMessages> compilationMessages) {
    // TODO(dawn:944): ensure the InjectCompilationMessages is properly handled for shader
    // module returned from cache.
    // InjectCompilationMessages should be called only once for a shader module, after it is
    // created. However currently InjectCompilationMessages may be called on a shader module
    // returned from cache rather than newly created, and violate the rule. We just skip the
    // injection in this case for now, but a proper solution including ensure the cache goes
    // before the validation is required.
    if (mCompilationMessages != nullptr) {
        return;
    }
    // Move the compilationMessages into the shader module and emit the tint errors and warnings
    mCompilationMessages = std::move(compilationMessages);
}

OwnedCompilationMessages* ShaderModuleBase::GetCompilationMessages() const {
    return mCompilationMessages.get();
}

MaybeError ShaderModuleBase::InitializeBase(ShaderModuleParseResult* parseResult,
                                            OwnedCompilationMessages* compilationMessages) {
    DAWN_TRY(mTintData.Use([&](auto tintData) -> MaybeError {
        tintData->tintProgram = std::move(parseResult->tintProgram);

        DAWN_TRY(ReflectShaderUsingTint(GetDevice(), &(tintData->tintProgram->program),
                                        compilationMessages, &mEntryPoints));
        return {};
    }));

    for (auto stage : IterateStages(kAllStages)) {
        mEntryPointCounts[stage] = 0;
    }
    for (auto& [name, metadata] : mEntryPoints) {
        SingleShaderStage stage = metadata->stage;
        if (mEntryPointCounts[stage] == 0) {
            mDefaultEntryPointNames[stage] = name;
        }
        mEntryPointCounts[stage]++;
    }

    return {};
}

void ShaderModuleBase::WillDropLastExternalRef() {
    mTintData.Use([&](auto tintData) { tintData->tintProgram = nullptr; });
}

}  // namespace dawn::native
