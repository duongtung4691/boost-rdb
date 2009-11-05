#ifndef BOOST_RDB_TEST_ODBC_HPP
#define BOOST_RDB_TEST_ODBC_HPP

#include <iostream>
#include <sstream>
#include <boost/fusion/include/io.hpp>
#include <boost/rdb/sql.hpp>
#include <boost/rdb/sql/dynamic.hpp>
#include <boost/rdb/odbc.hpp>

#include "../rdb/test_tables.hpp"

template<typename ResultSet>
std::string str_result_set(const ResultSet& results) {
  std::ostringstream os;
  os << results;
  return os.str();
}

struct springfield_fixture {

  boost::rdb::odbc::database db;

  springfield_fixture() : db("boost", "boost", "boost") {
    using namespace boost::rdb::odbc;
    using namespace boost::rdb::sql::test::springfield;
    try {
      db.execute(create_table(person::_));
    } catch (odbc_error) {
      db.execute(drop_table(person::_));
      db.execute(create_table(person::_));
    }

    try {
      db.execute(create_table(partner::_));
    } catch (odbc_error) {
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
    using namespace boost::rdb::sql::test::springfield;
    db.execute(drop_table(person::_));
    db.execute(drop_table(partner::_));
  }
};

struct object_model_fixture {

  boost::rdb::odbc::database db;

  object_model_fixture() : db("boost", "boost", "boost") {

    using namespace boost::rdb::odbc;
    using namespace boost::rdb::sql::test::object_model;

    try {
      db.execute(create_table(person::_));
    } catch (odbc_error) {
      db.execute(drop_table(person::_));
      db.execute(create_table(person::_));
    }

    try {
      db.execute(create_table(natural_person::_));
    } catch (odbc_error) {
      db.execute(drop_table(natural_person::_));
      db.execute(create_table(natural_person::_));
    }

    try {
      db.execute(create_table(legal_person::_));
    } catch (odbc_error) {
      db.execute(drop_table(legal_person::_));
      db.execute(create_table(legal_person::_));
    }
  }

  ~object_model_fixture() {
    using namespace boost::rdb::sql::test::object_model;
    db.execute(drop_table(person::_));
    db.execute(drop_table(natural_person::_));
    db.execute(drop_table(legal_person::_));
  }
};

#define BOOST_RDB_CHECK_SELECT_RESULTS(expr, expected) BOOST_CHECK(str_result_set(expr) == expected)

#endif
