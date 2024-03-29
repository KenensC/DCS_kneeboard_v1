add_library(detours-ext STATIC detours-ext.cpp)
target_link_libraries(detours-ext PUBLIC detours OpenKneeboard-dprint)

add_library(OpenKneeboard-DllLoadWatcher STATIC DllLoadWatcher.cpp)
target_link_libraries(
  OpenKneeboard-DllLoadWatcher
  PRIVATE
  detours-ext
  OpenKneeboard-dprint)

add_library(OpenKneeboard-function-patterns STATIC function-patterns.cpp)
target_link_libraries(
  OpenKneeboard-function-patterns
  PRIVATE
  OpenKneeboard-dprint)

add_library(OpenKneeboard-FindMainWindow STATIC FindMainWindow.cpp)

add_library(dxgi-hooks STATIC dxgi-offsets.c IDXGISwapChainPresentHook.cpp)
target_link_libraries(
  dxgi-hooks
  PRIVATE
  detours-ext
  OpenKneeboard-dprint
  OpenKneeboard-function-patterns
  OpenKneeboard-FindMainWindow)

add_library(d3d12-hooks STATIC d3d12-offsets.c ID3D12CommandQueueExecuteCommandListsHook.cpp)
target_link_libraries(d3d12-hooks PRIVATE detours-ext OpenKneeboard-dprint System::D3d12)

add_library(OpenKneeboard-ScopedRWX STATIC ScopedRWX.cpp)

add_library(
  oculus-hooks
  STATIC
  OculusEndFrameHook.cpp
  OculusKneeboard.cpp
  OVRProxy.cpp)
target_link_libraries(
  oculus-hooks
  PUBLIC
  OpenKneeboard-DllLoadWatcher
  OpenKneeboard-RayIntersectsRect
  OpenKneeboard-SHM
  OpenKneeboard-VRKneeboard
  OpenKneeboard-config
  OpenKneeboard-dprint
  detours-ext
  directxtk
  oculus-sdk-headers
)

add_library(openvr-hooks STATIC IVRCompositorWaitGetPosesHook.cpp)
target_link_libraries(
  openvr-hooks
  PUBLIC
  detours-ext
  OpenKneeboard-DllLoadWatcher
  OpenKneeboard-ScopedRWX
  openvr-headers
  OpenKneeboard-dprint)

# This is a marker library that tells the main app not to inject the DLL
add_module_library(OpenKneeboard-AutoInject-Marker AutoInjectMarker.cpp)

add_module_library(OpenKneeboard-nonvr-d3d11 NonVRD3D11Kneeboard.cpp)
target_link_libraries(
  OpenKneeboard-nonvr-d3d11
  PRIVATE
  dxgi-hooks
  detours-ext
  OpenKneeboard-dprint
  OpenKneeboard-version
  OpenKneeboard-D3D11
  OpenKneeboard-SHM
  OpenKneeboard-DllLoadWatcher)

add_module_library(
  OpenKneeboard-oculus-d3d11
  OculusD3D11Kneeboard.cpp
  OculusD3D11Kneeboard_DllMain.cpp
  OculusD3D11Kneeboard_StaticRender.cpp
)
target_link_libraries(
  OpenKneeboard-oculus-d3d11
  PRIVATE
  dxgi-hooks
  detours-ext
  OpenKneeboard-config
  OpenKneeboard-dprint
  OpenKneeboard-version
  OpenKneeboard-D3D11
  oculus-hooks
  oculus-sdk-headers)

add_module_library(
  OpenKneeboard-oculus-d3d12
  OculusD3D11Kneeboard_StaticRender.cpp
  OculusD3D12Kneeboard.cpp
  OculusD3D12Kneeboard_DllMain.cpp
)
target_link_libraries(
  OpenKneeboard-oculus-d3d12
  PRIVATE
  d3d12-hooks
  detours-ext
  OpenKneeboard-config
  OpenKneeboard-dprint
  OpenKneeboard-version
  OpenKneeboard-D3D11
  OpenKneeboard-ScopedRWX
  oculus-hooks
  oculus-sdk-headers)

add_module_library(
  OpenKneeboard-OpenXR
  OpenXRKneeboard.cpp
  OpenXRD3D11Kneeboard.cpp
  OpenXRD3D12Kneeboard.cpp
  OpenXRNext.cpp
)
target_link_libraries(
  OpenKneeboard-OpenXR
  PRIVATE
  ThirdParty::OpenXR
  ThirdParty::DirectXTK
  OpenKneeboard-D3D11
  OpenKneeboard-RayIntersectsRect
  OpenKneeboard-SHM
  OpenKneeboard-VRKneeboard
  OpenKneeboard-config
  OpenKneeboard-dprint
  OpenKneeboard-scope_guard
  OpenKneeboard-shims
  OpenKneeboard-version
)

install(FILES OpenKneeboard-OpenXR.json TYPE BIN)

add_module_library(OpenKneeboard-Injection-Bootstrapper InjectionBootstrapper.cpp)
target_link_libraries(
  OpenKneeboard-Injection-Bootstrapper
  PRIVATE
  OpenKneeboard-RuntimeFiles
  OpenKneeboard-version
  detours-ext
  dxgi-hooks
  oculus-hooks
  openvr-hooks)

add_module_library(OpenKneeboard-TabletProxy TabletProxy.cpp)
target_link_libraries(
  OpenKneeboard-TabletProxy
  PRIVATE
  detours-ext
  OpenKneeboard-dprint
  OpenKneeboard-FindMainWindow
  OpenKneeboard-GetMainHWND
  OpenKneeboard-tablets
  OpenKneeboard-version
)
