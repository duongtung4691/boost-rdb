//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_DATABASE_HPP
#define BOOST_RDB_DATABASE_HPP

namespace boost { namespace rdb {

  template<class Specific>
  class generic_database {

    Specific& spec() { return static_cast<Specific&>(*this); }

  public:
  };

} }

#endif