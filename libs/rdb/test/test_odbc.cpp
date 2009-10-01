#include <iostream>
#include <sstream>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>
#include <boost/fusion/include/io.hpp>

#define BOOST_TEST_MODULE odbc_backend
#include <boost/test/unit_test.hpp>

#include "test_tables.hpp"

using namespace boost;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

template<typename T>
std::ostream& operator <<(std::ostream& os, const std::deque<T>& seq) {
  const char* sep = "";
  os << "(";
  typename std::deque<T>::const_iterator iter = seq.begin(), last = seq.end();
  while (iter != last) {
    os << sep;
    os << *iter;
    sep = " ";
    ++iter;
  }
  return os << ")";
}

template<typename T>
std::string str(const std::deque<T>& seq) {
  std::ostringstream os;
  os << seq;
  return os.str();
}

#define BOOST_RDB_CHECK_SELECT_RESULTS(expr, expected) BOOST_CHECK(str(expr) == expected)

BOOST_AUTO_TEST_CASE(basic) {
  database db("boost", "boost", "boost");

  try {
    db.execute(drop_table(person::_));
  } catch (error) {
  }

  try {
    db.execute(drop_table(partner::_));
  } catch (error) {
  }

  db.execute(create_table(person::_));
  db.execute(create_table(partner::_));
  
  person p;

  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
  db.execute(update(p).set(p.age = p.age + 1).where(p.id == 1));

  {
    person h("h"), w("w");
    partner p;
    db.execute(insert_into(p)(p.husband, p.wife)
      .select(h.id, w.id).from(h, w)
      .where(h.name == w.name && h.id != w.id)); // duplicates couples but it's just a test
  }

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Homer Simpson 38) (2 Marge Simpson 34))"); // WRONG: assumes row order
}

BOOST_AUTO_TEST_CASE(tx) {

  database db("boost", "boost", "boost");
  person p;

  if (!db.is_txn_capable())
    return;

  db.execute(update(p).set(p.age = 37).where(p.id == 1));
  BOOST_CHECK(fusion::at_c<0>(db.execute(select(p.age).from(p).where(p.id == 1))[0]) == 37);

  db.set_autocommit(off);

  db.execute(update(p).set(p.age = 38).where(p.id == 1));
  BOOST_CHECK(fusion::at_c<0>(db.execute(select(p.age).from(p).where(p.id == 1))[0]) == 38);

  db.rollback();
  BOOST_CHECK(fusion::at_c<0>(db.execute(select(p.age).from(p).where(p.id == 1))[0]) == 37);

  db.execute(update(p).set(p.age = 38).where(p.id == 1));
  BOOST_CHECK(fusion::at_c<0>(db.execute(select(p.age).from(p).where(p.id == 1))[0]) == 38);
  db.commit();
  BOOST_CHECK(fusion::at_c<0>(db.execute(select(p.age).from(p).where(p.id == 1))[0]) == 38);

  db.set_autocommit(on);
  db.execute(update(p).set(p.age = 39).where(p.id == 1));

  db.close();
  db.open("boost", "boost", "boost");
  BOOST_CHECK(fusion::at_c<0>(db.execute(select(p.age).from(p).where(p.id == 1))[0]) == 39);
}