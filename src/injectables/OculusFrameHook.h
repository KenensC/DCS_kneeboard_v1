#pragma once

#include <OVR_CAPI.H>
#include <memory>

namespace OpenKneeboard {

class OculusFrameHook {
 private:
  bool mHooked;
 public:
  OculusFrameHook();
  virtual ~OculusFrameHook();

  void Unhook();

  virtual ovrResult onEndFrame(
    ovrSession session,
    long long frameIndex,
    const ovrViewScaleDesc* viewScaleDesc,
    ovrLayerHeader const* const* layerPtrList,
    unsigned int layerCount,
    decltype(&ovr_EndFrame) next) = 0;
};

}// namespace OpenKneeboard