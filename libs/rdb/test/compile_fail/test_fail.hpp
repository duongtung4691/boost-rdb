#include "../test.hpp"
#include <boost/rdb/sql/select.hpp>

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

template<class T>
void check_tag(const T&) {
  typedef typename T::tag tag;
};

person p;
person::_1 p1("p1");
person::_2 p2("p2");
