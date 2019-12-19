// Copyright 2019 The Dawn Authors
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

#include "dawn/webgpu_cpp.h"

#include <cstdint>
#include <functional>

namespace dawn_native {

    class Instance;

}  // namespace dawn_native

namespace DawnWireServerFuzzer {

    using MakeDeviceFn = std::function<wgpu::Device(dawn_native::Instance*)>;

    int Run(const uint8_t* data, size_t size, MakeDeviceFn MakeDevice);

}  // namespace DawnWireServerFuzzer
