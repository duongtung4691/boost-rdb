#include <boost/rdb/rdb.hpp>

#include "test.hpp"

using namespace boost::rdb;
using namespace boost::rdb::test::springfield;

BOOST_AUTO_TEST_CASE(test_update_table) {

  using namespace boost::rdb;

  person p;
  
  BOOST_RDB_CHECK_SQL(
    (p.id = 1),
    "id = 1");
    
  BOOST_RDB_CHECK_SQL(
    (p.first_name = "Homer"),
    "first_name = 'Homer'");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id = 1),
    "update person set id = 1");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id = 1, p.first_name = "Homer"),
    "update person set id = 1, first_name = 'Homer'");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.age = 46).where(p.id == 1),
    "update person set age = 46 where id = 1");

  // these won't compile
  //update(p).set(p.id = "Homer");
  //update(p).set(p.name = 1);
  //update(p).set(p.age = p.name);
}

