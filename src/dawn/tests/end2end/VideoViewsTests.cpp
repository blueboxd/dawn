// Copyright 2021 The Dawn Authors
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

#include "dawn/tests/end2end/VideoViewsTests.h"

#include <utility>

#include "dawn/common/Math.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

VideoViewsTestBackend::PlatformTexture::PlatformTexture(wgpu::Texture&& texture)
    : wgpuTexture(texture) {}
VideoViewsTestBackend::PlatformTexture::~PlatformTexture() = default;

VideoViewsTestBackend::~VideoViewsTestBackend() = default;

constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kYellowYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kWhiteYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kBlueYUVColor;
constexpr std::array<utils::RGBA8, 2> VideoViewsTestsBase::kRedYUVColor;

void VideoViewsTestsBase::SetUp() {
    DawnTestWithParams<Params>::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
}

std::vector<wgpu::FeatureName> VideoViewsTestsBase::GetRequiredFeatures() {
    std::vector<wgpu::FeatureName> requiredFeatures = {};
    mIsMultiPlanarFormatsSupported = SupportsFeatures({wgpu::FeatureName::DawnMultiPlanarFormats});
    if (mIsMultiPlanarFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::DawnMultiPlanarFormats);
    }
    mIsMultiPlanarFormatP010Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatP010});
    if (mIsMultiPlanarFormatP010Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatP010);
    }
    mIsNorm16TextureFormatsSupported = SupportsFeatures({wgpu::FeatureName::Norm16TextureFormats});
    if (mIsNorm16TextureFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::Norm16TextureFormats);
    }
    requiredFeatures.push_back(wgpu::FeatureName::DawnInternalUsages);
    return requiredFeatures;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatsSupported() const {
    return mIsMultiPlanarFormatsSupported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatP010Supported() const {
    return mIsMultiPlanarFormatP010Supported;
}

bool VideoViewsTestsBase::IsNorm16TextureFormatsSupported() const {
    return mIsNorm16TextureFormatsSupported;
}

bool VideoViewsTestsBase::IsFormatSupported() const {
    if (GetFormat() == wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm) {
        // DXGI_FORMAT_P010 can't be shared between D3D11 and D3D12.
        if (IsD3D12()) {
            return false;
        }
        // DXGI_FORMAT_P010 is not supported on WARP.
        if (IsWARP()) {
            return false;
        }
        return IsNorm16TextureFormatsSupported() && IsMultiPlanarFormatP010Supported();
    }

    return true;
}

// Returns a pre-prepared multi-planar formatted texture
// The encoded texture data represents a 4x4 converted image. When |isCheckerboard| is true,
// the top left is a 2x2 yellow block, bottom right is a 2x2 red block, top right is a 2x2
// blue block, and bottom left is a 2x2 white block. When |isCheckerboard| is false, the
// image is converted from a solid yellow 4x4 block.
// static
template <typename T>
std::vector<T> VideoViewsTestsBase::GetTestTextureData(wgpu::TextureFormat format,
                                                       bool isCheckerboard) {
    const uint8_t kLeftShiftBits = (sizeof(T) - 1) * 8;
    constexpr T Yy = kYellowYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Yu = kYellowYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Yv = kYellowYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    constexpr T Wy = kWhiteYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Wu = kWhiteYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Wv = kWhiteYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    constexpr T Ry = kRedYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Ru = kRedYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Rv = kRedYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    constexpr T By = kBlueYUVColor[kYUVLumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Bu = kBlueYUVColor[kYUVChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Bv = kBlueYUVColor[kYUVChromaPlaneIndex].g << kLeftShiftBits;

    switch (format) {
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // is half the number of bytes (subsampled by 2) but same bytes per line as luma
        // plane.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            if (isCheckerboard) {
                return {
                    Wy, Wy, Ry, Ry,  // plane 0, start + 0
                    Wy, Wy, Ry, Ry,  //
                    Yy, Yy, By, By,  //
                    Yy, Yy, By, By,  //
                    Wu, Wv, Ru, Rv,  // plane 1, start + 16
                    Yu, Yv, Bu, Bv,  //
                };
            } else {
                return {
                    Yy, Yy, Yy, Yy,  // plane 0, start + 0
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yu, Yv, Yu, Yv,  // plane 1, start + 16
                    Yu, Yv, Yu, Yv,  //
                };
            }
        case wgpu::TextureFormat::RGBA8Unorm:
            // Combines both NV12 planes by directly mapping back to RGB: R=Y, G=U, B=V.
            if (isCheckerboard) {
                return {
                    Yy, Yu, Yv, Yy, Yu, Yv, By, Bu, Bv, By, Bu, Bv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, By, Bu, Bv, By, Bu, Bv,  //
                    Wy, Wu, Wv, Wy, Wu, Wv, Ry, Ru, Rv, Ry, Ru, Rv,  //
                    Wy, Wu, Wv, Wy, Wu, Wv, Ry, Ru, Rv, Ry, Ru, Rv,  //
                };
            } else {
                return {
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                };
            }
        default:
            UNREACHABLE();
            return {};
    }
}

template std::vector<uint8_t> VideoViewsTestsBase::GetTestTextureData<uint8_t>(
    wgpu::TextureFormat format,
    bool isCheckerboard);
template std::vector<uint16_t> VideoViewsTestsBase::GetTestTextureData<uint16_t>(
    wgpu::TextureFormat format,
    bool isCheckerboard);

uint32_t VideoViewsTestsBase::NumPlanes(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return 2;
        default:
            UNREACHABLE();
            return 0;
    }
}
std::vector<uint8_t> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex(size_t planeIndex,
                                                                           size_t bytesPerRow,
                                                                           size_t height,
                                                                           bool isCheckerboard) {
    std::vector<uint8_t> texelData = VideoViewsTestsBase::GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::R8BG8Biplanar420Unorm, isCheckerboard);
    const uint32_t texelDataRowBytes = kYUVImageDataWidthInTexels;
    const uint32_t texelDataHeight =
        planeIndex == 0 ? kYUVImageDataHeightInTexels : kYUVImageDataHeightInTexels / 2;

    std::vector<uint8_t> texels(bytesPerRow * height, 0);
    uint32_t plane_first_texel_offset = 0;
    // The size of the test video frame is 4 x 4
    switch (planeIndex) {
        case VideoViewsTestsBase::kYUVLumaPlaneIndex:
            for (uint32_t i = 0; i < texelDataHeight; ++i) {
                if (i < texelDataHeight) {
                    for (uint32_t j = 0; j < texelDataRowBytes; ++j) {
                        texels[bytesPerRow * i + j] =
                            texelData[texelDataRowBytes * i + j + plane_first_texel_offset];
                    }
                }
            }
            return texels;
        case VideoViewsTestsBase::kYUVChromaPlaneIndex:
            // TexelData is 4 * 6 size, first 4 * 4 is Y plane, UV plane started
            // at index 16.
            plane_first_texel_offset = 16;
            for (uint32_t i = 0; i < texelDataHeight; ++i) {
                if (i < texelDataHeight) {
                    for (uint32_t j = 0; j < texelDataRowBytes; ++j) {
                        texels[bytesPerRow * i + j] =
                            texelData[texelDataRowBytes * i + j + plane_first_texel_offset];
                    }
                }
            }
            return texels;
        default:
            UNREACHABLE();
            return {};
    }
}

wgpu::TextureFormat VideoViewsTestsBase::GetFormat() const {
    return GetParam().mFormat;
}

wgpu::TextureFormat VideoViewsTestsBase::GetPlaneFormat(int plane) const {
    switch (GetFormat()) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return plane == 0 ? wgpu::TextureFormat::R8Unorm : wgpu::TextureFormat::RG8Unorm;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return plane == 0 ? wgpu::TextureFormat::R16Unorm : wgpu::TextureFormat::RG16Unorm;
        default:
            UNREACHABLE();
            return wgpu::TextureFormat::Undefined;
    }
}

// Vertex shader used to render a sampled texture into a quad.
wgpu::ShaderModule VideoViewsTestsBase::GetTestVertexShaderModule() const {
    return utils::CreateShaderModule(device, R"(
                struct VertexOut {
                    @location(0) texCoord : vec2 <f32>,
                    @builtin(position) position : vec4f,
                }

                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                    var pos = array(
                        vec2f(-1.0, 1.0),
                        vec2f(-1.0, -1.0),
                        vec2f(1.0, -1.0),
                        vec2f(-1.0, 1.0),
                        vec2f(1.0, -1.0),
                        vec2f(1.0, 1.0)
                    );
                    var output : VertexOut;
                    output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                    output.texCoord = vec2f(output.position.xy * 0.5) + vec2f(0.5, 0.5);
                    return output;
            })");
}

class VideoViewsTests : public VideoViewsTestsBase {
  protected:
    void SetUp() override {
        VideoViewsTestsBase::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
        DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

        mBackend = VideoViewsTestBackend::Create();
        mBackend->OnSetUp(device.Get());
    }

    void TearDown() override {
        if (mBackend) {
            mBackend->OnTearDown();
            mBackend = nullptr;
        }
        VideoViewsTestsBase::TearDown();
    }
    std::unique_ptr<VideoViewsTestBackend> mBackend;
};

namespace {

// Create video texture uninitialized.
TEST_P(VideoViewsTests, CreateVideoTextureWithoutInitializedData) {
    ASSERT_DEVICE_ERROR(std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
                            mBackend->CreateVideoTextureForTest(GetFormat(),
                                                                wgpu::TextureUsage::TextureBinding,
                                                                /*isCheckerboard*/ false,
                                                                /*initialized*/ false));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the luminance (Y) plane from an imported bi-planar 420 texture into a single channel of
// an RGBA output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, SampleYtoR) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView textureView = platformTexture->wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(texture, sampler0, texCoord).r;
               return vec4f(y, 0.0, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the luma plane in the top left corner of RGB image.
    EXPECT_TEXTURE_EQ(&kYellowYUVColor[kYUVLumaPlaneIndex], renderPass.color, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the chrominance (UV) plane from an imported bi-planar 420 texture into two channels of an
// RGBA output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, SampleUVtoRG) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = GetPlaneFormat(1);
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView textureView = platformTexture->wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let u : f32 = textureSample(texture, sampler0, texCoord).r;
               let v : f32 = textureSample(texture, sampler0, texCoord).g;
               return vec4f(u, v, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the chroma plane in the top left corner of RGB image.
    EXPECT_TEXTURE_EQ(&kYellowYUVColor[kYUVChromaPlaneIndex], renderPass.color, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGB quad, then checks the the entire
// contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVtoRGB) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, lumaTextureView}, {2, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGB quad with two samplers, then checks the the
// entire contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVtoRGBMultipleSamplers) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;
            @group(0) @binding(2) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(3) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler1, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler1, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler0 = device.CreateSampler();
    wgpu::Sampler sampler1 = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(
                   device, renderPipeline.GetBindGroupLayout(0),
                   {{0, sampler0}, {1, sampler1}, {2, lumaTextureView}, {3, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

class VideoViewsValidationTests : public VideoViewsTests {
  protected:
    void SetUp() override {
        VideoViewsTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    }
};

// Test explicitly creating a multiplanar format is not allowed
TEST_P(VideoViewsValidationTests, ExplicitCreation) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Test texture view creation rules.
TEST_P(VideoViewsValidationTests, CreateViewValidation) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor viewDesc = {};

    // Success case: Per plane view formats unspecified.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
    }

    // Success case: Per plane view formats specified and aspect.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        viewDesc.format = GetPlaneFormat(0);
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        viewDesc.format = GetPlaneFormat(1);
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
    }

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Correct plane index but incompatible view format.
    viewDesc.format = wgpu::TextureFormat::R8Uint;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong plane index.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong aspect.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::All;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Create a single plane texture.
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.usage = wgpu::TextureUsage::TextureBinding;
    desc.size = {1, 1, 1};

    wgpu::Texture texture = device.CreateTexture(&desc);

    // Plane aspect specified with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    // Planar views with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from one multi-planar format into another fails.
TEST_P(VideoViewsValidationTests, T2TCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture1 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture2 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::Texture srcTexture = platformTexture1->wgpuTexture;
    wgpu::Texture dstTexture = platformTexture2->wgpuTexture;

    wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0});

    wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture1));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture2));
}

// Test copying from one multi-planar format into another per plane fails.
TEST_P(VideoViewsValidationTests, T2TCopyPlaneAspectFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture1 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture2 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::Texture srcTexture = platformTexture1->wgpuTexture;
    wgpu::Texture dstTexture = platformTexture2->wgpuTexture;

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture1));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture2));
}

// Test copying from a multi-planar format to a buffer fails.
TEST_P(VideoViewsValidationTests, T2BCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture srcTexture = platformTexture->wgpuTexture;

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 1;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc = utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0});

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 4);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from multi-planar format per plane to a buffer fails.
TEST_P(VideoViewsValidationTests, T2BCopyPlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture srcTexture = platformTexture->wgpuTexture;

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 1;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 4);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from a buffer to a multi-planar format fails.
TEST_P(VideoViewsValidationTests, B2TCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture dstTexture = platformTexture->wgpuTexture;

    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(srcBuffer, 0, 12, 4);

    wgpu::ImageCopyTexture copyDst = utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from a buffer to a multi-planar format per plane fails.
TEST_P(VideoViewsValidationTests, B2TCopyPlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture dstTexture = platformTexture->wgpuTexture;

    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::ImageCopyBuffer copySrc = utils::CreateImageCopyBuffer(srcBuffer, 0, 12, 4);

    wgpu::ImageCopyTexture copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copyDst =
        utils::CreateImageCopyTexture(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests which multi-planar formats are allowed to be sampled.
TEST_P(VideoViewsValidationTests, SamplingMultiPlanarTexture) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    // R8BG8Biplanar420Unorm is allowed to be sampled, if plane 0 or plane 1 is selected.
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureViewDescriptor desc = {};

    desc.aspect = wgpu::TextureAspect::Plane0Only;
    utils::MakeBindGroup(device, layout, {{0, platformTexture->wgpuTexture.CreateView(&desc)}});

    desc.aspect = wgpu::TextureAspect::Plane1Only;
    utils::MakeBindGroup(device, layout, {{0, platformTexture->wgpuTexture.CreateView(&desc)}});

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests creating a texture with a multi-plane format.
TEST_P(VideoViewsValidationTests, RenderAttachmentInvalid) {
    // multi-planar formats are NOT allowed to be renderable.
    ASSERT_DEVICE_ERROR(auto platformTexture = mBackend->CreateVideoTextureForTest(
                            GetFormat(), wgpu::TextureUsage::RenderAttachment,
                            /*isCheckerboard*/ true,
                            /*initialized*/ true));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests writing into a multi-planar format fails.
TEST_P(VideoViewsValidationTests, WriteTextureAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 4, 4);

    wgpu::ImageCopyTexture imageCopyTexture =
        utils::CreateImageCopyTexture(platformTexture->wgpuTexture, 0, {0, 0, 0});

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyTexture, placeholderData.data(),
                                           placeholderData.size(), &textureDataLayout, &writeSize));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests writing into a multi-planar format per plane fails.
TEST_P(VideoViewsValidationTests, WriteTexturePlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureDataLayout textureDataLayout = utils::CreateTextureDataLayout(0, 12, 4);
    wgpu::ImageCopyTexture imageCopyTexture = utils::CreateImageCopyTexture(
        platformTexture->wgpuTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&imageCopyTexture, placeholderData.data(),
                                           placeholderData.size(), &textureDataLayout, &writeSize));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

class VideoViewsExtendedUsagesTests : public VideoViewsTestsBase {
  protected:
    void SetUp() override {
        VideoViewsTestsBase::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());

        DAWN_TEST_UNSUPPORTED_IF(
            !device.HasFeature(wgpu::FeatureName::MultiPlanarFormatExtendedUsages));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures =
            VideoViewsTestsBase::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatExtendedUsages})) {
            requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatExtendedUsages);
        }
        return requiredFeatures;
    }

    wgpu::Texture CreateMultiPlanarTexture(wgpu::TextureFormat format,
                                           wgpu::TextureUsage usage,
                                           bool isCheckerboard = false,
                                           bool initialized = true) {
        wgpu::TextureDescriptor desc;
        desc.format = format;
        desc.size = {VideoViewsTestsBase::kYUVImageDataWidthInTexels,
                     VideoViewsTestsBase::kYUVImageDataHeightInTexels, 1};
        desc.usage = usage;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
        desc.nextInChain = &internalDesc;

        auto texture = device.CreateTexture(&desc);
        if (texture == nullptr) {
            return nullptr;
        }

        if (initialized) {
            size_t numPlanes = VideoViewsTestsBase::NumPlanes(format);

            wgpu::DawnEncoderInternalUsageDescriptor encoderInternalDesc;
            encoderInternalDesc.useInternalUsages = true;
            wgpu::CommandEncoderDescriptor encoderDesc;
            encoderDesc.nextInChain = &encoderInternalDesc;

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

            for (size_t plane = 0; plane < numPlanes; ++plane) {
                size_t bytesPerRow = VideoViewsTestsBase::kYUVImageDataWidthInTexels;
                bytesPerRow = Align(bytesPerRow, 256);

                wgpu::ImageCopyTexture copyDst =
                    utils::CreateImageCopyTexture(texture, 0, {0, 0, 0});

                wgpu::Extent3D copySize{VideoViewsTestsBase::kYUVImageDataWidthInTexels,
                                        VideoViewsTestsBase::kYUVImageDataHeightInTexels, 1};
                switch (plane) {
                    case VideoViewsTestsBase::kYUVLumaPlaneIndex:
                        copyDst.aspect = wgpu::TextureAspect::Plane0Only;
                        break;
                    case VideoViewsTestsBase::kYUVChromaPlaneIndex:
                        copyDst.aspect = wgpu::TextureAspect::Plane1Only;
                        copySize.width /= 2;
                        copySize.height /= 2;
                        break;
                    default:
                        UNREACHABLE();
                }

                // Staging buffer.
                wgpu::BufferDescriptor bufferDesc;
                bufferDesc.size = bytesPerRow * copySize.height;
                bufferDesc.mappedAtCreation = true;
                bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

                auto buffer = device.CreateBuffer(&bufferDesc);

                std::vector<uint8_t> data = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex(
                    plane, bytesPerRow, VideoViewsTestsBase::kYUVImageDataHeightInTexels,
                    isCheckerboard);

                memcpy(buffer.GetMappedRange(), data.data(), bufferDesc.size);
                buffer.Unmap();

                wgpu::ImageCopyBuffer copySrc =
                    utils::CreateImageCopyBuffer(buffer, 0, bytesPerRow);

                encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
            }  // for plane

            auto cmdBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &cmdBuffer);
        }  // initialized

        return texture;
    }
};

// Test that creating multi-planar texture should success if device is created with
// MultiPlanarFormatExtendedUsages feature enabled.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureSucceeds) {
    auto texture = CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding);
    EXPECT_NE(texture, nullptr);
}

// Tests sampling a multi-planar texture.
TEST_P(VideoViewsExtendedUsagesTests, SamplingMultiPlanarTexture) {
    // TODO(crbug.com/dawn/1998): Failure on Intel's Vulkan device.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    auto texture = CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                            wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    EXPECT_NE(texture, nullptr);

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = wgpu::TextureFormat::R8Unorm;
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = texture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = wgpu::TextureFormat::RG8Unorm;
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = texture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, lumaTextureView}, {2, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard*/ true);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVImageDataWidthInTexels, kYUVImageDataHeightInTexels});
}

// Test copying from multi-planar format per plane to a buffer succeeds.
TEST_P(VideoViewsExtendedUsagesTests, T2BCopyPlaneAspectsSucceeds) {
    const std::vector<uint8_t> expectedData =
        GetTestTextureData<uint8_t>(wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard*/ false);

    auto srcTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);
    EXPECT_NE(srcTexture, nullptr);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyBuffer copyDst = utils::CreateImageCopyBuffer(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    // Plane0
    wgpu::ImageCopyTexture copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

        auto cmdBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &cmdBuffer);

        EXPECT_BUFFER_U8_EQ(expectedData[0], dstBuffer, 0);
    }

    // Plane1
    copySrc =
        utils::CreateImageCopyTexture(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

        auto cmdBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &cmdBuffer);

        uint16_t expectedUVData;
        memcpy(&expectedUVData, expectedData.data() + 1, sizeof(expectedUVData));
        EXPECT_BUFFER_U16_EQ(expectedUVData, dstBuffer, 0);
    }
}

DAWN_INSTANTIATE_TEST_B(VideoViewsTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());
DAWN_INSTANTIATE_TEST_B(VideoViewsValidationTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());

DAWN_INSTANTIATE_TEST_B(VideoViewsExtendedUsagesTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {wgpu::TextureFormat::R8BG8Biplanar420Unorm});

}  // anonymous namespace
}  // namespace dawn
