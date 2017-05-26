#pragma once

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
	bool Login(char* ID, char* PWD);
public:
	CDB();
	~CDB();
private:
	SQLHENV m_henv;
	SQLHDBC m_hdbc;
	SQLHSTMT m_hstmt;
	SQLRETURN m_retcode;
	SQLWCHAR m_ID[ID_LEN], m_PWD[PWD_LEN];
	int m_pos_x, m_pos_y;
	SQLLEN m_cbID, m_cbPWD, m_cb_pos_x, m_cb_pos_y;
};