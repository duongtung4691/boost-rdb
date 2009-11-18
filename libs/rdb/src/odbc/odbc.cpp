#ifdef WIN32
#include <windows.h>
#endif

#include <cstdio>
#include <vector>

#include <boost/rdb/odbc.hpp>
#include <boost/format.hpp>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#pragma comment(lib, "odbc32")
#endif

using namespace std;

namespace boost { namespace rdb {

const char* dynamic_value_mismatch::what() const throw() {
  return "dynamic_value_mismatch";
}

} }

namespace boost { namespace rdb { namespace odbc {

odbc_error::odbc_error(SQLSMALLINT handle_type, SQLHANDLE handle, long rc) : rc(rc) {
  sprintf(msg, "rc %ld", rc);
  SQLSMALLINT mlen;
  SQLGetDiagField(handle_type, handle, 1, SQL_DIAG_MESSAGE_TEXT, (SQLPOINTER) msg, sizeof msg, &mlen);
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

void database::exec_str(HSTMT hstmt, const string& sql) {
  if (trace_stream)
    *trace_stream << sql << "\n";
  //TR << sql << endl;
  sql_check(SQL_HANDLE_STMT, hstmt, SQLExecDirect(hstmt, (SQLCHAR*) sql.c_str(), SQL_NTS));
}

void database::prepare_str(HSTMT hstmt, const string& sql) {
  if (trace_stream)
    *trace_stream << sql << "\n";
  //TR << sql << endl;
  sql_check(SQL_HANDLE_STMT, hstmt, SQLPrepare(hstmt, (SQLCHAR*) sql.c_str(), SQL_NTS));
}

const char* odbc_error::what() const throw() {
  return msg;
}

std::ostream* trace_stream;

} } }
