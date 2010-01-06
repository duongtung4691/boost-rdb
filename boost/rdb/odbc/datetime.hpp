#ifndef BOOST_ODBC_DATETIME_HPP
#define BOOST_ODBC_DATETIME_HPP

#include <boost/rdb/odbc.hpp>
#include <boost/rdb/core/datetime.hpp>

#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace boost { namespace rdb { namespace odbc {

  template<int N>
  struct can_bind< core::datetime, odbc::varchar<N> > : mpl::true_ {
  };

  typedef simple_numeric_type<core::datetime, SQL_TIMESTAMP_STRUCT, boost::posix_time::ptime> datetime;

} } }

namespace boost { namespace rdb { namespace core {

  template<>
  struct cli_type<datetime, odbc::odbc_tag> {
    typedef odbc::datetime type;
  };

} } }

#endif // BOOST_ODBC_HPP
