#pragma once
#include "afxcmn.h"

class CStegoAppDlg : public CDialogEx
{
public:
	CStegoAppDlg(CWnd* pParent = nullptr);
	enum { IDD = IDD_STEGOAPP_DIALOG };

	// Переменные
	int m_teacherId = 0;
	CEdit m_editText;
	CEdit m_editDetails;
	CComboBox m_comboStudents;
	CComboBox m_comboWorks;
	CComboBox m_comboTemplates; // Наш новый ComboBox для шаблонов
	CButton m_checkSpaces;
	CButton m_checkInvisible;
	CButton m_checkHomoglyphs;

	// Кнопки
	CButton m_btnLoad;
	CButton m_btnAnalyze;
	CButton m_btnSave;
	CButton m_btnShowInfo;
	CButton m_btnAddWork;
	CButton m_btnHistory; // Новая кнопка
	CButton m_btnStats;   // Новая кнопка
	CButton m_btnSaveTemplate;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	bool CheckExtraSpaces(const CString& text, CString& details);
	bool CheckInvisibleChars(const CString& text, CString& details);
	bool CheckHomoglyphs(const CString& text, CString& details);

	HICON m_hIcon;

	// Обработчики сообщений
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClickedBtnAnalyze();
	afx_msg void OnBnClickedBtnLoadFromDb();
	afx_msg void OnBnClickedBtnSaveDb();
	afx_msg void OnCbnSelchangeComboStudents();
	afx_msg void OnBnClickedBtnShowInfo();
	afx_msg void OnBnClickedBtnAddWork();

	// Новые обработчики
	afx_msg void OnCbnSelchangeComboTemplates();
	afx_msg void OnBnClickedBtnHistory();
	afx_msg void OnBnClickedBtnStats();
	afx_msg void OnBnClickedBtnSaveTemplate();

	DECLARE_MESSAGE_MAP()
};