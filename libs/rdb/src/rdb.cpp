//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#include <boost/rdb/rdb.hpp>
#include <boost/rdb/select.hpp>

namespace boost { namespace rdb {

  void quote_text(std::ostream& os, const char* str) {
    os << "'";
    while (*str) {
      if (*str == '\'')
        os << *str;
      os << *str++;
    }
    os << "'";
  }

  //select_type<details::empty, void, void> select = details::empty();
  select_type<details::empty, void, void> select = details::empty();

} }