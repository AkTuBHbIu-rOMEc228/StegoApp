#pragma once
#include "afxdialogex.h"
#include "resource.h" // Убедись, что IDD_ADD_STUDENT_DIALOG определен в ресурсах

class CAddStudentDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddStudentDlg)

public:
	CAddStudentDlg(CWnd* pParent = nullptr);
	virtual ~CAddStudentDlg();

	enum { IDD = IDD_ADD_STUDENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	// Элементы управления
	CEdit m_editLastName;
	CEdit m_editFirstName;
	CEdit m_editMiddleName;
	CEdit m_editEmail;
	CComboBox m_comboGroup;
	afx_msg void OnBnClickedOk();
};