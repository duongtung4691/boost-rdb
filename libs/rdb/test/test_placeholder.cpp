#include "test.hpp"

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
  using namespace rdb::type;

  check_placeholders< fusion::vector< placeholder<integer> > >(p.age == _);

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF((p.age + _) == _)::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF((p.age + _) == _ && p.name.like(_) && !(p.age < _))::placeholder_vector,
    fusion::vector<placeholder<integer>, placeholder<integer>, placeholder< varchar<20> >, placeholder<integer>>
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(exists(select(p.id + _).from(p).where(p.name == _)))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(
      select(p.id).from(p).where(p.age > _ && p.id.in(
        select(p.id).from(p).where(p.name == _)))
    )::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(p.id.in(1, _, 2, _))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(p.id.in(1, _, 2, _))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, 66))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<30> >, placeholder< varchar<20> > >
    >));

  {
    typedef BOOST_TYPEOF(insert_into(p)(p.id).select(p.id).from(p).where(p.age > _))::placeholder_vector placeholders;
    BOOST_MPL_ASSERT((same_placeholders<placeholders, fusion::vector<placeholder<integer>>>));
  }

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(update(p).set(p.age = 33).where(p.first_name == _))::placeholder_vector,
    fusion::vector< placeholder< varchar<30> > >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(update(p).set(p.age = _).where(p.first_name == _))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<30> > >
    >));

  BOOST_MPL_ASSERT((same_placeholders<
    BOOST_TYPEOF(delete_from(p).where(p.first_name == _))::placeholder_vector,
    fusion::vector< placeholder< varchar<30> > >
    >));
}

