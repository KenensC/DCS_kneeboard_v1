#include "okDirectInputPageController.h"

#include <fmt/format.h>
#include <winrt/base.h>
#include <wx/gbsizer.h>
#include <wx/scopeguard.h>

#include "okEvents.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace {
winrt::com_ptr<IDirectInput8> GetDI8() {
  static winrt::com_ptr<IDirectInput8> inst;
  if (!inst) {
    DirectInput8Create(
      GetModuleHandle(NULL),
      DIRECTINPUT_VERSION,
      IID_IDirectInput8,
      inst.put_void(),
      NULL);
  }
  return inst;
}

using DeviceInstances = std::vector<DIDEVICEINSTANCE>;

BOOL CALLBACK EnumDeviceCallback(LPCDIDEVICEINSTANCE inst, LPVOID ctx) {
  auto& devices = *reinterpret_cast<DeviceInstances*>(ctx);
  devices.push_back(*inst);
  return DIENUM_CONTINUE;
}

DeviceInstances EnumDevices() {
  auto di = GetDI8();
  DeviceInstances ret;
  di->EnumDevices(
    DI8DEVCLASS_GAMECTRL, &EnumDeviceCallback, &ret, DIEDFL_ATTACHEDONLY);
  return ret;
}

struct DIButtonEvent {
  bool Valid = false;
  DIDEVICEINSTANCE Instance;
  uint8_t ButtonIndex;
  bool Pressed;

  operator bool() const {
    return Valid;
  }
};

class DIButtonListener final : public wxEvtHandler {
 private:
  struct DeviceInfo {
    DIDEVICEINSTANCE Instance;
    winrt::com_ptr<IDirectInputDevice8> Device;
    DIJOYSTATE2 State = {};
    HANDLE EventHandle = INVALID_HANDLE_VALUE;
  };
  std::vector<DeviceInfo> mDevices;
  HANDLE mCancelHandle = INVALID_HANDLE_VALUE;

 public:
  DIButtonListener(const DeviceInstances& instances) {
    auto di = GetDI8();
    for (const auto& it: instances) {
      winrt::com_ptr<IDirectInputDevice8> device;
      di->CreateDevice(it.guidInstance, device.put(), NULL);
      if (!device) {
        continue;
      }

      HANDLE event {CreateEvent(nullptr, false, false, nullptr)};
      if (!event) {
        continue;
      }

      device->SetEventNotification(event);
      device->SetDataFormat(&c_dfDIJoystick2);
      device->SetCooperativeLevel(
        static_cast<wxApp*>(wxApp::GetInstance())->GetTopWindow()->GetHandle(),
        DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
      device->Acquire();

      DIJOYSTATE2 state;
      device->GetDeviceState(sizeof(state), &state);

      mDevices.push_back({it, device, state, event});
    }
    mCancelHandle = CreateEvent(nullptr, false, false, nullptr);
  }

  ~DIButtonListener() {
    CloseHandle(mCancelHandle);
  }

  void Cancel() {
    SetEvent(mCancelHandle);
  }

  DIButtonEvent Poll() {
    std::vector<HANDLE> handles;
    for (const auto& it: mDevices) {
      handles.push_back(it.EventHandle);
    }
    handles.push_back(mCancelHandle);

    auto result
      = WaitForMultipleObjects(handles.size(), handles.data(), false, 100);
    auto idx = result - WAIT_OBJECT_0;
    if (idx <= 0 || idx >= (handles.size() - 1)) {
      return {};
    }

    auto& device = mDevices.at(idx);
    auto oldState = device.State;
    DIJOYSTATE2 newState;
    device.Device->Poll();
    device.Device->GetDeviceState(sizeof(newState), &newState);
    for (uint8_t i = 0; i < 128; ++i) {
      if (oldState.rgbButtons[i] != newState.rgbButtons[i]) {
        device.State = newState;
        return {
          true,
          device.Instance,
          static_cast<uint8_t>(i),
          (bool)(newState.rgbButtons[i] & (1 << 7))};
      }
    }
    return {};
  }
};

struct DIInputBinding {
  DIDEVICEINSTANCE Instance;
  uint8_t ButtonIndex;
  wxEventTypeTag<wxCommandEvent> EventType;
};

struct DIInputBindings {
  std::vector<DIInputBinding> Bindings;
  wxEvtHandler* Hook = nullptr;
};

}// namespace

wxDEFINE_EVENT(okEVT_DI_BUTTON_EVENT, wxThreadEvent);
class okDirectInputThread final : public wxThread {
 private:
  wxEvtHandler* mReceiver;

 public:
  okDirectInputThread(wxEvtHandler* receiver)
    : wxThread(wxTHREAD_JOINABLE), mReceiver(receiver) {
  }

 protected:
  virtual ExitCode Entry() override {
    auto listener = DIButtonListener(EnumDevices());
    while (!this->TestDestroy()) {
      DIButtonEvent bi = listener.Poll();
      if (!bi) {
        continue;
      }
      wxThreadEvent ev(okEVT_DI_BUTTON_EVENT);
      ev.SetPayload(bi);
      wxQueueEvent(mReceiver, ev.Clone());
    }

    return ExitCode(0);
  }
};

wxDEFINE_EVENT(okEVT_DI_CLEAR_BINDING_EVENT, wxCommandEvent);

class okDirectInputPageSettings final : public wxPanel {
 private:
  DeviceInstances mDevices;
  std::shared_ptr<DIInputBindings> mBindings;
  struct DeviceButtons {
    wxButton* PreviousTab = nullptr;
    wxButton* NextTab = nullptr;
    wxButton* PreviousPage = nullptr;
    wxButton* NextPage = nullptr;
    wxButton* Get(const wxEventTypeTag<wxCommandEvent>& evt) {
      if (evt == okEVT_PREVIOUS_TAB) {
        return PreviousTab;
      }
      if (evt == okEVT_NEXT_TAB) {
        return NextTab;
      }
      if (evt == okEVT_PREVIOUS_PAGE) {
        return PreviousPage;
      }
      if (evt == okEVT_NEXT_PAGE) {
        return NextPage;
      }
      return nullptr;
    }
  };
  std::vector<DeviceButtons> mDeviceButtons;

 public:
  okDirectInputPageSettings(
    wxWindow* parent,
    const std::shared_ptr<DIInputBindings>& bindings)
    : wxPanel(parent, wxID_ANY), mBindings(bindings) {
    mDevices = EnumDevices();

    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto panel = new wxPanel(this, wxID_ANY);
    sizer->Add(panel, 0, wxEXPAND);

    auto grid = new wxGridBagSizer(5, 5);
    grid->AddGrowableCol(0);

    auto bold = GetFont().MakeBold();
    for (const auto& column:
         {_("Device"),
          _("Previous Tab"),
          _("Next Tab"),
          _("Previous Page"),
          _("Next Page")}) {
      auto label = new wxStaticText(panel, wxID_ANY, column);
      label->SetFont(bold);
      grid->Add(label);
    }

    for (int i = 0; i < mDevices.size(); ++i) {
      const auto& device = mDevices.at(i);
      const auto row = i + 1;// headers

      auto label = new wxStaticText(panel, wxID_ANY, device.tszInstanceName);
      grid->Add(label, wxGBPosition(row, 0));

      auto previousTab = new wxButton(panel, wxID_ANY, _("Bind"));
      grid->Add(previousTab, wxGBPosition(row, 1));
      previousTab->Bind(wxEVT_BUTTON, [=](auto& ev) {
        this->OnBind(ev, i, okEVT_PREVIOUS_TAB);
      });

      auto nextTab = new wxButton(panel, wxID_ANY, _("Bind"));
      grid->Add(nextTab, wxGBPosition(row, 2));
      nextTab->Bind(
        wxEVT_BUTTON, [=](auto& ev) { this->OnBind(ev, i, okEVT_NEXT_TAB); });

      auto previousPage = new wxButton(panel, wxID_ANY, _("Bind"));
      grid->Add(previousPage, wxGBPosition(row, 3));
      previousPage->Bind(wxEVT_BUTTON, [=](auto& ev) {
        this->OnBind(ev, i, okEVT_PREVIOUS_PAGE);
      });

      auto nextPage = new wxButton(panel, wxID_ANY, _("Bind"));
      grid->Add(nextPage, wxGBPosition(row, 4));
      nextPage->Bind(
        wxEVT_BUTTON, [=](auto& ev) { this->OnBind(ev, i, okEVT_NEXT_PAGE); });

      mDeviceButtons.push_back({
        .PreviousTab = previousTab,
        .NextTab = nextTab,
        .PreviousPage = previousPage,
        .NextPage = nextPage,
      });
    }
    grid->SetCols(5);
    panel->SetSizerAndFit(grid);

    sizer->AddStretchSpacer();
    this->SetSizerAndFit(sizer);
    Refresh();
  }

 private:
  wxDialog* CreateBindInputDialog(bool haveExistingBinding) {
    auto d = new wxDialog(this, wxID_ANY, _("Bind Inputs"));

    auto s = new wxBoxSizer(wxVERTICAL);

    s->Add(
      new wxStaticText(d, wxID_ANY, _("Press button to bind input...")),
      0,
      wxALL,
      5);
    auto bs = d->CreateButtonSizer(wxCANCEL | wxNO_DEFAULT);
    s->Add(bs, 0, wxALL, 5);

    if (haveExistingBinding) {
      auto clear = new wxButton(d, wxID_ANY, _("Clear"));
      bs->Add(clear);

      clear->Bind(wxEVT_BUTTON, [=](auto&) {
        d->Close();
        wxQueueEvent(d, new wxCommandEvent(okEVT_DI_CLEAR_BINDING_EVENT));
      });
    }

    d->SetSizerAndFit(s);

    return d;
  }

  void OnBind(
    wxCommandEvent& ev,
    int deviceIndex,
    const wxEventTypeTag<wxCommandEvent>& eventType) {
    auto button = dynamic_cast<wxButton*>(ev.GetEventObject());
    auto& device = mDevices.at(deviceIndex);

    auto& bindings = this->mBindings->Bindings;

    // Used to implement a clear button
    auto currentBinding = bindings.end();;
    for (auto it = bindings.begin(); it != bindings.end(); ++it) {
      if (it->Instance.guidInstance != device.guidInstance) {
        continue;
      }
      if (it->EventType != eventType) {
        continue;
      }
      currentBinding = it;
      break;
    }
    auto d = CreateBindInputDialog(currentBinding != bindings.end());

    wxON_BLOCK_EXIT_SET(mBindings->Hook, nullptr);
    mBindings->Hook = this;

    auto f = [=, &bindings](wxThreadEvent& ev) {
      auto be = ev.GetPayload<DIButtonEvent>();
      if (be.Instance.guidInstance != device.guidInstance) {
        return;
      }

      // Not using `currentBinding` as we need to clear both the
      // current binding, and any other conflicting bindings
      for (auto it = bindings.begin(); it != bindings.end();) {
        if (it->Instance.guidInstance != device.guidInstance) {
          ++it;
          continue;
        }

        if (it->ButtonIndex != be.ButtonIndex && it->EventType != eventType) {
          ++it;
          continue;
        }

        auto button = this->mDeviceButtons.at(deviceIndex).Get(it->EventType);
        if (button) {
          button->SetLabel(_("Bind"));
        }
        it = bindings.erase(it);
      }

      button->SetLabel(
        fmt::format(_("Button {:d}").ToStdString(), be.ButtonIndex + 1));
      bindings.push_back(
        {.Instance = device,
         .ButtonIndex = be.ButtonIndex,
         .EventType = eventType});
      d->Close();
    };

    this->Bind(okEVT_DI_BUTTON_EVENT, f);
    d->Bind(okEVT_DI_CLEAR_BINDING_EVENT, [=,&bindings](auto&) {
      bindings.erase(currentBinding);
      auto button = this->mDeviceButtons.at(deviceIndex).Get(eventType);
      if (button) {
        button->SetLabel(_("Bind"));
      }
    });
    d->ShowModal();
    this->Unbind(okEVT_DI_BUTTON_EVENT, f);
  }
};

class okDirectInputPageController::Impl final {
 public:
  std::shared_ptr<DIInputBindings> Bindings;
  std::unique_ptr<okDirectInputThread> DirectInputThread;
};

okDirectInputPageController::okDirectInputPageController()
  : p(std::make_shared<Impl>()) {
  p->Bindings = std::make_shared<DIInputBindings>();
  p->DirectInputThread = std::make_unique<okDirectInputThread>(this);
  p->DirectInputThread->Run();
  this->Bind(
    okEVT_DI_BUTTON_EVENT, &okDirectInputPageController::OnDIButtonEvent, this);
}

okDirectInputPageController::~okDirectInputPageController() {
  p->DirectInputThread->Wait();
}

void okDirectInputPageController::OnDIButtonEvent(const wxThreadEvent& ev) {
  if (p->Bindings->Hook) {
    wxQueueEvent(p->Bindings->Hook, ev.Clone());
    return;
  }

  auto be = ev.GetPayload<DIButtonEvent>();
  if (!be.Pressed) {
    // We act on keydown
    return;
  }

  for (auto& it: p->Bindings->Bindings) {
    if (it.Instance.guidInstance != be.Instance.guidInstance) {
      continue;
    }
    if (it.ButtonIndex != be.ButtonIndex) {
      continue;
    }
    wxQueueEvent(this, new wxCommandEvent(it.EventType));
  }
}

wxString okDirectInputPageController::GetTitle() const {
  return _("DirectInput");
}

wxWindow* okDirectInputPageController::GetSettingsUI(wxWindow* parent) {
  return new okDirectInputPageSettings(parent, p->Bindings);
}