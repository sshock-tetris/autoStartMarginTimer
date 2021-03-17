#include "stdafx.h"
#include "resource.h"
#include "MainDlgModel.h"
#include "MainDlg.h"

MainDlgModel::MainDlgModel() : m_starter(this) {}

MainDlgModel::~MainDlgModel() {}

void MainDlgModel::Reset() {
  m_starter.Reset();
}

int MainDlgModel::Run() {
  m_dialog.SetModel(this);
  int return_code = (int)m_dialog.DoModal();
  return return_code;
}

bool MainDlgModel::Start() {
  return m_starter.Init();
}

bool MainDlgModel::SendStart() {
  return m_starter.SendStart();
}

void MainDlgModel::Invalidate() {}
