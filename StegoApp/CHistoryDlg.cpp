#include "pch.h"
#include "StegoApp.h"
#include "afxdialogex.h"
#include "CHistoryDlg.h"
#include <sql.h>
#include <sqlext.h>

IMPLEMENT_DYNAMIC(CHistoryDlg, CDialogEx)

CHistoryDlg::CHistoryDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HISTORY_DIALOG, pParent)
{
}

CHistoryDlg::~CHistoryDlg() {}

void CHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HISTORY, m_listHistory);
}

BEGIN_MESSAGE_MAP(CHistoryDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CHistoryDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Настраиваем колонки
	m_listHistory.InsertColumn(0, _T("Дата проверки"), LVCFMT_LEFT, 135);
	m_listHistory.InsertColumn(1, _T("Студент"), LVCFMT_LEFT, 140);
	m_listHistory.InsertColumn(2, _T("Работа"), LVCFMT_LEFT, 140);
	m_listHistory.InsertColumn(3, _T("Пробелы"), LVCFMT_CENTER, 80);
	m_listHistory.InsertColumn(4, _T("Невидимые"), LVCFMT_CENTER, 100);
	m_listHistory.InsertColumn(5, _T("Омоглифы"), LVCFMT_CENTER, 100);
	m_listHistory.InsertColumn(6, _T("Итого"), LVCFMT_CENTER, 70);

	m_listHistory.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// Подключаемся к БД
	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

		CString sql;
		// Увеличили VARCHAR(20) до VARCHAR(30), чтобы дата помещалась!
		sql.Format(_T("SELECT CAST(sc.check_date AS VARCHAR(30)), ")
			_T("CAST(s.last_name AS VARCHAR(50) CHARACTER SET UTF8) || ' ' || CAST(s.first_name AS VARCHAR(50) CHARACTER SET UTF8), ")
			_T("CAST(w.title AS VARCHAR(100) CHARACTER SET UTF8), ")
			_T("sc.spaces_found, sc.zw_found, sc.homo_found, sc.total_found ")
			_T("FROM StegoChecks sc ")
			_T("JOIN WrittenWorks w ON sc.work_id = w.work_id ")
			_T("JOIN Students s ON w.student_id = s.student_id ")
			_T("WHERE sc.teacher_id = %d ") // ВЕРНУЛИ ФИЛЬТР
			_T("ORDER BY sc.check_date DESC"), m_teacherId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			int row = 0;
			while (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLWCHAR date[30] = { 0 }, studentName[100] = { 0 }, title[150] = { 0 };
				SQLINTEGER sp, zw, homo, tot;
				SQLLEN cbDate, cbStudent, cbTitle, cbSp, cbZw, cbHomo, cbTot;

				SQLGetData(hStmt, 1, SQL_C_WCHAR, date, sizeof(date), &cbDate);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, studentName, sizeof(studentName), &cbStudent);
				SQLGetData(hStmt, 3, SQL_C_WCHAR, title, sizeof(title), &cbTitle);
				SQLGetData(hStmt, 4, SQL_C_SLONG, &sp, 0, &cbSp);
				SQLGetData(hStmt, 5, SQL_C_SLONG, &zw, 0, &cbZw);
				SQLGetData(hStmt, 6, SQL_C_SLONG, &homo, 0, &cbHomo);
				SQLGetData(hStmt, 7, SQL_C_SLONG, &tot, 0, &cbTot);

				m_listHistory.InsertItem(row, date);
				m_listHistory.SetItemText(row, 1, studentName);
				m_listHistory.SetItemText(row, 2, title);

				CString strSp, strZw, strHomo, strTot;
				strSp.Format(_T("%d"), sp);
				strZw.Format(_T("%d"), zw);
				strHomo.Format(_T("%d"), homo);
				strTot.Format(_T("%d"), tot);

				m_listHistory.SetItemText(row, 3, strSp);
				m_listHistory.SetItemText(row, 4, strZw);
				m_listHistory.SetItemText(row, 5, strHomo);
				m_listHistory.SetItemText(row, 6, strTot);
				row++;
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

	return TRUE;
}