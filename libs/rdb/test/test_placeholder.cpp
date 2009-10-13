#include "test.hpp"

#include <boost/rdb/sql.hpp>

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

template<class Expected, class T>
void check_placeholders(const T&) {
  BOOST_MPL_ASSERT((boost::is_same<typename T::placeholder_vector, Expected>));
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

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF((p.age + _) == _)::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF((p.age + _) == _ && p.name.like(_) && !(p.age < _))::placeholder_vector,
    fusion::vector<placeholder<integer>, placeholder<integer>, placeholder< varchar<20> >, placeholder<integer>>
    >));

  BOOST_MPL_ASSERT((is_same<
    placeholders_from_list< fusion::vector<BOOST_TYPEOF(p.age == _)> >::type,
    fusion::vector< placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((is_same<
    placeholders_from_list< fusion::vector<BOOST_TYPEOF(p.age == _), BOOST_TYPEOF(p.id < 20), BOOST_TYPEOF(p.name == _)> >::type,
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
    >));

  BOOST_MPL_ASSERT((is_same<
    placeholders_from_pair_list<BOOST_TYPEOF(select(p.id + _).from(p).where(p.name == _).data_)>::type,
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(exists(select(p.id + _).from(p).where(p.name == _)))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(
      select(p.id).from(p).where(p.age > _ && p.id.in(
        select(p.id).from(p).where(p.name == _)))
    )::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<20> > >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(p.id.in(1, _, 2, _))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(p.id.in(1, _, 2, _))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder<integer> >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, 66))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<30> >, placeholder< varchar<20> > >
    >));

  {
    typedef BOOST_TYPEOF(insert_into(p)(p.id).select(p.id).from(p).where(p.age > _))::placeholder_vector placeholder_vector;
    BOOST_MPL_ASSERT((is_same<placeholder_vector, fusion::vector<placeholder<integer>>>));
  }

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(update(p).set(p.age = 33).where(p.first_name == _))::placeholder_vector,
    fusion::vector< placeholder< varchar<30> > >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(update(p).set(p.age = _).where(p.first_name == _))::placeholder_vector,
    fusion::vector< placeholder<integer>, placeholder< varchar<30> > >
    >));

  BOOST_MPL_ASSERT((is_same<
    BOOST_TYPEOF(delete_from(p).where(p.first_name == _))::placeholder_vector,
    fusion::vector< placeholder< varchar<30> > >
    >));
}

