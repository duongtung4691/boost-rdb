#include "test_odbc.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;

namespace {

BOOST_RDB_BEGIN_TABLE(test1)
  BOOST_RDB_COLUMN(id, integer)
  BOOST_RDB_COLUMN(val, datetime)
BOOST_RDB_END_TABLE(test1)

struct fixture {

  boost::rdb::odbc::database db;

  fixture() : db("boost", "boost", "boost") {
    test1 test;
    try {
      db.execute(create_table(test));
    } catch (odbc_error) {
      try {
        db.execute(drop_table(test));
      } catch (...) {
      }
      db.execute(create_table(test));
    }
  }

  ~fixture() {
    using namespace boost::rdb::sql::test::springfield;
    db.execute(drop_table(test1::_));
  }
};

}

BOOST_FIXTURE_TEST_CASE(date_type, fixture) {

  using sql::select;

  test1 t;

  db.execute(insert_into(t)(t.id, t.val).values(1, "1963-08-13 03:11:17"));
  BOOST_RDB_CHECK_SELECT_RESULTS(db.execute(select(t.id).from(t)), "((1))");

  BOOST_AUTO(st, db.prepare(select(t.val).from(t)));
  varchar<30> val;
  st.bind_results(val);
  st.execute().fetch();
  BOOST_CHECK(!val.is_null());
  BOOST_CHECK_EQUAL(val.value(), "1963-08-13 03:11:17");
}
