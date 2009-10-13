#include "test.hpp"

#include <boost/rdb/sql.hpp>
#include <boost/rdb/sql/dynamic_expression.hpp>

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

struct empty { };
struct ptr { void* p; };

BOOST_AUTO_TEST_CASE(test_dynamic_expression) {
  person p("p");

  dynamic_boolean predicate = make_dynamic(p.age > 18);
  BOOST_RDB_CHECK_SQL(predicate, "p.age > 18");

  typedef BOOST_TYPEOF(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, _))::placeholder_vector placeholder_vector;

  BOOST_RDB_CHECK_SQL(select(p.id).from(p).where(predicate), "select p.id from person as p where p.age > 18");
}


