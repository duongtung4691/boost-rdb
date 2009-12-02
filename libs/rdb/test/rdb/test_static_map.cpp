#include <sstream>
#include <boost/test/unit_test.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/rdb/detail/static_map.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb;

namespace {
  template<class T>
  string str(const T& obj) {
    ostringstream os;
    os << obj;
    return os.str();
  }
}

struct transform_stutter {
  template<class T>
  struct result {
    typedef fusion::vector<T, T> type;
  };
  
  template<class T>
  fusion::vector<T, T> operator ()(const T& val) { return fusion::make_vector(val, val); }
};

struct accumulate_stutter {
  template<class Entry, class Seq>
  struct result {
    typedef typename fusion::result_of::as_vector<
      typename fusion::result_of::push_back<
        typename fusion::result_of::push_back<
          Seq,
          const typename Entry::value_type
        >::type,
        const typename Entry::value_type
      >::type
    >::type type;
  };
  
  template<class Entry, class Seq>
  typename result<const Entry, const Seq>::type
  operator ()(const Entry& entry, const Seq& seq) const {
    return fusion::as_vector(fusion::push_back(fusion::push_back(seq, entry.value), entry.value));
  }
};

BOOST_AUTO_TEST_CASE(test_static_map) {

  struct name;
  struct age;

  typedef static_map<name, string> map1;
  map1 m1("Homer");
  
  BOOST_MPL_ASSERT((is_same<result_of::static_map_get<name, map1>::type, string>));
  BOOST_CHECK(str(m1.get<name>()) == "Homer");
  BOOST_CHECK(str(m1.transform(transform_stutter())) == "((Homer Homer))");
  BOOST_MPL_ASSERT((is_same<
    result_of::static_map_accumulate< map1, accumulate_stutter, fusion::vector<> >::type,
    const fusion::vector<const string, const string>
  >));
  
  typedef static_map<name, string>::with<age, int>::type map2;
  map2 m2(37, m1);
  
  BOOST_MPL_ASSERT((is_same<result_of::static_map_get<name, map2>::type, string>));
  BOOST_CHECK(str(m1.get<name>()) == "Homer");
  BOOST_MPL_ASSERT((is_same<result_of::static_map_get<age, map2>::type, int>));
  BOOST_CHECK(m2.get<age>() == 37);
  BOOST_CHECK(str(m2.transform(transform_stutter())) == "((Homer Homer) (37 37))");
  BOOST_CHECK(str(m2.accumulate(accumulate_stutter(), fusion::make_vector())) == "(Homer Homer 37 37)");
  
}
