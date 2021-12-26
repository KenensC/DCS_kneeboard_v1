#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/image.h>

#include <memory>

namespace OpenKneeboard {
class Tab;
}

class okTabCanvas final : public wxPanel {
 public:
  okTabCanvas(wxWindow* parent, const std::shared_ptr<OpenKneeboard::Tab>&);
  virtual ~okTabCanvas();

  void OnPaint(wxPaintEvent& ev);
  void OnEraseBackground(wxEraseEvent& ev);

  uint16_t GetPageIndex() const;
  void SetPageIndex(uint16_t index);
  void NextPage();
  void PreviousPage();
  std::shared_ptr<OpenKneeboard::Tab> GetTab() const;

  wxDECLARE_EVENT_TABLE();

 private:
  class Impl;
  std::shared_ptr<Impl> p;
};