#ifdef WIN32
#include <windows.h>
#endif

#include <cstdio>

#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>
#include <boost/format.hpp>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#pragma comment(lib, "odbc32")
#endif

using namespace std;

namespace boost { namespace rdb { namespace odbc {

error::error(SQLSMALLINT handle_type, SQLHANDLE handle, long rc) : rc(rc) {
  sprintf(msg, "rc %ld", rc);
  SQLSMALLINT mlen;
  SQLGetDiagField(handle_type, handle, 1, SQL_DIAG_MESSAGE_TEXT, (SQLPOINTER) msg, sizeof msg, &mlen);
  cout << msg << endl;
}

database::~database() {
  close();
}

void database::open(const string& dsn, const string& user, const string& password) {

  dsn_ = dsn;
  user_ = user;
  password_ = password;
  henv_ = SQL_NULL_HANDLE;
  hdbc_ = SQL_NULL_HANDLE;

  sql_check(SQL_HANDLE_ENV, SQL_NULL_HANDLE, SQLAllocEnv(&henv_));

  //SQLSetEnvAttr(henv_, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

  sql_check(SQL_HANDLE_ENV, henv_, SQLAllocConnect(henv_, &hdbc_));

  //SQLSetConnectAttr(hdbc_, SQL_LOGIN_TIMEOUT, (SQLPOINTER*) 5, 0);

  sql_check(SQL_HANDLE_DBC, hdbc_,
    SQLConnect(hdbc_, (SQLCHAR*) dsn.c_str(), SQL_NTS,
      (SQLCHAR*) user.c_str(), SQL_NTS,
      (SQLCHAR*) password.c_str(), SQL_NTS));

  //  sql_check<Resource_Error>(hdbc_, SQLAllocHandle(SQL_HANDLE_STMT, hdbc_, &hstmt_));
  sql_check(SQL_HANDLE_DBC, hdbc_, SQLAllocStmt(hdbc_, &hstmt_));

  //SQLSMALLINT res;
  //SQLGetInfo(hdbc_, SQL_TXN_CAPABLE, &res, sizeof res, NULL);
}

void database::close() {
  if (hstmt_ != SQL_NULL_HANDLE)
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);

  if (hdbc_ != SQL_NULL_HANDLE) {
    SQLDisconnect(hdbc_);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
  }

  if (henv_ != SQL_NULL_HANDLE)
    SQLFreeHandle(SQL_HANDLE_ENV, henv_);
}

void database::exec_str(const string& sql) {
  //TR << sql << endl;
  sql_check(SQL_HANDLE_STMT, hstmt_, SQLExecDirect(hstmt_, (SQLCHAR*) sql.c_str(), SQL_NTS));
}

const char* error::what() const throw() {
  return msg;
}

} } }
