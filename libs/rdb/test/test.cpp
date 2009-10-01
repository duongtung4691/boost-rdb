#define BOOST_TEST_MODULE sql_composer
#include <boost/test/unit_test.hpp>

#include <boost/rdb/sql.hpp>
#include "test.hpp"

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
