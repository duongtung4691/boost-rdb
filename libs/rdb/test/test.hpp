#ifndef BOOST_RDB_TEST_HPP
#define BOOST_RDB_TEST_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/table.hpp>

#include "test_tables.hpp"

// Visual Studio regex to make error output readable
// ((boost|std|fusion|rdb|sql|test|springfield|detail)\:\:)|(,void_)

#define BOOST_RDB_CHECK_SQL(expr, sql) BOOST_CHECK(str(expr) == sql)

template<class T>
std::string str(const T& obj) {
  std::ostringstream os;
  obj.str(os);
  return os.str();
}

#include <boost/fusion/include/io.hpp>
#include <boost/test/unit_test.hpp>

#endif