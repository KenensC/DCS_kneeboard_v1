#pragma once

#include "OpenKneeboard/DCSTab.h"

namespace OpenKneeboard {

class FolderTab;

class DCSAircraftTab final : public DCSTab {
 public:
  DCSAircraftTab();
  virtual ~DCSAircraftTab();

  virtual void Reload() override;
  virtual uint16_t GetPageCount() const override;
  virtual wxImage RenderPage(uint16_t index) override;

 protected:
  virtual const char* GetGameEventName() const override;
  virtual void Update(
    const std::filesystem::path&,
    const std::filesystem::path&,
    const std::string&) override;

 private:
  std::shared_ptr<FolderTab> mDelegate;
};

}// namespace OpenKneeboard