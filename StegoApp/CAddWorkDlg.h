#pragma once
#include "afxdialogex.h"
#include <afxcmn.h> 

class CAddWorkDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddWorkDlg)

public:
	CAddWorkDlg(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~CAddWorkDlg();

	// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADD_WORK_DIALOG };
#endif

public:
	int m_teacherId = 0; // ID преподавателя, который добавляет работу

	CComboBox m_comboStudent;
	CEdit m_editTitle;
	CEdit m_editText;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog();
	void LoadStudents(); 
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	
	afx_msg void OnBnClickedBtnAddNewStudent();
};