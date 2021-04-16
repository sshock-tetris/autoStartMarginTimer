// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class MainDlgModel;

class CMainDlg : public CDialogImpl<CMainDlg> {
 public:
  enum { IDD = IDD_MAINDLG };

  BEGIN_MSG_MAP(CMainDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSyscommand)
    MESSAGE_HANDLER(WM_T99TIMER_OBSCLOSED, OnOBSClosed)
    MESSAGE_HANDLER(WM_T99TIMER_INITIALIZED, OnStatusInitialized)
    MESSAGE_HANDLER(WM_T99TIMER_MATCHING, OnStatusMatching)
    MESSAGE_HANDLER(WM_T99TIMER_READY, OnStatusReady)
    MESSAGE_HANDLER(WM_T99TIMER_START, OnStatusStart)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
    COMMAND_ID_HANDLER(IDC_DETECTOBS, OnDetectOBS)
    COMMAND_ID_HANDLER(IDC_SENDSTART, OnSendStart)
    COMMAND_ID_HANDLER(IDC_RESET, OnReset)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
  END_MSG_MAP()

  CMainDlg() : m_model(nullptr) {}
  ~CMainDlg() = default;
  void SetModel(MainDlgModel* const model) { m_model = model; }

 private:
  MainDlgModel* m_model;

  // Dialog Components
  CButton m_detect_obs_button;
  CButton m_manual_start_button;
  CEdit m_status;

  // Handler prototypes (uncomment arguments if needed):
  // LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  // LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  // LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnInitDialog(UINT /*uMsg*/,
                       WPARAM /*wParam*/,
                       LPARAM /*lParam*/,
                       BOOL& /*bHandled*/);
  LRESULT OnSyscommand(UINT /*uMsg*/,
                       WPARAM wParam,
                       LPARAM /*lParam*/,
                       BOOL& bHandled);
  LRESULT OnOBSClosed(UINT /*uMsg*/,
                      WPARAM /*wParam*/,
                      LPARAM /*lParam*/,
                      BOOL& /*bHandled*/);
  LRESULT OnStatusInitialized(UINT /*uMsg*/,
                              WPARAM /*wParam*/,
                              LPARAM /*lParam*/,
                              BOOL& /*bHandled*/);
  LRESULT OnStatusMatching(UINT /*uMsg*/,
                           WPARAM /*wParam*/,
                           LPARAM /*lParam*/,
                           BOOL& /*bHandled*/);
  LRESULT OnStatusReady(UINT /*uMsg*/,
                        WPARAM /*wParam*/,
                        LPARAM /*lParam*/,
                        BOOL& /*bHandled*/);
  LRESULT OnStatusStart(UINT /*uMsg*/,
                        WPARAM /*wParam*/,
                        LPARAM /*lParam*/,
                        BOOL& /*bHandled*/);
  LRESULT OnAppAbout(WORD /*wNotifyCode*/,
                     WORD /*wID*/,
                     HWND /*hWndCtl*/,
                     BOOL& /*bHandled*/);
  LRESULT OnDetectOBS(WORD /*wNotifyCode*/,
                      WORD wID,
                      HWND /*hWndCtl*/,
                      BOOL& /*bHandled*/);
  LRESULT OnSendStart(WORD /*wNotifyCode*/,
                      WORD /*wID*/,
                      HWND /*hWndCtl*/,
                      BOOL& /*bHandled*/);
  LRESULT OnReset(WORD /*wNotifyCode*/,
                  WORD /*wID*/,
                  HWND /*hWndCtl*/,
                  BOOL& /*bHandled*/);
  LRESULT OnCancel(WORD /*wNotifyCode*/,
                   WORD wID,
                   HWND /*hWndCtl*/,
                   BOOL& /*bHandled*/);

  void CloseDialog(int nVal);
};
