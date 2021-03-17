#pragma once
#include "MainDlg.h"
#include "TimerStarter.h"

class MainDlgModel {
 public:
  MainDlgModel();
  ~MainDlgModel();
  void Reset();
  int Run();
  bool Start();
  bool SendStart();
  void Invalidate();
  HWND GetWindow() const { return m_dialog; }

 private:
  CMainDlg m_dialog;
  TimerStarter m_starter;
};
