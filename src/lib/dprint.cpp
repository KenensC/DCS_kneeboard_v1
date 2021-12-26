#include "OpenKneeboard/dprint.h"

#include <Windows.h>

namespace OpenKneeboard {

static DPrintSettings g_Settings;

void dprint(const std::string& message) {
  if (g_Settings.Target == DPrintSettings::Target::DEBUG_STREAM) {
    auto output = fmt::format("[{}] {}\n", g_Settings.Prefix, message);
    OutputDebugStringA(output.c_str());
    return;
  }

  if (g_Settings.Target == DPrintSettings::Target::CONSOLE) {
    auto output = fmt::format("{}\n", message);
    AllocConsole();
    auto handle = GetStdHandle(STD_ERROR_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) {
      return;
    }
    WriteConsoleA(
      handle,
      output.c_str(),
      static_cast<DWORD>(output.size()),
      nullptr,
      nullptr);
  }
}

void DPrintSettings::Set(const DPrintSettings& settings) {
  g_Settings = settings;
}

}// namespace OpenKneeboard