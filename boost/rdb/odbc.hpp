#ifndef BOOST_ODBC_HPP
#define BOOST_ODBC_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/fusion/include/zip_view.hpp>

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

  class database;

  template<class Row>
  struct read_row {
    read_row(SQLHSTMT hstmt, Row& row) : hstmt_(hstmt), row_(row), i_(0) { }
    SQLHSTMT hstmt_;
    Row& row_;
    mutable int i_;

    template<class Expr, class Value>
    void operator ()(fusion::vector<Expr, Value&>& value) const {
      using namespace fusion;
      row_.set_null(i_, !sql_type_adapter<
        typename remove_reference<Expr>::type::sql_type,
        Value,
        odbc_tag
      >::get_data(hstmt_, i_ + 1, at_c<1>(value)));
      ++i_;
    }
  };

  template<class Select>
  class prepared_select_statement;

  template<class Statement>
  class prepared_statement;

  template<class ExprList, class Container, bool Own>
  class result_set;

  class database /*: public generic_database<database>*/ {
  public:
    database() { }
    ~database();

    database(const std::string& dsn, const std::string& user, const std::string& password) {
      open(dsn, user, password);
    }

    void open(const std::string& dsn, const std::string& user, const std::string& password);
    void close();

    template<class Tag, class Stat>
    struct discriminate {

      typedef typename Stat::result execute_return_type;

      static execute_return_type execute(database& db, const Stat& stat) {
        return db.exec_str(as_string(stat));
      }

      typedef prepared_statement<Stat> prepare_return_type;

      static prepare_return_type prepare(database& db, const Stat& st) {
        HSTMT hstmt;
        sql_check(SQL_HANDLE_DBC, db.hdbc_, SQLAllocStmt(db.hdbc_, &hstmt));
        db.prepare_str(hstmt, as_string(select));
        return prepare_return_type(hstmt);
      }

    };

    template<class Select>
    struct discriminate<sql::select_statement_tag, Select> {

      typedef result_set<typename Select::select_list, typename Select::result, false> execute_return_type;

      static execute_return_type execute(database& db, const Select& select) {
        HSTMT hstmt;
        sql_check(SQL_HANDLE_DBC, db.hdbc_, SQLAllocStmt(db.hdbc_, &hstmt));
        db.exec_str(hstmt, as_string(select));
        return execute_return_type(hstmt);
      }

      typedef prepared_select_statement<Select> prepare_return_type;

      static prepare_return_type prepare(database& db, const Select& select) {
        HSTMT hstmt;
        sql_check(SQL_HANDLE_DBC, db.hdbc_, SQLAllocStmt(db.hdbc_, &hstmt));
        db.prepare_str(hstmt, as_string(select));
        return prepare_return_type(hstmt);
      }
    };
    
    template<class Stat>
    // why doesn't the line below work ?
    // BOOST_CONCEPT_REQUIRES(((Statement<Stat>)), (typename discriminate<typename Stat::tag, Stat>::execute_return_type))
    typename discriminate<typename Stat::tag, Stat>::execute_return_type
    execute(const Stat& st) { 
      // error "tag is not a member..." probably means that you tried to execute a statement that is not complete, e.g. `insert(t)`
      return discriminate<typename Stat::tag, Stat>::execute(*this, st);
    }
    
    template<class Stat>
    typename discriminate<typename Stat::tag, Stat>::prepare_return_type
    prepare(const Stat& st) { 
      // error "tag is not a member..." probably means that you tried to prepare a statement that is not complete, e.g. `insert(t)`
      return discriminate<typename Stat::tag, Stat>::prepare(*this, st);
    }

    void prepare_str(HSTMT hstmt, const std::string& sql);
    void exec_str(HSTMT hstmt, const std::string& sql);
    void exec_str(const std::string& sql) { return exec_str(hstmt_, sql); }

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

    //template<class Expr, class Container> friend class result_set;
  };

  template<class Statement>
  class prepared_statement {

    SQLHSTMT hstmt_;
  
  public:
    prepared_statement(SQLHSTMT hstmt) : hstmt_(hstmt) { }

    ~prepared_statement() {
      SQLCloseCursor(hstmt_);
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
    }

  };

  template<class Select>
  class prepared_select_statement {

    SQLHSTMT hstmt_;
  
  public:
    typedef typename Select::select_list select_list;
    typedef typename Select::result container;

    prepared_select_statement(SQLHSTMT hstmt) : hstmt_(hstmt) { }

    ~prepared_select_statement() {
      SQLCloseCursor(hstmt_);
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
    }

    result_set<select_list, container, false> execute() {
      sql_check(SQL_HANDLE_STMT, hstmt_, SQLExecute(hstmt_));
      return result_set<select_list, container, false>(hstmt_);
    }
  };

  template<class ExprList, class Container, bool Own>
  class result_set {

    SQLHSTMT hstmt_;

  public:
    result_set(SQLHSTMT hstmt) : hstmt_(hstmt) { }

    ~result_set() {
      SQLCloseCursor(hstmt_);
      if (Own)
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
    }

    template<class T>
    struct enable {
      typedef T type;
    };

    typedef typename Container::value_type value_type;

    bool fetch(value_type& row) const {
      long rc = SQLFetch(hstmt_);

      if (rc == SQL_NO_DATA) {
        return false;
      }

      if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        throw error(SQL_HANDLE_STMT, hstmt_, rc);

      typedef fusion::vector<const ExprList&, typename value_type::value_vector_type&> zip;

      // TODO
      // Ugly hack: we don't need the expressions themselves, just their type. 
      // What we'd need here is the ability to zip a mpl::vector with a fusion::vector.
      fusion::for_each(fusion::zip_view<zip>(zip(*(const ExprList*) 0, row.values())),
        read_row<value_type>(hstmt_, row));

      return true;
    }

    value_type fetch() const {
      value_type row;
      fetch(row);
      return row;
    }

    Container all() const {
      Container results;
      value_type row;

      while (fetch(row)) {
        results.push_back(row);
      }

      return results;
    }

    template<class OtherContainer>
    void all(OtherContainer& results) const {
      value_type row;

      while (fetch(row)) {
        results.push_back(row);
      }
    }
  };

  template<class ResultSet>
  typename ResultSet::template enable<std::ostream&>::type
  operator <<(std::ostream& os, ResultSet& results) {
    const char* sep = "";
    os << "(";
    typename ResultSet::value_type row;
    while (results.fetch(row)) {
      os << sep;
      os << row;
      sep = " ";
    }
    return os << ")";
  }

} } }

#endif // BOOST_ODBC_HPP
