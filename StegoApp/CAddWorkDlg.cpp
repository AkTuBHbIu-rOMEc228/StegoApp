#include "pch.h"
#include "StegoApp.h"
#include "afxdialogex.h"
#include "CAddWorkDlg.h"
#include <sql.h>
#include <sqlext.h>
#include "CAddStudentDlg.h"

IMPLEMENT_DYNAMIC(CAddWorkDlg, CDialogEx)

CAddWorkDlg::CAddWorkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ADD_WORK_DIALOG, pParent)
{
}

CAddWorkDlg::~CAddWorkDlg()
{
}

void CAddWorkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_AW_STUDENT, m_comboStudent);
	DDX_Control(pDX, IDC_EDIT_AW_TITLE, m_editTitle);
	DDX_Control(pDX, IDC_EDIT_AW_TEXT, m_editText);
}

BEGIN_MESSAGE_MAP(CAddWorkDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddWorkDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_STUDENT, &CAddWorkDlg::OnBnClickedBtnAddNewStudent)
END_MESSAGE_MAP()

void CAddWorkDlg::LoadStudents()
{
	m_comboStudent.ResetContent();

	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

		// Загружаем ВСЕХ студентов (без фильтра по преподавателю, чтобы можно было добавить новую работу)
		CString sql = _T("SELECT s.student_id, CAST(s.last_name AS VARCHAR(50) CHARACTER SET UTF8), ")
			_T("CAST(s.first_name AS VARCHAR(50) CHARACTER SET UTF8), ")
			_T("CAST(g.name AS VARCHAR(20) CHARACTER SET UTF8) ")
			_T("FROM Students s JOIN Groups g ON s.group_id = g.group_id ")
			_T("ORDER BY s.last_name");

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			SQLINTEGER studentId;
			SQLWCHAR lastName[50] = { 0 }, firstName[50] = { 0 }, groupName[20] = { 0 };
			SQLLEN cbId, cbLast, cbFirst, cbGroup;
			while (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLGetData(hStmt, 1, SQL_C_SLONG, &studentId, 0, &cbId);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, lastName, sizeof(lastName), &cbLast);
				SQLGetData(hStmt, 3, SQL_C_WCHAR, firstName, sizeof(firstName), &cbFirst);
				SQLGetData(hStmt, 4, SQL_C_WCHAR, groupName, sizeof(groupName), &cbGroup);

				CString displayStr;
				displayStr.Format(_T("%s %s (%s)"), lastName, firstName, groupName);
				int index = m_comboStudent.AddString(displayStr);
				m_comboStudent.SetItemData(index, studentId);
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

	if (m_comboStudent.GetCount() > 0) m_comboStudent.SetCurSel(0);
}

BOOL CAddWorkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	LoadStudents(); // Загружаем студентов
	return TRUE;
}

void CAddWorkDlg::OnBnClickedOk()
{
    // 1. Получаем ID студента
    int sel = m_comboStudent.GetCurSel();
    if (sel == CB_ERR) {
        AfxMessageBox(_T("Выберите студента!"), MB_OK | MB_ICONWARNING);
        return;
    }
    int studentId = (int)m_comboStudent.GetItemData(sel);

    // 2. Получаем название работы и текст
    CString workTitle, workText;
    m_editTitle.GetWindowText(workTitle); 
    m_editText.GetWindowText(workText);   

    if (workTitle.IsEmpty()) {
        AfxMessageBox(_T("Введите название работы!"), MB_OK | MB_ICONWARNING);
        return;
    }

    // 3. Подключаемся к БД
    CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;Charset=UTF8;");
    SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(ret))
    {
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

        // 4. Подготавливаем запрос с параметрами (чтобы кириллица не ломалась)
        CString sql = _T("INSERT INTO WrittenWorks (student_id, discipline_id, title, text_content, submit_date) "
            "VALUES (?, ?, ?, ?, CURRENT_DATE)");

        ret = SQLPrepare(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS);

        if (SQL_SUCCEEDED(ret))
        {
            // Берем первую попавшуюся дисциплину из базы
            int disciplineId = 1; 

            SQLLEN cbStudentId = 0, cbDisciplineId = 0, cbTitle = SQL_NTS, cbText = SQL_NTS;

            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &studentId, 0, &cbStudentId);
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &disciplineId, 0, &cbDisciplineId);

            SQLWCHAR wTitle[200] = { 0 }, wText[8000] = { 0 };
            wcscpy_s(wTitle, 200, workTitle.GetString());
            wcscpy_s(wText, 8000, workText.GetString());

            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 200, 0, wTitle, 0, &cbTitle);
            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_LONGVARCHAR, 8000, 0, wText, 0, &cbText);

            ret = SQLExecute(hStmt);

            if (SQL_SUCCEEDED(ret))
            {
                AfxMessageBox(_T("Работа успешно добавлена!"), MB_OK | MB_ICONINFORMATION);
                CDialogEx::OnOK(); // Закрываем окно
            }
            else
            {
                
                SQLWCHAR sqlState[6], message[500];
                SQLSMALLINT msgLen;
                SQLINTEGER nativeError;
                SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, message, 500, &msgLen);
                CString errMsg;
                errMsg.Format(_T("Ошибка БД!\nКод: %s\nТекст: %s"), sqlState, message);
                AfxMessageBox(errMsg, MB_OK | MB_ICONERROR);
            }
        }
        else
        {
            AfxMessageBox(_T("Ошибка при подготовке запроса к БД!"), MB_OK | MB_ICONERROR);
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        SQLDisconnect(hDbc);
    }
    else
    {
        AfxMessageBox(_T("Не удалось подключиться к базе данных!"), MB_OK | MB_ICONERROR);
    }
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void CAddWorkDlg::OnBnClickedBtnAddNewStudent()
{
	CAddStudentDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		// Обновляем список студентов
		LoadStudents();

		// Ищем добавленного студента (по последнему ID или по имени)
		// Так как мы не возвращаем ID из диалога, просто выберем последнего в списке (нового)
		if (m_comboStudent.GetCount() > 0)
		{
			m_comboStudent.SetCurSel(m_comboStudent.GetCount() - 1); // Выбираем последнего добавленного
		}

		AfxMessageBox(_T("Студент успешно добавлен! Теперь он выбран в списке."));
	}
}