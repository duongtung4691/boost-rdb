#include <boost/rdb/rdb.hpp>

#include "test.hpp"

using namespace boost::rdb;
using namespace boost::rdb::test::springfield;

BOOST_AUTO_TEST_CASE(update_table) {

  using namespace boost::rdb;

  person p;

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id, 1),
    "update person set id = 1");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id, 1).set(p.first_name, "Homer"),
    "update person set id = 1, first_name = 'Homer'");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.age, 46).where(p.id == 1),
    "update person set age = 46 where id = 1");
}

