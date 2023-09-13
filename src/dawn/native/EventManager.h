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

#ifndef SRC_DAWN_NATIVE_EVENTMANAGER_H_
#define SRC_DAWN_NATIVE_EVENTMANAGER_H_

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SystemEvent.h"

namespace dawn::native {

struct InstanceDescriptor;

// Subcomponent of the Instance which tracks callback events for the Future-based callback
// entrypoints. All events from this instance (regardless of whether from an adapter, device, queue,
// etc.) are tracked here, and used by the instance-wide ProcessEvents and WaitAny entrypoints.
//
// TODO(crbug.com/dawn/1987): Can this eventually replace CallbackTaskManager?
// TODO(crbug.com/dawn/1987): There are various ways to optimize ProcessEvents/WaitAny:
// - Only pay attention to the earliest serial on each queue.
// - Spontaneously set events as "early-ready" in other places when we see serials advance, e.g.
//   Submit, or when checking a later wait before an earlier wait.
// - For thread-driven events (async pipeline compilation and Metal queue events), defer tracking
//   for ProcessEvents until the event is already completed.
// - Avoid creating OS events until they're actually needed (see the todo in TrackedEvent).
class EventManager final : NonMovable {
  public:
    EventManager();
    ~EventManager();

    MaybeError Initialize(const InstanceDescriptor*);
    // Called by WillDropLastExternalRef. Once shut down, the EventManager stops tracking anything.
    // It drops any refs to TrackedEvents, to break reference cycles. If doing so frees the last ref
    // of any uncompleted TrackedEvents, they'll get completed with EventCompletionType::Shutdown.
    void ShutDown();

    class TrackedEvent;
    // Track a TrackedEvent and give it a FutureID.
    [[nodiscard]] FutureID TrackEvent(WGPUCallbackModeFlags mode, Ref<TrackedEvent>&&);
    void ProcessPollEvents();
    [[nodiscard]] wgpu::WaitStatus WaitAny(size_t count,
                                           FutureWaitInfo* infos,
                                           Nanoseconds timeout);

  private:
    struct Trackers : dawn::NonMovable {
        // Tracks Futures (used by WaitAny).
        MutexProtected<std::unordered_map<FutureID, Ref<TrackedEvent>>> futures;
        // Tracks events polled by ProcessEvents.
        MutexProtected<std::unordered_map<FutureID, Ref<TrackedEvent>>> pollEvents;
    };

    bool mTimedWaitAnyEnable = false;
    size_t mTimedWaitAnyMaxCount = kTimedWaitAnyMaxCountDefault;
    std::atomic<FutureID> mNextFutureID = 1;

    // Freed once the user has dropped their last ref to the Instance, so can't call WaitAny or
    // ProcessEvents anymore. This breaks reference cycles.
    std::optional<Trackers> mTrackers;
};

// Base class for the objects that back WGPUFutures. TrackedEvent is responsible for the lifetime
// the callback it contains. If TrackedEvent gets destroyed before it completes, it's responsible
// for cleaning up (by calling the callback with an "Unknown" status).
//
// For Future-based and ProcessEvents-based TrackedEvents, the EventManager will track them for
// completion in WaitAny or ProcessEvents. However, once the Instance has lost all its external
// refs, the user can't call either of those methods anymore, so EventManager will stop holding refs
// to any TrackedEvents. Any which are not ref'd elsewhere (in order to be `Spontaneous`ly
// completed) will be cleaned up at that time.
class EventManager::TrackedEvent : public RefCounted {
  protected:
    // Note: TrackedEvents are (currently) only for Device events. Events like RequestAdapter and
    // RequestDevice complete immediately in dawn native, so should never need to be tracked.
    TrackedEvent(DeviceBase* device,
                 WGPUCallbackModeFlags callbackMode,
                 SystemEventReceiver&& receiver);

  public:
    // Subclasses must implement this to complete the event (if not completed) with
    // EventCompletionType::Shutdown.
    ~TrackedEvent() override;

    class WaitRef;

    const SystemEventReceiver& GetReceiver() const;
    DeviceBase* GetWaitDevice() const;

  protected:
    void EnsureComplete(EventCompletionType);
    void CompleteIfSpontaneous();

    // True if the event can only be waited using its device (e.g. with vkWaitForFences).
    // False if it can be waited using OS-level wait primitives (WaitAnySystemEvent).
    virtual bool MustWaitUsingDevice() const = 0;
    virtual void Complete(EventCompletionType) = 0;

    // This creates a temporary ref cycle (Device->Instance->EventManager->TrackedEvent).
    // This is OK because the instance will clear out the EventManager on shutdown.
    // TODO(crbug.com/dawn/1987): This is a bit fragile. Is it possible to remove the ref cycle?
    Ref<DeviceBase> mDevice;
    WGPUCallbackModeFlags mCallbackMode;

#if DAWN_ENABLE_ASSERTS
    std::atomic<bool> mCurrentlyBeingWaited;
#endif

  private:
    friend class EventManager;

    // TODO(crbug.com/dawn/1987): Optimize by creating an SystemEventReceiver only once actually
    // needed (the user asks for a timed wait or an OS event handle). This should be generally
    // achievable:
    // - For thread-driven events (async pipeline compilation and Metal queue events), use a mutex
    //   or atomics to atomically:
    //   - On wait: { check if mKnownReady. if not, create the SystemEventPipe }
    //   - On signal: { check if there's an SystemEventPipe. if not, set mKnownReady }
    // - For D3D12/Vulkan fences, on timed waits, first use GetCompletedValue/GetFenceStatus, then
    //   create an OS event if it's not ready yet (and we don't have one yet).
    //
    // This abstraction should probably be hidden from TrackedEvent - previous attempts to do
    // something similar in TrackedEvent turned out to be quite confusing. It can instead be an
    // "optimization" to the SystemEvent* or a layer between TrackedEvent and SystemEventReceiver.
    SystemEventReceiver mReceiver;
    // Callback has been called.
    std::atomic<bool> mCompleted = false;
};

// A Ref<TrackedEvent>, but ASSERTing that a future isn't used concurrently in multiple
// WaitAny/ProcessEvents call (by checking that there's never more than one WaitRef for a
// TrackedEvent). For WaitAny, this checks the embedder's behavior, but for ProcessEvents this is
// only an internal ASSERT (it's supposed to be synchronized so that this never happens).
class EventManager::TrackedEvent::WaitRef : dawn::NonCopyable {
  public:
    WaitRef(WaitRef&& rhs) = default;
    WaitRef& operator=(WaitRef&& rhs) = default;

    explicit WaitRef(TrackedEvent* future);
    ~WaitRef();

    TrackedEvent* operator->();
    const TrackedEvent* operator->() const;

  private:
    Ref<TrackedEvent> mRef;
};

// TrackedEvent::WaitRef plus a few extra fields needed for some implementations.
// Sometimes they'll be unused, but that's OK; it simplifies code reuse.
struct TrackedFutureWaitInfo {
    FutureID futureID;
    EventManager::TrackedEvent::WaitRef event;
    // Used by EventManager::ProcessPollEvents
    size_t indexInInfos;
    // Used by EventManager::ProcessPollEvents and ::WaitAny
    bool ready;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EVENTMANAGER_H_
