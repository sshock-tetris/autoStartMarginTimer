#include "stdafx.h"

#include "resource.h"
#include "TimerStarter.h"
#include "Timer.h"
#include "MainDlgModel.h"
#include "GraphicsCapture.h"

namespace {
bool DetectMatchingColor(cv::Mat* target) {
  // [マッチング開始準備中] -> [READY] -> [3] -> [2] -> [1] -> [GO] -> [ゴースト表示]
  //                          \  _________________________________________________/
  //                           \/
  //                           338f (5.61sec.)
  // [Matching Window]
  // Color(RGB): 250, 73, 60
  // ※cv::ScalarはBGRの順であることに注意
  // 960x540 => (386,413)-(573,444)
  // 1920x1080 => (772,826)-(1146,890)
  cv::Mat mw1 =
      target->operator()(cv::Range(413, 444), cv::Range(386, 573));
  cv::Scalar mw1mean = cv::mean(mw1);
  // r > 200 && 60 <= g < 120 && 60 <= b < 110
  return mw1mean[2] > 200 && mw1mean[1] >= 60 && mw1mean[1] < 120 &&
         mw1mean[0] >= 60 && mw1mean[0] < 110;
}

DWORD FindOBSProcess() {
  const int MODULE_NAME_SIZE = MAX_PATH;
  DWORD proc[1024];
  DWORD needed_size, result;
  HANDLE process_handle;
  wchar_t name[MODULE_NAME_SIZE];
  int processes, i;

  result = 0;

  if (!EnumProcesses(proc, sizeof(proc), &needed_size)) {
    return 0;
  }
  processes = (int)needed_size / sizeof(DWORD);

  for (i = 0; i < processes; i++) {
    process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                 FALSE, proc[i]);
    if (process_handle != NULL) {
      DWORD namesize = MODULE_NAME_SIZE - 1;
      QueryFullProcessImageName(process_handle, 0, name, &namesize);
      PathStripPath(name);
      if (lstrcmpi(L"obs32.exe", name) == 0 ||
          lstrcmpi(L"obs64.exe", name) == 0) {
        result = proc[i];
        break;
      }
      CloseHandle(process_handle);
    }
  }
  return result;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam) {
  const int CLASS_NAME_LENGTH = 256;
  const wchar_t OBS_WINDOW_CLASS_NAME_POSTFIX[] = L"QWindowIcon";
  const wchar_t OBS_PREVIEW_PREFIX[] = L"ウィンドウ プロジェクター (ソース) - ";
  const wchar_t OBS_PREVIEW_PREFIX_2[] = L"ウィンドウプロジェクター (ソース) - ";
  const wchar_t OBS_INTERACTIVE[] = L"'MarginTimer' との相互作用";

  TimerStarter* starter = reinterpret_cast<TimerStarter*>(lparam);

  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);
  if (pid != starter->obs_pid()) {
    return TRUE;
  }

  wchar_t class_name[CLASS_NAME_LENGTH];
  GetClassName(hwnd, class_name, CLASS_NAME_LENGTH - 1);
  if (lstrcmp(class_name + lstrlen(class_name) -
                  _countof(OBS_WINDOW_CLASS_NAME_POSTFIX) + 1,
              OBS_WINDOW_CLASS_NAME_POSTFIX) == 0) {
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    LRESULT window_text_length = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0) + 1;
    if (window_text_length == 0) {
      return TRUE;
    }
    std::unique_ptr<wchar_t[]> window_name =
        std::make_unique<wchar_t[]>(window_text_length);
    SendMessage(hwnd, WM_GETTEXT, static_cast<WPARAM>(window_text_length),
                reinterpret_cast<LPARAM>(window_name.get()));

    if (style & (WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_THICKFRAME) &&
        !(style & WS_DISABLED)) {
      if (!starter->ObsRecognized()) {
        // ウィンドウプロジェクター
        if (StrCmpN(window_name.get(), OBS_PREVIEW_PREFIX,
                    _countof(OBS_PREVIEW_PREFIX) - 1) == 0) {
          starter->SetObsPreviewHWND(hwnd);
          // FALSEを返さないのは、対話ウィンドウが未調査の可能性があるため
          return TRUE;
        } else if (StrCmpN(window_name.get(), OBS_PREVIEW_PREFIX_2,
                           _countof(OBS_PREVIEW_PREFIX_2) - 1) == 0) {
          starter->SetObsPreviewHWND(hwnd);
          // FALSEを返さないのは、対話ウィンドウが未調査の可能性があるため
          return TRUE;
        }
      }
      // ブラウザとの対話ウィンドウ
      if (lstrcmp(window_name.get(), OBS_INTERACTIVE) == 0) {
        starter->SetObsInteracativeHWND(hwnd);
        // FALSEを返さないのは、ウィンドウプロジェクターが未調査の可能性があるため
        return TRUE;
      }
    }
  }
  return TRUE;
}

void SendOBSClosedToMainDlg(HWND window) {
  SendNotifyMessage(window, WM_T99TIMER_OBSCLOSED, 0, 0);
}
}  // namespace

TimerStarter::TimerStarter(MainDlgModel* parent)
    : m_parent(parent),
      m_obs_pid(0),
      m_obs_preview(NULL),
      m_obs_interactive(NULL),
      m_monitor_tid(0),
      m_monitor_thread(NULL) {}

TimerStarter::~TimerStarter() {
  StopMonitorThread();
}

UINT __stdcall TimerStarter::ThreadLauncher(void* args) {
  HRESULT hr =
      ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (SUCCEEDED(hr)) {
    TimerStarter* starter = static_cast<TimerStarter*>(args);
    starter->ThreadInternal();
    starter->ResetOBSProperty();
    ::CoUninitialize();
  }
  _endthreadex(0);
  return 0;
}

bool TimerStarter::Init() {
  if (IsEnabled()) {
    return true;
  }
  bool sts = false;
  sts = FindOBS();
  StartMonitorThread();
  return sts;
}

bool TimerStarter::ObsRecognized() const {
  return IsWindow(m_obs_preview);
}

void TimerStarter::Reset() {
  SendOBSClosedToMainDlg(m_parent->GetWindow());
  StopMonitorThread();
}

bool TimerStarter::SendStart() const {
  if (!IsEnabled()) {
    return false;
  }
  SendMessage(m_obs_interactive, WM_CHAR, VK_ESCAPE, 0);
  SendNotifyMessage(m_obs_interactive, WM_CHAR, VK_RETURN, 0);
  return true;
}

bool TimerStarter::SendEnd() const {
  if (!IsEnabled()) {
    return false;
  }
  SendMessage(m_obs_interactive, WM_CHAR, VK_ESCAPE, 0);
  return true;
}

bool TimerStarter::IsEnabled() const {
  return IsWindow(m_obs_preview) && IsWindow(m_obs_interactive);
}

bool TimerStarter::FindOBS() {
  m_obs_pid = FindOBSProcess();
  if (m_obs_pid == 0) {
    return false;
  }

  // ウィンドウ検索
  EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(this));

  // ウィンドウプロジェクターの大きさを960x540に
  // ウィンドウサイズ変更不可能に変更
  if (ObsRecognized()) {
    LONG_PTR style = GetWindowLongPtr(m_obs_preview, GWL_STYLE);
    style = style & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX;
    SetWindowLongPtr(m_obs_preview, GWL_STYLE, style);
    RECT window_rect, client_rect;
    GetWindowRect(m_obs_preview, &window_rect);
    GetClientRect(m_obs_preview, &client_rect);
    const int new_width = (window_rect.right - window_rect.left) -
                          (client_rect.right - client_rect.left) + 960;
    const int new_height = (window_rect.bottom - window_rect.top) -
                           (client_rect.bottom - client_rect.top) + 540;
    SetWindowPos(m_obs_preview, NULL, 0, 0, new_width, new_height,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
  }
  return IsEnabled();
}

void TimerStarter::ResetOBSProperty() {
  SendOBSClosedToMainDlg(m_parent->GetWindow());
  m_monitor_tid = 0;
  m_obs_pid = 0;
  m_obs_preview = NULL;
  m_obs_interactive = NULL;
}

void TimerStarter::StartMonitorThread() {
  StopMonitorThread();
  m_monitor_thread = reinterpret_cast<HANDLE>(
      _beginthreadex(NULL, 0, ThreadLauncher, this, 0, (UINT*)&m_monitor_tid));
}

void TimerStarter::StopMonitorThread() {
  if (m_monitor_tid) {
    PostThreadMessage(m_monitor_tid, WM_QUIT, 0, 0);
    WaitForSingleObject(m_monitor_thread, INFINITE);
    CloseHandle(m_monitor_thread);
  }
}

void TimerStarter::ThreadInternal() {
  // FPSの数値をVSYNCより大きい数値(大体の環境で60)にすると、
  // Windows Graphics Captureでの画像取得が失敗する可能性があるので、
  // 上げすぎるのは推奨しない。
  const int FPS = 30;
  if (!ObsRecognized()) {
    return;
  }

  status current_status = status::INITIALIZED;
  unsigned int frames = 0;
  unsigned int ready_frame = 0;
  RECT rect;
  if (!GetClientRect(m_obs_preview, &rect)) {
    return;
  }

  cv::Mat preview_image[2] = {
      cv::Mat(rect.bottom, rect.right, CV_8UC4),
      cv::Mat(rect.bottom, rect.right, CV_8UC4)
  };
  GraphicsCapture capture;
  capture.Start(m_obs_preview);

  Timer timer;
  timer.Start(FPS);
  MSG msg;
  while (1) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        break;
      }
      continue;
    }
    if (!IsEnabled()) {
      // MainDlgに通知してスレッド終了
      break;
    }
    int loop = timer.Run();
    for (int i = 0; i < loop; i++) {
      cv::Mat* target = frames % 2 == 0 ? &preview_image[0] : &preview_image[1];
      if (!capture.GetNextFrame(*target)) {
        frames++;
        continue;
      }
      // 暗転時に初期状態に戻す
      // 黒が(R,G,B)=(16,16,16)の環境を想定して、normは大きめの値を取るようにする
      // (16,16,16)と"Now loading"を合わせて約27000000。
      if (cv::norm(*target, cv::NORM_L1) < 27000000 &&
          current_status != status::INITIALIZED) {
        SendNotifyMessage(m_parent->GetWindow(), WM_T99TIMER_INITIALIZED, 0, 0);
        current_status = status::INITIALIZED;
        SendEnd();
      } else if (current_status == status::INITIALIZED) {
        // [TETRIS field and next]
        // マッチング中の時点ではフィールド上部や左の方は動きがないはず
        // 上部: 960x540 => (322,42)-(668,314)
        // 左部: 960x540 => (322,42)-(452,492)
        cv::Mat fu1 = preview_image[0](cv::Range(42, 314), cv::Range(322, 668));
        cv::Mat fu2 = preview_image[1](cv::Range(42, 314), cv::Range(322, 668));
        double field_norm = cv::norm(cv::abs(fu1 - fu2), cv::NORM_L2);
        if (DetectMatchingColor(target) && field_norm < 3000) {
          if (current_status != status::MATCHING) {
            SendNotifyMessage(m_parent->GetWindow(), WM_T99TIMER_MATCHING, 0,
                              0);
          }
          current_status = status::MATCHING;
        } else {
          frames++;
          Sleep(500);
          break;
        }
      } else if (current_status == status::MATCHING &&
                 !DetectMatchingColor(target)) {
        SendNotifyMessage(m_parent->GetWindow(), WM_T99TIMER_READY, 0, 0);
        ready_frame = frames;
        current_status = status::READYGO;
      } else if (current_status == status::READYGO) {
        // マッチング中を認識した場合は初期状態に戻す
        if (DetectMatchingColor(target)) {
          current_status = status::INITIALIZED;
        }
        // [NEXT]
        // 960x540 => 外枠(608,30)-(668,310)
        // 960x540 => ミノ部のみ(612,64)-(654,294)
        // 1920x1080 => 外枠(1212,86)-(1338,626)
        // 1920x1080 => ミノ部のみ(1222,124)-(1312,588)
        cv::Mat p1 = preview_image[0](cv::Range(64, 294), cv::Range(612, 654));
        cv::Mat p2 = preview_image[1](cv::Range(64, 294), cv::Range(612, 654));
        double norm = cv::norm(cv::abs(p1 - p2), cv::NORM_L2);
        if (frames - ready_frame >= 337 / (60 / FPS) && norm > 4700) {
          SendNotifyMessage(m_parent->GetWindow(), WM_T99TIMER_START, 0, 0);
          SendStart();
          current_status = status::PLAYING;
        }
      } else if (current_status == status::PLAYING) {
        frames++;
        Sleep(500);
        break;
      }
      frames++;
    }
    Sleep(1);
  }
  capture.Stop();
}
