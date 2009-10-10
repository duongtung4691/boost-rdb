#include "test.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/rdb/sql.hpp>

BOOST_AUTO_TEST_CASE(add_key_test) {
  using namespace boost::fusion;
  using namespace boost::rdb::sql;

  typedef map<
    pair<int, std::string>
  , pair<double, std::string> >
  map_type;

  map_type m(
      make_pair<int>("X")
    , make_pair<double>("Men"));

  boost::rdb::sql::result_of::add_key<map_type, float, std::string>::type m2 = add_key<float>(m, "Origins");
  BOOST_CHECK(at_key<float>(m2) == "Origins");
}

BOOST_AUTO_TEST_CASE(replace_value_at_key_test) {
  using namespace boost::fusion;
  using namespace boost::rdb::sql;

  typedef map<
    pair<int, std::string>
  , pair<double, std::string> >
  map_type;

  map_type m(
      make_pair<int>("X")
    , make_pair<double>("Men"));

  #ifdef _WIN32
  #pragma message("put fusion exts in their own namespace")
  #endif
  boost::rdb::sql::result_of::replace_value_at_key<map_type, double, std::string>::type m2
    = replace_value_at_key<double>(m, "Rays");
  BOOST_CHECK(at_key<double>(m2) == "Rays");

}
