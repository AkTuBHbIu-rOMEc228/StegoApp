#include "pch.h"
#include "framework.h"
#include "StegoApp.h"
#include "StegoAppDlg.h"
#include "afxdialogex.h"
#include <sql.h>
#include <sqlext.h>
#include "CSteganographyInfoDlg.h"
#include "CAddWorkDlg.h"
#include "CHistoryDlg.h"
#include "CStatsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Функция для перевода русского текста в HEX (спасает от ошибок кодировки)
CString StringToHex(const CString& str)
{
	CStringA utf8(str);
	CString hex;
	for (int i = 0; i < utf8.GetLength(); i++)
	{
		CString temp;
		temp.Format(_T("%02X"), (unsigned char)utf8[i]);
		hex += temp;
	}
	return hex;
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
	enum { IDD = IDD_ABOUTBOX };
protected:
	virtual void DoDataExchange(CDataExchange* pDX) { CDialogEx::DoDataExchange(pDX); }
	DECLARE_MESSAGE_MAP()
};
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CStegoAppDlg::CStegoAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STEGOAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStegoAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TEXT, m_editText);
	DDX_Control(pDX, IDC_EDIT_DETAILS, m_editDetails);
	DDX_Control(pDX, IDC_COMBO_STUDENTS, m_comboStudents);
	DDX_Control(pDX, IDC_COMBO_WORKS, m_comboWorks);
	DDX_Control(pDX, IDC_CHECK_SPACES, m_checkSpaces);
	DDX_Control(pDX, IDC_CHECK_INVISIBLE, m_checkInvisible);
	DDX_Control(pDX, IDC_CHECK_HOMOGLYPHS, m_checkHomoglyphs);
	DDX_Control(pDX, IDC_BTN_LOAD_FROM_DB, m_btnLoad);
	DDX_Control(pDX, IDC_BTN_ANALYZE, m_btnAnalyze);
	DDX_Control(pDX, IDC_BTN_SAVE_DB, m_btnSave);
	DDX_Control(pDX, IDC_BTN_SHOW_INFO, m_btnShowInfo);
	DDX_Control(pDX, IDC_BTN_ADD_WORK, m_btnAddWork);
	DDX_Control(pDX, IDC_COMBO_TEMPLATES, m_comboTemplates);
	DDX_Control(pDX, IDC_BTN_HISTORY, m_btnHistory);
	DDX_Control(pDX, IDC_BTN_STATS, m_btnStats);
	DDX_Control(pDX, IDC_BTN_SAVE_TEMPLATE, m_btnSaveTemplate);
}

BEGIN_MESSAGE_MAP(CStegoAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_ANALYZE, &CStegoAppDlg::OnBnClickedBtnAnalyze)
	ON_BN_CLICKED(IDC_BTN_SAVE_DB, &CStegoAppDlg::OnBnClickedBtnSaveDb)
	ON_BN_CLICKED(IDC_BTN_LOAD_FROM_DB, &CStegoAppDlg::OnBnClickedBtnLoadFromDb)
	ON_BN_CLICKED(IDC_BTN_SHOW_INFO, &CStegoAppDlg::OnBnClickedBtnShowInfo)
	ON_CBN_SELCHANGE(IDC_COMBO_STUDENTS, &CStegoAppDlg::OnCbnSelchangeComboStudents)
	ON_BN_CLICKED(IDC_BTN_ADD_WORK, &CStegoAppDlg::OnBnClickedBtnAddWork)
	ON_CBN_SELCHANGE(IDC_COMBO_TEMPLATES, &CStegoAppDlg::OnCbnSelchangeComboTemplates)
	ON_BN_CLICKED(IDC_BTN_HISTORY, &CStegoAppDlg::OnBnClickedBtnHistory)
	ON_BN_CLICKED(IDC_BTN_STATS, &CStegoAppDlg::OnBnClickedBtnStats)
	ON_BN_CLICKED(IDC_BTN_SAVE_TEMPLATE, &CStegoAppDlg::OnBnClickedBtnSaveTemplate)
END_MESSAGE_MAP()

BOOL CStegoAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// === ЗАГРУЗКА СТУДЕНТОВ ===
	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
		CString sql = _T("SELECT DISTINCT s.student_id, CAST(s.last_name AS VARCHAR(50) CHARACTER SET UTF8), ")
			_T("CAST(s.first_name AS VARCHAR(50) CHARACTER SET UTF8), ")
			_T("CAST(g.name AS VARCHAR(20) CHARACTER SET UTF8) ")
			_T("FROM Students s ")
			_T("JOIN Groups g ON s.group_id = g.group_id ")
			_T("JOIN WrittenWorks w ON s.student_id = w.student_id ")  
			_T("JOIN Disciplines d ON w.discipline_id = d.discipline_id ");

		// Фильтр: только те студенты, у которых есть работы по дисциплинам, которые ведет этот преподаватель
		CString sqlFilter;
		sqlFilter.Format(_T("WHERE d.teacher_id = %d ORDER BY s.last_name"), m_teacherId);
		sql += sqlFilter;

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
				CString displayStr; displayStr.Format(_T("%s %s (%s)"), lastName, firstName, groupName);
				int index = m_comboStudents.AddString(displayStr);
				m_comboStudents.SetItemData(index, studentId);
			}
		}
		// === ЗАГРУЗКА ШАБЛОНОВ ===
		m_comboTemplates.ResetContent();
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

		CString sqlTemplates;
		sqlTemplates.Format(_T("SELECT template_id, CAST(template_name AS VARCHAR(50) CHARACTER SET UTF8) FROM CheckTemplates WHERE teacher_id = %d"), m_teacherId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sqlTemplates.GetString(), SQL_NTS)))
		{
			SQLINTEGER tId;
			SQLWCHAR tName[100] = { 0 };
			SQLLEN cbTId, cbTName;
			int idx = 0;

			while (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLGetData(hStmt, 1, SQL_C_SLONG, &tId, 0, &cbTId);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, tName, sizeof(tName), &cbTName);
				CString tDispStr(tName);

				int index = m_comboTemplates.AddString(tDispStr);
				// ВОТ ЭТА СТРОКА ПРИВЯЗЫВАЕТ ID К ШАБЛОНУ:
				m_comboTemplates.SetItemData(index, tId);
				idx++;
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		
		if (m_comboTemplates.GetCount() > 0)
		{
			m_comboTemplates.SetCurSel(0);
		}

		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

	return TRUE;
}

// === ЗАГРУЗКА РАБОТЫ ===
void CStegoAppDlg::OnBnClickedBtnLoadFromDb()
{
	int selIndex = m_comboWorks.GetCurSel();
	if (selIndex == CB_ERR) { AfxMessageBox(_T("Выберите работу!")); return; }
	int workId = (int)m_comboWorks.GetItemData(selIndex);

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
		sql.Format(_T("SELECT CAST(text_content AS VARCHAR(8000) CHARACTER SET UTF8) FROM WrittenWorks WHERE work_id = %d"), workId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			if (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLWCHAR buffer[8000] = { 0 };
				SQLLEN cbText;
				SQLGetData(hStmt, 1, SQL_C_WCHAR, buffer, sizeof(buffer), &cbText);
				CString text(buffer);
				text.Replace(_T("\r\n"), _T("\n"));
				text.Replace(_T("\n"), _T("\r\n"));
				m_editText.SetWindowText(text);
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

// === АНАЛИЗ ===
void CStegoAppDlg::OnBnClickedBtnAnalyze()
{
	CString text;
	m_editText.GetWindowText(text);

	if (text.IsEmpty())
	{
		AfxMessageBox(_T("Нет текста для анализа!"), MB_OK | MB_ICONWARNING);
		return;
	}

	CString spacesDetails, invisibleDetails, homoglyphsDetails;
	CString fullDetails = _T("ДЕТАЛЬНЫЙ АНАЛИЗ ТЕКСТА:\r\n================================\r\n\r\n");

	// Проверяем, какие галочки стоят, и запускаем только их
	bool hasSpaces = false;
	bool hasInvisible = false;
	bool hasHomoglyphs = false;

	if (m_checkSpaces.GetCheck() == BST_CHECKED)
	{
		hasSpaces = CheckExtraSpaces(text, spacesDetails);
		fullDetails += _T("ЛИШНИЕ ПРОБЕЛЫ:\r\n") + spacesDetails + _T("\r\n");
	}
	else
	{
		fullDetails += _T("ЛИШНИЕ ПРОБЕЛЫ: Проверка отключена\r\n\r\n");
		m_checkSpaces.SetCheck(BST_UNCHECKED); // На всякий случай
	}

	if (m_checkInvisible.GetCheck() == BST_CHECKED)
	{
		hasInvisible = CheckInvisibleChars(text, invisibleDetails);
		fullDetails += _T("НЕВИДИМЫЕ СИМВОЛЫ:\r\n") + invisibleDetails + _T("\r\n");
	}
	else
	{
		fullDetails += _T("НЕВИДИМЫЕ СИМВОЛЫ: Проверка отключена\r\n\r\n");
		m_checkInvisible.SetCheck(BST_UNCHECKED);
	}

	if (m_checkHomoglyphs.GetCheck() == BST_CHECKED)
	{
		hasHomoglyphs = CheckHomoglyphs(text, homoglyphsDetails);
		fullDetails += _T("ОМОГЛИФЫ:\r\n") + homoglyphsDetails + _T("\r\n");
	}
	else
	{
		fullDetails += _T("ОМОГЛИФЫ: Проверка отключена\r\n\r\n");
		m_checkHomoglyphs.SetCheck(BST_UNCHECKED);
	}

	// Обновляем галочки результата (показываем, что нашли, а что нет)
	m_checkSpaces.SetCheck(hasSpaces ? BST_CHECKED : BST_UNCHECKED);
	m_checkInvisible.SetCheck(hasInvisible ? BST_CHECKED : BST_UNCHECKED);
	m_checkHomoglyphs.SetCheck(hasHomoglyphs ? BST_CHECKED : BST_UNCHECKED);

	m_editDetails.SetWindowText(fullDetails);
}

// === СОХРАНЕНИЕ ===
void CStegoAppDlg::OnBnClickedBtnSaveDb()
{
	int selIndex = m_comboWorks.GetCurSel();
	if (selIndex == CB_ERR) { AfxMessageBox(_T("Выберите работу!")); return; }
	int workId = (int)m_comboWorks.GetItemData(selIndex);

	int spaces = m_checkSpaces.GetCheck() ? 1 : 0;
	int zw = m_checkInvisible.GetCheck() ? 1 : 0;
	int homo = m_checkHomoglyphs.GetCheck() ? 1 : 0;
	int total = spaces + zw + homo;

	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;Charset=UTF8;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

		int verdictId = (total > 0) ? 2 : 1;
		int levelId = 3;
		CTime now = CTime::GetCurrentTime();
		CString strDate = now.Format(_T("%Y-%m-%d %H:%M:%S"));

		// Формируем короткий вердикт для базы данных на основе результатов анализа
		CString detailsText;

		if (total == 0)
		{
			// Если ничего не найдено
			detailsText = _T("Проверка пройдена");
		}
		else if (total > 2)
		{
			// Если найдено много всего
			detailsText = _T("Множественные вставки");
		}
		else if (total == 1)
		{
			// Если найдено ровно что-то одно
			if (spaces) detailsText = _T("Найден скрытый пробел");
			else if (zw) detailsText = _T("Найдены скрытые символы");
			else if (homo) detailsText = _T("Найдены подозрительные символы");
		}
		else
		{
			// Если найдено два типа
			detailsText = _T("Найдены множественные скрытые символы");
		}

		// НАДЕЖНАЯ КОНВЕРТАЦИЯ В HEX
		// Переводим широкую строку (UTF-16) в UTF-8 байты, 
		// а затем каждый байт превращаем в HEX-символы.
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, detailsText, -1, NULL, 0, NULL, NULL);
		char* utf8Buf = new char[utf8Len];
		WideCharToMultiByte(CP_UTF8, 0, detailsText, -1, utf8Buf, utf8Len, NULL, NULL);

		CString hexDetails;
		for (int i = 0; i < utf8Len - 1; i++) // -1, чтобы не брать нулевой символ
		{
			CString temp;
			temp.Format(_T("%02X"), (unsigned char)utf8Buf[i]);
			hexDetails += temp;
		}
		delete[] utf8Buf;

		// Формируем запрос
		CString sqlQuery;
		sqlQuery.Format(_T("INSERT INTO StegoChecks (work_id, level_id, verdict_id, teacher_id, check_date, spaces_found, zw_found, homo_found, total_found, details) "
			"VALUES (%d, %d, %d, %d, '%s', %d, %d, %d, %d, CAST(X'%s' AS BLOB SUB_TYPE TEXT CHARACTER SET UTF8))"),
			workId, levelId, verdictId, m_teacherId, (LPCTSTR)strDate, spaces, zw, homo, total, (LPCTSTR)hexDetails);

		// Выполняем запрос
		ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlQuery.GetString(), SQL_NTS);

		if (SQL_SUCCEEDED(ret))
		{
			AfxMessageBox(_T("Сохранено!"));
		}
		else
		{
			SQLWCHAR sqlState[6], message[500];
			SQLSMALLINT msgLen;
			SQLINTEGER nativeError;
			SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, message, 500, &msgLen);
			CString errMsg;
			errMsg.Format(_T("Ошибка БД!\nКод: %s\nТекст: %s"), sqlState, message);
			AfxMessageBox(errMsg);
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	else
	{
		AfxMessageBox(_T("Не удалось подключиться к базе данных!"));
	}

	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

// === ВЫБОР СТУДЕНТА ===
void CStegoAppDlg::OnCbnSelchangeComboStudents()
{
	m_comboWorks.ResetContent();
	int selIndex = m_comboStudents.GetCurSel();
	if (selIndex == CB_ERR) return;
	int studentId = (int)m_comboStudents.GetItemData(selIndex);

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
		sql.Format(_T("SELECT work_id, CAST(title AS VARCHAR(200) CHARACTER SET UTF8) FROM WrittenWorks WHERE student_id = %d"), studentId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			SQLINTEGER workId;
			SQLWCHAR title[200] = { 0 };
			SQLLEN cbId, cbTitle;
			while (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLGetData(hStmt, 1, SQL_C_SLONG, &workId, 0, &cbId);
				SQLGetData(hStmt, 2, SQL_C_WCHAR, title, sizeof(title), &cbTitle);
				CString displayStr(title);
				int index = m_comboWorks.AddString(displayStr);
				m_comboWorks.SetItemData(index, workId);
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void CStegoAppDlg::OnBnClickedBtnShowInfo()
{
	CSteganographyInfoDlg dlg;
	dlg.DoModal();
}

void CStegoAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) CAboutDlg().DoModal();
	else CDialogEx::OnSysCommand(nID, lParam);
}

void CStegoAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect; GetClientRect(&rect);
		dc.DrawIcon((rect.Width() - cxIcon + 1) / 2, (rect.Height() - cyIcon + 1) / 2, m_hIcon);
	}
	else CDialogEx::OnPaint();
}

HCURSOR CStegoAppDlg::OnQueryDragIcon() { return static_cast<HCURSOR>(m_hIcon); }

// === ФУНКЦИИ АНАЛИЗА ===
bool CStegoAppDlg::CheckExtraSpaces(const CString& text, CString& details)
{
	details.Empty();
	bool found = false;
	int spaceCount = 0;

	for (int i = 0; i < text.GetLength(); i++)
	{
		if (text[i] == ' ')
			spaceCount++;
		else
		{
			if (spaceCount > 1)
			{
				found = true;
				details.AppendFormat(_T("  • Найдено %d пробелов в позиции %d\r\n"), spaceCount, i - spaceCount + 1);
			}
			spaceCount = 0;
		}

		if (text[i] == '\n' || i == text.GetLength() - 1)
		{
			if (spaceCount > 0)
			{
				found = true;
				details.AppendFormat(_T("  • Найдено %d пробелов в конце строки\r\n"), spaceCount);
			}
			spaceCount = 0;
		}
	}

	if (!found) details = _T("  Лишних пробелов не обнаружено\r\n");
	return found;
}

bool CStegoAppDlg::CheckInvisibleChars(const CString& text, CString& details)
{
	details.Empty();
	bool found = false;

	static const wchar_t invisibleChars[] = { 0x200B, 0x200C, 0x200D, 0xFEFF, 0x200E, 0x200F, 0x202A, 0x202B, 0x202C, 0x202D, 0x202E };
	static const int numChars = sizeof(invisibleChars) / sizeof(wchar_t);

	for (int i = 0; i < text.GetLength(); i++)
	{
		wchar_t ch = text[i];
		for (int j = 0; j < numChars; j++)
		{
			if (ch == invisibleChars[j])
			{
				found = true;
				static const struct { int code; LPCTSTR name; } names[] = {
					{0x200B, _T("Zero Width Space")}, {0x200C, _T("Zero Width Non-Joiner")},
					{0x200D, _T("Zero Width Joiner")}, {0xFEFF, _T("Zero Width No-Break Space")},
					{0x200E, _T("Left-to-Right Mark")}, {0x200F, _T("Right-to-Left Mark")}
				};
				LPCTSTR charName = _T("Неизвестный");
				for (int k = 0; k < 6; k++)
					if (ch == names[k].code) { charName = names[k].name; break; }

				details.AppendFormat(_T("  • Позиция %d: %s (U+%04X)\r\n"), i + 1, charName, ch);
				break;
			}
		}
	}

	if (!found) details = _T("  Невидимых символов не обнаружено\r\n");
	return found;
}

bool CStegoAppDlg::CheckHomoglyphs(const CString& text, CString& details)
{
	details.Empty();
	bool found = false;
	CString word;
	int wordStart = 0;

	static const wchar_t latinOmoglyphs[] = { L'A',L'B',L'E',L'H',L'K',L'M',L'O',L'P',L'T',L'X',L'Y',L'a',L'e',L'y',L'p',L'x',L'c',L'o' };
	static const wchar_t cyrillicOmoglyphs[] = { L'А',L'В',L'Е',L'Н',L'К',L'М',L'О',L'Р',L'Т',L'Х',L'У',L'а',L'е',L'р',L'у',L'х',L'с',L'о' };

	auto IsOmoglyph = [](wchar_t ch, const wchar_t* list, int size) {
		for (int i = 0; i < size; i++) if (ch == list[i]) return true;
		return false;
		};

	for (int i = 0; i <= text.GetLength(); i++)
	{
		if (i == text.GetLength() || text[i] == L' ' || text[i] == L'\n' || text[i] == L'\r')
		{
			if (!word.IsEmpty())
			{
				bool hasLatin = false, hasCyrillic = false;
				CString wordInfo;

				for (int j = 0; j < word.GetLength(); j++)
				{
					wchar_t ch = word[j];
					if ((ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z'))
					{
						hasLatin = true;
						wordInfo += IsOmoglyph(ch, latinOmoglyphs, 18) ? _T("[лат-омоглиф]") : _T("[лат]");
					}
					else if ((ch >= L'А' && ch <= L'Я') || (ch >= L'а' && ch <= L'я'))
					{
						hasCyrillic = true;
						wordInfo += IsOmoglyph(ch, cyrillicOmoglyphs, 18) ? _T("[кир-омоглиф]") : _T("[кир]");
					}
					else
						wordInfo += _T("[др]");
				}

				if (hasLatin && hasCyrillic)
				{
					found = true;
					details.AppendFormat(_T("  • Слово \"%s\" (позиция %d) содержит смешение алфавитов:\r\n       %s\r\n"),
						(LPCTSTR)word, wordStart + 1, (LPCTSTR)wordInfo);
				}
			}
			word.Empty();
			wordStart = i + 1;
		}
		else
			word += text[i];
	}

	if (!found) details = _T("  Омоглифов не обнаружено\r\n");
	return found;
}
void CStegoAppDlg::OnBnClickedBtnAddWork()
{
	CAddWorkDlg dlg;
	dlg.m_teacherId = m_teacherId; // Передаем ID преподавателя

	if (dlg.DoModal() == IDOK) {
		// Если работа добавлена, перезагрузим список студентов/работ
		m_comboStudents.ResetContent();
		OnInitDialog(); // перезагружаем данные
	}
}

void CStegoAppDlg::OnCbnSelchangeComboTemplates()
{
	int sel = m_comboTemplates.GetCurSel();
	if (sel == CB_ERR) return;

	// Получаем ID
	DWORD_PTR templateId = m_comboTemplates.GetItemData(sel);
	if (templateId == 0 || templateId == CB_ERR)
	{
		AfxMessageBox(_T("Ошибка: не удалось получить ID шаблона. Попробуйте перезагрузить программу."));
		return;
	}

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
		sql.Format(_T("SELECT check_spaces, check_zw, check_homo FROM CheckTemplates WHERE template_id = %d"), (int)templateId);

		if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
		{
			if (SQLFetch(hStmt) == SQL_SUCCESS)
			{
				SQLINTEGER sp, zw, homo;
				SQLLEN cbSp, cbZw, cbHomo;
				SQLGetData(hStmt, 1, SQL_C_SLONG, &sp, 0, &cbSp);
				SQLGetData(hStmt, 2, SQL_C_SLONG, &zw, 0, &cbZw);
				SQLGetData(hStmt, 3, SQL_C_SLONG, &homo, 0, &cbHomo);

				m_checkSpaces.SetCheck(sp ? BST_CHECKED : BST_UNCHECKED);
				m_checkInvisible.SetCheck(zw ? BST_CHECKED : BST_UNCHECKED);
				m_checkHomoglyphs.SetCheck(homo ? BST_CHECKED : BST_UNCHECKED);
			}
		}
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void CStegoAppDlg::OnBnClickedBtnHistory()
{
	CHistoryDlg dlg;
	dlg.m_teacherId = m_teacherId; // Передаем ID преподавателя
	dlg.DoModal();
}

void CStegoAppDlg::OnBnClickedBtnStats()
{
	CStatsDlg dlg;
	dlg.m_teacherId = m_teacherId; // Передаем ID преподавателя
	dlg.DoModal();
}

void CStegoAppDlg::OnBnClickedBtnSaveTemplate()
{
	// 1. Собираем текущие настройки (какие галочки стоят)
	int sp = m_checkSpaces.GetCheck() ? 1 : 0;
	int zw = m_checkInvisible.GetCheck() ? 1 : 0;
	int homo = m_checkHomoglyphs.GetCheck() ? 1 : 0;

	// 2. Формируем имя шаблона
	CTime now = CTime::GetCurrentTime();
	CString finalName;
	finalName.Format(_T("Свой шаблон (%s)"), now.Format(_T("%H:%M:%S")));

	// 3. Подключаемся к БД (оставляем Charset=UTF8)
	CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;Charset=UTF8;");
	SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

	ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(ret))
	{
		SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

		// 4. Подготавливаем SQL-запрос с параметрами (знаками вопроса)
		CString sql = _T("INSERT INTO CheckTemplates (teacher_id, template_name, check_spaces, check_zw, check_homo) "
			"VALUES (?, ?, ?, ?, ?)");

		ret = SQLPrepare(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS);

		if (SQL_SUCCEEDED(ret))
		{
			// 5. Привязываем параметры (SQLBindParameter)
			// Параметр 1: teacher_id
			SQLLEN cbTeacherId = 0;
			SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &m_teacherId, 0, &cbTeacherId);

			// Параметр 2: template_name (широкая строка)
			SQLWCHAR templateName[100] = { 0 };
			wcscpy_s(templateName, 100, finalName.GetString());
			SQLLEN cbTemplateName = SQL_NTS;
			SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 100, 0, templateName, 0, &cbTemplateName);

			// Параметр 3: check_spaces
			SQLLEN cbSp = 0;
			SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sp, 0, &cbSp);

			// Параметр 4: check_zw
			SQLLEN cbZw = 0;
			SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &zw, 0, &cbZw);

			// Параметр 5: check_homo
			SQLLEN cbHomo = 0;
			SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &homo, 0, &cbHomo);

			// 6. Выполняем запрос
			ret = SQLExecute(hStmt);

			if (SQL_SUCCEEDED(ret))
			{
				AfxMessageBox(_T("Шаблон успешно сохранен!\nОн появится в выпадающем списке."));
				// Перезагружаем окно, чтобы новый шаблон появился в списке
				OnInitDialog();
			}
			else
			{
				// Вытаскиваем точный текст ошибки из Firebird
				SQLWCHAR sqlState[6], message[500];
				SQLSMALLINT msgLen;
				SQLINTEGER nativeError;
				SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, message, 500, &msgLen);
				CString errMsg;
				errMsg.Format(_T("Ошибка БД (Execute)!\nКод: %s\nТекст: %s"), sqlState, message);
				AfxMessageBox(errMsg);
			}
		}
		else
		{
			// Ошибка на этапе подготовки запроса
			SQLWCHAR sqlState[6], message[500];
			SQLSMALLINT msgLen;
			SQLINTEGER nativeError;
			SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlState, &nativeError, message, 500, &msgLen);
			CString errMsg;
			errMsg.Format(_T("Ошибка БД (Prepare)!\nКод: %s\nТекст: %s"), sqlState, message);
			AfxMessageBox(errMsg);
		}

		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		SQLDisconnect(hDbc);
	}
	else
	{
		AfxMessageBox(_T("Не удалось подключиться к базе данных!"));
	}

	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}
// Финальная версия программы готова к защите