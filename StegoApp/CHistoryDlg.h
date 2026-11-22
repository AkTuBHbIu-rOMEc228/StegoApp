#pragma once
#include "afxdialogex.h"
#include <afxcmn.h> // Для CListCtrl

class CHistoryDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHistoryDlg)

public:
	CHistoryDlg(CWnd* pParent = nullptr);
	virtual ~CHistoryDlg();

	enum { IDD = IDD_HISTORY_DIALOG };

public:
	int m_teacherId = 0; // ID преподавателя
	CListCtrl m_listHistory; // Таблица

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog(); 

	DECLARE_MESSAGE_MAP()
};