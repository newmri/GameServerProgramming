#include "CDB.h"

#include <iostream>

using namespace std;

CDB::CDB()
{
	m_hstmt = 0;
	for (int i = 0; i < MAX_DBINFO; ++i) m_cb[i] = 0;

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
				m_retcode = SQLConnect(m_hdbc, (SQLCHAR*)"ChessGameDB", SQL_NTS, (SQLCHAR*)NULL, 0, NULL, 0);
				// Allocate statement handle  
				if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
					cout << "DB has been connected!" << endl;
				}
				else cout << "DB Connection has been failed!" << endl;
			}
		}
	}



}


 DBInfo& CDB::Login(char* ID, char* PWD)
{
	DBInfo CInfo;
	char command[100];
	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);

	wsprintf(command, "EXEC dbo.Login '%s', '%s'", ID, PWD);
	m_retcode = SQLExecDirect(m_hstmt, (SQLCHAR*)command, SQL_NTS);

	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {

		m_retcode = SQLBindCol(m_hstmt, 1, SQL_CHAR, CInfo.ID, ID_LEN, &m_cb[0]);
		m_retcode = SQLBindCol(m_hstmt, 2, SQL_SMALLINT, &CInfo.m_Level, sizeof(CInfo.m_Level), &m_cb[1]);
		m_retcode = SQLBindCol(m_hstmt, 3, SQL_SMALLINT, &CInfo.m_HP, sizeof(CInfo.m_HP), &m_cb[2]);
		m_retcode = SQLBindCol(m_hstmt, 4, SQL_INTEGER, &CInfo.m_EXP, sizeof(CInfo.m_EXP), &m_cb[3]);
		m_retcode = SQLBindCol(m_hstmt, 5, SQL_SMALLINT, &CInfo.m_pos.m_wX, sizeof(CInfo.m_pos.m_wX), &m_cb[4]);
		m_retcode = SQLBindCol(m_hstmt, 6, SQL_SMALLINT, &CInfo.m_pos.m_wY, sizeof(CInfo.m_pos.m_wY), &m_cb[5]);
		m_retcode = SQLBindCol(m_hstmt, 7, SQL_SMALLINT, &CInfo.m_pos.m_wZone, sizeof(CInfo.m_pos.m_wZone), &m_cb[6]);
		m_retcode = SQLBindCol(m_hstmt, 8, SQL_SMALLINT, &CInfo.m_pos.m_wXZone, sizeof(CInfo.m_pos.m_wXZone), &m_cb[7]);
		m_retcode = SQLBindCol(m_hstmt, 9, SQL_SMALLINT, &CInfo.m_pos.m_wYZone, sizeof(CInfo.m_pos.m_wYZone), &m_cb[8]);
		m_retcode = SQLBindCol(m_hstmt, 10, SQL_SMALLINT, &CInfo.m_MAX_HP, sizeof(CInfo.m_MAX_HP), &m_cb[9]);
		m_retcode = SQLBindCol(m_hstmt, 11, SQL_SMALLINT, &CInfo.m_Damage, sizeof(CInfo.m_Damage), &m_cb[10]);
		m_retcode = SQLBindCol(m_hstmt, 12, SQL_INTEGER, &CInfo.m_MAX_EXP, sizeof(CInfo.m_MAX_EXP), &m_cb[11]);

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

const bool CDB::SignUp(char* ID, char* PWD)
{
	char command[100];
	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);

	wsprintf(command, "EXEC dbo.SignUp '%s', '%s'", ID, PWD);
	m_retcode = SQLExecDirect(m_hstmt, (SQLCHAR*)command, SQL_NTS);

	if (m_retcode == SQL_SUCCESS || m_retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(m_hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);

		return true;
	}

	return false;

}
void CDB::Update(const DBInfo a_Info)
{
	char command[100];

	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
	wsprintf(command, "EXEC dbo.UpdateUserData '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 
		a_Info.ID, a_Info.m_Level, a_Info.m_HP, a_Info.m_EXP, a_Info.m_pos.m_wX, a_Info.m_pos.m_wY,
		a_Info.m_pos.m_wZone, a_Info.m_pos.m_wXZone, a_Info.m_pos.m_wYZone, a_Info.m_MAX_HP, a_Info.m_Damage, a_Info.m_MAX_EXP);
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