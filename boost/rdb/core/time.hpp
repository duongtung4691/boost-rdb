//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_CORE_TIME_HPP
#define BOOST_RDB_CORE_TIME_HPP

#include <boost/rdb/core.hpp>

namespace boost { namespace rdb { namespace core {
  
  struct time {
    BOOST_STATIC_CONSTANT(int, id = 8);
    BOOST_STATIC_CONSTANT(size_t, length = 1);
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    static void str(std::ostream& os) { os << "time"; }
  };

} } }

#endif
