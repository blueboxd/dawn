// Copyright 2023 The Dawn Authors
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

#include <utility>
#include <vector>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/OpenGLBackend.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ScopedGLTexture {
  public:
    ScopedGLTexture(PFNGLDELETETEXTURESPROC deleteTextures, GLuint texture)
        : mDeleteTextures(deleteTextures), mTexture(texture) {}

    ScopedGLTexture(ScopedGLTexture&& other) {
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
        mDeleteTextures = std::move(other.mDeleteTextures);
        mTexture = std::move(other.mTexture);
    }

    ~ScopedGLTexture() {
        if (mTexture != 0) {
            mDeleteTextures(1, &mTexture);
        }
    }

    GLuint Get() const { return mTexture; }

  private:
    PFNGLDELETETEXTURESPROC mDeleteTextures = nullptr;
    GLuint mTexture = 0;
};

class GLTextureTestBase : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ANGLETextureSharing})) {
            return {
                wgpu::FeatureName::DawnInternalUsages,
                wgpu::FeatureName::ANGLETextureSharing,
            };
        } else {
            return {
                wgpu::FeatureName::DawnInternalUsages,
            };
        }
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::ANGLETextureSharing}));
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

  public:
    ScopedGLTexture CreateGLTexture(uint32_t width,
                                    uint32_t height,
                                    GLenum internalFormat,
                                    GLenum format,
                                    GLenum type,
                                    void* data,
                                    size_t size) {
        native::opengl::Device* openglDevice =
            native::opengl::ToBackend(native::FromAPI(device.Get()));
        const native::opengl::OpenGLFunctions& gl = openglDevice->GetGL();
        GLuint tex;
        gl.GenTextures(1, &tex);
        gl.BindTexture(GL_TEXTURE_2D, tex);
        gl.TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

        return ScopedGLTexture(gl.DeleteTextures, tex);
    }
    wgpu::Texture WrapGLTexture(const wgpu::TextureDescriptor* descriptor, GLuint texture) {
        native::opengl::ExternalImageDescriptorGLTexture externDesc;
        externDesc.cTextureDescriptor = reinterpret_cast<const WGPUTextureDescriptor*>(descriptor);
        externDesc.texture = texture;
        return wgpu::Texture::Acquire(
            native::opengl::WrapExternalGLTexture(device.Get(), &externDesc));
    }
};

// A small fixture used to initialize default data for the GLTexture validation tests.
// These tests are skipped if the harness is using the wire.
class GLTextureValidationTests : public GLTextureTestBase {
  public:
    GLTextureValidationTests() {
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.size = {10, 10, 1};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    }

    ScopedGLTexture CreateDefaultGLTexture() {
        return CreateGLTexture(10, 10, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, 0);
    }

  protected:
    wgpu::TextureDescriptor descriptor;
};

class GLTextureValidationNoANGLETextureSharingTests : public GLTextureValidationTests {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }
};

// Test a successful wrapping of a GL texture in a WebGPU texture
TEST_P(GLTextureValidationTests, Success) {
    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an unsuccessful wrapping of a GL texture in a WebGPU texture when
// the device does not support the ANGLETextureSharing Feature
TEST_P(GLTextureValidationNoANGLETextureSharingTests, Failure) {
    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test a successful wrapping of a GL texture in a texture with DawnTextureInternalUsageDescriptor
TEST_P(GLTextureValidationTests, SuccessWithInternalUsageDescriptor) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    descriptor.nextInChain = &internalDesc;
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
    internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an error occurs if an invalid sType is the nextInChain
TEST_P(GLTextureValidationTests, InvalidTextureDescriptor) {
    wgpu::ChainedStruct chainedDescriptor;
    chainedDescriptor.sType = wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel;
    descriptor.nextInChain = &chainedDescriptor;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor dimension isn't 2D
TEST_P(GLTextureValidationTests, InvalidTextureDimension) {
    descriptor.dimension = wgpu::TextureDimension::e3D;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the texture usage is not RenderAttachment
TEST_P(GLTextureValidationTests, InvalidTextureUsage) {
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    wgpu::Texture texture;
    ASSERT_DEVICE_ERROR(texture = WrapGLTexture(&descriptor, glTexture.Get()));

    ASSERT_EQ(texture.Get(), nullptr);
    descriptor.usage = wgpu::TextureUsage::StorageBinding;

    ASSERT_DEVICE_ERROR(texture = WrapGLTexture(&descriptor, glTexture.Get()));

    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(GLTextureValidationTests, InvalidMipLevelCount) {
    descriptor.mipLevelCount = 2;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(GLTextureValidationTests, InvalidDepth) {
    descriptor.size.depthOrArrayLayers = 2;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(GLTextureValidationTests, InvalidSampleCount) {
    descriptor.sampleCount = 4;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the surface's
TEST_P(GLTextureValidationTests, InvalidWidth) {
    descriptor.size.width = 11;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the surface's
TEST_P(GLTextureValidationTests, InvalidHeight) {
    descriptor.size.height = 11;

    ScopedGLTexture glTexture = CreateDefaultGLTexture();
    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapGLTexture(&descriptor, glTexture.Get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Fixture to test using GL textures through different usages.
// These tests are skipped if the harness is using the wire.
class GLTextureUsageTests : public GLTextureTestBase {
  public:
    // Test that clearing using BeginRenderPass writes correct data in the GL texture.
    void DoClearTest(GLuint texture,
                     wgpu::TextureFormat format,
                     GLenum glFormat,
                     GLenum glType,
                     void* data,
                     size_t dataSize) {
        native::opengl::Device* openglDevice =
            native::opengl::ToBackend(native::FromAPI(device.Get()));
        const native::opengl::OpenGLFunctions& gl = openglDevice->GetGL();

        // Get a texture view for the GL texture.
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.sampleCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
        textureDescriptor.nextInChain = &internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
        internalDesc.sType = wgpu::SType::DawnTextureInternalUsageDescriptor;

        wgpu::Texture wrappedTexture = WrapGLTexture(&textureDescriptor, texture);
        ASSERT_NE(wrappedTexture, nullptr);

        wgpu::TextureView wrappedView = wrappedTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPassDescriptor({wrappedView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {1 / 255.0f, 2 / 255.0f, 3 / 255.0f,
                                                                4 / 255.0f};

        // Execute commands to clear the wrapped texture
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check the correct data was written
        std::vector<uint8_t> result(dataSize);
        GLuint fbo;
        gl.GenFramebuffers(1, &fbo);
        gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl.FramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                                0);
        gl.ReadPixels(0, 0, 1, 1, glFormat, glType, result.data());
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
        gl.DeleteFramebuffers(1, &fbo);
        ASSERT_EQ(0, memcmp(result.data(), data, dataSize));
    }
};

// Test clearing a R8 GL texture
TEST_P(GLTextureUsageTests, ClearR8GLTexture) {
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE, nullptr, 0);

    uint8_t data = 0x01;
    DoClearTest(glTexture.Get(), wgpu::TextureFormat::R8Unorm, GL_RED, GL_UNSIGNED_BYTE, &data,
                sizeof(data));
}

// Test clearing a RG8 GL texture
TEST_P(GLTextureUsageTests, ClearRG8GLTexture) {
    ScopedGLTexture glTexture = CreateGLTexture(1, 1, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, nullptr, 0);

    uint16_t data = 0x0201;
    DoClearTest(glTexture.Get(), wgpu::TextureFormat::RG8Unorm, GL_RG, GL_UNSIGNED_BYTE, &data,
                sizeof(data));
}

// Test clearing an RGBA8 GL texture
TEST_P(GLTextureUsageTests, ClearRGBA8GLTexture) {
    ScopedGLTexture glTexture =
        CreateGLTexture(1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, 0);

    uint32_t data = 0x04030201;
    DoClearTest(glTexture.Get(), wgpu::TextureFormat::RGBA8Unorm, GL_RGBA, GL_UNSIGNED_BYTE, &data,
                sizeof(data));
}

DAWN_INSTANTIATE_TEST(GLTextureValidationTests, OpenGLESBackend());
DAWN_INSTANTIATE_TEST(GLTextureValidationNoANGLETextureSharingTests, OpenGLESBackend());
DAWN_INSTANTIATE_TEST(GLTextureUsageTests, OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn
