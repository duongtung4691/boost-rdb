#include "test.hpp"

#include <boost/rdb/sql.hpp>
#include <boost/rdb/sql/dynamic_expression.hpp>

using namespace boost;
using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

struct empty { };
struct ptr { void* p; };

BOOST_AUTO_TEST_CASE(test_dynamic_expression) {
  person p("p");

  dynamic_boolean predicate = make_dynamic(p.age > 18);
  BOOST_RDB_CHECK_SQL(predicate, "p.age > 18");

  BOOST_RDB_CHECK_SQL(select(p.id).from(p).where(predicate), "select p.id from person as p where p.age > 18");
}

BOOST_AUTO_TEST_CASE(test_dynamic_placeholder) {
  person p("p");
  dynamic_boolean predicate = make_dynamic(p.age > _);
  fusion::vector< std::vector<dynamic_placeholder> > placeholders = (select(p.id).from(p).where(predicate)).placeholders();
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].type() == rdb::type::integer::id);
  BOOST_CHECK(fusion::at_c<0>(placeholders)[0].length() == 1);
}


