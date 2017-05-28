#pragma once

#include <Windows.h>
#include <sqlext.h>  
#include "DBInfo.h"

#define UNICODE
#define MAX_DBINFO 12

class CDB
{

public:
	void Connect();
	void Release();

public:
	DBInfo& Login(char* ID, char* PWD);
	const bool SignUp(char* ID, char* PWD);
	void Update(const DBInfo a_Info);
public:
	CDB();
	~CDB();
private:
	SQLHENV m_henv;
	SQLHDBC m_hdbc;
	SQLHSTMT m_hstmt;
	SQLRETURN m_retcode;

	SQLLEN m_cb[MAX_DBINFO];
};