#include "OpenKneeboard/DCSAircraftTab.h"

#include <filesystem>

#include "OpenKneeboard/FolderTab.h"
#include "OpenKneeboard/GameEvent.h"
#include "OpenKneeboard/Games/DCSWorld.h"
#include "OpenKneeboard/dprint.h"

using DCS = OpenKneeboard::Games::DCSWorld;

namespace OpenKneeboard {

DCSAircraftTab::DCSAircraftTab()
  : DCSTab(_("Aircraft")), mDelegate(std::make_shared<FolderTab>("", "")) {
}

DCSAircraftTab::~DCSAircraftTab() {
}

void DCSAircraftTab::Reload() {
  mDelegate->Reload();
}

uint16_t DCSAircraftTab::GetPageCount() const {
  return mDelegate->GetPageCount();
}
wxImage DCSAircraftTab::RenderPage(uint16_t index) {
  return mDelegate->RenderPage(index);
}

const char* DCSAircraftTab::GetGameEventName() const {
  return DCS::EVT_AIRCRAFT;
}

void DCSAircraftTab::Update(
  const std::filesystem::path& installPath,
  const std::filesystem::path& savedGamesPath,
  const std::string& aircraft) {
  auto path = savedGamesPath / "KNEEBOARD" / aircraft;
  mDelegate->SetPath(path);
}

}// namespace OpenKneeboard