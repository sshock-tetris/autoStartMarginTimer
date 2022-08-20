#pragma once

class GraphicsCapture {
 public:
  GraphicsCapture() = default;
  ~GraphicsCapture() = default;
  bool Start(HWND hwnd);
  void Stop();
  bool GetNextFrame(cv::Mat& figure);

 private:
  winrt::com_ptr<ID3D11Device> m_device;
  winrt::com_ptr<ID3D11DeviceContext> m_dc;
  winrt::com_ptr<ID3D11Texture2D> m_captured;
  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice
      m_device_winrt = nullptr;
  winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_capture_item =
      nullptr;
  winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_frame_pool =
      nullptr;
  winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session = nullptr;
  winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::
      FrameArrived_revoker m_frameArrivedRevoker;
  SIZE m_client_size = {0};
};
