// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"
#include "aboutdlg.h"
#include "MainDlgModel.h"

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/,
                               WPARAM /*wParam*/,
                               LPARAM /*lParam*/,
                               BOOL& /*bHandled*/) {
  // set icons
  HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
                                 ::GetSystemMetrics(SM_CXICON),
                                 ::GetSystemMetrics(SM_CYICON));
  SetIcon(hIcon, TRUE);
  HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
                                      ::GetSystemMetrics(SM_CXSMICON),
                                      ::GetSystemMetrics(SM_CYSMICON));
  SetIcon(hIconSmall, FALSE);

  m_detect_obs_button = GetDlgItem(IDC_DETECTOBS);
  m_manual_start_button = GetDlgItem(IDC_SENDSTART);
  m_status = GetDlgItem(IDC_STATUS);

  BOOL dummy;
  OnOBSClosed(0, 0, 0, dummy);

  CMenu SysMenu = GetSystemMenu(FALSE);
  if (::IsMenu(SysMenu)) {
    SysMenu.AppendMenu(MF_SEPARATOR);
    SysMenu.AppendMenu(MF_STRING, IDC_RESET, L"状態をリセット(&I)");
    SysMenu.AppendMenu(MF_STRING, ID_APP_ABOUT, L"&About...");
  }
  SysMenu.Detach();

  return TRUE;
}

LRESULT CMainDlg::OnSyscommand(UINT, WPARAM wparam, LPARAM, BOOL& handled) {
  if (wparam == ID_APP_ABOUT) {
    OnAppAbout(0, 0, 0, handled);
  } else if (wparam == IDC_RESET) {
    OnReset(0, 0, 0, handled);
  } else {
    handled = FALSE;
  }
  return TRUE;
}

LRESULT CMainDlg::OnOBSClosed(UINT, WPARAM, LPARAM, BOOL&) {
  m_detect_obs_button.EnableWindow(TRUE);
  m_manual_start_button.EnableWindow(FALSE);
  m_status.SetSelAll();
  m_status.ReplaceSel(L"[自動開始できません]");
  return TRUE;
}

LRESULT CMainDlg::OnStatusInitialized(UINT, WPARAM, LPARAM, BOOL&) {
  m_status.SetSelAll();
  m_status.ReplaceSel(L"準備完了。待機中...");
  return TRUE;
}

LRESULT CMainDlg::OnStatusMatching(UINT, WPARAM, LPARAM, BOOL&) {
  m_status.SetSelAll();
  m_status.ReplaceSel(L"マッチング中/ゲーム開始準備中");
  return TRUE;
}

LRESULT CMainDlg::OnStatusReady(UINT, WPARAM, LPARAM, BOOL&) {
  m_status.SetSelAll();
  m_status.ReplaceSel(L"READY");
  return TRUE;
}

LRESULT CMainDlg::OnStatusStart(UINT, WPARAM, LPARAM, BOOL&) {
  m_status.SetSelAll();
  m_status.ReplaceSel(L"GO");
  return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/,
                             WORD /*wID*/,
                             HWND /*hWndCtl*/,
                             BOOL& /*bHandled*/) {
  CAboutDlg dlg;
  dlg.DoModal();
  return 0;
}

LRESULT CMainDlg::OnDetectOBS(WORD, WORD /*wID*/, HWND, BOOL&) {
  if (m_model->Start()) {
    m_detect_obs_button.EnableWindow(FALSE);
    m_manual_start_button.EnableWindow(TRUE);
    BOOL dummy;
    OnStatusInitialized(0, 0, 0, dummy);
  } else {
    MessageBox(
        L"OBSのウィンドウを検出できません。\n"
        L"場面認識のためにウィンドウプロジェクターが、タイマー操作のためにタイマーとの対話ウィンドウが必要です。",
        L"", MB_OK | MB_ICONEXCLAMATION);
  }
  return 0;
}

LRESULT CMainDlg::OnSendStart(WORD, WORD, HWND, BOOL&) {
  if (!m_model->SendStart()) {
    MessageBox(
        L"OBSの対話ウィンドウが見つからないため、送信できません。",
        L"", MB_OK | MB_ICONEXCLAMATION);
  }
  return 0;
}

LRESULT CMainDlg::OnReset(WORD, WORD, HWND, BOOL&) {
  m_model->Reset();
  return TRUE;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/,
                           WORD wID,
                           HWND /*hWndCtl*/,
                           BOOL& /*bHandled*/) {
  CloseDialog(wID);
  return 0;
}

void CMainDlg::CloseDialog(int nVal) {
  EndDialog(nVal);
}
