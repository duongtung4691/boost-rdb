#ifndef BOOST_ODBC_DATETIME_HPP
#define BOOST_ODBC_DATETIME_HPP

#include <boost/rdb/odbc.hpp>

#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace boost { namespace rdb { namespace odbc {

  struct datetime {
    typedef boost::posix_time::ptime cpp_type;
  };

  template<int N>
  struct can_bind< core::datetime, odbc::varchar<N> > : mpl::true_ {
  };

} } }

namespace boost { namespace rdb { namespace core {

  template<>
  struct cli_type<datetime, odbc::odbc_tag> {
    typedef odbc::datetime type;
  };

} } }

#endif // BOOST_ODBC_HPP
