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

#include <dxgi1_2.h>
#include <shims/winrt.h>
#include <shims/wx.h>

#include <memory>

#include <OpenKneeboard/Events.h>

namespace OpenKneeboard {
struct CursorEvent;
struct DXResources;
class KneeboardState;
class TabState;
}// namespace OpenKneeboard

class okTab final : public wxPanel, private OpenKneeboard::EventReceiver {
 public:
  okTab(
    wxWindow* parent,
    const OpenKneeboard::DXResources&,
    const std::shared_ptr<OpenKneeboard::KneeboardState>&,
    const std::shared_ptr<OpenKneeboard::TabState>&);
  virtual ~okTab();

 private:
  std::shared_ptr<OpenKneeboard::TabState> mState;

  void UpdateButtonStates();

  void InitUI(
    const OpenKneeboard::DXResources&,
    const std::shared_ptr<OpenKneeboard::KneeboardState>&);
  wxButton* mNormalModeButton = nullptr;
  wxButton* mNavigationModeButton = nullptr;
  wxButton* mFirstPageButton = nullptr;
  wxButton* mPreviousPageButton = nullptr;
  wxButton* mNextPageButton = nullptr;
};