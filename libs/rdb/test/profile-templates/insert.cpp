#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>

#include "../rdb/test_tables.hpp"

using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

void test() {
  database db("boost", "boost", "boost");
  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
}
