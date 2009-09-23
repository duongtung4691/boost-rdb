#include <iostream>
#include <sstream>

#define BOOST_TEST_MODULE sql_composer
#include <boost/test/unit_test.hpp>
#include <boost/rdb/rdb.hpp>
#include "test_tables.hpp"

// Visual Studio regex to make error output readable
// (boost|std|fusion|rdb|test|springfield|details)\:\:

template<class Statement>
std::string str(const Statement& statement) {
  std::ostringstream os;
  statement.str(os);
  return os.str();
}

namespace rdb = boost::rdb;

template<class SelectList, class FromList, class WhereList>
std::string str(const rdb::select_type<SelectList, FromList, WhereList>& select) {
  std::ostringstream os;
  select.str(os);
  return os.str();
}

#define scope

#define BOOST_RDB_CHECK_SQL(expr, sql) BOOST_CHECK(str(expr) == sql)

using namespace std;
using namespace boost::rdb::test::springfield;

BOOST_AUTO_TEST_CASE(create_statement) {

  using namespace boost::rdb;

  BOOST_RDB_CHECK_SQL(create_table<person>(),
    "create table person(id integer, name varchar(20), first_name varchar(30), age integer)");
}

BOOST_AUTO_TEST_CASE(insert_values) {

  using namespace boost::rdb;

  person p;

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.name).values(p.first_name),
    "insert into person (name) values (first_name)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id)(p.name).values(p.age)(p.first_name), // meaningless but...
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
    insert_into(p)(p.first_name)(p.name).values("Homer")("Simpson"),
    "insert into person (first_name, name) values ('Homer', 'Simpson')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.first_name, p.name).values("Homer")("Simpson"),
    "insert into person (first_name, name) values ('Homer', 'Simpson')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id)(p.age).values(1)(46),
    "insert into person (id, age) values (1, 46)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.age).values(1, 46),
    "insert into person (id, age) values (1, 46)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.age).values(47, p.id + 1),
    "insert into person (id, age) values (47, id + 1)");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id)(p.first_name).values(1)("Homer"),
    "insert into person (id, first_name) values (1, 'Homer')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id)(p.first_name)(p.name).values(1)("Homer")("Simpson"),
    "insert into person (id, first_name, name) values (1, 'Homer', 'Simpson')");

  BOOST_RDB_CHECK_SQL(
    insert_into(p)(p.id, p.first_name, p.name).values(1, "Homer", "Simpson"),
    "insert into person (id, first_name, name) values (1, 'Homer', 'Simpson')");

  // these won't compile
  //insert_into(p)(partner::_.husband); // not in same table
  //insert_into(p)(p.name).values(p.id); // type mismatch
}

BOOST_AUTO_TEST_CASE(insert_set) {

  using namespace boost::rdb;

  person p;

  BOOST_RDB_CHECK_SQL(
    insert_into(p).set(p.id, 1),
    "insert into person set id = 1");

  BOOST_RDB_CHECK_SQL(
    insert_into(p).set(p.id, 1).set(p.first_name, "Homer"),
    "insert into person set id = 1, set first_name = 'Homer'");
}

BOOST_AUTO_TEST_CASE(update_set) {

  using namespace boost::rdb;

  person p;

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id, 1),
    "update person set id = 1");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id, 1).set(p.first_name, "Homer"),
    "update person set id = 1, set first_name = 'Homer'");
}

BOOST_AUTO_TEST_CASE(select_from) {

  using namespace boost::rdb;
  using boost::rdb::select;
  
  scope {
    person husband;
    BOOST_RDB_CHECK_SQL(
      select(husband.id).from(husband),
      "select id from person");
  }

  scope {
    BOOST_RDB_CHECK_SQL(
      select(person::_.id).from(person::_),
      "select id from person");
  }
}

BOOST_AUTO_TEST_CASE(select_literals) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id)(1).from(p),
    "select id, 1 from person");
  
  BOOST_RDB_CHECK_SQL(
    select(1)(p.id).from(p),
    "select 1, id from person");
}

BOOST_AUTO_TEST_CASE(simple_where_clause) {

  using namespace boost::rdb;
  using boost::rdb::select;
  
  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age == p.id),
    "select id from person where age = id");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age > p.id),
    "select id from person where age > id");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age != p.id),
    "select id from person where age <> id");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.name == p.name),
    "select id from person where name = name");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.name == "Simpson"),
    "select id from person where name = 'Simpson'");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.name == "O'Hara"),
    "select id from person where name = 'O''Hara'");
    
  // these won't compile    
  #if 0
  select(p.id).from(p).where(p.name == 666);
  #endif
  #if 0
  select(p.id).from(p).where(p.name == p.id);
  #endif
}

BOOST_AUTO_TEST_CASE(alias) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person husband;
  person_<1> wife("wife");
  
  BOOST_RDB_CHECK_SQL(
    select(husband.id)(wife.name).from(husband)(wife),
    "select id, wife.name from person, person as wife");

  person_<1> p1("p1");
  person_<2> p2("p2");

  BOOST_RDB_CHECK_SQL(
    select(p1.id).from(p1),
    "select p1.id from person as p1");

  BOOST_RDB_CHECK_SQL(
    select(p1.id, p2.id).from(p1, p2),
    "select p1.id, p2.id from person as p1, person as p2");
}

BOOST_AUTO_TEST_CASE(numerical_operators) {
  using namespace boost::rdb;
  using boost::rdb::select;

  person p("p");
  person_<1> p1("p1");
  person_<2> p2("p2");

  BOOST_RDB_CHECK_SQL(
    select(p1.id + p2.id).from(p1)(p2),
    "select p1.id + p2.id from person as p1, person as p2");

  BOOST_RDB_CHECK_SQL(
    select(p1.age + 1).from(p1),
    "select p1.age + 1 from person as p1");

  BOOST_RDB_CHECK_SQL(
    select(1 + p1.age).from(p1),
    "select 1 + p1.age from person as p1");

  BOOST_RDB_CHECK_SQL(
    select(p1.age + p2.age / 2).from(p1)(p2),
    "select p1.age + p2.age / 2 from person as p1, person as p2");

  BOOST_RDB_CHECK_SQL(
    select((p1.age + p2.age) / 2).from(p1)(p2),
    "select (p1.age + p2.age) / 2 from person as p1, person as p2");

  BOOST_RDB_CHECK_SQL(
    select(p1.id).from(p1)(p2).where(p1.id + p2.age > p1.age),
    "select p1.id from person as p1, person as p2 where p1.id + p2.age > p1.age");

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(!(p.age > 18)),
    "select p.id from person as p where not (p.age > 18)");

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age > 18 && p.age < 65),
    "select p.id from person as p where p.age > 18 and p.age < 65");
}

BOOST_AUTO_TEST_CASE(select_variadic) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id, p.name).from(p),
    "select id, name from person");

  BOOST_RDB_CHECK_SQL(
    select(p.id, p.name, p.age).from(p),
    "select id, name, age from person");
}

BOOST_AUTO_TEST_CASE(select_comma_operator) {

  using namespace boost::rdb;
  using boost::rdb::select;
  using namespace boost::rdb::comma;

  person p("p");

  BOOST_RDB_CHECK_SQL(
    select((p.id, p.name)).from(p),
    "select p.id, p.name from person as p");

  BOOST_RDB_CHECK_SQL(
    select((p.id, p.name, p.age)).from(p),
    "select p.id, p.name, p.age from person as p");
}
