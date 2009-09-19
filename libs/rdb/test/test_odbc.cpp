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
  BOOST_AUTO(sel1, insert_into<person>(p.id));//
  BOOST_AUTO(sel2, insert_into<person>(p.id, p.first_name));//
  BOOST_AUTO(sel5, insert_into<person>(p.id)(p.first_name));//
  BOOST_AUTO(sel4, sel2.values(1));//
  BOOST_AUTO(sel3, insert_into<person>(p.id, p.first_name).values(1));//
  //BOOST_AUTO(sel3, insert_into<person>(p.id)(p.first_name).values(1)("Homer"));//
  db.execute(insert_into<person>(p.id, p.first_name).values(1)("Homer"));
  //db.execute(insert_into<person>(p.id)(p.first_name).values(1)("Homer"));
  //db.execute(insert_into<person>(p.id, p.first_name, p.name).values(1)("Homer")("Simpson"));

  return 0;
}
