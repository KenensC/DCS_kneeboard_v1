/*
 * OpenKneeboard
 *
 * Copyright (C) 2022 Fred Emmott <fred@fredemmott.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
// clang-format off
#include "pch.h"
#include "TabPage.xaml.h"
#include "TabPage.g.cpp"
// clang-format on

#include <OpenKneeboard/KneeboardState.h>
#include <OpenKneeboard/Tab.h>
#include <OpenKneeboard/TabState.h>
#include <microsoft.ui.xaml.media.dxinterop.h>

#include "Globals.h"

using namespace ::OpenKneeboard;

namespace winrt::OpenKneeboardApp::implementation {

TabPage::TabPage() {
  InitializeComponent();

  auto brush
    = Background().as<::winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
  auto color = brush.Color();
  mBackgroundColor = {
    color.R / 255.0f,
    color.G / 255.0f,
    color.B / 255.0f,
    color.A / 255.0f,
  };
}

TabPage::~TabPage() {
}

void TabPage::OnNavigatedTo(const NavigationEventArgs& args) {
  const auto id = winrt::unbox_value<uint64_t>(args.Parameter());
  for (auto tab: gKneeboard->GetTabs()) {
    if (tab->GetInstanceID() != id) {
      continue;
    }
    SetTab(tab);
    break;
  }
}

void TabPage::SetTab(const std::shared_ptr<TabState>& state) {
  mState = state;
  AddEventListener(state->evNeedsRepaintEvent, &TabPage::PaintNow, this);
}

void TabPage::OnSizeChanged(
  const IInspectable&,
  const SizeChangedEventArgs& args) {
  auto size = args.NewSize();
  mCanvasSize = {
    static_cast<FLOAT>(size.Width),
    static_cast<FLOAT>(size.Height),
  };
  if (mSwapChain) {
    ResizeSwapChain();
  } else {
    InitializeSwapChain();
  }
  PaintNow();
}

void TabPage::ResizeSwapChain() {
  gDXResources.mD2DDeviceContext->SetTarget(nullptr);

  DXGI_SWAP_CHAIN_DESC desc;
  winrt::check_hresult(mSwapChain->GetDesc(&desc));
  winrt::check_hresult(mSwapChain->ResizeBuffers(
    desc.BufferCount,
    mCanvasSize.width * Canvas().CompositionScaleX(),
    mCanvasSize.height * Canvas().CompositionScaleY(),
    desc.BufferDesc.Format,
    desc.Flags));
}

void TabPage::InitializeSwapChain() {
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc {
    .Width
    = static_cast<UINT>(mCanvasSize.width * Canvas().CompositionScaleX()),
    .Height
    = static_cast<UINT>(mCanvasSize.height * Canvas().CompositionScaleY()),
    .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
    .SampleDesc = {1, 0},
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = 2,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
  };
  const DXResources& dxr = gDXResources;
  winrt::check_hresult(dxr.mDXGIFactory->CreateSwapChainForComposition(
    dxr.mDXGIDevice.get(), &swapChainDesc, nullptr, mSwapChain.put()));
  Canvas().as<ISwapChainPanelNative>()->SetSwapChain(mSwapChain.get());
}

void TabPage::PaintNow() {
  auto metrics = GetPageMetrics();

  auto ctx = gDXResources.mD2DDeviceContext.get();
  winrt::com_ptr<IDXGISurface> surface;
  winrt::check_hresult(mSwapChain->GetBuffer(0, IID_PPV_ARGS(surface.put())));
  winrt::com_ptr<ID2D1Bitmap1> bitmap;
  winrt::check_hresult(
    ctx->CreateBitmapFromDxgiSurface(surface.get(), nullptr, bitmap.put()));
  ctx->SetTarget(bitmap.get());
  ctx->BeginDraw();
  ctx->Clear(mBackgroundColor);
  mState->GetTab()->RenderPage(
    ctx, mState->GetPageIndex(), metrics.mRenderRect);
  ctx->EndDraw();
  mSwapChain->Present(0, 0);
}

TabPage::PageMetrics TabPage::GetPageMetrics() {
  const auto contentNativeSize = mState->GetNativeContentSize();

  const auto scaleX = mCanvasSize.width / contentNativeSize.width;
  const auto scaleY = mCanvasSize.height / contentNativeSize.height;
  const auto scale = std::min(scaleX, scaleY);

  const D2D1_SIZE_F contentRenderSize {
    contentNativeSize.width * scale, contentNativeSize.height * scale};
  const auto padX = (mCanvasSize.width - contentRenderSize.width) / 2;
  const auto padY = (mCanvasSize.height - contentRenderSize.height) / 2;

  const D2D1_RECT_F contentRenderRect {
    .left = padX,
    .top = padY,
    .right = mCanvasSize.width - padX,
    .bottom = mCanvasSize.height - padY,
  };

  return {contentNativeSize, contentRenderRect, contentRenderSize, scale};
}

void TabPage::OnPointerEvent(
  const IInspectable&,
  const PointerRoutedEventArgs& args) {
}

}// namespace winrt::OpenKneeboardApp::implementation
