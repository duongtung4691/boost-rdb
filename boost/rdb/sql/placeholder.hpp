//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_PLACEHOLDER_HPP
#define BOOST_RDB_PLACEHOLDER_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  struct placeholder_type {
    typedef universal_type kind;
    typedef boost::mpl::true_::type is_numeric;
  };

  template<int N>
  struct placeholder {
    typedef placeholder_type sql_type;
    enum { precedence = precedence_level::highest };
    void str(std::ostream& os) const {
      os << "%";
    }
  };

  const expression< placeholder<0> > _;

} } }

#endif
