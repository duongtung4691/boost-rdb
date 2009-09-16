#include <windows.h>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>
#include <boost/format.hpp>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

using namespace std;

namespace boost { namespace rdb { namespace odbc {

inline bool sql_fail(long rc) {
  return rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO;
}

inline void sql_check(SQLSMALLINT handle_type, SQLHANDLE handle, long rc) {
  if (sql_fail(rc)) {
    error ex(handle_type, handle, rc);
    //TR << "ODBC Exception: " << ex.what() << endl;
    throw ex;
  }
}

error::error(SQLSMALLINT handle_type, SQLHANDLE handle, long rc) : rc(rc) {
  sprintf(msg, "rc %d", rc);
  SQLSMALLINT mlen;
  SQLGetDiagField(handle_type, handle, 1, SQL_DIAG_MESSAGE_TEXT, (SQLPOINTER) msg, sizeof msg, &mlen);
  cout << msg << endl;
}

database::~database() {
  disconnect();
}

void database::connect(const string& dsn, const string& user, const string& password) {

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

void database::disconnect() {
  if (hstmt_ != SQL_NULL_HANDLE)
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);

  if (hdbc_ != SQL_NULL_HANDLE) {
    SQLDisconnect(hdbc_);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc_);
  }

  if (henv_ != SQL_NULL_HANDLE)
    SQLFreeHandle(SQL_HANDLE_ENV, henv_);
}

void database::execute(const string& sql) {
  //TR << sql << endl;
  sql_check(SQL_HANDLE_STMT, hstmt_, SQLExecDirect(hstmt_, (SQLCHAR*) sql.c_str(), SQL_NTS));
}

const char* error::what() const throw() {
  return msg;
}

} } }

#if 0

#include <sql.h>
#include <stdlib.h>
#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

#include <tangram/relational.hpp>

using namespace std;

namespace Tangram {

  namespace Relational {

    namespace ODBC {

      using namespace Relational;
      using namespace Trace;
      using namespace Utils;

      Connection_Error::Connection_Error(SQLSMALLINT handle_type, SQLHENV henv, long rc) : error(handle_type, henv, rc) {
      }

      Resource_Error::Resource_Error(SQLSMALLINT handle_type, SQLHENV henv, long rc) : error(handle_type, henv, rc) {
      }


      void database::create(const Schema* schema, const string& dsn, const string& user, const string& password) {
        start(schema);
        odbc_connect(dsn, user, password);
        odbc_create();
      }

      void database::create(const Schema* schema, const Specific_Mapping* mapping, const string& dsn, const string& user, const string& password) {
        start(schema, mapping);
        odbc_connect(dsn, user, password);
        odbc_create();
      }

      void database::create(Specific_Engine* engine, const string& dsn, const string& user, const string& password) {
        start(engine);
        odbc_connect(dsn, user, password);
        odbc_create();
      }

      void database::odbc_create() {
        SQLSMALLINT res;
        SQLGetInfo(hdbc_, SQL_TXN_CAPABLE, &res, sizeof res, NULL);

        if (tx_capable_ && res == SQL_TC_DML)
          SQLSetConnectOption(hdbc_, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);

        engine()->create(*this);

        if (tx_capable_ && res == SQL_TC_DML)
          SQLSetConnectOption(hdbc_, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
      }

      void database::connect(const Schema* schema, const string& dsn, const string& user, const string& password) {
        start(schema);
        odbc_connect(dsn, user, password);
      }

      void database::connect(const Schema* schema, const Specific_Mapping* mapping, const string& dsn, const string& user, const string& password) {
        start(schema, mapping);
        odbc_connect(dsn, user, password);
      }

      void database::connect(Specific_Engine* engine, const string& dsn, const string& user, const string& password) {
        start(engine);
        odbc_connect(dsn, user, password);
      }

      void database::clear(const string& dsn, const string& user, const string& password) {

        database database;

        database.odbc_connect(dsn, user, password);

        sql_check<error>(SQL_HANDLE_STMT, database.hstmt_, SQLTables(database.hstmt_, NULL, 0, (SQLCHAR*) "%", SQL_NTS, (SQLCHAR*) "%", SQL_NTS, (SQLCHAR*) "TABLE", SQL_NTS));
        char buf[1024];
        vector<string> tables;
        SQLLEN actual_length;
        sql_check<error>(SQL_HANDLE_STMT, database.hstmt_, SQLBindCol(database.hstmt_, 3, SQL_C_CHAR, (PTR) buf, sizeof buf, &actual_length));
        while (SQLFetch(database.hstmt_) == SQL_SUCCESS) {
          tables.push_back(buf);
        }
        sql_check<error>(SQL_HANDLE_STMT, database.hstmt_, SQLFreeStmt(database.hstmt_, SQL_CLOSE));
        vector<string>::const_iterator iter = tables.begin(), last = tables.end();
        if (database.tx_capable_)
          SQLSetConnectOption(database.hdbc_, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
        while (iter != last) {
          string sql = "DROP TABLE " + *iter++;
          TR << sql << endl;
          sql_check<error>(SQL_HANDLE_STMT, database.hstmt_, SQLExecDirect(database.hstmt_, (SQLCHAR*) sql.c_str(), SQL_NTS));
        }
      }

      void database::test() {
        try { sql_do("DROP TABLE Person"); } catch (...) { }
        sql_do("CREATE TABLE Person (name VARCHAR(255))");
        sql_do("INSERT INTO Person (name) VALUES ('Homer')");

        sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLPrepare(hstmt_, (SQLCHAR*) "SELECT name FROM Person", SQL_NTS));
        sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLExecute(hstmt_));
        
        SQLINTEGER length;
        sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLColAttribute(hstmt_, 1, SQL_COLUMN_LENGTH, NULL, 0, NULL, &length));

        /*
          sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLBindParameter(hstmt_, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, strlen(name[i]), 0, (PTR) name[i], strlen(name[i]), NULL));
        */
      }

      void database::sql_do(const string& sql) {
        TR << sql << endl;
        sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLExecDirect(hstmt_, (SQLCHAR*) sql.c_str(), SQL_NTS));
      }

      void database::sql_tx_begin() {
        TR << trace_this("ODBC::database", this) << " : begin tx" << endl;
      }

      void database::sql_tx_commit() {
        TR << trace_this("ODBC::database", this) << " : commit tx" << endl;
        SQLTransact(henv_, hdbc_, SQL_COMMIT);
      }

      void database::sql_tx_rollback() {
        TR << trace_this("ODBC::database", this) << " : rollback tx" << endl;
        SQLTransact(henv_, hdbc_, SQL_ROLLBACK);
      }
	  
      struct ODBC_Prepared_Statement : Prepared_Statement {

        ODBC_Prepared_Statement(database* database) : Prepared_Statement(database) { }

        vector< pair<int, int> > string_buffers;
        vector<SQLLEN*> length;

        virtual void bind_param(const int* value) {
          if (value) {
            TR << "ODBC_Prepared_Statement(" << this << "): bind param #" << param << " to " << value << " (" << *value << ")" << endl;
            sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLBindParameter(hstmt_, 1 + param++, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 10, 0, (PTR) value, sizeof(int), NULL));
          } else {
            TR << "ODBC_Prepared_Statement(" << this << "): bind param #" << param << " to NULL" << endl;
            static SQLINTEGER null = SQL_NULL_DATA;
            sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLBindParameter(hstmt_, 1 + param++, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 13, 6, NULL, 0, &null));
          }
        }

        virtual void bind_param(const char* value) {
          if (value) {
            TR << "ODBC_Prepared_Statement(" << this << "): bind param #" << param << " to " << (void*) value << " (" << value << ")" << endl;
            size_t length = strlen(value);
            sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLBindParameter(hstmt_, 1 + param++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, length, 0, (PTR) value, length, NULL));
          } else
            NOT_YET();
        }

        virtual void alloc_bind_col(int*) {
          PTR p = alloc_buffer(sizeof(long));
          buffers.push_back(p);
          length.push_back(new SQLLEN);
          TR << "ODBC_Prepared_Statement(" << this << "): bind col #" << col << " to " << p << endl;
          sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLBindCol(hstmt_, col + 1, SQL_C_LONG, p, sizeof(long), length.back()));
          ++col;
        }

        virtual void alloc_bind_col(bool*) {
          NOT_YET();
        }

        virtual void alloc_bind_col(char*) {
          string_buffers.push_back(pair<int, int>(col, buffers.size()));
          buffers.push_back(0);
          length.push_back(new SQLLEN);
          TR << "ODBC_Prepared_Statement(" << this << "): don't bind text col #" << col << " yet" << endl;
          ++col;
        }

        virtual void prepare(const string& sql) {
          TR << "prepare " << sql << " -> ODBC_Prepared_Statement(" << this << ")" << endl;
#ifdef _TANGRAM_TRACE_
          sql_ = sql;
#endif
          sql_check<error>(SQL_HANDLE_DBC, hstmt_, SQLPrepare(hstmt_, (SQLCHAR*) sql.c_str(), SQL_NTS));
        }

        virtual void execute() {
          TR << "execute ODBC_Prepared_Statement(" << this << ")" << endl;
          sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLExecute(hstmt_));
          vector< pair<int, int> >::const_iterator iter(string_buffers.begin());

          while (iter != string_buffers.end()) {
            SQLINTEGER n;
            sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLColAttribute(hstmt_, iter->first + 1, SQL_COLUMN_LENGTH, NULL, 0, NULL, &n));
            ++n;
            TR << "execute ODBC_Prepared_Statement(" << this << "): resize string buffer #" << iter->second << " to " << n << endl;
            vector<void*>::iterator p = buffers.begin() + iter->second;
            free_buffer(*p);
            *p = alloc_buffer(n);
            TR << "ODBC_Prepared_Statement(" << this << "): bind col #" << iter->first << " to " << *p << endl;
            sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLBindCol(hstmt_, iter->first + 1, SQL_C_CHAR, (PTR) *p, n, length[iter->second]));
            ++iter;
          }
        }

        virtual bool fetch_row() {
          TR << "fetch from ODBC_Prepared_Statement(" << this << ")" << endl;
          col = 0;
          SQLRETURN rc = SQLFetch(hstmt_);

          if (rc == SQL_NO_DATA) {
            SQLCloseCursor(hstmt_);
            return false;
          }

          sql_check<error>(SQL_HANDLE_STMT, hstmt_, rc);
          return true;
        }

        virtual bool is_null() const {
          return *length[col] == SQL_NULL_DATA;
        }

        virtual void close() {
          TR << "close ODBC_Prepared_Statement(" << this << ")" << endl;
          SQLCloseCursor(hstmt_);
        }

        virtual int affected_rows() const {
          SQLINTEGER rows;
          SQLRowCount(hstmt_, &rows);
          TR << "ODBC_SQLGetDiagRec" << endl;
          SQLCHAR state[10], error[100];
          SQLINTEGER native_error;
          SQLSMALLINT text_length;
          sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt_, 1, state, &native_error, error, sizeof error - 1, &text_length));
          TR << "ODBC_SQLGetDiagField(SQL_DIAG_ROW_COUNT)" << flush;
          sql_check<error>(SQL_HANDLE_STMT, hstmt_, SQLGetDiagField(SQL_HANDLE_STMT, hstmt_, 1, SQL_DIAG_ROW_COUNT, &rows, 0, 0));
          TR << " -> " << rows << endl;
          return rows; 
        }

        ~ODBC_Prepared_Statement();

        SQLHSTMT hstmt_;
      };

      ODBC_Prepared_Statement::~ODBC_Prepared_Statement() {
        if (hstmt_ != SQL_NULL_HANDLE)
          SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
        for_each(length.rbegin(), length.rend(), delete_ptr<SQLLEN>);
      }

      Prepared_Statement* database::make_statement() {
        auto_ptr<ODBC_Prepared_Statement> statement(new ODBC_Prepared_Statement(this));
        statement->hstmt_ = SQL_NULL_HANDLE;
        sql_check<Resource_Error>(SQL_HANDLE_DBC, hdbc_, SQLAllocStmt(hdbc_, &statement->hstmt_));
        return statement.release();
        
      }
    }
  }
}

#endif