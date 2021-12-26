#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/thread.h>

wxDECLARE_EVENT(okEVT_GAME_EVENT, wxThreadEvent);

class okGameEventMailslotThread final : public wxThread {
 private:
  wxFrame* mParent;

 public:
  struct Payload {
    std::string Name;
    std::string Value;
  };
  okGameEventMailslotThread(wxFrame* parent);
  ~okGameEventMailslotThread();

 protected:
  virtual ExitCode Entry() override;
};