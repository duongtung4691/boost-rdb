#ifndef BOOST_ODBC_HPP
#define BOOST_ODBC_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

namespace boost { namespace rdb { namespace odbc {

  struct error : std::exception {
    error(SQLSMALLINT handle_type, SQLHANDLE handle, long rc);
    virtual const char* what() const throw();
    long rc;
    SQLCHAR stat[10]; // Status SQL
    SQLINTEGER err;
    char msg[200];
  };

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

  struct on_type { };
  const on_type on = on_type();

  struct off_type { };
  const off_type off = off_type();

  template<class Specific>
  class generic_database {
    Specific& spec() { return static_cast<Specific&>(*this); }

  public:
  };

  template<class SqlType, class Value, class Tag>
  struct sql_type_adapter;

  struct odbc_tag { };

  template<>
  struct sql_type_adapter<sql::integer, long, odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, long& value) {    
      SQLLEN n;
      SQLGetData(hstmt, col, SQL_C_LONG, &value, 0, &n);
      return n != SQL_NULL_DATA;
    }
  };

  template<int N>
  struct sql_type_adapter<sql::varchar<N>, std::string, odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, std::string& value) {
      char buf[N];
      SQLLEN n;
      SQLGetData(hstmt, col, SQL_C_CHAR, buf, sizeof buf, &n);
      if (n == SQL_NULL_DATA)
        return false;
      value.assign(buf, buf + n);
      return true;
    }
  };

  class database /*: public generic_database<database>*/ {
  public:
    database() { }
    ~database();

    database(const std::string& dsn, const std::string& user, const std::string& password) {
      open(dsn, user, password);
    }

    void open(const std::string& dsn, const std::string& user, const std::string& password);
    void close();
    
    template<class Stat>
    // why doesn't the line below work ?
    // BOOST_CONCEPT_REQUIRES(((Statement<Stat>)), (typename Stat::result))
    typename Stat::result
    execute(const Stat& st) { 
      return execute(typename Stat::tag(), st);
    }
    
    template<class Stat, class Tag>
    /*BOOST_CONCEPT_REQUIRES(((Statement<Stat>)), (typename Stat::result))*/
    void execute(Tag, const Stat& st) {
      exec_str(as_string(st));
    }

    template<class Select>
    typename Select::result
    execute(sql::select_statement_tag tag, const Select& select)
    {
      typename Select::result results;
      execute_select(select, results);
      return results; // optimize later
    }

    template<class Row>
    struct read_row {
      read_row(database& db, Row& row) : db_(db), row_(row), i_(0) { }
      database& db_;
      Row& row_;
      mutable int i_;

      template<class Expr, class Value>
      void operator ()(fusion::vector<Expr, Value&>& value) const {
        using namespace fusion;
        row_.set_null(i_, !sql_type_adapter<
          typename remove_reference<Expr>::type::sql_type,
          Value,
          odbc_tag
        >::get_data(db_.hstmt_, i_ + 1, at_c<1>(value)));
        ++i_;
      }
    };

    template<class Select, class ResultSet>
    void execute(const Select& select, ResultSet& results) {
      execute_select(select, results);
    }

    template<class Select, class ResultSet>
    void execute_select(const Select& select, ResultSet& results)
    {
      exec_str(as_string(select));

      typedef typename ResultSet::value_type row_type;
      typedef typename Select::select_list select_list;

      while (true) {
        int rc = SQLFetch(hstmt_);

        if (rc == SQL_NO_DATA)
          break;

        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
          throw error(SQL_HANDLE_STMT, hstmt_, rc);

        row_type row;
        typedef fusion::vector<const select_list&, typename row_type::value_vector_type&> zip;
        fusion::for_each(fusion::zip_view<zip>(zip(select.exprs(), row.values())),
          read_row<row_type>(*this, row));
        results.push_back(row);
      }
    }

    void exec_str(const std::string& sql);

    void set_autocommit(on_type) {
      SQLSetConnectOption(hdbc_, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
    }

    void set_autocommit(off_type) {
      SQLSetConnectOption(hdbc_, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
    }

    bool is_txn_capable() const {
      SQLSMALLINT res;
      SQLGetInfo(hdbc_, SQL_TXN_CAPABLE, &res, sizeof res, NULL);
      return res != SQL_TC_NONE;
    }

    void commit() {
      SQLTransact(henv_, hdbc_, SQL_COMMIT);
    }

    void rollback() {
      SQLTransact(henv_, hdbc_, SQL_ROLLBACK);
    }

  private:
    std::string dsn_, user_, password_;
    SQLHENV	henv_;
    SQLHDBC hdbc_;
    SQLHSTMT hstmt_;
  };

} } }

#endif // BOOST_ODBC_HPP
