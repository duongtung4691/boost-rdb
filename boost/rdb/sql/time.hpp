//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_TIME_HPP
#define BOOST_RDB_SQL_TIME_HPP

#include <boost/rdb/core/time.hpp>

namespace boost { namespace rdb { namespace core {

  template<>
  struct make_literal<time, const char*> {
    typedef literal<std::string, time> type;
    static type value(const std::string& val) { return type(val); }
  };
  
  template<int N>
  struct make_literal<time, char[N]> : make_literal<time, const char*> { };
  
} } }

#endif
