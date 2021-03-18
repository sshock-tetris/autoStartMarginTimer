#pragma once

class MainDlgModel;

enum status {
  INITIALIZED = 0,
  MATCHING,
  READYGO,
  PLAYING,
};

class TimerStarter {
 public:
  explicit TimerStarter(MainDlgModel* parent);
  ~TimerStarter();

  static UINT WINAPI ThreadLauncher(void* args);

  bool Init();
  bool IsEnabled() const;
  bool ObsRecognized() const;
  DWORD obs_pid() const { return m_obs_pid; }
  void Reset();
  bool SendStart() const;
  bool SendEnd() const;
  void SetObsPreviewHWND(HWND hwnd) { m_obs_preview = hwnd; }
  void SetObsInteracativeHWND(HWND hwnd) { m_obs_interactive = hwnd; }
 private:
  MainDlgModel* m_parent;
  DWORD m_obs_pid;
  HWND m_obs_preview;
  HWND m_obs_interactive;
  DWORD m_monitor_tid;
  HANDLE m_monitor_thread;

  bool FindOBS();
  void ResetOBSProperty();
  void StartMonitorThread();
  void StopMonitorThread();
  void ThreadInternal();
};
