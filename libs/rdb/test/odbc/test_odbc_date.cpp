#include "test_odbc.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/rdb/odbc/date.hpp>
#include <boost/date_time/gregorian/gregorian_io.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;

namespace {

BOOST_RDB_BEGIN_TABLE(test_date)
  BOOST_RDB_COLUMN(id, integer)
  BOOST_RDB_COLUMN(val, date)
BOOST_RDB_END_TABLE(test_date)

struct fixture {

  boost::rdb::odbc::database db;

  fixture() : db("boost", "boost", "boost") {
    test_date test;
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
    using namespace boost::rdb::test::springfield;
    //db.execute(drop_table(test_date::_));
  }
};

}

BOOST_FIXTURE_TEST_CASE(test_date_bind_varchar, fixture) {

  using sql::select;

  test_date t;

  varchar<30> write = "1963-08-13";
  db.prepare(insert_into(t)(t.id, t.val).values(1, _)).bind_parameters(write).execute();

  varchar<30> read;
  db.prepare(select(t.val).from(t)).bind_results(read).execute().fetch();
  BOOST_CHECK(!read.is_null());
  BOOST_CHECK_EQUAL(read.value(), "1963-08-13");
}

BOOST_FIXTURE_TEST_CASE(test_date_bind_date, fixture) {

  using sql::select;

  test_date t;
  
  date write(1963, 8, 13);
  db.prepare(insert_into(t)(t.id, t.val).values(1, _)).bind_parameters(write).execute();

  date read;
  db.prepare(select(t.val).from(t)).bind_results(read).execute().fetch();
  
  BOOST_CHECK(!read.is_null());
  BOOST_CHECK_EQUAL(read.value().year, 1963);
  BOOST_CHECK_EQUAL(read.value().month, 8);
  BOOST_CHECK_EQUAL(read.value().day, 13);
}

BOOST_FIXTURE_TEST_CASE(test_date_insert_date_select, fixture) {
  using sql::select;
  test_date t;
  date write(1963, 8, 13);
  db.execute(insert_into(t)(t.id, t.val).values(1, write));
  BOOST_RDB_CHECK_SELECT_RESULTS(db.execute(select(t.id, t.val).from(t)), "((1 1963-Aug-13))");
}

BOOST_FIXTURE_TEST_CASE(test_date_insert_string_select, fixture) {
  using sql::select;
  test_date t;
  db.execute(insert_into(t)(t.id, t.val).values(1, "1963-08-13"));
  BOOST_RDB_CHECK_SELECT_RESULTS(db.execute(select(t.id, t.val).from(t)), "((1 1963-Aug-13))");
}
