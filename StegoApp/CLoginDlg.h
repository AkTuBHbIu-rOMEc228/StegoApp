#pragma once
#include "afxdialogex.h"


// Диалоговое окно CLoginDlg

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	int m_loggedTeacherId = 0; 
	CEdit m_editLogin;
	CEdit m_editPassword;
	afx_msg void OnBnClickedBtnLogin();
	CLoginDlg(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~CLoginDlg();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
};
