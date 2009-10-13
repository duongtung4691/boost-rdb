#ifndef BOOST_ODBC_HPP
#define BOOST_ODBC_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/rdb/types.hpp>

namespace boost { namespace rdb {

  template<class SqlType, class Value, class Tag>
  struct sql_type_adapter;

} }

namespace boost { namespace rdb { namespace odbc {

  struct odbc_tag { };

  template<size_t N>
  class varchar {
  public:
    BOOST_STATIC_CONSTANT(size_t, size = N);
    operator std::string() const { return std::string(chars_, chars_ + length()); }
    const char* chars() const { return chars_; }
    size_t length() const { return ulength_; }

    template<int Length>
    varchar& operator =(const char (&str)[Length]) {
      BOOST_STATIC_ASSERT(Length <= N);
      const char* src = str;
      char* dest = chars_;
      while (*dest++ = *src++);
      ulength_ = dest - chars_ - 1;
      return *this;
    }

    varchar& operator =(const char* src) {
      char* dest = chars_;
      char* last = chars_ + N;
      while (*src) {
        if (dest == last)
          throw std::range_error("overflow in varchar");
        *dest++ = *src++;
      }
      *dest = 0;
      ulength_ = dest - chars_;
      return *this;
    }

    varchar& operator =(const std::string& src) {
      if (src.length() > N)
        throw std::range_error("overflow in varchar");
      *std::copy(src.begin(), src.end(), chars_) = 0;
      ulength_ = src.length();
      return *this;
    }

//  private:
    union {
      long length_;
      unsigned long ulength_;
    };
    char chars_[N + 1];

    template<class SqlType, class Value, class Tag> friend struct sql_type_adapter;
  };

} } }

namespace boost { namespace rdb { namespace type {

  template<int N>
  struct cli_type<type::varchar<N>, odbc::odbc_tag> {
    typedef odbc::varchar<N> type;
  };

  template<>
  struct cli_type<type::integer, odbc::odbc_tag> {
    typedef long type;
  };

  template<>
  struct cli_type<type::boolean, odbc::odbc_tag> {
    typedef bool type;
  };

} } }

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

  class database;


  template<size_t N>
  std::ostream& operator <<(std::ostream& os, const varchar<N>& str) {
    os.write(str.chars(), str.length());
    return os;
  }

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
        try {
          db.prepare_str(hstmt, as_string(st));
          return prepare_return_type(hstmt);
        } catch (...) {
          SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
          throw;
        }
      }

    };

    template<class Select>
    struct discriminate<sql::select_statement_tag, Select> {

      typedef result_set<typename Select::select_list, typename Select::result, false> execute_return_type;

      static execute_return_type execute(database& db, const Select& select) {
        HSTMT hstmt;
        sql_check(SQL_HANDLE_DBC, db.hdbc_, SQLAllocStmt(db.hdbc_, &hstmt));
        try {
          db.exec_str(hstmt, as_string(select));
          return execute_return_type(hstmt);
        } catch (...) {
          SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
          throw;
        }
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
  };

  template<class Tag>
  struct make_param_vector {

    template<typename Sig>
    struct result;

    template<class Self, class SqlType, class Vector>
    struct result<Self(type::placeholder<SqlType>&, Vector&)> {
      typedef typename fusion::result_of::push_back<
        Vector,
        typename type::cli_type<SqlType, Tag>::type
      >::type type;
    };
  };

  struct bind_parameters {
    bind_parameters(SQLHSTMT hstmt) : hstmt_(hstmt), i_(1) { }
    SQLHSTMT hstmt_;
    mutable int i_;

    void operator ()(fusion::vector<const type::placeholder<type::integer>&, long&>& zip) const {
      using namespace fusion;
      SQLINTEGER length = sizeof(&at_c<1>(zip));
      sql_check(SQL_HANDLE_STMT, hstmt_, SQLBindParameter(hstmt_, i_, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_INTEGER, 0, 0,
        &at_c<1>(zip), 0, &length));
      ++i_;
    }

    template<size_t N>
    void operator ()(fusion::vector<const type::placeholder< type::varchar<N> >&, varchar<N>&>& zip) const {
      using namespace fusion;
      sql_check(SQL_HANDLE_STMT, hstmt_, SQLBindParameter(hstmt_, i_, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, N, 0,
        at_c<1>(zip).chars_, 0, &at_c<1>(zip).length_));
      ++i_;
    }
  };


  template<class Statement>
  class prepared_statement {
  
  public:
    prepared_statement(SQLHSTMT hstmt) : hstmt_(hstmt) {
      typedef fusion::vector<const placeholder_vector&, param_vector&> zip;

    fusion::for_each(fusion::zip_view<zip>(zip(*(const placeholder_vector*) 0, params_)),
      bind_parameters(hstmt_));
    }

    ~prepared_statement() {
      SQLCloseCursor(hstmt_);
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
    }

    template<class Vector>
    void executev(const Vector& params) {
      // if you have this on your error stack, it's likely that the types of the values don't agree with the types of the parameters
      // TODO: assert on this
      params_ = params;
      sql_check(SQL_HANDLE_STMT, hstmt_, SQLExecute(hstmt_));
    }

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/odbc/detail/execute.hpp>
    #include BOOST_PP_ITERATE()

  protected:
    SQLHSTMT hstmt_;

    typedef typename Statement::placeholder_vector placeholder_vector;

    typedef typename fusion::result_of::as_vector<
      typename fusion::result_of::accumulate<
        placeholder_vector, 
        fusion::vector<>, 
        make_param_vector<odbc_tag>
      >::type
    >::type param_vector;

    param_vector params_;
    
  };

  template<class Select>
  class prepared_select_statement : prepared_statement<Select> {
  
  public:
    typedef typename Select::select_list select_list;
    typedef typename Select::result container;
    typedef prepared_statement<Select> base;

    prepared_select_statement(SQLHSTMT hstmt) : base(hstmt) { }

    result_set<select_list, container, false> execute() {
      sql_check(SQL_HANDLE_STMT, this->hstmt_, SQLExecute(this->hstmt_));
      return result_set<select_list, container, false>(this->hstmt_);
    }

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/odbc/detail/execute_select.hpp>
    #include BOOST_PP_ITERATE()
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

namespace boost { namespace rdb {

  template<>
  struct sql_type_adapter<type::integer, long, odbc::odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, long& value) {    
      SQLLEN n;
      SQLGetData(hstmt, col, SQL_C_LONG, &value, 0, &n);
      return n != SQL_NULL_DATA;
    }
  };

  template<int N>
  struct sql_type_adapter<type::varchar<N>, odbc::varchar<N>, odbc::odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, odbc::varchar<N>& value) {
      // TODO: post-fetch step to deal with signed/unsigned issue
      SQLGetData(hstmt, col, SQL_C_CHAR, value.chars_, sizeof value.chars_, &value.length_);
      if (value.length_ == SQL_NULL_DATA)
        return false;
      return true;
    }
  };

  template<int N>
  struct sql_type_adapter<type::varchar<N>, std::string, odbc::odbc_tag> {
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

} }

#endif // BOOST_ODBC_HPP
