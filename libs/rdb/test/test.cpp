#define BOOST_TEST_MODULE sql_composer
#include <boost/test/unit_test.hpp>

#include "test.hpp"

#include <boost/rdb/sql/delete.hpp>
#include <boost/rdb/sql/table.hpp>
#include <boost/rdb/sql/select.hpp>
#include <boost/rdb/sql/verbatim.hpp>

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

BOOST_AUTO_TEST_CASE(create_statement) {

  BOOST_RDB_CHECK_SQL(create_table(person::_),
    "create table person(id integer, name varchar(20), first_name varchar(30), age integer)");
}

BOOST_AUTO_TEST_CASE(delete_from_table) {

  person p;

  BOOST_RDB_CHECK_SQL(
    delete_from(p),
    "delete from person");


  BOOST_RDB_CHECK_SQL(
    delete_from(p).where(p.id == 1),
    "delete from person where id = 1");
}

BOOST_AUTO_TEST_CASE(test_verbatim) {

  person p;
  using namespace boost::rdb::type;
  BOOST_RDB_CHECK_SQL(
    select(verbatim<integer>("count(*)")).from(p),
    "select count(*) from person");
}
