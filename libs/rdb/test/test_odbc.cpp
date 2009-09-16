#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>

using namespace std;
using namespace boost::rdb;

#define scope

BOOST_RDB_BEGIN_TABLE(person) 
  BOOST_RDB_COLUMN(id, integer)
  BOOST_RDB_COLUMN(name, varchar<20>)
BOOST_RDB_END_TABLE(person) 

int test_main( int, char *[] )
{
  using namespace boost::rdb;
  using namespace boost::rdb::odbc;

  database db("boost", "boost", "boost");
  db.drop_table<person>();
  db.create_table<person>();
      
  return 0;
}
