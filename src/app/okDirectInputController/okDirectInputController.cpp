#include "okDirectInputController.h"

#include <Rpc.h>
#include <dinput.h>
#include <winrt/base.h>

#include "GetDirectInputDevices.h"
#include "okDirectInputController_DIBinding.h"
#include "okDirectInputController_DIButtonEvent.h"
#include "okDirectInputController_DIButtonListener.h"
#include "okDirectInputController_DIThread.h"
#include "okDirectInputController_SettingsUI.h"
#include "okDirectInputController_SharedState.h"
#include "okEvents.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Rpcrt4.lib")

using namespace OpenKneeboard;

namespace {
struct JSONBinding final {
  std::string Device;
  uint8_t ButtonIndex;
  std::string Action;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JSONBinding, Device, ButtonIndex, Action);

struct JSONDevice {
  std::string InstanceName;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JSONDevice, InstanceName);

struct JSONSettings {
  std::map<std::string, JSONDevice> Devices;
  std::vector<JSONBinding> Bindings;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JSONSettings, Devices, Bindings);
}// namespace

struct okDirectInputController::State : public SharedState {
  std::unique_ptr<DIThread> DirectInputThread;
};

okDirectInputController::okDirectInputController(
  const nlohmann::json& jsonSettings)
  : p(std::make_shared<State>()) {
  DirectInput8Create(
    GetModuleHandle(nullptr),
    DIRECTINPUT_VERSION,
    IID_IDirectInput8,
    p->DI8.put_void(),
    NULL);

  p->DirectInputThread = std::make_unique<DIThread>(this, p->DI8);
  p->DirectInputThread->Run();

  this->Bind(okEVT_DI_BUTTON, &okDirectInputController::OnDIButtonEvent, this);

  JSONSettings settings;
  try {
    jsonSettings.get_to(settings);
  } catch (nlohmann::json::exception&) {
    return;
  }

  for (const auto& binding: settings.Bindings) {
    const auto& deviceName = settings.Devices.find(binding.Device);
    if (deviceName == settings.Devices.end()) {
      continue;
    }

    std::optional<wxEventTypeTag<wxCommandEvent>> eventType;
#define ACTION(x) \
  if (binding.Action == #x) { \
    eventType = okEVT_##x; \
  }
    ACTION(PREVIOUS_TAB);
    ACTION(NEXT_TAB);
    ACTION(PREVIOUS_PAGE);
    ACTION(NEXT_PAGE);
#undef ACTION
    if (!eventType) {
      continue;
    }

    UUID uuid;
    UuidFromStringA(
      reinterpret_cast<RPC_CSTR>(const_cast<char*>(binding.Device.c_str())),
      &uuid);
    p->Bindings.push_back({
      .InstanceGuid = uuid,
      .InstanceName = deviceName->second.InstanceName,
      .ButtonIndex = binding.ButtonIndex,
      .EventType = *eventType,
    });
  }
}

okDirectInputController::~okDirectInputController() {
  p->DirectInputThread->Wait();
}

void okDirectInputController::OnDIButtonEvent(const wxThreadEvent& ev) {
  if (p->Hook) {
    wxQueueEvent(p->Hook, ev.Clone());
    return;
  }

  auto be = ev.GetPayload<DIButtonEvent>();
  if (!be.Pressed) {
    // We act on keydown
    return;
  }

  for (auto& it: p->Bindings) {
    if (it.InstanceGuid != be.Instance.guidInstance) {
      continue;
    }
    if (it.ButtonIndex != be.ButtonIndex) {
      continue;
    }
    wxQueueEvent(this, new wxCommandEvent(it.EventType));
  }
}

wxWindow* okDirectInputController::GetSettingsUI(wxWindow* parent) {
  auto ret
    = new okDirectInputController::SettingsUI(parent, p);
  ret->Bind(okEVT_SETTINGS_CHANGED, [this](auto& ev) {
    wxQueueEvent(this, ev.Clone());
  });
  return ret;
}

nlohmann::json okDirectInputController::GetSettings() const {
  JSONSettings settings;

  for (const auto& binding: p->Bindings) {
    RPC_CSTR rpcUuid;
    UuidToStringA(&binding.InstanceGuid, &rpcUuid);
    const std::string uuid(reinterpret_cast<char*>(rpcUuid));
    RpcStringFreeA(&rpcUuid);

    settings.Devices[uuid] = {binding.InstanceName};
    std::string action;
#define ACTION(x) \
  if (binding.EventType == okEVT_##x) { \
    action = #x; \
  }
    ACTION(PREVIOUS_TAB);
    ACTION(NEXT_TAB);
    ACTION(PREVIOUS_PAGE);
    ACTION(NEXT_PAGE);
#undef ACTION
    if (action.empty()) {
      continue;
    }

    settings.Bindings.push_back({
      .Device = uuid,
      .ButtonIndex = binding.ButtonIndex,
      .Action = action,
    });
  }
  return settings;
}