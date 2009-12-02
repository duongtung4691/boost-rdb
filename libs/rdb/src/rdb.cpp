//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#include <boost/rdb/sql.hpp>

namespace boost { namespace rdb { namespace sql {

  void quote_text(std::ostream& os, const char* str) {
    os << "'";
    while (*str) {
      if (*str == '\'')
        os << *str;
      os << *str++;
    }
    os << "'";
  }

  select_statement<sql2003, sql2003::select, static_map0, sql2003> select = static_map0();
} } }
