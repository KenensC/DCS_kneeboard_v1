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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class wxListView;

wxDECLARE_EVENT(okEVT_PATH_SELECTED, wxCommandEvent);

class okSelectExecutableDialog : public wxDialog {
 private:
  wxListView* mList = nullptr;

  void OnBrowseButton(wxCommandEvent&);
  void OnChooseSelectedProcess(wxCommandEvent&);

 public:
  okSelectExecutableDialog(
    wxWindow* parent,
    wxWindowID id,
    const wxString& title);
};
