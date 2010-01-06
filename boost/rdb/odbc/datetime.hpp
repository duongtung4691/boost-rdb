#ifndef BOOST_ODBC_DATETIME_HPP
#define BOOST_ODBC_DATETIME_HPP

#include <boost/rdb/odbc.hpp>
#include <boost/rdb/core/datetime.hpp>

#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace boost { namespace rdb { namespace odbc {

  class datetime {
  public:
    datetime() : length_(SQL_NULL_DATA) { }
    
    datetime(SQLSMALLINT year, SQLSMALLINT month, SQLSMALLINT day,
      SQLSMALLINT hour = 0, SQLSMALLINT minute = 0, SQLSMALLINT second = 0,
      SQLUINTEGER fraction = 0) {
      value_.year = year;
      value_.month = month;
      value_.day = day;
      value_.hour = hour;
      value_.minute = minute;
      value_.second = second;
      value_.fraction = fraction;
      length_ = sizeof(SQL_TIMESTAMP_STRUCT);
    }
    
    void set_null() { length_ = SQL_NULL_DATA; }
    bool is_null() const { return length_ == SQL_NULL_DATA; }
    const SQL_TIMESTAMP_STRUCT& value() const { return value_; }
    SQL_TIMESTAMP_STRUCT& value() { return value_; }
    boost::posix_time::ptime cpp_value() const; /* later */
    
    typedef core::datetime rdb_type;
    typedef boost::posix_time::ptime cpp_type;
  
  //private:
    SQL_TIMESTAMP_STRUCT value_;
    SQLLEN length_;
  };

  template<int N>
  struct can_bind< core::datetime, odbc::varchar<N> > : mpl::true_ {
  };

  namespace detail {
    
    template<>
    struct bind<datetime> {
      static void result(SQLHSTMT hstmt, SQLUSMALLINT i, datetime& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_TIMESTAMP,
          &var.value_, 0, &var.length_));
      }
    
      static void parameter(SQLHSTMT hstmt, SQLUSMALLINT i, datetime& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT,
          SQL_C_TIMESTAMP, SQL_TIMESTAMP, 0, 0,
          (SQLPOINTER) &var.value_, sizeof(SQL_TIMESTAMP_STRUCT), (SQLINTEGER*) &var.length_));
      }
    };
  }

} } }

namespace boost { namespace rdb { namespace core {

  template<>
  struct cli_type<datetime, odbc::odbc_tag> {
    typedef odbc::datetime type;
  };

  template<>
  struct literal<odbc::datetime, datetime> : any_literal  {
    typedef datetime rdb_type;
    literal(const odbc::datetime& value) : value_(value) { }
    void str(std::ostream& os) const;
    odbc::datetime value_;
  };
  
  template<>
  struct make_literal<datetime, odbc::datetime> {
    typedef literal<odbc::datetime, datetime> type;
    static type value(const odbc::datetime& val) { return type(val); }
  };

} } }

#endif // BOOST_ODBC_HPP
