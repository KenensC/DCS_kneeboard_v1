#include "OpenKneeboard\Games\DCSWorld.h"

#include <Windows.h>
#include <fmt/format.h>

#include "OpenKneeboard/dprint.h"

namespace OpenKneeboard::Games {

static std::string GetDCSPath(const char* lastSubKey) {
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

  return buffer;
}

std::string DCSWorld::GetStablePath() {
  return GetDCSPath("DCS World");
}

std::string DCSWorld::GetOpenBetaPath() {
  return GetDCSPath("DCS World OpenBeta");
}

std::string DCSWorld::GetNewestPath() {
  auto path = GetOpenBetaPath();
  if (path.empty()) {
    path = GetStablePath();
  }
  if (path.empty()) {
    dprint("Failed to find any DCS World installation path.");
  }

  return path;
}

}// namespace OpenKneeboard::Games