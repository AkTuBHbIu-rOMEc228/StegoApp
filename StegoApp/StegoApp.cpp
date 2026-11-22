// StegoApp.cpp : Defines the class behaviors for the application.
//
#include "pch.h"
#include "framework.h"
#include "StegoApp.h"
#include "StegoAppDlg.h"
#include "CLoginDlg.h" // Подключаем окно логина

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CStegoAppApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CStegoAppApp::CStegoAppApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CStegoAppApp theApp;

BOOL CStegoAppApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	AfxEnableControlContainer();

	CShellManager* pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("Локальные приложения"));

	// 1. СНАЧАЛА ОКНО ЛОГИНА
	CLoginDlg loginDlg;
	if (loginDlg.DoModal() != IDOK)
	{
		return FALSE; // Если нажали Отмена - программа закрывается
	}

	// 2. ПОТОМ ГЛАВНОЕ ОКНО
	CStegoAppDlg dlg;
	dlg.m_teacherId = loginDlg.m_loggedTeacherId; // Передаем ID вошедшего препода
	m_pMainWnd = &dlg;
	dlg.DoModal();

	if (pShellManager != nullptr) delete pShellManager;
	ControlBarCleanUp();
	return FALSE;
}