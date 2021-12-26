#include "okTabCanvas.h"

#include <wx/dcbuffer.h>

#include "OpenKneeboard/Tab.h"

class okTabCanvas::Impl final {
 public:
  std::shared_ptr<OpenKneeboard::Tab> Tab;
  uint16_t PageIndex = 0;
};

okTabCanvas::okTabCanvas(
  wxWindow* parent,
  const std::shared_ptr<OpenKneeboard::Tab>& tab)
  : wxPanel(
    parent,
    wxID_ANY,
    wxDefaultPosition,
    wxSize(OPENKNEEBOARD_TEXTURE_WIDTH / 2, OPENKNEEBOARD_TEXTURE_HEIGHT / 2)),
    p(new Impl {.Tab = tab}) {
}

okTabCanvas::~okTabCanvas() {
}

void okTabCanvas::OnPaint(wxPaintEvent& ev) {
  const auto count = p->Tab->GetPageCount();
  if (count == 0) {
    // TODO: render error thing;
    return;
  }
  p->PageIndex = std::clamp(p->PageIndex, 0ui16, count);

  auto image = p->Tab->RenderPage(p->PageIndex);
  if (!image.IsOk()) {
    return;
  }

  const auto imageSize = image.GetSize();
  const auto clientSize = this->GetSize();
  const float xScale = (float)clientSize.GetWidth() / imageSize.GetWidth();
  const float yScale = (float)clientSize.GetHeight() / imageSize.GetHeight();
  const auto scale = std::min(xScale, yScale);
  const auto scaled = image.Scale(
    (int)imageSize.GetWidth() * scale, (int)imageSize.GetHeight() * scale);

  wxBufferedPaintDC dc(this);
  dc.Clear();
  dc.DrawBitmap(scaled, wxPoint {0, 0});
}

void okTabCanvas::OnEraseBackground(wxEraseEvent& ev) {
  // intentionally empty
}

uint16_t okTabCanvas::GetPageIndex() const {
  return p->PageIndex;
}

void okTabCanvas::SetPageIndex(uint16_t index) {
  const auto count = p->Tab->GetPageCount();
  if (count == 0) {
    return;
  }
  p->PageIndex = std::clamp(index, 0ui16, count);
  Refresh();
}

void okTabCanvas::NextPage() {
  SetPageIndex(GetPageIndex() + 1);
}

void okTabCanvas::PreviousPage() {
  const auto current = GetPageIndex();
  if (current == 0) {
    return;
  }
  SetPageIndex(current - 1);
}

std::shared_ptr<OpenKneeboard::Tab> okTabCanvas::GetTab() const {
  return p->Tab;
}

// clang-format off
wxBEGIN_EVENT_TABLE(okTabCanvas, wxPanel)
  EVT_PAINT(okTabCanvas::OnPaint)
  EVT_ERASE_BACKGROUND(okTabCanvas::OnEraseBackground)
wxEND_EVENT_TABLE();
// clang-format on