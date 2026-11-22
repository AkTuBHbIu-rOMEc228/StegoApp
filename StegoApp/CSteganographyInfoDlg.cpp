#include "pch.h"
#include "StegoApp.h"
#include "afxdialogex.h"
#include "CSteganographyInfoDlg.h"

// Диалоговое окно CSteganographyInfoDlg
IMPLEMENT_DYNAMIC(CSteganographyInfoDlg, CDialogEx)

CSteganographyInfoDlg::CSteganographyInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_INFO, pParent)
{
}

CSteganographyInfoDlg::~CSteganographyInfoDlg()
{
}

void CSteganographyInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INFO, m_infoText); // Привязка поля
}

BEGIN_MESSAGE_MAP(CSteganographyInfoDlg, CDialogEx)
END_MESSAGE_MAP()

// Инициализация окна справки
BOOL CSteganographyInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString infoText;
	infoText = _T("МЕТОДЫ СТЕГАНОГРАФИИ В ТЕКСТЕ\r\n");
	infoText += _T("========================================\r\n\r\n");
	infoText += _T("1. ЛИШНИЕ ПРОБЕЛЫ\r\n");
	infoText += _T("Скрытие информации за счет количества пробелов (например, 1 пробел = 0, 2 пробела = 1).\r\n\r\n");
	infoText += _T("2. НЕВИДИМЫЕ СИМВОЛЫ (Zero-Width Characters)\r\n");
	infoText += _T("Использование символов Unicode, которые не отображаются на экране (U+200B, U+200D и др.).\r\n\r\n");
	infoText += _T("3. ОМОГЛИФЫ\r\n");
	infoText += _T("Замена букв одного алфавита визуально похожими буквами другого (например, русская 'А' и английская 'A').\r\n");

	m_infoText.SetWindowText(infoText);

	return TRUE;
}