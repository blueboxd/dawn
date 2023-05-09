// Copyright 2018 The Dawn Authors
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

#include "dawn/native/PhysicalDevice.h"

#include <algorithm>
#include <memory>

#include "dawn/common/Constants.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

PhysicalDeviceBase::PhysicalDeviceBase(InstanceBase* instance,
                                       wgpu::BackendType backend,
                                       const TogglesState& adapterToggles)
    : mInstance(instance), mBackend(backend), mTogglesState(adapterToggles) {
    ASSERT(adapterToggles.GetStage() == ToggleStage::Adapter);
}

PhysicalDeviceBase::~PhysicalDeviceBase() = default;

MaybeError PhysicalDeviceBase::Initialize() {
    DAWN_TRY_CONTEXT(InitializeImpl(), "initializing adapter (backend=%s)", mBackend);
    InitializeVendorArchitectureImpl();

    EnableFeature(Feature::DawnNative);
    EnableFeature(Feature::DawnInternalUsages);
    EnableFeature(Feature::ImplicitDeviceSynchronization);
    InitializeSupportedFeaturesImpl();

    DAWN_TRY_CONTEXT(
        InitializeSupportedLimitsImpl(&mLimits),
        "gathering supported limits for \"%s\" - \"%s\" (vendorId=%#06x deviceId=%#06x "
        "backend=%s type=%s)",
        mName, mDriverDescription, mVendorId, mDeviceId, mBackend, mAdapterType);

    NormalizeLimits(&mLimits.v1);

    return {};
}

InstanceBase* PhysicalDeviceBase::APIGetInstance() const {
    auto instance = GetInstance();
    ASSERT(instance != nullptr);
    instance->APIReference();
    return instance;
}

bool PhysicalDeviceBase::APIGetLimits(SupportedLimits* limits) const {
    return GetLimits(limits);
}

void PhysicalDeviceBase::APIGetProperties(AdapterProperties* properties) const {
    MaybeError result = ValidateSingleSType(properties->nextInChain,
                                            wgpu::SType::DawnAdapterPropertiesPowerPreference);
    if (result.IsError()) {
        mInstance->ConsumedError(result.AcquireError());
        return;
    }

    DawnAdapterPropertiesPowerPreference* powerPreferenceDesc = nullptr;
    FindInChain(properties->nextInChain, &powerPreferenceDesc);
    if (powerPreferenceDesc != nullptr) {
        powerPreferenceDesc->powerPreference = wgpu::PowerPreference::Undefined;
    }

    properties->vendorID = mVendorId;
    properties->vendorName = mVendorName.c_str();
    properties->architecture = mArchitectureName.c_str();
    properties->deviceID = mDeviceId;
    properties->name = mName.c_str();
    properties->driverDescription = mDriverDescription.c_str();
    properties->adapterType = mAdapterType;
    properties->backendType = mBackend;
}

bool PhysicalDeviceBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mSupportedFeatures.IsEnabled(feature);
}

size_t PhysicalDeviceBase::APIEnumerateFeatures(wgpu::FeatureName* features) const {
    return mSupportedFeatures.EnumerateFeatures(features);
}

DeviceBase* PhysicalDeviceBase::CreateDevice(AdapterBase* adapter,
                                             const DeviceDescriptor* descriptor) {
    DeviceDescriptor defaultDesc = {};
    if (descriptor == nullptr) {
        descriptor = &defaultDesc;
    }
    auto result = CreateDeviceInternal(adapter, descriptor);
    if (result.IsError()) {
        mInstance->ConsumedError(result.AcquireError());
        return nullptr;
    }
    return result.AcquireSuccess().Detach();
}

void PhysicalDeviceBase::RequestDevice(AdapterBase* adapter,
                                       const DeviceDescriptor* descriptor,
                                       WGPURequestDeviceCallback callback,
                                       void* userdata) {
    static constexpr DeviceDescriptor kDefaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &kDefaultDescriptor;
    }
    auto result = CreateDeviceInternal(adapter, descriptor);

    if (result.IsError()) {
        std::unique_ptr<ErrorData> errorData = result.AcquireError();
        // TODO(crbug.com/dawn/1122): Call callbacks only on wgpuInstanceProcessEvents
        callback(WGPURequestDeviceStatus_Error, nullptr, errorData->GetFormattedMessage().c_str(),
                 userdata);
        return;
    }

    Ref<DeviceBase> device = result.AcquireSuccess();

    WGPURequestDeviceStatus status =
        device == nullptr ? WGPURequestDeviceStatus_Unknown : WGPURequestDeviceStatus_Success;
    // TODO(crbug.com/dawn/1122): Call callbacks only on wgpuInstanceProcessEvents
    callback(status, ToAPI(device.Detach()), nullptr, userdata);
}

void PhysicalDeviceBase::InitializeVendorArchitectureImpl() {
    mVendorName = gpu_info::GetVendorName(mVendorId);
    mArchitectureName = gpu_info::GetArchitectureName(mVendorId, mDeviceId);
}

uint32_t PhysicalDeviceBase::GetVendorId() const {
    return mVendorId;
}

uint32_t PhysicalDeviceBase::GetDeviceId() const {
    return mDeviceId;
}

const gpu_info::DriverVersion& PhysicalDeviceBase::GetDriverVersion() const {
    return mDriverVersion;
}

wgpu::BackendType PhysicalDeviceBase::GetBackendType() const {
    return mBackend;
}

InstanceBase* PhysicalDeviceBase::GetInstance() const {
    return mInstance.Get();
}

FeaturesSet PhysicalDeviceBase::GetSupportedFeatures() const {
    return mSupportedFeatures;
}

bool PhysicalDeviceBase::SupportsAllRequiredFeatures(
    const ityp::span<size_t, const wgpu::FeatureName>& features) const {
    for (wgpu::FeatureName f : features) {
        if (!mSupportedFeatures.IsEnabled(f)) {
            return false;
        }
    }
    return true;
}

bool PhysicalDeviceBase::GetLimits(SupportedLimits* limits) const {
    ASSERT(limits != nullptr);
    if (limits->nextInChain != nullptr) {
        return false;
    }
    if (mUseTieredLimits) {
        limits->limits = ApplyLimitTiers(mLimits.v1);
    } else {
        limits->limits = mLimits.v1;
    }
    return true;
}

const TogglesState& PhysicalDeviceBase::GetTogglesState() const {
    return mTogglesState;
}

void PhysicalDeviceBase::EnableFeature(Feature feature) {
    mSupportedFeatures.EnableFeature(feature);
}

MaybeError PhysicalDeviceBase::ValidateFeatureSupportedWithToggles(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    DAWN_TRY(ValidateFeatureName(feature));
    DAWN_INVALID_IF(!mSupportedFeatures.IsEnabled(feature),
                    "Requested feature %s is not supported.", feature);

    const FeatureInfo* featureInfo = GetInstance()->GetFeatureInfo(feature);
    // Experimental features are guarded by the AllowUnsafeAPIs toggle.
    if (featureInfo->featureState == FeatureInfo::FeatureState::Experimental) {
        // AllowUnsafeAPIs toggle is by default disabled if not explicitly enabled.
        DAWN_INVALID_IF(toggles.IsEnabled(Toggle::DisallowUnsafeAPIs) &&
                            !toggles.IsEnabled(Toggle::AllowUnsafeAPIs),
                        "Feature %s is guarded by toggle allow_unsafe_apis.", featureInfo->name);
    }

    // Do backend-specific validation.
    return ValidateFeatureSupportedWithTogglesImpl(feature, toggles);
}

void PhysicalDeviceBase::SetSupportedFeaturesForTesting(
    const std::vector<wgpu::FeatureName>& requiredFeatures) {
    mSupportedFeatures = {};
    for (wgpu::FeatureName f : requiredFeatures) {
        mSupportedFeatures.EnableFeature(f);
    }
}

ResultOrError<Ref<DeviceBase>> PhysicalDeviceBase::CreateDeviceInternal(
    AdapterBase* adapter,
    const DeviceDescriptor* descriptor) {
    ASSERT(descriptor != nullptr);

    // Create device toggles state from required toggles descriptor and inherited adapter toggles
    // state.
    const DawnTogglesDescriptor* deviceTogglesDesc = nullptr;
    FindInChain(descriptor->nextInChain, &deviceTogglesDesc);

    // Create device toggles state.
    TogglesState deviceToggles =
        TogglesState::CreateFromTogglesDescriptor(deviceTogglesDesc, ToggleStage::Device);
    deviceToggles.InheritFrom(mTogglesState);
    // Default toggles for all backend
    deviceToggles.Default(Toggle::LazyClearResourceOnFirstUse, true);

    // Backend-specific forced and default device toggles
    SetupBackendDeviceToggles(&deviceToggles);

    // Validate all required features are supported by the adapter and suitable under given toggles.
    // Note that certain toggles in device toggles state may be overriden by user and different from
    // the adapter toggles state.
    // TODO(dawn:1495): After implementing adapter toggles, decide whether we should validate
    // supported features using adapter toggles or device toggles.
    for (uint32_t i = 0; i < descriptor->requiredFeaturesCount; ++i) {
        wgpu::FeatureName feature = descriptor->requiredFeatures[i];
        DAWN_TRY(ValidateFeatureSupportedWithToggles(feature, deviceToggles));
    }

    if (descriptor->requiredLimits != nullptr) {
        DAWN_TRY_CONTEXT(ValidateLimits(mUseTieredLimits ? ApplyLimitTiers(mLimits.v1) : mLimits.v1,
                                        descriptor->requiredLimits->limits),
                         "validating required limits");

        DAWN_INVALID_IF(descriptor->requiredLimits->nextInChain != nullptr,
                        "nextInChain is not nullptr.");
    }
    return CreateDeviceImpl(adapter, descriptor, deviceToggles);
}

void PhysicalDeviceBase::SetUseTieredLimits(bool useTieredLimits) {
    mUseTieredLimits = useTieredLimits;
}

void PhysicalDeviceBase::ResetInternalDeviceForTesting() {
    mInstance->ConsumedError(ResetInternalDeviceForTestingImpl());
}

MaybeError PhysicalDeviceBase::ResetInternalDeviceForTestingImpl() {
    return DAWN_INTERNAL_ERROR(
        "ResetInternalDeviceForTesting should only be used with the D3D12 backend.");
}

}  // namespace dawn::native
