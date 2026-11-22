#pragma once
#include "afxdialogex.h"
#include "afxcmn.h"
#include "afxdtctl.h"

class CStatsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStatsDlg)

public:
	CStatsDlg(CWnd* pParent = nullptr);
	virtual ~CStatsDlg();
	enum { IDD = IDD_STATS_DIALOG };

	int m_teacherId = 0; // ID преподавателя

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	// Элементы управления для фильтров
	CComboBox m_comboFilterStudent;
	CComboBox m_comboFilterGroup;
	CComboBox m_comboFilterWork;
	CDateTimeCtrl m_dateFrom;
	CDateTimeCtrl m_dateTo;

	// Методы
	void LoadFilters();
	void UpdateStats();

	// Обработчик кнопки
	afx_msg void OnBnClickedBtnApplyFilter();
};