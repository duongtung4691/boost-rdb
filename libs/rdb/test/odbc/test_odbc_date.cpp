#include "test_odbc.hpp"
#include <boost/rdb/sql/dynamic.hpp>
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

}

BOOST_AUTO_TEST_CASE(date_type) {
  
  using sql::select;
  
  database db("boost", "boost", "boost");
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
