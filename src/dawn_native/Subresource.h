// Copyright 2020 The Dawn Authors
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

#ifndef DAWNNATIVE_SUBRESOURCE_H_
#define DAWNNATIVE_SUBRESOURCE_H_

#include "dawn_native/EnumClassBitmasks.h"
#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    // Note: Subresource indices are computed by iterating the aspects in increasing order.
    // D3D12 uses these directly, so the order much match D3D12's indices.
    //  - Depth/Stencil textures have Depth as Plane 0, and Stencil as Plane 1.
    enum class Aspect : uint8_t {
        None = 0x0,
        Color = 0x1,
        Depth = 0x2,
        Stencil = 0x4,
    };

    template <>
    struct EnumBitmaskSize<Aspect> {
        static constexpr unsigned value = 3;
    };

    // Convert the TextureAspect to an Aspect mask for the format. ASSERTs if the aspect
    // does not exist in the format.
    // Also ASSERTs if "All" is selected and results in more than one aspect.
    Aspect ConvertSingleAspect(const Format& format, wgpu::TextureAspect aspect);

    // Convert the TextureAspect to an Aspect mask for the format. ASSERTs if the aspect
    // does not exist in the format.
    Aspect ConvertAspect(const Format& format, wgpu::TextureAspect aspect);

    // Try to convert the TextureAspect to an Aspect mask for the format. May return
    // Aspect::None.
    Aspect TryConvertAspect(const Format& format, wgpu::TextureAspect aspect);

    struct SubresourceRange {
        uint32_t baseMipLevel;
        uint32_t levelCount;
        uint32_t baseArrayLayer;
        uint32_t layerCount;
        Aspect aspects;

        static SubresourceRange SingleMipAndLayer(uint32_t baseMipLevel,
                                                  uint32_t baseArrayLayer,
                                                  Aspect aspects);
    };

    // Helper function to use aspects as linear indices in arrays.
    uint8_t GetAspectIndex(Aspect aspect);
    uint8_t GetAspectCount(Aspect aspects);

}  // namespace dawn_native

namespace wgpu {

    template <>
    struct IsDawnBitmask<dawn_native::Aspect> {
        static constexpr bool enable = true;
    };

}  // namespace wgpu

#endif  // DAWNNATIVE_SUBRESOURCE_H_
