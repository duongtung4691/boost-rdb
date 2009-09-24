#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>
#include <boost/fusion/include/io.hpp>

#include "test_tables.hpp"

using namespace std;
namespace rdb = boost::rdb;
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
    db.execute(drop_table(person::_));
    db.execute(drop_table(partner::_));
  } catch (error) {
  }

  db.execute(create_table(person::_));
  db.execute(create_table(partner::_));
  
  person p;

  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
  db.execute(update(p).set(p.age, p.age + 1).where(p.id == 1));

  {
    person h("h"), w("w");
    partner p;
    db.execute(insert_into(p)(p.husband, p.wife)(rdb::select(h.id, w.id).from(h, w)
      .where(h.name == w.name && h.id != w.id))); // duplicates couples but it's just a test
  }

  using boost::rdb::select;

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Homer Simpson 38) (2 Marge Simpson 34))"); // WRONG: assumes row order
 
  return 0;
}
