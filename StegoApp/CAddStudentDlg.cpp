#include "pch.h"
#include "StegoApp.h"
#include "CAddStudentDlg.h"
#include "afxdialogex.h"
#include <sql.h>
#include <sqlext.h>

IMPLEMENT_DYNAMIC(CAddStudentDlg, CDialogEx)

// КОНСТРУКТОР (здесь ничего не трогаем с контролами!)
CAddStudentDlg::CAddStudentDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ADD_STUDENT_DIALOG, pParent)
{
}

CAddStudentDlg::~CAddStudentDlg()
{
}

// ПРИВЯЗКА ПЕРЕМЕННЫХ (DDX)
void CAddStudentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LASTNAME, m_editLastName);
	DDX_Control(pDX, IDC_EDIT_FIRSTNAME, m_editFirstName);
	DDX_Control(pDX, IDC_EDIT_MIDDLENAME, m_editMiddleName);
	DDX_Control(pDX, IDC_EDIT_EMAIL, m_editEmail);
	DDX_Control(pDX, IDC_COMBO_GROUP, m_comboGroup);
}

BEGIN_MESSAGE_MAP(CAddStudentDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddStudentDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// ИНИЦИАЛИЗАЦИЯ ДИАЛОГА
BOOL CAddStudentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Загружаем список групп из БД
	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
		CString sql = _T("SELECT group_id, CAST(name AS VARCHAR(50) CHARACTER SET UTF8) FROM Groups");

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			SQLINTEGER gId;
			SQLWCHAR gName[50] = { 0 };
			SQLLEN cbId, cbName;
			int idx = 0;
			while (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLGetData(hStmt, 1, SQL_C_SLONG, &gId, 0, &cbId);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, gName, sizeof(gName), &cbName);

				int index = m_comboGroup.AddString(gName);
				m_comboGroup.SetItemData(index, gId);
				idx++;
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

	return TRUE;
}
void CAddStudentDlg::OnBnClickedOk()
{
    // 1. Получаем данные из полей
    CString lastName, firstName, middleName, email;
    m_editLastName.GetWindowText(lastName);
    m_editFirstName.GetWindowText(firstName);
    m_editMiddleName.GetWindowText(middleName);
    m_editEmail.GetWindowText(email);

    // 2. Проверяем, что фамилия и имя не пустые
    if (lastName.IsEmpty() || firstName.IsEmpty())
    {
        AfxMessageBox(_T("Пожалуйста, заполните фамилию и имя!"), MB_OK | MB_ICONWARNING);
        return;
    }

    // 3. Получаем ID выбранной группы
    int sel = m_comboGroup.GetCurSel();
    if (sel == CB_ERR) {
        AfxMessageBox(_T("Пожалуйста, выберите группу!"), MB_OK | MB_ICONWARNING);
        return;
    }
    int groupId = (int)m_comboGroup.GetItemData(sel);

    // 4. Подключаемся к БД и вставляем
    CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;Charset=UTF8;");
    SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(ret))
    {
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

        // Используем параметры, чтобы не было проблем с русским языком
        CString sql = _T("INSERT INTO Students (group_id, last_name, first_name, middle_name, email) "
            "VALUES (?, ?, ?, ?, ?)");

        ret = SQLPrepare(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS);

        if (SQL_SUCCEEDED(ret))
        {
            SQLLEN cbGroupId = 0, cbLast = SQL_NTS, cbFirst = SQL_NTS, cbMid = SQL_NTS, cbEmail = SQL_NTS;

            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &groupId, 0, &cbGroupId);

            SQLWCHAR wLast[100] = { 0 }, wFirst[100] = { 0 }, wMid[100] = { 0 }, wEmail[100] = { 0 };
            wcscpy_s(wLast, 100, lastName.GetString());
            wcscpy_s(wFirst, 100, firstName.GetString());
            wcscpy_s(wMid, 100, middleName.GetString());
            wcscpy_s(wEmail, 100, email.GetString());

            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0, wLast, 0, &cbLast);
            SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0, wFirst, 0, &cbFirst);
            SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0, wMid, 0, &cbMid);
            SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0, wEmail, 0, &cbEmail);

            ret = SQLExecute(hStmt);

            if (SQL_SUCCEEDED(ret))
            {
                // Успех! Закрываем диалог с результатом IDOK
                CDialogEx::OnOK();
            }
            else
            {
                AfxMessageBox(_T("Ошибка при добавлении студента в базу!"), MB_OK | MB_ICONERROR);
            }
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