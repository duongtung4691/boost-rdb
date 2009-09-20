#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>
#include <boost/fusion/include/io.hpp>

#include "test_tables.hpp"

using namespace std;
using namespace boost::rdb;
using namespace boost::rdb::odbc;
using namespace boost::rdb::test::springfield;

template<typename T>
ostream& operator <<(ostream& os, const deque<T>& seq) {
  const char* sep = "";
  os << "(";
  typename deque<T>::const_iterator iter = seq.begin(), last = seq.end();
  while (iter != last) {
    os << sep;
    os << *iter;
    sep = " ";
    ++iter;
  }
  return os << ")";
}

template<typename T>
string str(const deque<T>& seq) {
  ostringstream os;
  os << seq;
  return os.str();
}

#define BOOST_RDB_CHECK_SELECT_RESULTS(expr, expected) BOOST_CHECK(str(expr) == expected)

#define scope

int test_main( int, char *[] )
{
  database db("boost", "boost", "boost");

  try {
    db.drop_table<person>();
    db.drop_table<partner>();
  } catch (error) {
  }

  db.create_table<person>();
  db.create_table<partner>();
  
  person p;

  db.execute(insert_into<person>(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
  db.execute(insert_into<person>(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));

  using boost::rdb::select;

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Homer Simpson 37) (2 Marge Simpson 34))"); // WRONG: assumes row order
 
  return 0;
}
//( ((ostringstream() << db.execute(select(p.id, p.first_name, p.name, p.age).from(p))).str() == "((1 Homer Simpson 37) (2 Marge Simpson 34))") ? static_cast<void>(0) : boost::minimal_test::report_error("(ostringstream() << db.execute(select(p.id, p.first_name, p.name, p.age).from(p))).str() == \"((1 Homer Simpson 37) (2 Marge Simpson 34))\"","c:\\users\\jll\\documents\\visual studio 2008\\projects\\rdb\\libs\\rdb\\test\\test_odbc.cpp",55,  __FUNCSIG__  ) ); 
