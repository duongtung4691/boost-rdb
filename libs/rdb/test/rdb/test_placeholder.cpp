#include "test.hpp"

#include <boost/test/unit_test.hpp>

#include <boost/rdb/sql.hpp>

using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::test::springfield;

template<class Actual, class Expected>
struct same_placeholders : boost::is_same<
  typename boost::fusion::result_of::as_vector<Actual>::type,
  Expected
> {
};

template<class Expected, class T>
void check_placeholders(const T& st) {
  //st.placeholders();
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
  using namespace rdb::core;

  check_placeholders<
    fusion::vector< placeholder<float_> >
  >(p.age == _);

  check_placeholders<
    fusion::vector< placeholder<float_>, placeholder<float_> >
  >((p.age + _) == _);

  check_placeholders<
    fusion::vector<placeholder<float_>, placeholder<float_>, placeholder< varchar<20> >, placeholder<float_> >
  >((p.age + _) == _ && p.name.like(_) && !(p.age < _));
  
  select(p.id + _).placeholders();
  
  check_placeholders<
    fusion::vector< placeholder<integer> >
  >(select(p.id + _).from(p));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder<float_> >
  >(select(p.id + _, p.age + _).from(p));

  check_placeholders<
    fusion::vector< placeholder< varchar<20> > >
  >(select(p.id).from(p).where(p.name == _));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
  >(select(p.id + _).from(p).where(p.name == _));

  check_placeholders<
    fusion::vector< placeholder<integer> >
  >(exists(select(p.id + _).from(p)));
  
  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
  >(select(p.id + _).from(p).where(p.name == _));

  check_placeholders<
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
  >(exists(select(p.id + _).from(p).where(p.name == _)));

  check_placeholders<
    fusion::vector< placeholder<float_>, placeholder< varchar<20> > >
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
    fusion::vector<placeholder<float_> >
  >(insert_into(p)(p.id).select(p.id).from(p).where(p.age > _));

  check_placeholders<
    fusion::vector< placeholder< varchar<30> > >
  >(update(p).set(p.age = 33).where(p.first_name == _));

  check_placeholders<
    fusion::vector< placeholder<float_>, placeholder< varchar<30> > >
  >(update(p).set(p.age = _).where(p.first_name == _));

  check_placeholders<
    fusion::vector< placeholder< varchar<30> > >
  >(delete_from(p).where(p.first_name == _));
}

