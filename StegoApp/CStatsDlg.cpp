#include "pch.h"
#include "StegoApp.h"
#include "CStatsDlg.h"
#include "afxdialogex.h"
#include <sql.h>
#include <sqlext.h>

IMPLEMENT_DYNAMIC(CStatsDlg, CDialogEx)

CStatsDlg::CStatsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STATS_DIALOG, pParent)
{
}

CStatsDlg::~CStatsDlg() {}

void CStatsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FILTER_STUDENT, m_comboFilterStudent);
	DDX_Control(pDX, IDC_COMBO_FILTER_GROUP, m_comboFilterGroup);
	DDX_Control(pDX, IDC_COMBO_FILTER_WORK, m_comboFilterWork);
	DDX_Control(pDX, IDC_DATETIME_FROM, m_dateFrom);
	DDX_Control(pDX, IDC_DATETIME_TO, m_dateTo);
}

BEGIN_MESSAGE_MAP(CStatsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_APPLY_FILTER, &CStatsDlg::OnBnClickedBtnApplyFilter)
END_MESSAGE_MAP()

BOOL CStatsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Даты по умолчанию (с 21.10.2026 по 19.11.2026)
	CTime timeFrom(2026, 10, 21, 0, 0, 0);
	CTime timeTo(2026, 11, 19, 23, 59, 59);

	m_dateFrom.SetTime(&timeFrom);
	m_dateTo.SetTime(&timeTo);

	LoadFilters();
	UpdateStats();

	return TRUE;
}

void CStatsDlg::LoadFilters()
{
	m_comboFilterStudent.ResetContent();
	m_comboFilterGroup.ResetContent();
	m_comboFilterWork.ResetContent();

	// Все студенты
	m_comboFilterStudent.AddString(_T("Все студенты"));
	m_comboFilterStudent.SetItemData(0, 0);

	// Все группы
	m_comboFilterGroup.AddString(_T("Все группы"));
	m_comboFilterGroup.SetItemData(0, 0);

	// Виды работ (строго 4 вида + "Все")
	m_comboFilterWork.AddString(_T("Все виды работ"));
	m_comboFilterWork.AddString(_T("Курсовая"));
	m_comboFilterWork.AddString(_T("Лабораторная"));
	m_comboFilterWork.AddString(_T("Реферат"));
	m_comboFilterWork.AddString(_T("Эссе"));

	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;Charset=UTF8;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

		// Студенты (только закрепленные за преподавателем)
		CString sqlSt;
		sqlSt.Format(_T("SELECT DISTINCT s.student_id, CAST(s.last_name AS VARCHAR(50) CHARACTER SET UTF8) || ' ' || CAST(s.first_name AS VARCHAR(50) CHARACTER SET UTF8) ")
			_T("FROM Students s ")
			_T("JOIN WrittenWorks w ON s.student_id = w.student_id ")
			_T("JOIN Disciplines d ON w.discipline_id = d.discipline_id ")
			_T("WHERE d.teacher_id = %d"), m_teacherId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sqlSt.GetString(), SQL_NTS))) {
			while (SQLFetch(hStmt) == SQL_SUCCESS) {
				SQLINTEGER id; SQLWCHAR name[100] = { 0 }; SQLLEN cbId, cbName;
				SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), &cbName);
				int idx = m_comboFilterStudent.AddString(name);
				m_comboFilterStudent.SetItemData(idx, id);
			}
		}
		SQLFreeStmt(hStmt, SQL_CLOSE);

		// Группы (только закрепленные)
		CString sqlGr;
		sqlGr.Format(_T("SELECT DISTINCT g.group_id, CAST(g.name AS VARCHAR(50) CHARACTER SET UTF8) ")
			_T("FROM Groups g ")
			_T("JOIN Students s ON g.group_id = s.group_id ")
			_T("JOIN WrittenWorks w ON s.student_id = w.student_id ")
			_T("JOIN Disciplines d ON w.discipline_id = d.discipline_id ")
			_T("WHERE d.teacher_id = %d"), m_teacherId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sqlGr.GetString(), SQL_NTS))) {
			while (SQLFetch(hStmt) == SQL_SUCCESS) {
				SQLINTEGER id; SQLWCHAR name[50] = { 0 }; SQLLEN cbId, cbName;
				SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, name, sizeof(name), &cbName);
				int idx = m_comboFilterGroup.AddString(name);
				m_comboFilterGroup.SetItemData(idx, id);
			}
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

	m_comboFilterStudent.SetCurSel(0);
	m_comboFilterGroup.SetCurSel(0);
	m_comboFilterWork.SetCurSel(0);
}

void CStatsDlg::UpdateStats()
{
	// 1. Получаем ID студента
	int studentId = 0;
	int selSt = m_comboFilterStudent.GetCurSel();
	if (selSt != CB_ERR) studentId = (int)m_comboFilterStudent.GetItemData(selSt);

	// 2. Получаем ID группы
	int groupId = 0;
	int selGr = m_comboFilterGroup.GetCurSel();
	if (selGr != CB_ERR) groupId = (int)m_comboFilterGroup.GetItemData(selGr);

	// 3. Получаем вид работы
	CString workType;
	int selWk = m_comboFilterWork.GetCurSel();
	if (selWk > 0) {
		m_comboFilterWork.GetLBText(selWk, workType);
	}

	// 4. Даты
	CTime fromTime, toTime;
	m_dateFrom.GetTime(fromTime);
	m_dateTo.GetTime(toTime);
	CString fromStr = fromTime.Format(_T("%Y-%m-%d 00:00:00"));
	CString toStr = toTime.Format(_T("%Y-%m-%d 23:59:59"));

	// 5. Запрос: получаем СПИСОК работ (а не COUNT/SUM)
	CString sql;
	sql.Format(_T("SELECT sc.check_id, sc.spaces_found, sc.zw_found, sc.homo_found, w.title ")
		_T("FROM StegoChecks sc ")
		_T("JOIN WrittenWorks w ON sc.work_id = w.work_id ")
		_T("JOIN Students s ON w.student_id = s.student_id ")
		_T("WHERE sc.check_date BETWEEN '%s' AND '%s' "), (LPCTSTR)fromStr, (LPCTSTR)toStr);

	// Фильтр по преподавателю
	CString condTeach; condTeach.Format(_T("AND sc.teacher_id = %d "), m_teacherId); sql += condTeach;

	if (studentId > 0) {
		CString cond; cond.Format(_T("AND s.student_id = %d "), studentId); sql += cond;
	}
	if (groupId > 0) {
		CString cond; cond.Format(_T("AND s.group_id = %d "), groupId); sql += cond;
	}

	// 6. Выполняем запрос и считаем статистику В C++
	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			int totalW = 0, totalS = 0, spaces = 0, zw = 0, homo = 0;
			CString title;

			while (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLINTEGER sp, zwF, ho;
				SQLWCHAR titleW[200] = { 0 };
				SQLLEN cbSp, cbZw, cbHo, cbTitle;

				SQLGetData(hStmt, 2, SQL_C_SLONG, &sp, 0, &cbSp);
				SQLGetData(hStmt, 3, SQL_C_SLONG, &zwF, 0, &cbZw);
				SQLGetData(hStmt, 4, SQL_C_SLONG, &ho, 0, &cbHo);
				SQLGetData(hStmt, 5, SQL_C_WCHAR, titleW, sizeof(titleW), &cbTitle);

				title = titleW;

				// Если выбран тип работы, но в названии его нет — пропускаем
				if (!workType.IsEmpty() && title.Find(workType) == -1) {
					continue;
				}

				totalW++;
				spaces += sp;
				zw += zwF;
				homo += ho;
				// Считаем уникальных студентов
				if (totalW == 1) totalS = 1;
			}

			CString str;
			str.Format(_T("%d"), totalW); SetDlgItemText(IDC_STATIC_TOTAL_WORKS, str);
			str.Format(_T("%d"), totalS); SetDlgItemText(IDC_STATIC_TOTAL_STUDENTS, str);
			str.Format(_T("%d"), spaces); SetDlgItemText(IDC_STATIC_TOTAL_SPACES, str);
			str.Format(_T("%d"), zw); SetDlgItemText(IDC_STATIC_TOTAL_ZW, str);
			str.Format(_T("%d"), homo); SetDlgItemText(IDC_STATIC_TOTAL_HOMO, str);
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void CStatsDlg::OnBnClickedBtnApplyFilter()
{
	UpdateStats();
}