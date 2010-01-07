#ifndef BOOST_ODBC_TIME_HPP
#define BOOST_ODBC_TIME_HPP

#include <boost/rdb/odbc.hpp>
#include <boost/rdb/core/time.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace boost { namespace rdb { namespace odbc {

  class time {
  public:
    time() : length_(SQL_NULL_DATA) { }
    
    time(SQLSMALLINT hour, SQLSMALLINT minute, SQLSMALLINT second) {
      value_.hour = hour;
      value_.minute = minute;
      value_.second = second;
      length_ = sizeof(SQL_TIME_STRUCT);
    }
    
    void set_null() { length_ = SQL_NULL_DATA; }
    bool is_null() const { return length_ == SQL_NULL_DATA; }
    const SQL_TIME_STRUCT& value() const { return value_; }
    SQL_TIME_STRUCT& value() { return value_; }
    
    boost::posix_time::time_duration cpp_value() const {
      using namespace boost::posix_time;
      return time_duration(value_.hour, value_.minute, value_.second);
    }
    
    typedef core::time rdb_type;
    typedef boost::posix_time::time_duration cpp_type;
  
  //private:
    SQL_TIME_STRUCT value_;
    SQLLEN length_;
  };
  
  std::ostream& operator <<(std::ostream& os, const time& t);

  template<int N>
  struct can_bind< core::time, odbc::varchar<N> > : mpl::true_ {
  };

  namespace detail {
    
    template<>
    struct bind<time> {
      static void result(SQLHSTMT hstmt, SQLUSMALLINT i, time& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_TIME,
          &var.value_, 0, &var.length_));
      }
    
      static void parameter(SQLHSTMT hstmt, SQLUSMALLINT i, time& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT,
          SQL_C_TIME, SQL_TIME, 0, 0,
          (SQLPOINTER) &var.value_, sizeof(SQL_TIME_STRUCT), (SQLINTEGER*) &var.length_));
      }
    };
  }

} } }

namespace boost { namespace rdb { namespace core {

  template<>
  struct cli_type<time, odbc::odbc_tag> {
    typedef odbc::time type;
  };

  template<>
  struct literal<odbc::time, time> : any_literal  {
    typedef time rdb_type;
    literal(const odbc::time& value) : value_(value) { }
    void str(std::ostream& os) const;
    odbc::time value_;
  };
  
  template<>
  struct make_literal<time, odbc::time> {
    typedef literal<odbc::time, time> type;
    static type value(const odbc::time& val) { return type(val); }
  };

} } }

#endif // BOOST_ODBC_HPP
