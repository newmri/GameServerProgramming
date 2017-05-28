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
				m_retcode = SQLConnect(m_hdbc, (SQLCHAR*)"GAME_DB_2014180050", SQL_NTS, (SQLCHAR*)NULL, 0, NULL, 0);
				// Allocate statement handle  
				if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "DB has been connected!" << endl;
				}
			}
		}
	}



}


const DBInfo CDB::Login(char* ID, char* PWD)
{
	DBInfo CInfo;
	char command[100];

	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
	
	wsprintf(command, "EXEC dbo.Login '%s', '%s'", ID, PWD);
	m_retcode = SQLExecDirect(m_hstmt, (SQLCHAR*)command, SQL_NTS);

	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {

		m_retcode = SQLBindCol(m_hstmt, 1, SQL_CHAR, &CInfo.ID, ID_LEN, &m_cbID);
		m_retcode = SQLBindCol(m_hstmt, 2, SQL_INTEGER, &CInfo.Pos_X, sizeof(int), &m_cb_pos_x);
		m_retcode = SQLBindCol(m_hstmt, 3, SQL_INTEGER, &CInfo.Pos_Y, sizeof(int), &m_cb_pos_y);
		m_retcode = SQLFetch(m_hstmt);
		if (m_retcode == SQL_NO_DATA_FOUND) return CInfo;
		
		if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
			SQLCancel(m_hstmt);
			SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
			return CInfo;
		}
	}
	return CInfo;
}


void CDB::Update(const DBInfo a_Info)
{
	char command[100];

	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
	wsprintf(command, "EXEC dbo.UpdateUserData '%s', %d, %d", a_Info.ID, a_Info.Pos_X, a_Info.Pos_Y);
	m_retcode = SQLExecDirect(m_hstmt, (SQLCHAR*)command, SQL_NTS);

	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(m_hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
	}

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