// Copyright 2017 The Dawn Authors
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

#include "SampleUtils.h"

#include "common/Assert.h"
#include "common/Platform.h"
#include "utils/BackendBinding.h"
#include "utils/TerribleCommandBuffer.h"

#include <dawn/dawn.h>
#include <dawn/dawn_wsi.h>
#include <dawn/dawncpp.h>
#include <dawn_native/DawnNative.h>
#include <dawn_wire/WireClient.h>
#include <dawn_wire/WireServer.h>
#include "GLFW/glfw3.h"

#include <algorithm>
#include <cstring>
#include <iostream>

void PrintDeviceError(const char* message, dawn::CallbackUserdata) {
    std::cout << "Device error: " << message << std::endl;
}

void PrintGLFWError(int code, const char* message) {
    std::cout << "GLFW error: " << code << " - " << message << std::endl;
}

enum class CmdBufType {
    None,
    Terrible,
    //TODO(cwallez@chromium.org) double terrible cmdbuf
};

// Default to D3D12, Metal, Vulkan, OpenGL in that order as D3D12 and Metal are the preferred on
// their respective platforms, and Vulkan is preferred to OpenGL
#if defined(DAWN_ENABLE_BACKEND_D3D12)
    static dawn_native::BackendType backendType = dawn_native::BackendType::D3D12;
#elif defined(DAWN_ENABLE_BACKEND_METAL)
    static dawn_native::BackendType backendType = dawn_native::BackendType::Metal;
#elif defined(DAWN_ENABLE_BACKEND_OPENGL)
    static dawn_native::BackendType backendType = dawn_native::BackendType::OpenGL;
#elif defined(DAWN_ENABLE_BACKEND_VULKAN)
    static dawn_native::BackendType backendType = dawn_native::BackendType::Vulkan;
#else
    #error
#endif

static CmdBufType cmdBufType = CmdBufType::Terrible;
static std::unique_ptr<dawn_native::Instance> instance;
static utils::BackendBinding* binding = nullptr;

static GLFWwindow* window = nullptr;

static dawn_wire::WireServer* wireServer = nullptr;
static dawn_wire::WireClient* wireClient = nullptr;
static utils::TerribleCommandBuffer* c2sBuf = nullptr;
static utils::TerribleCommandBuffer* s2cBuf = nullptr;

dawn::Device CreateCppDawnDevice() {
    glfwSetErrorCallback(PrintGLFWError);
    if (!glfwInit()) {
        return dawn::Device();
    }

    // Create the test window and discover adapters using it (esp. for OpenGL)
    utils::SetupGLFWWindowHintsForBackend(backendType);
    window = glfwCreateWindow(640, 480, "Dawn window", nullptr, nullptr);
    if (!window) {
        return dawn::Device();
    }

    instance = std::make_unique<dawn_native::Instance>();
    utils::DiscoverAdapter(instance.get(), window, backendType);

    // Get an adapter for the backend to use, and create the device.
    dawn_native::Adapter backendAdapter;
    {
        std::vector<dawn_native::Adapter> adapters = instance->GetAdapters();
        auto adapterIt = std::find_if(adapters.begin(), adapters.end(),
                                      [](const dawn_native::Adapter adapter) -> bool {
            return adapter.GetBackendType() == backendType;
        });
        ASSERT(adapterIt != adapters.end());
        backendAdapter = *adapterIt;
    }

    dawnDevice backendDevice = backendAdapter.CreateDevice();
    dawnProcTable backendProcs = dawn_native::GetProcs();

    binding = utils::CreateBinding(backendType, window, backendDevice);
    if (binding == nullptr) {
        return dawn::Device();
    }

    // Choose whether to use the backend procs and devices directly, or set up the wire.
    dawnDevice cDevice = nullptr;
    dawnProcTable procs;

    switch (cmdBufType) {
        case CmdBufType::None:
            procs = backendProcs;
            cDevice = backendDevice;
            break;

        case CmdBufType::Terrible:
            {
                c2sBuf = new utils::TerribleCommandBuffer();
                s2cBuf = new utils::TerribleCommandBuffer();

                wireServer = new dawn_wire::WireServer(backendDevice, backendProcs, s2cBuf);
                c2sBuf->SetHandler(wireServer);

                wireClient = new dawn_wire::WireClient(c2sBuf);
                dawnDevice clientDevice = wireClient->GetDevice();
                dawnProcTable clientProcs = wireClient->GetProcs();
                s2cBuf->SetHandler(wireClient);

                procs = clientProcs;
                cDevice = clientDevice;
            }
            break;
    }

    dawnSetProcs(&procs);
    procs.deviceSetErrorCallback(cDevice, PrintDeviceError, 0);
    return dawn::Device::Acquire(cDevice);
}

uint64_t GetSwapChainImplementation() {
    return binding->GetSwapChainImplementation();
}

dawn::TextureFormat GetPreferredSwapChainTextureFormat() {
    DoFlush();
    return static_cast<dawn::TextureFormat>(binding->GetPreferredSwapChainTextureFormat());
}

dawn::SwapChain GetSwapChain(const dawn::Device &device) {
    return device.CreateSwapChainBuilder()
        .SetImplementation(GetSwapChainImplementation())
        .GetResult();
}

dawn::TextureView CreateDefaultDepthStencilView(const dawn::Device& device) {
    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.size.width = 640;
    descriptor.size.height = 480;
    descriptor.size.depth = 1;
    descriptor.arraySize = 1;
    descriptor.sampleCount = 1;
    descriptor.format = dawn::TextureFormat::D32FloatS8Uint;
    descriptor.levelCount = 1;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment;
    auto depthStencilTexture = device.CreateTexture(&descriptor);
    return depthStencilTexture.CreateDefaultTextureView();
}

void GetNextRenderPassDescriptor(const dawn::Device& device,
    const dawn::SwapChain& swapchain,
    const dawn::TextureView& depthStencilView,
    dawn::Texture* backbuffer,
    dawn::RenderPassDescriptor* info) {
    *backbuffer = swapchain.GetNextTexture();
    auto backbufferView = backbuffer->CreateDefaultTextureView();
    dawn::RenderPassColorAttachmentDescriptor colorAttachment;
    colorAttachment.attachment = backbufferView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    colorAttachment.loadOp = dawn::LoadOp::Clear;
    colorAttachment.storeOp = dawn::StoreOp::Store;

    dawn::RenderPassDepthStencilAttachmentDescriptor depthStencilAttachment;
    depthStencilAttachment.attachment = depthStencilView;
    depthStencilAttachment.depthLoadOp = dawn::LoadOp::Clear;
    depthStencilAttachment.stencilLoadOp = dawn::LoadOp::Clear;
    depthStencilAttachment.clearDepth = 1.0f;
    depthStencilAttachment.clearStencil = 0;
    depthStencilAttachment.depthStoreOp = dawn::StoreOp::Store;
    depthStencilAttachment.stencilStoreOp = dawn::StoreOp::Store;

    *info = device.CreateRenderPassDescriptorBuilder()
        .SetColorAttachments(1, &colorAttachment)
        .SetDepthStencilAttachment(&depthStencilAttachment)
        .GetResult();
}

bool InitSample(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string("-b") == argv[i] || std::string("--backend") == argv[i]) {
            i++;
            if (i < argc && std::string("d3d12") == argv[i]) {
                backendType = dawn_native::BackendType::D3D12;
                continue;
            }
            if (i < argc && std::string("metal") == argv[i]) {
                backendType = dawn_native::BackendType::Metal;
                continue;
            }
            if (i < argc && std::string("null") == argv[i]) {
                backendType = dawn_native::BackendType::Null;
                continue;
            }
            if (i < argc && std::string("opengl") == argv[i]) {
                backendType = dawn_native::BackendType::OpenGL;
                continue;
            }
            if (i < argc && std::string("vulkan") == argv[i]) {
                backendType = dawn_native::BackendType::Vulkan;
                continue;
            }
            fprintf(stderr, "--backend expects a backend name (opengl, metal, d3d12, null, vulkan)\n");
            return false;
        }
        if (std::string("-c") == argv[i] || std::string("--command-buffer") == argv[i]) {
            i++;
            if (i < argc && std::string("none") == argv[i]) {
                cmdBufType = CmdBufType::None;
                continue;
            }
            if (i < argc && std::string("terrible") == argv[i]) {
                cmdBufType = CmdBufType::Terrible;
                continue;
            }
            fprintf(stderr, "--command-buffer expects a command buffer name (none, terrible)\n");
            return false;
        }
        if (std::string("-h") == argv[i] || std::string("--help") == argv[i]) {
            printf("Usage: %s [-b BACKEND] [-c COMMAND_BUFFER]\n", argv[0]);
            printf("  BACKEND is one of: d3d12, metal, null, opengl, vulkan\n");
            printf("  COMMAND_BUFFER is one of: none, terrible\n");
            return false;
        }
    }
    return true;
}

void DoFlush() {
    if (cmdBufType == CmdBufType::Terrible) {
        bool c2sSuccess = c2sBuf->Flush();
        bool s2cSuccess = s2cBuf->Flush();

        ASSERT(c2sSuccess && s2cSuccess);
    }
    glfwPollEvents();
}

bool ShouldQuit() {
    return glfwWindowShouldClose(window);
}

GLFWwindow* GetGLFWWindow() {
    return window;
}
