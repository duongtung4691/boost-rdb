#ifndef BOOST_ODBC_HPP
#define BOOST_ODBC_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include <boost/rdb/core.hpp>
#include <boost/rdb/dynamic.hpp>

#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/zip_view.hpp>
#include <boost/fusion/include/find_if.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/end.hpp>

#include "boost/date_time/posix_time/posix_time_types.hpp"

#include <deque>
#include <stdexcept>

namespace boost { namespace rdb {

  template<class SqlType, class Value, class Tag>
  struct sql_type_adapter;
  
  struct dynamic_value_mismatch : std::exception {
    virtual const char* what() const throw();
  };

} }

namespace boost { namespace rdb { namespace odbc {
  
  template<class Seq>
  struct nullable {
    Seq values_;
    typedef std::bitset<fusion::result_of::size<Seq>::value> status_vector_type;
    typedef Seq value_vector_type;
    status_vector_type status_;
    bool is_null(int pos) const { return !status_[pos]; }
    template<int I> bool is_null() const { return !status_[I]; }
    void set_null(int pos, bool to_null) { status_[pos] = !to_null; }
    template<int I> typename fusion::result_of::at_c<const Seq, I>::type get() const {
      return fusion::at_c<I>(values_);
    }
    template<int I> typename fusion::result_of::at_c<Seq, I>::type ref() {
      return fusion::at_c<I>(values_);
    }
    const Seq& values() const { return values_; }
    Seq& values() { return values_; }
    const status_vector_type& status() const { return status_; }
    status_vector_type& status() { return status_; }
    nullable& operator =(const Seq& values) { values_ = values; return *this; }
  };

  template<class Row>
  struct print_row_element {

    print_row_element(std::ostream& os, const Row& r) : os_(os), r_(r), bit_(0) { }
    
    std::ostream& os_;
    const Row& r_;
    mutable int bit_;
    
    template<class T>
    void operator ()(const T& value) const {
      if (bit_)
        os_ << " ";
      if (r_.is_null(bit_++))
        os_ << "null";
      else
        os_ << value;
    }
  };

  template<class Seq>
  std::ostream& operator <<(std::ostream& os, const nullable<Seq>& r) {
    os << "(";
    fusion::for_each(r.values(), print_row_element< nullable<Seq> >(os, r));
    os << ")";
    return os;
  }

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

  namespace detail {
    template<class CliType>
    struct bind;
  }

  template<class SqlType, class CliType>
  struct can_bind : is_same<SqlType, typename CliType::rdb_type> {
  };

  class dynamic_value : public dynamic::abstract_dynamic_value {
  public:
    dynamic_value(int type, int length) : abstract_dynamic_value(type, length) { }
    virtual void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i) = 0;
    virtual void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT i) = 0;
  };

  typedef std::vector< intrusive_ptr<dynamic_value> > dynamic_values;

  template<class RdbType, class CliType, class CppType = CliType>
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
    const CliType& value() const { return value_; }
    CliType& value() { return value_; }
    CppType cpp_value() const { return value_; }
    
    typedef RdbType rdb_type;
    typedef CppType cpp_type;
  
  //private:
    CliType value_;
    SQLLEN length_;
  };
  
  typedef simple_numeric_type<core::integer, SQLINTEGER> integer;
  
  namespace detail {
    
    template<>
    struct bind<integer> {
      static void result(SQLHSTMT hstmt, SQLUSMALLINT i, integer& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_SLONG,
          &var.value_, 0, &var.length_));
      }
    
      static void parameter(SQLHSTMT hstmt, SQLUSMALLINT i, integer& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
          (SQLPOINTER) &var.value_, 0, (SQLINTEGER*) &var.length_));
      }
    };
  }

  template<class CliType>
  class generic_dynamic_value : public dynamic_value {
  public:

    generic_dynamic_value(const CliType& var) :
      dynamic_value(CliType::rdb_type::id, CliType::rdb_type::length), var_(const_cast<CliType&>(var)) { }
    
    virtual void bind_result(SQLHSTMT hstmt, SQLUSMALLINT i) {
      detail::bind<CliType>::result(hstmt, i, var_);
    }
    
    virtual void bind_parameter(SQLHSTMT hstmt, SQLUSMALLINT i) {
      detail::bind<CliType>::parameter(hstmt, i, var_);
    }
    
  private:
    CliType& var_;
  };

  template<class RdbType, class CliType>  
  inline intrusive_ptr<dynamic_value> make_dynamic(simple_numeric_type<RdbType, CliType>& lvalue) {
    return new generic_dynamic_value< simple_numeric_type<RdbType, CliType> >(lvalue);
  }
  
  typedef simple_numeric_type<core::float_, double> float_;
    
  namespace detail {
    
    template<>
    struct bind<float_> {
      static void result(SQLHSTMT hstmt, SQLUSMALLINT i, float_& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_DOUBLE,
          &var.value_, 0, &var.length_));
      }
    
      static void parameter(SQLHSTMT hstmt, SQLUSMALLINT i, float_& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
          (SQLPOINTER) &var.value_, 0, (SQLINTEGER*) &var.length_));
      }
    };
    
  }

  template<size_t N>
  class varchar {
  public:
    BOOST_STATIC_CONSTANT(size_t, size = N);

    typedef core::varchar<N> rdb_type;
    typedef std::string cpp_type;

    varchar() : length_(0) {
    }

    varchar(const char* str) : length_(0) {
      *this = str;
    }

    varchar(const std::string& str) : length_(0) {
      *this = str;
    }

    operator std::string() const { return std::string(chars_, chars_ + length()); }
    std::string cpp_value() const { return std::string(chars_, chars_ + length()); }
    const char* value() const { return reinterpret_cast<const char*>(chars_); }
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
          throw std::overflow_error("overflow in varchar");
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

  template<size_t N>
  inline intrusive_ptr<dynamic_value> make_dynamic(varchar<N>& lvalue) {
    return new generic_dynamic_value< varchar<N> >(lvalue);
  }
    
  namespace detail {

    template<size_t N>
    struct bind< varchar<N> > {
      static void result(SQLHSTMT hstmt, SQLUSMALLINT i, varchar<N>& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_CHAR,
          var.chars_, N + 1, &var.length_));
      }

      static void parameter(SQLHSTMT hstmt, SQLUSMALLINT i, varchar<N>& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, N, 0,
          (SQLPOINTER) var.chars_, 0, (SQLINTEGER*) &var.length_));
      }
    };
  }

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

namespace boost { namespace rdb { namespace core {

  template<size_t N>
  struct cli_type<varchar<N>, odbc::odbc_tag> {
    typedef odbc::varchar<N> type;
  };

  template<>
  struct cli_type<integer, odbc::odbc_tag> {
    typedef odbc::integer type;
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
  struct cli_type<dynamic::dynamic_expressions, odbc::odbc_tag> {
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
    if (str.is_null())
      os << "<null>";
    else
      os.write(str.value(), str.length());
    return os;
  }

  template<class Row>
  struct read_row {
    read_row(Row& row) : row_(row), i_(0) { }
    Row& row_;
    mutable int i_;

    template<class CliType, class Value>
    void operator ()(const fusion::vector<CliType, Value&>& value) const {
    
      using namespace fusion;
      
      row_.set_null(i_, at_c<0>(value).is_null());
      
      if (!at_c<0>(value).is_null())
        at_c<1>(value) = at_c<0>(value).cpp_value();
        
      ++i_;
    }
  };

  template<class Select>
  class prepared_select_statement;

  template<class Statement>
  class prepared_statement;

  template<class ExprList, bool Own>
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
    struct discriminate<core::tabular_result_tag, Select> {

      typedef result_set<typename core::statement_result_type<Select>::type, false> execute_return_type;

      static execute_return_type execute(database& db, const Select& select) {
        HSTMT hstmt;
        sql_check(SQL_HANDLE_DBC, db.hdbc_, SQLAllocStmt(db.hdbc_, &hstmt));
        try {
          db.exec_str(hstmt, as_string(select));
          return execute_return_type(hstmt, exprs(select));
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
      detail::bind<T2>::parameter(hstmt_, i_, fusion::at_c<1>(zip));
      ++i_;
    }
    
    void operator ()(const fusion::vector<const dynamic::dynamic_placeholders&, dynamic_values&>& zip) const;
  };
  
  template<class Tag>    
  struct make_cli_result_vector {
    typedef make_cli_result_vector Self;
    
    template<typename Sig>
    struct result;

    template<class Expr, class CliVector>
    struct result<Self(Expr, const CliVector&)> {
      typedef typename fusion::result_of::push_back<
        CliVector,
        typename core::cli_type<
          typename remove_reference<Expr>::type::rdb_type,
          Tag
        >::type
      >::type type;
    };
  };
  
  template<class CliType>
  struct compact_result {
    typedef typename CliType::cpp_type type;
  };
  
  template<>
  struct compact_result<dynamic_values> {
    typedef dynamic_values type;
  };
  
  struct make_compact_result_vector {
    typedef make_compact_result_vector Self;
    
    template<typename Sig>
    struct result;

    template<class CliType, class Vector>
    struct result<Self(CliType, const Vector&)> {
      typedef typename fusion::result_of::push_back<
        Vector,
        typename compact_result<typename remove_reference<CliType>::type>::type
      >::type type;
    };
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
        typename core::cli_type<
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
      cli_param_vector cli_params(params);
      typedef fusion::vector<const placeholder_vector&, cli_param_vector&> zip;
      fusion::for_each(fusion::zip_view<zip>(zip(placeholders_, cli_params)),
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
    void bind_parameters_(Params& params) {
      typedef fusion::vector<const placeholder_vector&, Params&> zip;
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

  template<>
  struct can_bind<dynamic::dynamic_expressions, dynamic_values> : mpl::true_ {
  };
  
  struct results_binder {
    results_binder(SQLHSTMT hstmt) : hstmt_(hstmt), i_(1) { }
    SQLHSTMT hstmt_;
    mutable SQLUSMALLINT i_;

    template<class Expr, class CliType>
    void operator ()(const fusion::vector<const Expr&, CliType&>& zip) const {
      BOOST_MPL_ASSERT((can_bind<typename Expr::rdb_type, CliType>));
      detail::bind<CliType>::result(hstmt_, i_, fusion::at_c<1>(zip));
      ++i_;
    }

    void operator ()(const fusion::vector<const dynamic::dynamic_expressions&, dynamic_values&>& zip) const;
  };

  template<class Select>
  class prepared_select_statement : public prepared_statement<Select> {
  
  public:
    typedef typename core::statement_result_type<Select>::type select_list;
    typedef prepared_statement<Select> base;

    prepared_select_statement(const Select& select, SQLHSTMT hstmt) :
      base(select, hstmt), exprs_(exprs(select)) { }
      
    select_list exprs_;

    result_set<select_list, false> execute() {
      sql_check(SQL_HANDLE_STMT, this->hstmt_, SQLExecute(this->hstmt_));
      return result_set<select_list, false>(this->hstmt_, exprs_);
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
  
  namespace detail {
  
    struct is_dynamic_expression_lambda {
      template<class Expr>
      struct apply : is_same<Expr, dynamic::dynamic_expressions> { };
    };
      
    template<class ExprList>
    struct contains_only_static_expressions : fusion::result_of::equal_to<
      typename fusion::result_of::find_if<ExprList, is_dynamic_expression_lambda>::type,
      typename fusion::result_of::end<ExprList>::type
    > {
    };

  }

  template<class ExprList, bool Own>
  class result_set {

    SQLHSTMT hstmt_;
    const ExprList& exprs_;
    bool bound_;

  public:
    
    typedef typename fusion::result_of::as_vector<
      typename fusion::result_of::accumulate<
        ExprList,
        fusion::vector<>,
        make_cli_result_vector<odbc_tag>
      >::type
    >::type result_vector;
    
    result_vector results_;
    
    typedef nullable<
      typename fusion::result_of::as_vector<
        typename fusion::result_of::accumulate<
          result_vector,
          fusion::vector<>,
          make_compact_result_vector
        >::type
      >::type
    > compact_result_vector;
    
    typedef std::deque<compact_result_vector> Container;
  
    typedef typename Container::value_type value_type;

    result_set(SQLHSTMT hstmt, const ExprList& exprs) : hstmt_(hstmt), exprs_(exprs), bound_(false) {
    }
  
    result_set(const result_set& other) : hstmt_(other.hstmt_), exprs_(other.exprs_), bound_(false) {
    }

    void bind() {
      if (!bound_) {
        typedef fusion::vector<const ExprList&, result_vector&> zip;
        fusion::for_each(fusion::zip_view<zip>(zip(exprs_, results_)),
          results_binder(this->hstmt_));
        bound_ = true;
      }
    }

    ~result_set() {
    
      SQLCloseCursor(hstmt_);
      
      if (Own)
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt_);
    }

    bool fetch(value_type& row) {
      bind();
      return fetch2(row);
    }

    bool fetch2(value_type& row) {
    
      // error here indicates that at least one select expression is dynamic
      // this is not supported - explicitly bind results instead
      BOOST_MPL_ASSERT((detail::contains_only_static_expressions<ExprList>));
      
      long rc = SQLFetch(hstmt_);

      if (rc == SQL_NO_DATA) {
        return false;
      }

      if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        throw odbc_error(SQL_HANDLE_STMT, hstmt_, rc);

      typedef fusion::vector<const result_vector&, typename value_type::value_vector_type&> zip;

      fusion::for_each(fusion::zip_view<zip>(zip(results_, row.values())),
        read_row<value_type>(row));

      return true;
    }

    bool fetch() {
      long rc = SQLFetch(hstmt_);

      if (rc == SQL_NO_DATA) {
        return false;
      }

      if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
        throw odbc_error(SQL_HANDLE_STMT, hstmt_, rc);

      return true;
    }

    Container all() {
    
      // error here indicates that at least one select expression is dynamic
      // this is not supported - explicitly bind results instead
      BOOST_MPL_ASSERT((detail::contains_only_static_expressions<ExprList>));
      
      bind();
      
      Container results;
      value_type row;

      while (fetch2(row)) {
        results.push_back(row);
      }

      return results;
    }

    template<class OtherContainer>
    void all(OtherContainer& results) {
      value_type row;

      while (fetch(row)) {
        results.push_back(row);
      }
    }
  };

  template<class ExprList, bool Own>
  std::ostream& operator <<(std::ostream& os, const result_set<ExprList, Own>& results) {
    const char* sep = "";
    os << "(";
    typename result_set<ExprList, Own>::value_type row;
    while (const_cast<result_set<ExprList, Own>&>(results).fetch(row)) {
      os << sep;
      os << row;
      sep = " ";
    }
    return os << ")";
  }

} } }

#endif // BOOST_ODBC_HPP
