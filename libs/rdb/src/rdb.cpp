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

  select_begin< fusion::map<> > select;
  select_begin< fusion::map<fusion::pair< select_impl::distinct, int> > > select_begin< fusion::map<> >::distinct;
  select_begin< fusion::map<fusion::pair< select_impl::all, int> > > select_begin< fusion::map<> >::all;

} }