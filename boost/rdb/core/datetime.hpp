//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_CORE_DATETIME_HPP
#define BOOST_RDB_CORE_DATETIME_HPP

#include <boost/rdb/core.hpp>

namespace boost { namespace rdb { namespace core {
  
  struct datetime {
    BOOST_STATIC_CONSTANT(int, id = 6);
    BOOST_STATIC_CONSTANT(size_t, length = 1);
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    static void str(std::ostream& os) { os << "datetime"; }
  };

} } }

#endif
