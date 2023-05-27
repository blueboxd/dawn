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

#ifndef SRC_DAWN_NATIVE_D3D_FENCED3D_H_
#define SRC_DAWN_NATIVE_D3D_FENCED3D_H_

#include "dawn/common/RefCounted.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d {

class Fence : public RefCounted {
  public:
    UINT64 GetFenceValue() const;

  protected:
    Fence(UINT64 fenceValue, HANDLE sharedHandle);
    ~Fence() override;

    const UINT64 mFenceValue;
    const HANDLE mSharedHandle;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_FENCE_H_
