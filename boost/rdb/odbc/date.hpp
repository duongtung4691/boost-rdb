#ifndef BOOST_ODBC_DATE_HPP
#define BOOST_ODBC_DATE_HPP

#include <boost/rdb/odbc.hpp>
#include <boost/rdb/core/date.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace boost { namespace rdb { namespace odbc {

  class date {
  public:
    date() : length_(SQL_NULL_DATA) { }
    
    date(SQLSMALLINT year, SQLSMALLINT month, SQLSMALLINT day) {
      value_.year = year;
      value_.month = month;
      value_.day = day;
      length_ = sizeof(SQL_DATE_STRUCT);
    }
    
    void set_null() { length_ = SQL_NULL_DATA; }
    bool is_null() const { return length_ == SQL_NULL_DATA; }
    const SQL_DATE_STRUCT& value() const { return value_; }
    SQL_DATE_STRUCT& value() { return value_; }
    
    boost::gregorian::date cpp_value() const {
      return boost::gregorian::date(value_.year, value_.month, value_.day);
    }
    
    typedef core::date rdb_type;
    typedef boost::gregorian::date cpp_type;
  
  //private:
    SQL_DATE_STRUCT value_;
    SQLLEN length_;
  };
  
  std::ostream& operator <<(std::ostream& os, const date& t);

  template<int N>
  struct can_bind< core::date, odbc::varchar<N> > : mpl::true_ {
  };

  namespace detail {
    
    template<>
    struct bind<date> {
      static void result(SQLHSTMT hstmt, SQLUSMALLINT i, date& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindCol(hstmt, i, SQL_C_DATE,
          &var.value_, 0, &var.length_));
      }
    
      static void parameter(SQLHSTMT hstmt, SQLUSMALLINT i, date& var) {
        sql_check(SQL_HANDLE_STMT, hstmt, SQLBindParameter(hstmt, i, SQL_PARAM_INPUT,
          SQL_C_DATE, SQL_DATE, 0, 0,
          (SQLPOINTER) &var.value_, sizeof(SQL_DATE_STRUCT), (SQLINTEGER*) &var.length_));
      }
    };
  }

} } }

namespace boost { namespace rdb { namespace core {

  template<>
  struct cli_type<date, odbc::odbc_tag> {
    typedef odbc::date type;
  };

  template<>
  struct literal<odbc::date, date> : any_literal  {
    typedef date rdb_type;
    literal(const odbc::date& value) : value_(value) { }
    void str(std::ostream& os) const;
    odbc::date value_;
  };
  
  template<>
  struct make_literal<date, odbc::date> {
    typedef literal<odbc::date, date> type;
    static type value(const odbc::date& val) { return type(val); }
  };

} } }

#endif // BOOST_ODBC_HPP
