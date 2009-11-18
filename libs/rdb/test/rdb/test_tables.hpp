//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_TEST_TABLES_HPP
#define BOOST_RDB_TEST_TABLES_HPP

namespace boost { namespace rdb { namespace sql { namespace test {

  namespace springfield {
//[ schema
    BOOST_RDB_BEGIN_TABLE(person) 
      BOOST_RDB_COLUMN(id, integer)
      BOOST_RDB_COLUMN(name, varchar<20>)
      BOOST_RDB_COLUMN(first_name, varchar<30>)
      BOOST_RDB_COLUMN(age, float_)
    BOOST_RDB_END_TABLE(person) 

    BOOST_RDB_BEGIN_TABLE(partner) 
      BOOST_RDB_COLUMN(husband, integer)
      BOOST_RDB_COLUMN(wife, integer)
    BOOST_RDB_END_TABLE(partner) 
//]
  }
  
  namespace object_model {
    BOOST_RDB_BEGIN_TABLE(person) 
      BOOST_RDB_COLUMN(id, integer)
      BOOST_RDB_COLUMN(nationality, varchar<20>)
    BOOST_RDB_END_TABLE(person) 
    
    BOOST_RDB_BEGIN_TABLE(natural_person) 
      BOOST_RDB_COLUMN(id, integer)
      BOOST_RDB_COLUMN(name, varchar<20>)
      BOOST_RDB_COLUMN(first_name, varchar<30>)
      BOOST_RDB_COLUMN(age, integer)
    BOOST_RDB_END_TABLE(natural_person) 
    
    BOOST_RDB_BEGIN_TABLE(legal_person) 
      BOOST_RDB_COLUMN(id, integer)
      BOOST_RDB_COLUMN(name, varchar<20>)
    BOOST_RDB_END_TABLE(legal_person) 
  }

} } } }

#endif