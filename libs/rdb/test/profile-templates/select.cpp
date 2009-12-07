#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>

#include "../rdb/test_tables.hpp"

using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

void test() {
  database db("boost", "boost", "boost");
  using boost::rdb::sql::select;
  person p;
  db.execute(select(p.first_name, p.age).from(p).where(p.id == 1));
}
