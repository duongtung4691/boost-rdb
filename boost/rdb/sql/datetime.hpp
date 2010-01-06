//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DATETIME_HPP
#define BOOST_RDB_SQL_DATETIME_HPP

#include <boost/rdb/core/datetime.hpp>

namespace boost { namespace rdb { namespace core {

  template<>
  struct make_literal<datetime, const char*> {
    typedef literal<std::string, datetime> type;
    static type value(const std::string& val) { return type(val); }
  };
  
  template<int N>
  struct make_literal<datetime, char[N]> : make_literal<datetime, const char*> { };
  
} } }

#endif
