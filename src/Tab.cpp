#include "YAVRK/Tab.h"

namespace YAVRK {

class Tab::Impl final {
 public:
  std::string Title;
};

Tab::Tab(const std::string& title) : p(new Impl {.Title = title}) {
}

Tab::~Tab() {}

std::string Tab::GetTitle() const {
  return p->Title;
}

void Tab::Reload() {
}

}// namespace YAVRK