// autoStartMarginTimer.cpp : main source file for autoStartMarginTimer.exe
//

#include "stdafx.h"

#include "resource.h"

#include "aboutdlg.h"
#include "MainDlgModel.h"

namespace {
HANDLE CheckMultipleInstance() {
  HANDLE mutex = ::CreateMutex(NULL, TRUE, L"T99_AUTOSTARTMARGINTIMER");
  ATLASSERT(mutex != NULL);
  if (mutex == NULL) {
    return NULL;
  } else if (::GetLastError() == ERROR_ALREADY_EXISTS) {
    ::CloseHandle(mutex);
    return NULL;
  }
  return mutex;
}
}  // namespace

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int /*nCmdShow*/ = SW_SHOWDEFAULT) {
  MainDlgModel dialog;
  int nRet = dialog.Run();
  return nRet;
}

int WINAPI _tWinMain(HINSTANCE /*hInstance*/,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR lpstrCmdLine,
                     int nCmdShow) {
  HRESULT hRes = ::CoInitialize(NULL);
  if (FAILED(hRes)) {
    return 2;
  }

  if (!CheckMultipleInstance()) {
    return 1;
  }

  int nRet = Run(lpstrCmdLine, nCmdShow);

  ::CoUninitialize();

  return nRet;
}
