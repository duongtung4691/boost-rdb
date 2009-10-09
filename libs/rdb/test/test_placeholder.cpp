#include <boost/rdb/sql/update.hpp>
#include <boost/rdb/sql/placeholder.hpp>

#include "test.hpp"

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

BOOST_AUTO_TEST_CASE(test_placeholder) {

  using namespace boost::rdb::sql;

  person p;

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id = _),
    "update person set id = %");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.name = _),
    "update person set name = %");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id = p.age + _),
    "update person set id = age + %");

  //BOOST_RDB_CHECK_SQL(
  //  update(p).set(p.age = 33).where(p.name.like(_)),
  //  "update person set age = 33 where name like %");
}
