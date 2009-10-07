#include <boost/rdb/sql/select.hpp>

#include "test.hpp"

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

BOOST_AUTO_TEST_CASE(test_select_temp) {
  using namespace boost::rdb::sql;
  
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
    select.distinct(p.name),
    "select distinct name");

  BOOST_RDB_CHECK_SQL(
    select.all(p.name),
    "select all name");
}

BOOST_AUTO_TEST_CASE(select_simple) {

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p),
    "select id from person");
}

BOOST_AUTO_TEST_CASE(select_from) {

  using namespace boost::rdb::sql;
  
  person p;

  // this will not compile
  //select(p.id).from(p).from(p);

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

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id, 1).from(p),
    "select id, 1 from person");
  
  BOOST_RDB_CHECK_SQL(
    select(1, p.id).from(p),
    "select 1, id from person");
}

BOOST_AUTO_TEST_CASE(simple_where_clause) {

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
  // select(p.id).from(p).where(p.name == 666);
  // select(p.id).from(p).where(p.name == p.id);
}

BOOST_AUTO_TEST_CASE(alias) {

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

BOOST_AUTO_TEST_CASE(test_qualified) {

  person::qualified p;
  partner::qualified l;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p, l).where(p.id == l.wife),
    "select person.id from person, partner where person.id = partner.wife");
}

BOOST_AUTO_TEST_CASE(test_typed_alias) {

  person::_1 p("p");
  partner::_1 l("l");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p, l).where(p.id == l.wife),
    "select p.id from person as p, partner as l where p.id = l.wife");
}

BOOST_AUTO_TEST_CASE(numerical_operators) {

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

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id, p.name).from(p),
    "select id, name from person");

  BOOST_RDB_CHECK_SQL(
    select(p.id, p.name, p.age).from(p),
    "select id, name, age from person");
}

BOOST_AUTO_TEST_CASE(select_exists) {

  person m("m");
  partner p;
  
  BOOST_RDB_CHECK_SQL(
    select(m.id).from(m).where(exists(
      select(p.husband).from(p).where(m.id == p.husband)
      )),
    "select m.id from person as m where exists (select husband from partner where m.id = husband)");
}

BOOST_AUTO_TEST_CASE(test_is_null) {

  person p;
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age == null),
    "select id from person where age is null");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(!(p.age == null)),
    "select id from person where not (age is null)");
  
  BOOST_RDB_CHECK_SQL(
    select(p.id).from(p).where(p.age != null),
    "select id from person where age is not null");
}

BOOST_AUTO_TEST_CASE(locals) {

  person_<1> p1("p1");
  person_<2> p2("p2");
  
  BOOST_AUTO(s, select(p1.id).from(p1, p2).where(p1.id + p2.age > p1.age));
  
  BOOST_RDB_CHECK_SQL(
    s,
    "select p1.id from person as p1, person as p2 where p1.id + p2.age > p1.age");
}

