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

#include "dawn/common/SystemUtils.h"

#include "dawn/common/Assert.h"

#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>

namespace dawn {

void GetMacOSVersion(int32_t* majorVersion, int32_t* minorVersion) {
    if (@available(macOS 10.10, *)) {
      NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
      ASSERT(majorVersion != nullptr);
      *majorVersion = version.majorVersion;
      if (minorVersion != nullptr) {
          *minorVersion = version.minorVersion;
      }
    } else {
      extern OSErr Gestalt(OSType selector, SInt32 *response) __attribute__((weak_import, weak));
      ASSERT(majorVersion != nullptr);
      ::Gestalt(gestaltSystemVersionMajor, reinterpret_cast<SInt32*>(&majorVersion));
      if (minorVersion != nullptr) {
        ::Gestalt(gestaltSystemVersionMinor, reinterpret_cast<SInt32*>(&minorVersion));
      }
    }
}

bool IsMacOSVersionAtLeast(uint32_t majorVersion, uint32_t minorVersion) {
    if (@available(macOS 10.10, *)) {
      return
          [NSProcessInfo.processInfo isOperatingSystemAtLeastVersion:{majorVersion, minorVersion, 0}];
    } else {
      int32_t curMajorVersion, curMinorVersion;
      GetMacOSVersion(&curMajorVersion, &curMinorVersion);
      if(curMajorVersion>majorVersion)
          return true;

      if(curMajorVersion==majorVersion && curMinorVersion>=minorVersion)
          return true;

      return false;
    }
}

}  // namespace dawn
