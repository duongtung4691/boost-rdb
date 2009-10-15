//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_TYPES_HPP
#define BOOST_RDB_TYPES_HPP

namespace boost { namespace rdb {

  namespace type {
    // types as they exist independantly of any implementation
    // name `type` is already used in namespace `boost` but
    // namespace `rdb` is not meant to be imported as a whole
    // so it should do no harm
    struct integer { BOOST_STATIC_CONSTANT(int, id = 1); };
    struct boolean { BOOST_STATIC_CONSTANT(int, id = 2); };
    template<int N> struct varchar { BOOST_STATIC_CONSTANT(int, id = 3); };

    // metafunction that returns a type for holding a value of a given (rdb) type
    // for a specific database
    template<class T, class Tag>
    struct cli_type;

    template<class Type>
    struct placeholder {
      // Empty for statically typed placeholders, but dynamic placeholders
      // contain type and length information 
    };
  }

} }

#endif
