#include "okMainWindow.h"

#include <wx/frame.h>
#include <wx/notebook.h>

#include "OpenKneeboard/FolderTab.h"
#include "OpenKneeboard/GameEvent.h"
#include "OpenKneeboard/RenderError.h"
#include "OpenKneeboard/SHM.h"
#include "OpenKneeboard/dprint.h"
#include "Settings.h"
#include "okDirectInputController.h"
#include "okEvents.h"
#include "okGameEventMailslotThread.h"
#include "okGamesList.h"
#include "okTab.h"
#include "okTabsList.h"

using namespace OpenKneeboard;

class okMainWindow::Impl {
 public:
  std::vector<okConfigurableComponent*> Configurables;
  std::vector<okTab*> Tabs;
  OpenKneeboard::SHM::Writer SHM;
  wxNotebook* Notebook;
  int CurrentTab = 0;
  Settings Settings = ::Settings::Load();
};

static wxImage CreateErrorImage(const wxString& text) {
	wxBitmap bm(768, 1024);
	wxMemoryDC dc(bm);

	dc.SetBrush(*wxWHITE_BRUSH);
	dc.Clear();
	RenderError(bm.GetSize(), dc, text);

	return bm.ConvertToImage();
}

okMainWindow::okMainWindow()
  : wxFrame(
    nullptr,
    wxID_ANY,
    "OpenKneeboard",
    wxDefaultPosition,
    wxDefaultSize,
    wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER),
    p(std::make_unique<Impl>()) {
  this->Bind(okEVT_GAME_EVENT, &okMainWindow::OnGameEvent, this);
  auto menuBar = new wxMenuBar();
  {
    auto fileMenu = new wxMenu();
    menuBar->Append(fileMenu, _("&File"));

    fileMenu->Append(wxID_EXIT, _("E&xit"));
    Bind(wxEVT_MENU, &okMainWindow::OnExit, this, wxID_EXIT);
  }
  {
    auto editMenu = new wxMenu();
    menuBar->Append(editMenu, _("&Edit"));

    auto settingsId = wxNewId();
    editMenu->Append(settingsId, _("&Settings..."));
    Bind(wxEVT_MENU, &okMainWindow::OnShowSettings, this, settingsId);
  }
  SetMenuBar(menuBar);

  auto sizer = new wxBoxSizer(wxVERTICAL);

  p->Notebook = new wxNotebook(this, wxID_ANY);
  p->Notebook->Bind(
    wxEVT_BOOKCTRL_PAGE_CHANGED, &okMainWindow::OnTabChanged, this);
  sizer->Add(p->Notebook);

  auto listener = new okGameEventMailslotThread(this);
  listener->Run();

  {
    // TODO: settings
    auto tabs = new okTabsList(nlohmann::json {});
    for (const auto& tab: tabs->GetTabs()) {
      auto ui = new okTab(p->Notebook, tab);
      p->Notebook->AddPage(ui, tab->GetTitle());
      ui->Bind(okEVT_TAB_UPDATED, [this](auto) { this->UpdateSHM(); });
      ui->Bind(okEVT_PAGE_CHANGED, [this](auto) { this->UpdateSHM(); });
      p->Tabs.push_back(ui);
    }
  }

  this->SetSizerAndFit(sizer);

  UpdateSHM();

  {
    auto gl = new okGamesList(p->Settings.Games);
    gl->Bind(okEVT_SETTINGS_CHANGED, [=](auto&) {
      this->p->Settings.Games = gl->GetSettings();
      p->Settings.Save();
    });
    p->Configurables.push_back(gl);
  }

  {
    auto dipc = new okDirectInputController(p->Settings.DirectInput);
    p->Configurables.push_back(dipc);

    dipc->Bind(okEVT_PREVIOUS_TAB, &okMainWindow::OnPreviousTab, this);
    dipc->Bind(okEVT_NEXT_TAB, &okMainWindow::OnNextTab, this);
    dipc->Bind(okEVT_PREVIOUS_PAGE, &okMainWindow::OnPreviousPage, this);
    dipc->Bind(okEVT_NEXT_PAGE, &okMainWindow::OnNextPage, this);

    dipc->Bind(okEVT_SETTINGS_CHANGED, [=](auto&) {
      this->p->Settings.DirectInput = dipc->GetSettings();
      p->Settings.Save();
    });
  }
}

okMainWindow::~okMainWindow() {
}

void okMainWindow::OnTabChanged(wxBookCtrlEvent& ev) {
  const auto tab = ev.GetSelection();
  if (tab == wxNOT_FOUND) {
    return;
  }
  p->CurrentTab = tab;
  UpdateSHM();
}

void okMainWindow::OnGameEvent(wxThreadEvent& ev) {
  const auto ge = ev.GetPayload<GameEvent>();
  dprintf("GameEvent: '{}' = '{}'", ge.Name, ge.Value);
  for (auto tab: p->Tabs) {
    tab->GetTab()->OnGameEvent(ge);
  }
}

void okMainWindow::UpdateSHM() {
  if (!p->SHM) {
    return;
  }

  auto tab = p->Tabs.at(p->CurrentTab);
  auto image = tab->GetImage();
  if (!image.IsOk()) {
    if (tab->GetTab()->GetPageCount() == 0) {
      image = CreateErrorImage(_("No Pages"));
    } else {
      image = CreateErrorImage(_("Invalid Page Image"));
    }
  }

  auto ratio = float(image.GetHeight()) / image.GetWidth();
  OpenKneeboard::SHM::Header header {
    .Flags = {},// OpenKneeboard::Flags::DISCARD_DEPTH_INFORMATION,
    .floorY = 0.6f,
    .eyeY = -0.7f,
    .z = -0.25f,
    .rx = float(M_PI / 2),
    .VirtualWidth = 0.5f / ratio,
    .VirtualHeight = 0.5f,
    .ImageWidth = static_cast<uint16_t>(image.GetWidth()),
    .ImageHeight = static_cast<uint16_t>(image.GetHeight()),
  };

  using Pixel = OpenKneeboard::SHM::Pixel;

  std::vector<Pixel> pixels(image.GetWidth() * image.GetHeight());
  for (int x = 0; x < image.GetWidth(); ++x) {
    for (int y = 0; y < image.GetHeight(); ++y) {
      pixels[x + (y * image.GetWidth())] = {
        image.GetRed(x, y),
        image.GetGreen(x, y),
        image.GetBlue(x, y),
        image.HasAlpha() ? image.GetAlpha(x, y) : 255ui8,
      };
    }
  }

  p->SHM.Update(header, pixels);
}

void okMainWindow::OnExit(wxCommandEvent& ev) {
  Close(true);
}

void okMainWindow::OnShowSettings(wxCommandEvent& ev) {
  auto w = new wxFrame(this, wxID_ANY, _("Settings"));
  auto s = new wxBoxSizer(wxVERTICAL);

  auto nb = new wxNotebook(w, wxID_ANY);
  s->Add(nb, 1, wxEXPAND);

  for (auto& component: p->Configurables) {
    auto p = new wxPanel(nb, wxID_ANY);
    auto ui = component->GetSettingsUI(p);
    if (!ui) {
      continue;
    }

    auto ps = new wxBoxSizer(wxVERTICAL);
    ps->Add(ui, 1, wxEXPAND, 5);
    p->SetSizerAndFit(ps);

    nb->AddPage(p, ui->GetLabel());
  }

  w->SetSizerAndFit(s);
  w->Show(true);
}

void okMainWindow::OnPreviousTab(wxCommandEvent& ev) {
  p->Notebook->AdvanceSelection(false);
}

void okMainWindow::OnNextTab(wxCommandEvent& ev) {
  p->Notebook->AdvanceSelection(true);
}

void okMainWindow::OnPreviousPage(wxCommandEvent& ev) {
  p->Tabs[p->CurrentTab]->PreviousPage();
}

void okMainWindow::OnNextPage(wxCommandEvent& ev) {
  p->Tabs[p->CurrentTab]->NextPage();
}