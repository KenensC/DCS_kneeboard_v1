/*
 * OpenKneeboard
 *
 * Copyright (C) 2022 Fred Emmott <fred@fredemmott.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#pragma once

#include <shims/winrt.h>
#include <memory>

#include "okConfigurableComponent.h"

struct IDirectInput8W;

namespace OpenKneeboard {
enum class UserAction {
  PREVIOUS_TAB,
  NEXT_TAB,
  PREVIOUS_PAGE,
  NEXT_PAGE,
  TOGGLE_VISIBILITY
};
}

// Using DirectInput instead of wxJoystick so we can use
// all 128 buttons, rather than just the first 32
class okDirectInputController final : public okConfigurableComponent {
 public:
  struct DIBinding;
  struct DIButtonEvent;
  class DIButtonListener;

  okDirectInputController() = delete;
  okDirectInputController(const nlohmann::json& settings);
  virtual ~okDirectInputController();

  virtual wxWindow* GetSettingsUI(wxWindow* parent) override;
  virtual nlohmann::json GetSettings() const override;

  OpenKneeboard::Event<OpenKneeboard::UserAction> evUserAction;

  IDirectInput8W* GetDirectInput() const;
  std::vector<DIBinding> GetBindings() const;
  void SetBindings(const std::vector<DIBinding>&);

  void SetHook(std::function<void(const DIButtonEvent&)>);

 private:

  winrt::com_ptr<IDirectInput8W> mDI8;
  std::vector<DIBinding> mBindings;
  std::function<void(const DIButtonEvent&)> mHook;

  std::jthread mDIThread;

  void OnDIButtonEvent(const DIButtonEvent&);
};
