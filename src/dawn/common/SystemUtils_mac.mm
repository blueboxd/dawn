// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/common/SystemUtils.h"

#include "dawn/common/Assert.h"

#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>

namespace dawn {

void GetMacOSVersion(int32_t* majorVersion, int32_t* minorVersion) {
    if (@available(macOS 10.10, *)) {
      NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
      DAWN_ASSERT(majorVersion != nullptr);
      *majorVersion = version.majorVersion;
      if (minorVersion != nullptr) {
        *minorVersion = version.minorVersion;
      }
    } else {
      extern OSErr Gestalt(OSType selector, SInt32 *response) __attribute__((weak_import, weak));
      DAWN_ASSERT(majorVersion != nullptr);
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
