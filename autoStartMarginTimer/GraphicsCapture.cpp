#include "stdafx.h"
#include "GraphicsCapture.h"

#include <inspectable.h>
#include <windows.foundation.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace winrt::Windows;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace ::Windows::Graphics::DirectX::Direct3D11;

#pragma comment(lib, "windowsapp")

namespace {
bool IsCursorCaptureEnabledApiAvailable() {
  return Foundation::Metadata::ApiInformation::
      IsPropertyPresent(L"Windows.Graphics.Capture.GraphicsCaptureSession",
                        L"IsCursorCaptureEnabled");
}
}  // namespace

bool GraphicsCapture::Start(HWND hwnd) {
  if (!::IsWindow(hwnd)) {
    return false;
  }
  RECT rect;
  if (!::GetClientRect(hwnd, &rect)) {
    return false;
  }
  m_client_size.cx = rect.right;
  m_client_size.cy = rect.bottom;

  HRESULT hr = ::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                   D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
                                   D3D11_SDK_VERSION, m_device.put(), nullptr,
                                   m_dc.put());
  if (FAILED(hr)) {
    return false;
  }

  auto dxgi = m_device.as<IDXGIDevice>();
  com_ptr<::IInspectable> device_rt;
  hr = ::CreateDirect3D11DeviceFromDXGIDevice(dxgi.get(), device_rt.put());
  if (FAILED(hr)) {
    return false;
  }
  m_device_winrt = device_rt.as<IDirect3DDevice>();

  const auto factory = get_activation_factory<GraphicsCaptureItem>();
  const auto interop = factory.as<IGraphicsCaptureItemInterop>();
  hr = interop->CreateForWindow(
      hwnd, guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
      reinterpret_cast<void**>(put_abi(m_capture_item)));
  if (FAILED(hr)) {
    m_device_winrt = nullptr;
    return false;
  }
  _ASSERT(m_capture_item);

  D3D11_TEXTURE2D_DESC desc = {0};
  desc.Width = m_client_size.cx;
  desc.Height = m_client_size.cy;
  desc.MipLevels = desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_STAGING;
  desc.BindFlags = 0;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.MiscFlags = 0;
  hr = m_device->CreateTexture2D(&desc, NULL, m_captured.put());
  if (FAILED(hr)) {
    return false;
  }

  const auto window_size = m_capture_item.Size();
  m_frame_pool = Direct3D11CaptureFramePool::CreateFreeThreaded(
      m_device_winrt, DirectXPixelFormat::B8G8R8A8UIntNormalized, 1,
      window_size);

  m_session =
      m_frame_pool.CreateCaptureSession(m_capture_item);
  if (IsCursorCaptureEnabledApiAvailable()) {
    m_session.IsCursorCaptureEnabled(false);
  }
  m_session.StartCapture();
  return true;
}

void GraphicsCapture::Stop() {
  m_session.Close();
  m_session = nullptr;

  m_frame_pool.Close();
  m_frame_pool = nullptr;

  m_device_winrt = nullptr;
  m_capture_item = nullptr;
}

bool GraphicsCapture::GetNextFrame(cv::Mat& figure) {
  const auto frame = m_frame_pool.TryGetNextFrame();
  if (!frame) {
    return false;
  }

  const auto surface = frame.Surface();
  if (!surface) {
    return false;
  }

  auto access = surface.as<IDirect3DDxgiInterfaceAccess>();
  com_ptr<ID3D11Texture2D> texture;
  HRESULT hr =
      access->GetInterface(guid_of<ID3D11Texture2D>(), texture.put_void());
  if (FAILED(hr)) {
    return false;
  }

  const auto size = frame.ContentSize();
  const int frame_width = (size.Width - m_client_size.cx) / 2;
  const int title_bar_height = size.Height - m_client_size.cy - frame_width;

  // キャプチャ画像にはウィンドウ枠とタイトルバーが含まれているので、
  // クライアント領域部分を切り抜く
  D3D11_BOX src_region{};
  src_region.left = frame_width;
  src_region.right = m_client_size.cx + frame_width;
  src_region.top = title_bar_height;
  src_region.bottom = m_client_size.cy + title_bar_height;
  src_region.front = 0;
  src_region.back = 1;
  m_dc->CopySubresourceRegion(m_captured.get(), 0, 0, 0, 0, texture.get(),
                                  0, &src_region);
  D3D11_MAPPED_SUBRESOURCE sr;
  hr = m_dc->Map(m_captured.get(), 0, D3D11_MAP_READ, 0, &sr);
  if (FAILED(hr)) {
    return false;
  }
  // 0xAARRGGBB; アルファチャンネルの値を常に0にする
  const __m128i mask = _mm_set1_epi32(0x00ffffff);
  const int simd_strides = m_client_size.cx / 4;
  for (int i = 0; i < m_client_size.cy; i++) {
    for (int j = 0; j < simd_strides; j++) {
      __m128i src = _mm_load_si128(static_cast<__m128i*>(sr.pData) +
                                   i * simd_strides + j);
      __m128i dst = _mm_and_epi32(src, mask);
      _mm_store_si128(
          reinterpret_cast<__m128i*>(figure.data) + i * simd_strides + j, dst);
    }
  }
  m_dc->Unmap(m_captured.get(), 0);

  return true;
}
