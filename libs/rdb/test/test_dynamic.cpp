#include "test.hpp"

#include <boost/rdb/sql.hpp>
#include <boost/rdb/sql/dynamic.hpp>

using namespace boost;
using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

struct empty { };
struct ptr { void* p; };

BOOST_AUTO_TEST_CASE(test_dynamic_expression_in_predicate) {

  using sql::select;

  person p("p");

  dynamic_boolean predicate = make_dynamic(p.age > 18);
  BOOST_RDB_CHECK_SQL(predicate, "p.age > 18");

  BOOST_RDB_CHECK_SQL(select(p.id).from(p).where(predicate), "select p.id from person as p where p.age > 18");
}

BOOST_AUTO_TEST_CASE(test_dynamic_integer_placeholder) {
  using sql::select;
  person p("p");
  dynamic_boolean predicate = make_dynamic(p.age > _);
  fusion::vector< std::vector<dynamic_placeholder> > placeholders = (select(p.id).from(p).where(predicate)).placeholders();
  BOOST_REQUIRE(fusion::at_c<0>(placeholders).size() == 1);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].type() == rdb::type::integer::id);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].length() == 1);
}

BOOST_AUTO_TEST_CASE(test_dynamic_varchar_placeholder) {
  using sql::select;
  person p("p");
  dynamic_boolean predicate = make_dynamic(p.name == _);
  fusion::vector< std::vector<dynamic_placeholder> > placeholders = (select(p.id).from(p).where(predicate)).placeholders();
  BOOST_REQUIRE(fusion::at_c<0>(placeholders).size() == 1);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].type() == rdb::type::varchar<20>::id);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].length() == 20);
}

BOOST_AUTO_TEST_CASE(test_dynamic_integer_placeholder_insert) {
  person p;
  
  dynamic_columns cols;
  cols.push_back(make_dynamic(p.id));

  dynamic_expressions values;
  values.push_back(make_dynamic(_, p.id));

  BOOST_REQUIRE(fusion::at_c<0>(values.placeholders()).size() == 1);

  BOOST_RDB_CHECK_SQL(insert_into(p)(cols).values(values), "insert into person (id) values (?)");
  
  fusion::vector< std::vector<dynamic_placeholder> > placeholders = 
    (insert_into(p)(cols).values(values)).placeholders();
    
  BOOST_REQUIRE(fusion::at_c<0>(placeholders).size() == 1);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].type() == rdb::type::integer::id);
}

BOOST_AUTO_TEST_CASE(test_dynamic_update) {
  person p;
  dynamic_updates updates;
  updates.push_back(make_dynamic(p.age = 39));
  BOOST_RDB_CHECK_SQL(update(p).set(updates), "update person set age = 39");
  BOOST_RDB_CHECK_SQL(update(p).set(p.first_name = "Homer", updates), "update person set first_name = 'Homer', age = 39");
}

BOOST_AUTO_TEST_CASE(test_dynamic_update_placeholders) {
  person p;

  dynamic_updates updates;
  updates.push_back(make_dynamic(p.age = _));

  BOOST_RDB_CHECK_SQL(update(p).set(updates), "update person set age = ?");

  (update(p).set(p.first_name = _)).placeholders();
  (update(p).set(updates)).placeholders();
  (update(p).set(updates, p.first_name = _)).placeholders();
  (update(p).set(p.first_name = _, updates)).placeholders();

  BOOST_RDB_CHECK_SQL(update(p).set(p.first_name = _, updates), "update person set first_name = ?, age = ?");

  using namespace rdb::type;

  fusion::vector< placeholder< varchar<30> >, std::vector<dynamic_placeholder> > placeholders = 
    (update(p).set(p.first_name = _, updates)).placeholders();

  BOOST_REQUIRE(fusion::at_c<1>(placeholders).size() == 1);
  BOOST_CHECK(fusion::at_c<1>(placeholders)[0].type() == rdb::type::integer::id);
}

BOOST_AUTO_TEST_CASE(test_dynamic_update_where_placeholders) {
  person p;

  dynamic_updates updates;
  updates.push_back(make_dynamic(p.age = _));
  
  dynamic_boolean predicate = make_dynamic(p.first_name == _);

  BOOST_RDB_CHECK_SQL(update(p).set(updates).where(predicate), "update person set age = ? where first_name = ?");

  fusion::vector< std::vector<dynamic_placeholder>, std::vector<dynamic_placeholder> > placeholders = 
    (update(p).set(updates).where(predicate)).placeholders();

  BOOST_REQUIRE(fusion::at_c<0>(placeholders).size() == 1);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].type() == rdb::type::integer::id);
  BOOST_REQUIRE(fusion::at_c<1>(placeholders).size() == 1);
  BOOST_CHECK(fusion::at_c<1>(placeholders)[0].type() == rdb::type::varchar<20>::id);
}
