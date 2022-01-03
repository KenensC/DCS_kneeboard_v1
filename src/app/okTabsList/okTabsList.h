#pragma once

#include <OpenKneeboard/Tab.h>

#include "okConfigurableComponent.h"

class okTabsList final : public okConfigurableComponent {
 private:
  class SettingsUI;
  struct SharedState;
  struct State;
  std::shared_ptr<State> p;

 public:
  okTabsList() = delete;
  okTabsList(const nlohmann::json& config);

  std::vector<std::shared_ptr<OpenKneeboard::Tab>> GetTabs() const;

  virtual ~okTabsList();
  virtual wxWindow* GetSettingsUI(wxWindow* parent) override;
  virtual nlohmann::json GetSettings() const override;
};