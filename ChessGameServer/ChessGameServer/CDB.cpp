#include "CDB.h"

CDB::CDB()
{
	m_hstmt = 0;
	m_cbID = 0, m_cbPWD = 0, m_cb_pos_x = 0, m_cb_pos_y = 0;

}

CDB::~CDB() { Release(); }

void CDB::Connect()
{
	// Allocate environment handle  
	m_retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);

	// Set the ODBC version environment attribute  
	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
		m_retcode = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
			m_retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc);

			// Set login timeout to 5 seconds  
			if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(m_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				m_retcode = SQLConnect(m_hdbc, (SQLCHAR*)"GAME_DB", SQL_NTS, (SQLCHAR*)NULL, 0, NULL, 0);
				// Allocate statement handle  
				if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "DB has been connected!" << endl;
				}
			}
		}
	}



}


bool CDB::Login(char* ID, char* PWD)
{
	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);

	char command[100];
	wsprintf(command, "EXEC dbo.Login '%s', '%s'", ID, PWD);
	m_retcode = SQLExecDirect(m_hstmt, (SQLCHAR*)command, SQL_NTS);

	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {

		m_retcode = SQLBindCol(m_hstmt, 1, SQL_WCHAR, &m_ID, ID_LEN, &m_cbID);
		m_retcode = SQLBindCol(m_hstmt, 2, SQL_INTEGER, &m_pos_x, sizeof(int), &m_cbID);
		m_retcode = SQLBindCol(m_hstmt, 3, SQL_INTEGER, &m_pos_y, sizeof(int), &m_cbID);
		m_retcode = SQLFetch(m_hstmt);
		if (m_retcode == SQL_NO_DATA_FOUND) {
			return false;
		}
		if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
			printf("ssss");
			SQLCancel(m_hstmt);
			SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
			return true;
		}
	}
	return false;
}
void CDB::Release()
{
	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(m_hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
	}

	SQLDisconnect(m_hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, m_henv);

}