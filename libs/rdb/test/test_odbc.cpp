#include <iostream>
#include <sstream>
#include <boost/fusion/include/io.hpp>
#include <boost/rdb/sql.hpp>
#include <boost/rdb/sql/dynamic.hpp>
#include <boost/rdb/odbc.hpp>

#include "test_tables.hpp"

#define BOOST_TEST_MODULE odbc_backend
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

template<typename ResultSet>
std::string str(const ResultSet& results) {
  std::ostringstream os;
  os << results;
  return os.str();
}

struct springfield_fixture {

  database db;

  springfield_fixture() : db("boost", "boost", "boost") {

    try {
      db.execute(create_table(person::_));
    } catch (odbc_error) {
      db.execute(drop_table(person::_));
      db.execute(create_table(person::_));
    }

    try {
      db.execute(create_table(partner::_));
    } catch (odbc_error) {
      db.execute(drop_table(partner::_));
      db.execute(create_table(partner::_));
    }
    
    person p;
    partner l;

    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
    db.execute(insert_into(l)(l.husband, l.wife).values(1, 2));
  }

  ~springfield_fixture() {
    db.execute(drop_table(person::_));
    db.execute(drop_table(partner::_));
  }
};

#define BOOST_RDB_CHECK_SELECT_RESULTS(expr, expected) BOOST_CHECK(str(expr) == expected)

BOOST_AUTO_TEST_CASE(basic) {
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

  person p;
  db.execute(update(p).set(p.age = null).where(p.id == 1));
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.age).from(p).where(p.id == 1)),
    "((1 null))");
}

BOOST_FIXTURE_TEST_CASE(tx, springfield_fixture) {

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

template<class Results1, class Results2>
vector< pair<string, string> > fetch_parallel(const Results1& results1, const Results2& results2) {
  vector< pair<string, string> > res;
  res.push_back(make_pair(results1.fetch().get<0>(), results2.fetch().get<0>()));
  res.push_back(make_pair(results1.fetch().get<0>(), results2.fetch().get<0>()));
  return res;
}

BOOST_FIXTURE_TEST_CASE(parallel_result_sets, springfield_fixture) {
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
  person p;
  BOOST_AUTO(st, db.prepare(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, _)));
  st.execute(3, (char*) "Bart", "Simpson", 9);
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
  person p;
  BOOST_AUTO(st, db.prepare(select(p.first_name).from(p).where(p.id == _)));
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(1), "((Homer))");
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(2), "((Marge))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_set, springfield_fixture) {
  person p;
  BOOST_AUTO(st, db.prepare(update(p).set(p.age = _).where(p.id == 1)));
  st.execute(38);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age).from(p)),
    "((38) (34))");
  st.execute(39);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age).from(p)),
    "((39) (34))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_where, springfield_fixture) {
  person p;
  BOOST_AUTO(st, db.prepare(update(p).set(p.age = 66).where(p.id == _)));
  st.execute(1);
  st.execute(2);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age).from(p)),
    "((66) (66))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_both, springfield_fixture) {
  person p;
  BOOST_AUTO(st, db.prepare(update(p).set(p.age = _).where(p.id == _)));
  st.execute(38, 1);
  st.execute(35, 2);
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.age).from(p)),
    "((38) (35))");
}

BOOST_FIXTURE_TEST_CASE(prepared_delete, springfield_fixture) {
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
  person p;
  BOOST_AUTO(st, db.prepare(select(p.id).from(p).where(p.first_name == _)));
  
  varchar<30> param;
  st.bind_parameters(param);
  
  param = "Homer";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((1))");
  
  param = "Marge";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((2))");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_dynamic_integer_param, springfield_fixture) {

  person p;

  dynamic_boolean predicate = make_dynamic(p.id == _);
  
  BOOST_AUTO(st, db.prepare(select(p.first_name).from(p).where(predicate)));
  
  integer id_param;
  dynamic_values params;
  params.push_back(make_dynamic(id_param));
  st.bind_parameters(params);
  
  id_param = 1;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((Homer))");
  
  id_param = 2;
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((Marge))");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_dynamic_varchar_param, springfield_fixture) {

  person p;

  dynamic_boolean predicate = make_dynamic(p.first_name == _);
  
  BOOST_AUTO(st, db.prepare(select(p.id).from(p).where(predicate)));
  
  varchar<30> first_name_param;
  dynamic_values params;
  params.push_back(make_dynamic(first_name_param));
  st.bind_parameters(params);
  
  first_name_param = "Homer";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((1))");
  
  first_name_param = "Marge";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((2))");
}

BOOST_FIXTURE_TEST_CASE(prepared_insert_orm_style, springfield_fixture) {

  person p;

  dynamic_expressions exprs;
  exprs.push_back(make_dynamic(p.id));
  exprs.push_back(make_dynamic(p.first_name));
  exprs.push_back(make_dynamic(p.name));
  exprs.push_back(make_dynamic(p.age));
  
  //dynamic_expressions values;
  //exprs.push_back(make_dynamic(_, p.id));
  //exprs.push_back(make_dynamic(_, p.first_name));
  //exprs.push_back(make_dynamic(_, p.name));
  //exprs.push_back(make_dynamic(_, p.age));
  //
  //BOOST_AUTO(st, db.prepare(insert_into(p)(exprs).values(values)));

  //dynamic_values params;

  //integer id_param;  
  //params.push_back(make_dynamic(id_param));

  //varchar<30> first_name_param;
  //params.push_back(make_dynamic(first_name_param));

  //varchar<20> name_param;
  //params.push_back(make_dynamic(name_param));

  //integer age_param;  
  //params.push_back(make_dynamic(age_param));
  //
  //st.bind_parameters(params);

}
