#pragma once

#include "DBClientInfo.h"
#include <windows.h>  
#include <sqlext.h>  
#include <iostream>

using namespace std;

#define UNICODE  
#define ID_LEN 10  
#define PWD_LEN 10  


class CDB
{

public:
	void Connect();
	void Release();

public:
	const DBInfo Login(char* ID, char* PWD);
	void Update(const DBInfo a_Info);
public:
	CDB();
	~CDB();
private:
	SQLHENV m_henv;
	SQLHDBC m_hdbc;
	SQLHSTMT m_hstmt;
	SQLRETURN m_retcode;
	SQLCHAR m_ID[ID_LEN], m_PWD[PWD_LEN];

	SQLLEN m_cbID, m_cbPWD, m_cb_pos_x, m_cb_pos_y;
};