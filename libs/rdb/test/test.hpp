#ifndef BOOST_RDB_TEST_HPP
#define BOOST_RDB_TEST_HPP

#include <boost/fusion/include/io.hpp>
#include <boost/test/unit_test.hpp>

#include "test_tables.hpp"

// Visual Studio regex to make error output readable
// (boost|std|fusion|rdb|test|springfield|detail)\:\:

#define BOOST_RDB_CHECK_SQL(expr, sql) BOOST_CHECK(str(expr) == sql)

template<class Stat>
std::string str(const Stat& statement) {
  std::ostringstream os;
  statement.str(os);
  return os.str();
}

#endif