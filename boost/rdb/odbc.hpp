#ifndef BOOST_ODBC_HPP
#define BOOST_ODBC_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/rdb/common.hpp>

#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/zip_view.hpp>

namespace boost { namespace rdb {

  template<class SqlType, class Value, class Tag>
  struct sql_type_adapter;
  
  struct dynamic_value_mismatch : std::exception {
    virtual const char* what() const throw();
  };

} }

namespace boost { namespace rdb { namespace odbc {

  extern std::ostream* trace_stream;

  struct odbc_tag { };

  struct odbc_error : std::exception {
    odbc_error(SQLSMALLINT handle_type, SQLHANDLE handle, long rc);
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
      odbc_error ex(handle_type, handle, rc);
      //TR << "ODBC Exception: " << ex.what() << endl;
      throw ex;
    }
  }

  template<class RdbType, class CliType>  
  class simple_numeric_type {
  public:
    simple_numeric_type() : value_(CliType()), length_(SQL_NULL_DATA) { }
    
    template<typename T>
    simple_numeric_type(const T& value) : value_(static_cast<T>(value)), length_(sizeof(value_)) { }

    template<typename T>
    simple_numeric_type& operator =(const T& value) {
      value_ = static_cast<T>(value);
      length_ = sizeof(value_);
      return *this;
    }
    
    void set_null() { length_ = SQL_NULL_DATA; }
    bool is_null() const { return length_ == SQL_NULL_DATA; }
    CliType value() const { return value_; }
    
    typedef RdbType rdb_type;
  
  //private:
    CliType value_;
    SQLLEN length_;
  };
  
  typedef simple_numeric_type<type::integer, SQLINTEGER> integer;
  
  namespace detail {
    
    inline void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i, integer& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_SLONG,
        &var.value_, 0, &var.length_));
    }
  
    inline void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT& i, const type::placeholder<type::integer>&, const integer& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
        (SQLPOINTER) &var.value_, 0, (SQLINTEGER*) &var.length_));
      ++i;
    }
    
  }

  typedef simple_numeric_type<type::real, float> real;
  
  typedef simple_numeric_type<type::float_, double> float_;
    
  namespace detail {
  
    inline void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT& i, const type::placeholder<type::real>&, const real& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0,
        (SQLPOINTER) &var.value_, 0, (SQLINTEGER*) &var.length_));
      ++i;
    }
    
    inline void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i, float_& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_DOUBLE,
        &var.value_, 0, &var.length_));
    }
  
    inline void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT& i, const type::placeholder<type::float_>&, const float_& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
        (SQLPOINTER) &var.value_, 0, (SQLINTEGER*) &var.length_));
      ++i;
    }
    
  }

  template<size_t N>
  class varchar {
  public:
    BOOST_STATIC_CONSTANT(size_t, size = N);

    typedef type::varchar<N> rdb_type;

    varchar() : length_(0) {
    }

    varchar(const char* str) : length_(0) {
      *this = str;
    }

    varchar(const std::string& str) : length_(0) {
      *this = str;
    }

    operator std::string() const { return std::string(chars_, chars_ + length()); }
    const char* chars() const { return chars_; }
    size_t length() const { return length_; }

    template<int Length>
    varchar& operator =(const char (&str)[Length]) {
      BOOST_STATIC_ASSERT(Length <= N);
      const char* src = str;
      char* dest = chars_;
      while (*dest++ = *src++);
      length_ = dest - chars_ - 1;
      return *this;
    }

    varchar& operator =(const char* src) {
      SQLCHAR* dest = chars_;
      SQLCHAR* last = chars_ + N;
      while (*src) {
        if (dest == last)
          throw std::range_error("overflow in varchar");
        *dest++ = *src++;
      }
      *dest = 0;
      length_ = dest - chars_;
      return *this;
    }

    varchar& operator =(const std::string& src) {
      if (src.length() > N)
        throw std::range_error("overflow in varchar");
      *std::copy(src.begin(), src.end(), chars_) = 0;
      length_ = src.length();
      return *this;
    }

    void set_null() { length_ = SQL_NULL_DATA; }
    bool is_null() const { return length_ == SQL_NULL_DATA; }

//  private:
    SQLLEN length_;
    SQLCHAR chars_[N + 1];

    template<class SqlType, class Value, class Tag> friend struct sql_type_adapter;
  };
    
  namespace detail {

    template<size_t N>
    inline void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i, varchar<N>& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_CHAR,
        var.chars_, N + 1, &var.length_));
    }

    template<size_t N>
    inline void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT& i, const type::placeholder< type::varchar<N> >&, const varchar<N>& var) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, N, 0,
        (SQLPOINTER) var.chars_, 0, (SQLINTEGER*) &var.length_));
      ++i;
    }
    
  }

  class dynamic_value : public abstract_dynamic_value {
  public:
    dynamic_value(int type, int length) : abstract_dynamic_value(type, length) { }
    virtual void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i) = 0;
    virtual void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT i) = 0;
  };

  typedef std::vector< intrusive_ptr<dynamic_value> > dynamic_values;

  class dynamic_integer_value : public dynamic_value {
  public:

    dynamic_integer_value(int type, int length, integer& var) : dynamic_value(type, length), var_(var) { }
    
    virtual void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i) {
      detail::bind_result(hstmt, i, var_);
    }
    
    virtual void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT i) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
        &var_.value_, 0, &var_.length_));
    }
    
  private:
    integer& var_;
  };

  template<size_t N>
  class dynamic_varchar_value : public dynamic_value {
  public:

    dynamic_varchar_value(int type, int length, varchar<N>& var) : dynamic_value(type, length), var_(var) { }
    
    virtual void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_CHAR,
        var_.chars_, N + 1, &var_.length_));
    }
    
    virtual void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT i) {
      sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, N, 0,
        &var_.chars_, 0, &var_.length_));
    }
    
  private:
    varchar<N>& var_;
  };

} } }

namespace boost { namespace rdb { namespace type {

  template<size_t N>
  struct cli_type<type::varchar<N>, odbc::odbc_tag> {
    typedef odbc::varchar<N> type;
  };

  template<>
  struct cli_type<integer, odbc::odbc_tag> {
    typedef odbc::integer type;
  };

  template<>
  struct cli_type<real, odbc::odbc_tag> {
    typedef odbc::real type;
  };

  template<>
  struct cli_type<float_, odbc::odbc_tag> {
    typedef odbc::float_ type;
  };

  template<>
  struct cli_type<boolean, odbc::odbc_tag> {
    typedef bool type;
  };

  template<>
  struct cli_type<dynamic_expressions, odbc::odbc_tag> {
    typedef odbc::dynamic_values type;
  };

} } }

namespace boost { namespace rdb { namespace odbc {

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
    void operator ()(const fusion::vector<Expr, Value&>& value) const {
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
          return prepare_return_type(st, hstmt);
        } catch (...) {
          SQLFreeHandle(SQL_HANDLE_STMT, &hstmt);
          throw;
        }
      }
    };

    template<class Select>
    struct discriminate<select_statement_tag, Select> {

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
        return prepare_return_type(select, hstmt);
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

  struct parameter_binder {
    parameter_binder(SQLHSTMT hstmt) : hstmt_(hstmt), i_(1) { }
    SQLHSTMT hstmt_;
    mutable SQLUSMALLINT i_;

    template<class T1, class T2>
    void operator ()(const fusion::vector<T1&, T2&>& zip) const {
      detail::bind_parameter(hstmt_, i_, fusion::at_c<0>(zip), fusion::at_c<1>(zip));
    }
    
    void operator ()(const fusion::vector<const dynamic_placeholders&, const dynamic_values&>& zip) const;
  };
  
  template<class Tag>    
  struct make_cli_param_vector {
    typedef make_cli_param_vector Self;
    
    template<typename Sig>
    struct result;

    template<class Placeholder, class CliVector>
    struct result<Self(Placeholder, const CliVector&)> {
      typedef typename fusion::result_of::push_back<
        CliVector,
        typename type::cli_type<
          typename remove_reference<Placeholder>::type::rdb_type,
          Tag
        >::type
      >::type type;
    };
  };
  
  template<class Statement>
  class prepared_statement {

    typedef typename fusion::result_of::as_vector<typename Statement::placeholder_vector>::type placeholder_vector;
    placeholder_vector placeholders_;
  
  public:
    prepared_statement(const Statement& st, SQLHSTMT hstmt) :
      placeholders_(st.placeholders()), hstmt_(hstmt) {
    }

    ~prepared_statement() {
      SQLCloseCursor(hstmt_);
      SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
    }
    
    template<class Params>
    void executev(const Params& params) {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::accumulate<
          typename Statement::placeholder_vector,
          fusion::vector<>,
          make_cli_param_vector<odbc_tag>
        >::type
      >::type cli_param_vector;
      const cli_param_vector cli_params(params);
      typedef fusion::vector<const placeholder_vector&, cli_param_vector&> zip;
      fusion::for_each(fusion::zip_view<zip>(zip(placeholders_, const_cast<cli_param_vector&>(cli_params))),
        parameter_binder(hstmt_));
      sql_check(SQL_HANDLE_STMT, hstmt_, SQLExecute(hstmt_));
    }

    void execute() {
      sql_check(SQL_HANDLE_STMT, hstmt_, SQLExecute(hstmt_));
    }

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/odbc/detail/execute.hpp>
    #include BOOST_PP_ITERATE()

    template<class Params>
    void bind_parameters_(const Params& params) {
      typedef fusion::vector<const placeholder_vector&, const Params&> zip;
      fusion::for_each(fusion::zip_view<zip>(zip(placeholders_, params)),
        parameter_binder(hstmt_));
    }

    //void bind_parameters_(param_ref_vector& params) {
    //  typedef fusion::vector<const placeholder_vector&, param_ref_vector&> zip;
    //  fusion::for_each(fusion::zip_view<zip>(zip(placeholders_, params)),
    //    parameter_binder(hstmt_));
    //}

    #define BOOST_RDB_ADD_REF(z, n, type) BOOST_PP_COMMA_IF(n) type##n&
    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/odbc/detail/bind_parameters.hpp>
    #include BOOST_PP_ITERATE()

  protected:
    SQLHSTMT hstmt_;    
  };

  template<class Expr, class CliType>
  struct can_bind : is_same<typename Expr::sql_type, typename CliType::rdb_type> {
  };

  template<>
  struct can_bind<dynamic_expressions, dynamic_values> : mpl::true_ {
  };
  
  struct results_binder {
    results_binder(SQLHSTMT hstmt) : hstmt_(hstmt), i_(1) { }
    SQLHSTMT hstmt_;
    mutable SQLUSMALLINT i_;

    template<class Expr, class CliType>
    void operator ()(const fusion::vector<const Expr&, CliType&>& zip) const {
      BOOST_MPL_ASSERT((can_bind<Expr, CliType>));
      detail::bind_result(hstmt_, i_, fusion::at_c<1>(zip));
      ++i_;
    }

    void operator ()(const fusion::vector<const dynamic_expressions&, dynamic_values&>& zip) const;
  };

  template<class Select>
  class prepared_select_statement : public prepared_statement<Select> {
  
  public:
    typedef typename Select::select_list select_list;
    typedef typename Select::result container;
    typedef prepared_statement<Select> base;

    prepared_select_statement(const Select& select, SQLHSTMT hstmt) :
      base(select, hstmt), exprs_(select.exprs()) { }
      
    select_list exprs_;

    result_set<select_list, container, false> execute() {
      sql_check(SQL_HANDLE_STMT, this->hstmt_, SQLExecute(this->hstmt_));
      return result_set<select_list, container, false>(this->hstmt_);
    }

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/odbc/detail/execute_select.hpp>
    #include BOOST_PP_ITERATE()

    template<class Results>
    void bind_results_(const Results& results) {
      typedef fusion::vector<const select_list&, Results&> zip;
      fusion::for_each(fusion::zip_view<zip>(zip(exprs_, const_cast<Results&>(results))),
        results_binder(this->hstmt_));
    }

    #define BOOST_RDB_ADD_REF(z, n, type) BOOST_PP_COMMA_IF(n) type##n&
    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/odbc/detail/bind_results.hpp>
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
        throw odbc_error(SQL_HANDLE_STMT, hstmt_, rc);

      typedef fusion::vector<const ExprList&, typename value_type::value_vector_type&> zip;

      // TODO
      // Ugly hack: we don't need the expressions themselves, just their type. 
      // What we'd need here is the ability to zip a mpl::vector with a fusion::vector.
      fusion::for_each(fusion::zip_view<zip>(zip(*(const ExprList*) 0, row.values())),
        read_row<value_type>(hstmt_, row));

      return true;
    }

    bool fetch() const {
      long rc = SQLFetch(hstmt_);

      if (rc == SQL_NO_DATA) {
        return false;
      }

      if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        throw odbc_error(SQL_HANDLE_STMT, hstmt_, rc);

      return true;
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

  template<class ExprList, class Container, bool Own>
  std::ostream& operator <<(std::ostream& os, const result_set<ExprList, Container, Own>& results) {
    const char* sep = "";
    os << "(";
    typename result_set<ExprList, Container, Own>::value_type row;
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

  template<>
  struct sql_type_adapter<type::real, float, odbc::odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, float& value) {    
      SQLLEN n;
      SQLGetData(hstmt, col, SQL_C_FLOAT, &value, 0, &n);
      return n != SQL_NULL_DATA;
    }
  };

  template<>
  struct sql_type_adapter<type::float_, double, odbc::odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, double& value) {    
      SQLLEN n;
      SQLGetData(hstmt, col, SQL_C_DOUBLE, &value, 0, &n);
      return n != SQL_NULL_DATA;
    }
  };

  template<size_t N>
  struct sql_type_adapter<type::varchar<N>, odbc::varchar<N>, odbc::odbc_tag> {
    static bool get_data(SQLHSTMT hstmt, int col, odbc::varchar<N>& value) {
      // TODO: post-fetch step to deal with signed/unsigned issue
      SQLGetData(hstmt, col, SQL_C_CHAR, value.chars_, sizeof value.chars_, &value.length_);
      if (value.length_ == SQL_NULL_DATA)
        return false;
      return true;
    }
  };

  template<size_t N>
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

namespace boost { namespace rdb { namespace sql {

  inline intrusive_ptr<odbc::dynamic_value> make_dynamic(odbc::integer& lvalue) {
    return new odbc::dynamic_integer_value(type::integer::id, type::integer::length, lvalue);
  }

  template<size_t N>
  inline intrusive_ptr<odbc::dynamic_value> make_dynamic(odbc::varchar<N>& lvalue) {
    return new odbc::dynamic_varchar_value<N>(type::varchar<N>::id, type::varchar<N>::length, lvalue);
  }

} } }

#endif // BOOST_ODBC_HPP
