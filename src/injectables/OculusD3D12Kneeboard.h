#pragma once

#include <winrt/base.h>

#include <vector>

#include "ID3D12CommandQueueExecuteCommandListsHook.h"
#include "OculusKneeboard.h"

namespace DirectX {
  class ResourceUploadBatch;
};

namespace OpenKneeboard {

class OculusD3D12Kneeboard final : public OculusKneeboard::Renderer {
 public:
  OculusD3D12Kneeboard();
  virtual ~OculusD3D12Kneeboard();

  void UninstallHook();

  virtual ovrTextureSwapChain CreateSwapChain(
    ovrSession session,
    const SHM::Config& config) override;

  virtual bool Render(
    ovrSession session,
    ovrTextureSwapChain swapChain,
    const SHM::Snapshot& snapshot) override;

 private:
  ID3D12CommandQueueExecuteCommandListsHook mExecuteCommandListsHook;
  OculusKneeboard mOculusKneeboard;

  winrt::com_ptr<ID3D12Device> mDevice;
  winrt::com_ptr<ID3D12CommandQueue> mCommandQueue;
  winrt::com_ptr<ID3D12DescriptorHeap> mDescriptorHeap;
  std::unique_ptr<DirectX::ResourceUploadBatch> mResourceUpload;

  void OnID3D12CommandQueue_ExecuteCommandLists(
    ID3D12CommandQueue* this_,
    UINT NumCommandLists,
    ID3D12CommandList* const* ppCommandLists,
    const decltype(&ID3D12CommandQueue::ExecuteCommandLists)& next);
};
}// namespace OpenKneeboard
