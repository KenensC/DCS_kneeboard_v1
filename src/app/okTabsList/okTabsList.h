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

#include <OpenKneeboard/DXResources.h>
#include <OpenKneeboard/Tab.h>
#include <shims/winrt.h>

#include <nlohmann/json_fwd.hpp>

namespace OpenKneeboard {
class KneeboardState;
}

class okTabsList final {
 public:
  okTabsList() = delete;
  okTabsList(
    const OpenKneeboard::DXResources&,
    const std::shared_ptr<OpenKneeboard::KneeboardState>& kneeboard,
    const nlohmann::json& config);

  virtual ~okTabsList();
  virtual nlohmann::json GetSettings() const;

 private:
  class SettingsUI;
  OpenKneeboard::DXResources mDXR;
  std::shared_ptr<OpenKneeboard::KneeboardState> mKneeboard;

  void LoadConfig(const nlohmann::json&);
  void LoadDefaultConfig();
};
