// CLoginDlg.cpp: файл реализации
//

#include "pch.h"
#include "StegoApp.h"
#include "afxdialogex.h"
#include "CLoginDlg.h"
#include <sql.h>
#include <sqlext.h>
#include <afxcmn.h>

// ============ РЕАЛИЗАЦИЯ MD5 ============
struct MD5Context { uint32_t state[4]; uint32_t count[2]; uint8_t buffer[64]; };
static void MD5Init(MD5Context* context) { context->count[0] = context->count[1] = 0; context->state[0] = 0x67452301; context->state[1] = 0xefcdab89; context->state[2] = 0x98badcfe; context->state[3] = 0x10325476; }
static void MD5Transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    for (int i = 0, j = 0; j < 64; i++, j += 4) x[i] = ((uint32_t)block[j]) | (((uint32_t)block[j + 1]) << 8) | (((uint32_t)block[j + 2]) << 16) | (((uint32_t)block[j + 3]) << 24);
#define F(x,y,z) (((x) & (y)) | ((~x) & (z)))
#define G(x,y,z) (((x) & (z)) | ((y) & (~z)))
#define H(x,y,z) ((x) ^ (y) ^ (z))
#define I(x,y,z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x,n) (((x) << (n)) | ((x) >> (32-(n))))
#define FF(a,b,c,d,x,s,ac) { (a) += F((b),(c),(d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define GG(a,b,c,d,x,s,ac) { (a) += G((b),(c),(d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define HH(a,b,c,d,x,s,ac) { (a) += H((b),(c),(d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define II(a,b,c,d,x,s,ac) { (a) += I((b),(c),(d)) + (x) + (uint32_t)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
    FF(a, b, c, d, x[0], 7, 0xd76aa478); FF(d, a, b, c, x[1], 12, 0xe8c7b756); FF(c, d, a, b, x[2], 17, 0x242070db); FF(b, c, d, a, x[3], 22, 0xc1bdceee);
    FF(a, b, c, d, x[4], 7, 0xf57c0faf); FF(d, a, b, c, x[5], 12, 0x4787c62a); FF(c, d, a, b, x[6], 17, 0xa8304613); FF(b, c, d, a, x[7], 22, 0xfd469501);
    FF(a, b, c, d, x[8], 7, 0x698098d8); FF(d, a, b, c, x[9], 12, 0x8b44f7af); FF(c, d, a, b, x[10], 17, 0xffff5bb1); FF(b, c, d, a, x[11], 22, 0x895cd7be);
    FF(a, b, c, d, x[12], 7, 0x6b901122); FF(d, a, b, c, x[13], 12, 0xfd987193); FF(c, d, a, b, x[14], 17, 0xa679438e); FF(b, c, d, a, x[15], 22, 0x49b40821);
    GG(a, b, c, d, x[1], 5, 0xf61e2562); GG(d, a, b, c, x[6], 9, 0xc040b340); GG(c, d, a, b, x[11], 14, 0x265e5a51); GG(b, c, d, a, x[0], 20, 0xe9b6c7aa);
    GG(a, b, c, d, x[5], 5, 0xd62f105d); GG(d, a, b, c, x[10], 9, 0x02441453); GG(c, d, a, b, x[15], 14, 0xd8a1e681); GG(b, c, d, a, x[4], 20, 0xe7d3fbc8);
    GG(a, b, c, d, x[9], 5, 0x21e1cde6); GG(d, a, b, c, x[14], 9, 0xc33707d6); GG(c, d, a, b, x[3], 14, 0xf4d50d87); GG(b, c, d, a, x[8], 20, 0x455a14ed);
    GG(a, b, c, d, x[13], 5, 0xa9e3e905); GG(d, a, b, c, x[2], 9, 0xfcefa3f8); GG(c, d, a, b, x[7], 14, 0x676f02d9); GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);
    HH(a, b, c, d, x[5], 4, 0xfffa3942); HH(d, a, b, c, x[8], 11, 0x8771f681); HH(c, d, a, b, x[11], 16, 0x6d9d6122); HH(b, c, d, a, x[14], 23, 0xfde5380c);
    HH(a, b, c, d, x[1], 4, 0xa4beea44); HH(d, a, b, c, x[4], 11, 0x4bdecfa9); HH(c, d, a, b, x[7], 16, 0xf6bb4b60); HH(b, c, d, a, x[10], 23, 0xbebfbc70);
    HH(a, b, c, d, x[13], 4, 0x289b7ec6); HH(d, a, b, c, x[0], 11, 0xeaa127fa); HH(c, d, a, b, x[3], 16, 0xd4ef3085); HH(b, c, d, a, x[6], 23, 0x04881d05);
    HH(a, b, c, d, x[9], 4, 0xd9d4d039); HH(d, a, b, c, x[12], 11, 0xe6db99e5); HH(c, d, a, b, x[15], 16, 0x1fa27cf8); HH(b, c, d, a, x[2], 23, 0xc4ac5665);
    II(a, b, c, d, x[0], 6, 0xf4292244); II(d, a, b, c, x[7], 10, 0x432aff97); II(c, d, a, b, x[14], 15, 0xab9423a7); II(b, c, d, a, x[5], 21, 0xfc93a039);
    II(a, b, c, d, x[12], 6, 0x655b59c3); II(d, a, b, c, x[3], 10, 0x8f0ccc92); II(c, d, a, b, x[10], 15, 0xffeff47d); II(b, c, d, a, x[1], 21, 0x85845dd1);
    II(a, b, c, d, x[8], 6, 0x6fa87e4f); II(d, a, b, c, x[15], 10, 0xfe2ce6e0); II(c, d, a, b, x[6], 15, 0xa3014314); II(b, c, d, a, x[13], 21, 0x4e0811a1);
    II(a, b, c, d, x[4], 6, 0xf7537e82); II(d, a, b, c, x[11], 10, 0xbd3af235); II(c, d, a, b, x[2], 15, 0x2ad7d2bb); II(b, c, d, a, x[9], 21, 0xeb86d391);
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}
static void MD5Update(MD5Context* context, const uint8_t* input, size_t inputLen) {
    size_t i = 0, index = 0, partLen = 0; index = (context->count[0] >> 3) & 0x3F; partLen = 64 - index;
    context->count[0] += (uint32_t)(inputLen << 3); if (context->count[0] < (uint32_t)(inputLen << 3)) context->count[1]++; context->count[1] += (uint32_t)(inputLen >> 29);
    if (inputLen >= partLen) { memcpy(&context->buffer[index], input, partLen); MD5Transform(context->state, context->buffer); for (i = partLen; i + 63 < inputLen; i += 64) MD5Transform(context->state, &input[i]); index = 0; }
    else i = 0;
    memcpy(&context->buffer[index], &input[i], inputLen - i);
}
static void MD5Final(uint8_t digest[16], MD5Context* context) {
    static const uint8_t PADDING[64] = { 0x80 }; uint8_t bits[8]; size_t index, padLen;
    for (int i = 0; i < 8; i++) bits[i] = (uint8_t)(context->count[i >> 2] >> ((i & 3) << 3));
    index = (context->count[0] >> 3) & 0x3F; padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(context, PADDING, padLen); MD5Update(context, bits, 8);
    for (int i = 0; i < 4; i++) { digest[i * 4] = (uint8_t)(context->state[i] & 0xFF); digest[i * 4 + 1] = (uint8_t)((context->state[i] >> 8) & 0xFF); digest[i * 4 + 2] = (uint8_t)((context->state[i] >> 16) & 0xFF); digest[i * 4 + 3] = (uint8_t)((context->state[i] >> 24) & 0xFF); }
}
CString GetMD5(const CString& input) {
    CStringA inputA(input); MD5Context ctx; MD5Init(&ctx); MD5Update(&ctx, (const uint8_t*)(LPCSTR)inputA, inputA.GetLength());
    uint8_t digest[16]; MD5Final(digest, &ctx); CString result;
    for (int i = 0; i < 16; i++) { CString hex; hex.Format(_T("%02x"), digest[i]); result += hex; } return result;
}

// Диалоговое окно CLoginDlg

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent)
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_LOGIN, m_editLogin);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CLoginDlg::OnBnClickedBtnLogin)
END_MESSAGE_MAP()


// Обработчики сообщений CLoginDlg

void CLoginDlg::OnBnClickedBtnLogin()
{
    CString login, password;
    m_editLogin.GetWindowText(login);
    m_editPassword.GetWindowText(password);
    if (login.IsEmpty() || password.IsEmpty()) { AfxMessageBox(_T("Введите логин и пароль!")); return; }

    CString connStr = _T("DSN=StudentDB;Uid=SYSDBA;Pwd=masterkey;");
    SQLHENV hEnv = NULL; SQLHDBC hDbc = NULL; SQLHSTMT hStmt = NULL; SQLRETURN ret;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    ret = SQLDriverConnect(hDbc, NULL, (SQLWCHAR*)connStr.GetString(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(ret))
    {
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        CString hashedPassword = GetMD5(password);
        CString sql;
        sql.Format(_T("SELECT teacher_id, password_hash, is_active FROM Teachers WHERE login = '%s'"), (LPCTSTR)login);

        if (SQL_SUCCEEDED(SQLExecDirect(hStmt, (SQLWCHAR*)sql.GetString(), SQL_NTS)))
        {
            if (SQLFetch(hStmt) == SQL_SUCCESS)
            {
                SQLINTEGER teacherId, isActive; SQLLEN cbId, cbAct; SQLWCHAR dbHash[100] = { 0 }; SQLLEN cbHash;
                SQLGetData(hStmt, 1, SQL_C_SLONG, &teacherId, 0, &cbId);
                SQLGetData(hStmt, 2, SQL_C_WCHAR, dbHash, sizeof(dbHash), &cbHash);
                SQLGetData(hStmt, 3, SQL_C_SLONG, &isActive, 0, &cbAct);

                CString dbHashStr(dbHash); dbHashStr.Trim();

                if (dbHashStr.CompareNoCase(hashedPassword) == 0) {
                    if (isActive == 0) AfxMessageBox(_T("Доступ запрещен. Преподаватель уволен."));
                    else { m_loggedTeacherId = teacherId; CDialogEx::OnOK(); }
                }
                else AfxMessageBox(_T("Неверный пароль!"));
            }
            else AfxMessageBox(_T("Пользователь не найден!"));
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt); SQLDisconnect(hDbc);
    }
    else AfxMessageBox(_T("Ошибка подключения к DSN!"));

    SQLFreeHandle(SQL_HANDLE_DBC, hDbc); SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

