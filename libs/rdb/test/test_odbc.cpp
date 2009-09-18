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
  db.execute(insert_into<person>(p.id, p.first_name).values(1)("Homer"));
  //insert_into<person>(p.first_name)(p.name).values("Homer")("Simpson")      
  /*
  using namespace boost;
  BOOST_MPL_ASSERT((boost::is_same<
    fusion::result_of::make_vector<int, char>::type,
    fusion::result_of::push_back< fusion::result_of::make_vector<int>::type, char >::type
    >));
  */
  return 0;
}
