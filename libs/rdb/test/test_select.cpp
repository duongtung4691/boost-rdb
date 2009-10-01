#include <boost/rdb/rdb.hpp>

#include "test.hpp"

using namespace boost::rdb;
using namespace boost::rdb::test::springfield;

BOOST_AUTO_TEST_CASE(test_select_temp) {
  using namespace boost::rdb;
  using boost::rdb::select;
  
  person p;
  using namespace boost;
  //boost::fusion::make_map(boost::fusion::make_pair<int>(1), boost::fusion::make_pair<double>(1));

  BOOST_RDB_CHECK_SQL(
    select(p.id),
    "select id");

  BOOST_RDB_CHECK_SQL(
    select(1),
    "select 1");

  BOOST_RDB_CHECK_SQL(
    select(p.id, p.name),
    "select id, name");

  BOOST_RDB_CHECK_SQL(
    select.distinct(p.name),
    "select distinct name");

  BOOST_RDB_CHECK_SQL(
    select.all(p.name),
    "select all name");
}

BOOST_AUTO_TEST_CASE(select_simple) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p),
    "select id from person");
}

BOOST_AUTO_TEST_CASE(select_from) {

  using namespace boost::rdb;
  using boost::rdb::select;
  
  person p;

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p),
    "select id from person");

  BOOST_RDB_CHECK_SQL(
    select(p.id, p.age).from(p),
    "select id, age from person");

  BOOST_RDB_CHECK_SQL(
    select(person::_.id).from(person::_),
    "select id from person");
}

BOOST_AUTO_TEST_CASE(select_literals) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id, 1).from(p),
    "select id, 1 from person");
  
  BOOST_RDB_CHECK_SQL(
    select(1, p.id).from(p),
    "select 1, id from person");
}

BOOST_AUTO_TEST_CASE(simple_where_clause) {

  using namespace boost::rdb;
  using boost::rdb::select;
  
  person p;
  partner l;
  
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
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p, l).where(p.id.in(select(l.husband).from(l))),
    "select id from person, partner where id in (select husband from partner)");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.id.in(1)),
    "select id from person where id in (1)");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.id.in(1, 2)),
    "select id from person where id in (1, 2)");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.id.in(p.age + 1, 2)),
    "select id from person where id in (age + 1, 2)");
    
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
    select(husband.id, wife.name).from(husband, wife),
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
    select(p1.id + p2.id).from(p1, p2),
    "select p1.id + p2.id from person as p1, person as p2");

  BOOST_RDB_CHECK_SQL(
    select(p1.age + 1).from(p1),
    "select p1.age + 1 from person as p1");

  BOOST_RDB_CHECK_SQL(
    select(1 + p1.age).from(p1),
    "select 1 + p1.age from person as p1");

  BOOST_RDB_CHECK_SQL(
    select(p1.age + p2.age / 2).from(p1, p2),
    "select p1.age + p2.age / 2 from person as p1, person as p2");

  BOOST_RDB_CHECK_SQL(
    select((p1.age + p2.age) / 2).from(p1, p2),
    "select (p1.age + p2.age) / 2 from person as p1, person as p2");

  BOOST_RDB_CHECK_SQL(
    select(p1.id).from(p1, p2).where(p1.id + p2.age > p1.age),
    "select p1.id from person as p1, person as p2 where p1.id + p2.age > p1.age");

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(!(p.age > 18)),
    "select p.id from person as p where not (p.age > 18)");

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age > 18 && p.age < 65),
    "select p.id from person as p where p.age > 18 and p.age < 65");
}

BOOST_AUTO_TEST_CASE(char_operators) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person p;

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.name.like("S%")),
    "select id from person where name like 'S%'");

  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.name.like("O'%")),
    "select id from person where name like 'O''%'");
    
  // won't compile:
  // select(p.id).from(p).where(p.id.like("X%"));
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

BOOST_AUTO_TEST_CASE(select_exists) {

  using namespace boost::rdb;
  using boost::rdb::select;

  person m("m");
  partner p;
  
  BOOST_RDB_CHECK_SQL(
    select(m.id).from(m).where(exists(
      select(p.husband).from(p).where(m.id == p.husband)
      )),
    "select m.id from person as m where exists (select husband from partner where m.id = husband)");
}
