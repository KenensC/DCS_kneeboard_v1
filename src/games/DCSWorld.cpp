#include "OpenKneeboard\Games\DCSWorld.h"

#include <ShlObj.h>
#include <Windows.h>
#include <fmt/format.h>

#include "OpenKneeboard/dprint.h"

namespace OpenKneeboard::Games {

static std::filesystem::path GetDCSPath(const char* lastSubKey) {
  const auto subkey = fmt::format("SOFTWARE\\Eagle Dynamics\\{}", lastSubKey);
  char buffer[MAX_PATH];
  DWORD length = sizeof(buffer);

  if (
    RegGetValueA(
      HKEY_CURRENT_USER,
      subkey.c_str(),
      "Path",
      RRF_RT_REG_SZ,
      nullptr,
      reinterpret_cast<void*>(buffer),
      &length)
    != ERROR_SUCCESS) {
    return {};
  }

  return std::filesystem::canonical(std::filesystem::path(buffer));
}

static std::filesystem::path GetSavedGamesPath() {
  static std::filesystem::path sPath;
  if (!sPath.empty()) {
    return sPath;
  }

  wchar_t* buffer = nullptr;
  if (
    SHGetKnownFolderPath(FOLDERID_SavedGames, NULL, NULL, &buffer) == S_OK
    && buffer) {
    sPath = std::filesystem::canonical(std::wstring(buffer));
  }
  return sPath;
}

std::filesystem::path DCSWorld::GetInstalledPath(Version version) {
  switch (version) {
    case Version::OPEN_BETA:
      return GetDCSPath("DCS World OpenBeta");
    case Version::STABLE:
      return GetDCSPath("DCS World");
  }
  return {};
}

std::filesystem::path DCSWorld::GetSavedGamesPath(Version version) {
  switch (version) {
    case Version::OPEN_BETA:
      return Games::GetSavedGamesPath() / "DCS.openbeta";
    case Version::STABLE:
      return Games::GetSavedGamesPath() / "DCS";
  }
  return {};
}

wxString DCSWorld::GetUserFriendlyName(
  const std::filesystem::path& _path) const {
  const auto path = std::filesystem::canonical(_path);
  if (path == GetInstalledPath(Version::OPEN_BETA) / "bin" / "DCS.exe") {
    return _("DCS World - Open Beta");
  }
  if (path == GetInstalledPath(Version::STABLE) / "bin" / "DCS.exe") {
    return _("DCS World - Stable");
  }
  return _("DCS World");
}
std::vector<std::filesystem::path> DCSWorld::GetInstalledPaths() const {
  std::vector<std::filesystem::path> ret;
  for (const auto& path: {
         GetInstalledPath(Version::OPEN_BETA),
         GetInstalledPath(Version::STABLE),
       }) {
    if (!std::filesystem::is_directory(path)) {
      continue;
    }
    auto exe = path / "bin" / "DCS.exe";
    if (std::filesystem::is_regular_file(exe)) {
      ret.push_back(exe);
    }
  }
  return ret;
}

bool DCSWorld::DiscardOculusDepthInformationDefault() const {
  return true;
}

const char* DCSWorld::GetNameForConfigFile() const {
  return "DCSWorld";
}

}// namespace OpenKneeboard::Games
