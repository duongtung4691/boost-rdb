#include "test_odbc.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

BOOST_AUTO_TEST_CASE(basic) {

  using boost::rdb::sql::select;

  BOOST_TEST_CHECKPOINT("entry");

  database db("boost", "boost", "boost");

  try {
    db.execute(drop_table(person::_));
  } catch (odbc_error) {
  }

  try {
    db.execute(drop_table(partner::_));
  } catch (odbc_error) {
  }

  db.execute(create_table(person::_));
  db.execute(create_table(partner::_));
  
  person p;

  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
  db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
  db.execute(update(p).set(p.age = p.age + 1).where(p.id == 1));

  {
    person h("h"), w("w");
    partner p;
    db.execute(insert_into(p)(p.husband, p.wife)
      .select(h.id, w.id).from(h, w)
      .where(h.name == w.name && h.id != w.id)); // duplicates couples but it's just a test
  }

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Homer Simpson 38) (2 Marge Simpson 34))"); // WRONG: assumes row order
}

BOOST_FIXTURE_TEST_CASE(test_null, springfield_fixture) {

  using boost::rdb::sql::select;

  person p;

  db.execute(update(p).set(p.age = null).where(p.id == 1));
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.age).from(p).where(p.id == 1)),
    "((1 null))");
}

BOOST_FIXTURE_TEST_CASE(tx, springfield_fixture) {

  using boost::rdb::sql::select;

  person p;

  if (!db.is_txn_capable())
    return;

  db.execute(update(p).set(p.age = 37).where(p.id == 1));
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 37);

  db.set_autocommit(off);

  db.execute(update(p).set(p.age = 38).where(p.id == 1));
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 38);

  db.rollback();
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 37);

  db.execute(update(p).set(p.age = 38).where(p.id == 1));
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 38);
  db.commit();
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 38);

  db.set_autocommit(on);
  db.execute(update(p).set(p.age = 39).where(p.id == 1));

  db.close();
  db.open("boost", "boost", "boost");
  BOOST_CHECK(db.execute(select(p.age).from(p).where(p.id == 1)).all()[0].get<0>() == 39);
}

BOOST_FIXTURE_TEST_CASE(expression_in_select_results, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age + 1).from(p).where(p.id == 1)),
    "((38))");
}

template<class Results1, class Results2>
vector< pair<string, string> > fetch_parallel(const Results1& results1, const Results2& results2) {
  nullable< fusion::vector<string> > row1, row2;
  vector< pair<string, string> > res;
  BOOST_CHECK(results1.fetch(row1));
  BOOST_CHECK(results2.fetch(row2));
  res.push_back(make_pair(row1.get<0>(), row2.get<0>()));
  BOOST_CHECK(results1.fetch(row1));
  BOOST_CHECK(results2.fetch(row2));
  res.push_back(make_pair(row1.get<0>(), row2.get<0>()));
  return res;
}

BOOST_FIXTURE_TEST_CASE(parallel_result_sets, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  vector< pair<string, string> > res = fetch_parallel(
    db.execute(select(p.first_name).from(p)),
    db.execute(select(p.first_name).from(p)));
  BOOST_CHECK(res[0].first == "Homer");
  BOOST_CHECK(res[0].first == res[0].second);
  BOOST_CHECK(res[1].first == "Marge");
  BOOST_CHECK(res[1].first == res[1].second);
}

BOOST_FIXTURE_TEST_CASE(parameterless_prepared_statements, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.id, p.first_name, p.name, p.age).from(p)));
  BOOST_RDB_CHECK_SELECT_RESULTS(
    st.execute(),
    "((1 Homer Simpson 37) (2 Marge Simpson 34))");
  // again
  BOOST_RDB_CHECK_SELECT_RESULTS(
    st.execute(),
    "((1 Homer Simpson 37) (2 Marge Simpson 34))");
}

BOOST_FIXTURE_TEST_CASE(prepared_insert, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, _)));
  st.execute(3, "Bart", "Simpson", 9);
  st.execute(4, (const char*) "Lisa", "Simpson", 7);
  st.execute(5, string("Maggie"), "Simpson", 0);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Homer Simpson 37)"
    " (2 Marge Simpson 34)"
    " (3 Bart Simpson 9)"
    " (4 Lisa Simpson 7)"
    " (5 Maggie Simpson 0))"    
    );
}

BOOST_FIXTURE_TEST_CASE(prepared_select, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.first_name).from(p).where(p.id == _)));
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(1), "((Homer))");
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(2), "((Marge))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_set, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(update(p).set(p.age = _, p.first_name = _).where(p.id == 1)));
  st.execute(9, "Bart");
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.first_name, p.age).from(p).where(p.id == 1)),
    "((Bart 9))");
  st.execute(7, "Lisa");
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.first_name, p.age).from(p).where(p.id == 1)),
    "((Lisa 7))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_where, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(update(p).set(p.age = 66).where(p.id == _)));
  st.execute(1);
  st.execute(2);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age).from(p)),
    "((66) (66))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_both, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(update(p).set(p.age = _).where(p.id == _)));
  st.execute(38, 1);
  st.execute(35, 2);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age).from(p)),
    "((38) (35))");
}

BOOST_FIXTURE_TEST_CASE(prepared_delete, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(delete_from(p).where(p.id == _)));
  st.execute(1);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id).from(p)),
    "((2))");
  st.execute(2);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id).from(p)),
    "()");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_integer_param, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.first_name).from(p).where(p.id == _)));
  
  integer id_param;
  st.bind_parameters(id_param);
  
  id_param = 1;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((Homer))");
  
  id_param = 2;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((Marge))");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_varchar_param, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.id).from(p).where(p.first_name == _)));
  
  varchar<30> param;
  st.bind_parameters(param);
  
  param = "Homer";
  cerr << st.execute() << endl;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((1))");
  
  param = "Marge";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((2))");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_integer_param_in_exprs, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.age + _).from(p).where(p.id == 1)));
  
  float_ param;
  st.bind_parameters(param);
  
  param = 1;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((38))");
  
  param = 2;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((39))");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_results, springfield_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.id, p.first_name, p.age).from(p)));
  integer id;
  varchar<30> first_name;
  float_ age;
  st.bind_results(id, first_name, age);
  BOOST_AUTO(results, st.execute());

  results.fetch();
  BOOST_CHECK(!id.is_null());
  BOOST_CHECK_EQUAL(id.value(), 1);
  BOOST_CHECK(!first_name.is_null());
  BOOST_CHECK_EQUAL(string(first_name), "Homer");
  BOOST_CHECK(!age.is_null());
  BOOST_CHECK_EQUAL(age.value(), 37);

  results.fetch();
  BOOST_CHECK(!id.is_null());
  BOOST_CHECK_EQUAL(id.value(), 2);
  BOOST_CHECK(!first_name.is_null());
  BOOST_CHECK_EQUAL(string(first_name), "Marge");
  BOOST_CHECK(!age.is_null());
  BOOST_CHECK_EQUAL(age.value(), 34);
}
