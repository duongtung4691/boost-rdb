#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>

#include "test_tables.hpp"

using namespace std;
using namespace boost::rdb;
using namespace boost::rdb::test::springfield;

template<class Statement>
std::string str(const Statement& statement) {
  std::ostringstream os;
  statement.str(os);
  return os.str();
}

template<class SelectList, class FromList, class WhereList>
std::string str(const select_type<SelectList, FromList, WhereList>& select) {
  std::ostringstream os;
  select.str(os);
  return os.str();
}

#define scope

#define BOOST_RDB_CHECK_SQL(expr, sql) BOOST_CHECK(str(expr) == sql)

int test_main( int, char *[] )
{
  using namespace boost::rdb;
  using boost::rdb::select;

  BOOST_RDB_CHECK_SQL(create_table<person>(),
    "create table person(id integer, name varchar(20), first_name varchar(30), age integer)");

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

  scope {
    person husband;
    person_<1> wife("wife");
    BOOST_RDB_CHECK_SQL(
      select(husband.id)(wife.name).from(husband)(wife),
      "select id, wife.name from person, person as wife");
  }
  
  scope {
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

  scope {
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
      select(p1.id).from(p1)(p2).where((p1.id + p2.age) > p1.age),
      "select p1.id from person as p1, person as p2 where (p1.id + p2.age) > p1.age");
  }

  scope {
  
    person p("p");

    BOOST_RDB_CHECK_SQL(
      select(p.id).from(p).where(!(p.age > 18)),
      "select p.id from person as p where not (p.age > 18)");
  }

  scope {
  
    person p("p");

    BOOST_RDB_CHECK_SQL(
      select(p.id).from(p).where(p.age > 18 && p.age < 65),
      "select p.id from person as p where (p.age > 18) and (p.age < 65)");
  }

  scope {
    using namespace boost::rdb::comma;
    person p("p");

    BOOST_RDB_CHECK_SQL(
      select((p.id, p.name)).from(p),
      "select p.id, p.name from person as p");

    BOOST_RDB_CHECK_SQL(
      select((p.id, p.name, p.age)).from(p),
      "select p.id, p.name, p.age from person as p");
  }

  scope {
    person p;

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.name).values(p.first_name),
      "insert into person (name) values (first_name)");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.id)(p.name).values(p.age)(p.first_name), // meaningless but...
      "insert into person (id, name) values (age, first_name)");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.id).values(1),
      "insert into person (id) values (1)");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.first_name).values("Homer"),
      "insert into person (first_name) values ('Homer')");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.first_name)(p.name).values("Homer")("Simpson"),
      "insert into person (first_name, name) values ('Homer', 'Simpson')");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.id)(p.age).values(1)(46),
      "insert into person (id, age) values (1, 46)");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.id)(p.first_name).values(1)("Homer"),
      "insert into person (id, first_name) values (1, 'Homer')");

    BOOST_RDB_CHECK_SQL(
      insert_into<person>(p.id)(p.first_name)(p.name).values(1)("Homer")("Simpson"),
      "insert into person (id, first_name, name) values (1, 'Homer', 'Simpson')");

    // these won't compile
    //insert_into<person>(partner::_.husband); // not in same table
    //insert_into<person>(p.name).values(p.id); // type mismatch
  }
      
  return 0;
}
