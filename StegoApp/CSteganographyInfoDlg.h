#pragma once
#include "afxdialogex.h"

// Диалоговое окно CSteganographyInfoDlg
class CSteganographyInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSteganographyInfoDlg)

public:
	CSteganographyInfoDlg(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~CSteganographyInfoDlg();

	// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	virtual BOOL OnInitDialog(); // <--- ДОБАВЛЕНО

	DECLARE_MESSAGE_MAP()

public:
	CEdit m_infoText; // Переменная для текстового поля
};