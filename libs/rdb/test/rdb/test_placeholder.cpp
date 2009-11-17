#include "test.hpp"

#include <boost/test/unit_test.hpp>

#include <boost/rdb/sql.hpp>

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

template<class Actual, class Expected>
struct same_placeholders : boost::is_same<
  typename boost::fusion::result_of::as_vector<Actual>::type,
  Expected
> {
};

template<class Expected, class T>
void check_placeholders(const T&) {
  BOOST_MPL_ASSERT((same_placeholders<typename T::placeholder_vector, Expected>));
}

BOOST_AUTO_TEST_CASE(test_placeholder) {

  using namespace boost::rdb::sql;

  person p;

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id = _),
    "update person set id = ?");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.name = _),
    "update person set name = ?");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.id = p.age + _),
    "update person set id = age + ?");

  BOOST_RDB_CHECK_SQL(
    update(p).set(p.age = 75).where(p.name.like(_)),
    "update person set age = 75 where name like ?");

  using namespace boost;
  using rdb::sql::select;
  using namespace rdb::type;

  check_placeholders<
    fusion::vector< placeholder<real> >
  >(p.age == _);

  check_placeholders<
    fusion::vector< placeholder<real>, placeholder<real> >
  >((p.age + _) == _);

  check_placeholders<
    fusion::vector<placeholder<real>, placeholder<real>, placeholder< varchar<20> >, placeholder<real> >
  >((p.age + _) == _ && p.name.like(_) && !(p.age < _));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
  >(exists(select(p.id + _).from(p).where(p.name == _)));

  check_placeholders<
    fusion::vector< placeholder<real>, placeholder< varchar<20> > >
  >(select(p.id).from(p).where(p.age > _ && p.id.in(
      select(p.id).from(p).where(p.name == _))));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder<integer> >
  >(p.id.in(1, _, 2, _));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder<integer> >
  >(p.id.in(1, _, 2, _));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder< varchar<30> >, placeholder< varchar<20> > >
  >(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, 66));

  check_placeholders<
    fusion::vector<placeholder<real> >
  >(insert_into(p)(p.id).select(p.id).from(p).where(p.age > _));

  check_placeholders<
    fusion::vector< placeholder< varchar<30> > >
  >(update(p).set(p.age = 33).where(p.first_name == _));

  check_placeholders<
    fusion::vector< placeholder<real>, placeholder< varchar<30> > >
  >(update(p).set(p.age = _).where(p.first_name == _));

  check_placeholders<
    fusion::vector< placeholder< varchar<30> > >
  >(delete_from(p).where(p.first_name == _));
}

