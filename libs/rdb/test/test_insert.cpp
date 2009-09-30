#include <boost/rdb/rdb.hpp>

#include "test.hpp"

using namespace boost::rdb;
using namespace boost::rdb::test::springfield;

BOOST_AUTO_TEST_CASE(test_insert_incremental) {

  person p;

  BOOST_RDB_CHECK_SQL(
    insert_into(p),
    "insert into person");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.name),
    "insert into person (name)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.name),
    "insert into person (id, name)");
}

#if 1

BOOST_AUTO_TEST_CASE(test_insert_values) {

  person p;

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.name).values(p.first_name),
    "insert into person (name) values (first_name)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.name).values(p.age, p.first_name), // meaningless but...
    "insert into person (id, name) values (age, first_name)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.name).values(p.age, p.first_name), // meaningless but...
    "insert into person (id, name) values (age, first_name)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id).values(1),
    "insert into person (id) values (1)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.first_name).values("Homer"),
    "insert into person (first_name) values ('Homer')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.first_name, p.name).values("Homer", "Simpson"),
    "insert into person (first_name, name) values ('Homer', 'Simpson')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.first_name, p.name).values("Homer", "Simpson"),
    "insert into person (first_name, name) values ('Homer', 'Simpson')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.age).values(1, 46),
    "insert into person (id, age) values (1, 46)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.age).values(1, 46),
    "insert into person (id, age) values (1, 46)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.age).values(47, p.id + 1),
    "insert into person (id, age) values (47, id + 1)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.first_name).values(1, "Homer"),
    "insert into person (id, first_name) values (1, 'Homer')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.first_name, p.name).values(1, "Homer", "Simpson"),
    "insert into person (id, first_name, name) values (1, 'Homer', 'Simpson')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.first_name, p.name).values(1, "Homer", "Simpson"),
    "insert into person (id, first_name, name) values (1, 'Homer', 'Simpson')");

  // these won't compile
  //insert_into(p, partner::_.husband); // not in same table
  //insert_into(p, p.name).values(p.id); // type mismatch
}

//BOOST_AUTO_TEST_CASE(insert_set) {
//
//  using namespace boost::rdb;
//
//  person p;
//
//  BOOST_RDB_CHECK_SQL(
//    insert_into(p).set(p.id, 1),
//    "insert into person set id = 1");
//
//  BOOST_RDB_CHECK_SQL(
//    insert_into(p).set(p.id, 1)(p.first_name, "Homer"),
//    "insert into person set id = 1, first_name = 'Homer'");
//}

BOOST_AUTO_TEST_CASE(test_insert_select) {

  person h("h"), w("w");
  partner p;

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.husband, p.wife).
      select(h.id, w.id).from(h, w),
      "insert into partner (husband, wife) select h.id, w.id from person as h, person as w");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.husband, p.wife).
      select(h.id, w.id).from(h, w).where(h.name == w.name),
      "insert into partner (husband, wife) select h.id, w.id from person as h, person as w where h.name = w.name");

  //these won't compile
  //insert_into(p)(p.husband, p.wife).select(h.id).from(h, w).where(h.name == w.name);
  //insert_into(p)(p.husband, p.wife).select(h.id, w.name).from(h, w).where(h.name == w.name);
}

#endif
