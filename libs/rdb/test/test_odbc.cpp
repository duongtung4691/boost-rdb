#include <iostream>
#include <sstream>
#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>
#include <boost/fusion/include/io.hpp>

#define BOOST_TEST_MODULE odbc_backend
#include <boost/test/unit_test.hpp>

#include "test_tables.hpp"

using namespace std;
using namespace boost;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

template<typename ResultSet>
std::string str(const ResultSet& results) {
  std::ostringstream os;
  os << results;
  return os.str();
}

struct springfield_fixture {

  database db;

  springfield_fixture() : db("boost", "boost", "boost") {

    try {
      db.execute(create_table(person::_));
    } catch (error) {
      db.execute(drop_table(person::_));
      db.execute(create_table(person::_));
    }

    try {
      db.execute(create_table(partner::_));
    } catch (error) {
      db.execute(drop_table(partner::_));
      db.execute(create_table(partner::_));
    }
    
    person p;
    partner l;

    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
    db.execute(insert_into(l)(l.husband, l.wife).values(1, 2));
  }

  ~springfield_fixture() {
    db.execute(drop_table(person::_));
    db.execute(drop_table(partner::_));
  }
};

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

BOOST_FIXTURE_TEST_CASE(test_null, springfield_fixture) {

  person p;
  db.execute(update(p).set(p.age = null).where(p.id == 1));
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.age).from(p).where(p.id == 1)),
    "((1 null))");
}

BOOST_FIXTURE_TEST_CASE(tx, springfield_fixture) {

  person p;

  if (!db.is_txn_capable())
    return;

  db.execute(update(p).set(p.age = 37).where(p.id == 1));
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 37);

  db.set_autocommit(off);

  db.execute(update(p).set(p.age = 38).where(p.id == 1));
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 38);

  db.rollback();
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 37);

  db.execute(update(p).set(p.age = 38).where(p.id == 1));
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 38);
  db.commit();
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 38);

  db.set_autocommit(on);
  db.execute(update(p).set(p.age = 39).where(p.id == 1));

  db.close();
  db.open("boost", "boost", "boost");
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 39);
}

template<class Results1, class Results2>
vector< pair<string, string> > fetch_parallel(const Results1& results1, const Results2& results2) {
  vector< pair<string, string> > res;
  res.push_back(make_pair(results1.fetch().get<0>(), results2.fetch().get<0>()));
  res.push_back(make_pair(results1.fetch().get<0>(), results2.fetch().get<0>()));
  return res;
}

BOOST_FIXTURE_TEST_CASE(parallel_result_sets, springfield_fixture) {
  person p;
  vector< pair<string, string> > res = fetch_parallel(
    db.execute(select(p.first_name).from(p)),
    db.execute(select(p.first_name).from(p)));
  BOOST_CHECK(res[0].first == "Homer");
  BOOST_CHECK(res[0].first == res[0].second);
  BOOST_CHECK(res[1].first == "Marge");
  BOOST_CHECK(res[1].first == res[1].second);
}

BOOST_FIXTURE_TEST_CASE(parameterless_prepared_statements, springfield_fixture) {
  person p;
  BOOST_AUTO(st, db.prepare(select(p.id, p.first_name, p.name, p.age).from(p)));
  BOOST_RDB_CHECK_SELECT_RESULTS(
    st.execute(),
    "((1 Homer Simpson 37) (2 Marge Simpson 34))"); // WRONG: assumes row order
  // again
  BOOST_RDB_CHECK_SELECT_RESULTS(
    st.execute(),
    "((1 Homer Simpson 37) (2 Marge Simpson 34))"); // WRONG: assumes row order
}