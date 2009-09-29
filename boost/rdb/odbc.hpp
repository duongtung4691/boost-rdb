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
    execute(select_statement_tag tag, const Select& select)
    {
      typename Select::result results;
      execute_select(select, results);
      return results; // optimize later
    }

    struct read_row {
      read_row(database& db) : db_(db), i_(1) { }
      database& db_;
      mutable int i_;

      template<class Expr>
      void operator ()(fusion::vector<Expr, long&> value) const {
        using namespace fusion;
        SQLLEN n;
        SQLGetData(db_.hstmt_, i_++, SQL_C_LONG, &at_c<1>(value), 0, &n);
      }

      template<class Expr>
      void operator ()(fusion::vector<Expr, std::string&> value) const {
        using namespace fusion;
        char buf[remove_reference<Expr>::type::sql_type::size];
        SQLLEN n;
        SQLGetData(db_.hstmt_, i_++, SQL_C_CHAR, buf, sizeof buf, &n);
        &at_c<1>(value).assign(buf, buf + n);
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
        typedef fusion::vector<const select_list&, row_type&> zip;
        fusion::for_each(fusion::zip_view<zip>(zip(select.exprs(), row)), read_row(*this));
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
