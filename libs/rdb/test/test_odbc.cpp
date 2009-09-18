#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>

#include "test_tables.hpp"

using namespace std;
using namespace boost::rdb;
using namespace boost::rdb::test::springfield;

#define scope

int test_main( int, char *[] )
{
  using namespace boost::rdb;
  using namespace boost::rdb::odbc;

  database db("boost", "boost", "boost");

  try {
    db.drop_table<person>();
    db.drop_table<partner>();
  } catch (error) {
  }

  db.create_table<person>();
  db.create_table<partner>();
  
  person p;
  db.execute(insert_into<person>(p.id).values(1));
      
  return 0;
}
