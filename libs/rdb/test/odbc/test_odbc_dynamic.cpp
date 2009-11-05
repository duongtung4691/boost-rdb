#include "test_odbc.hpp"
#include <boost/rdb/sql/dynamic.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_dynamic_integer_param, springfield_fixture) {
  
  using sql::select;

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

  using sql::select;

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

  using sql::select;

  person p;

  dynamic_columns cols;
  cols.push_back(make_dynamic(p.id));
  cols.push_back(make_dynamic(p.first_name));
  cols.push_back(make_dynamic(p.name));
  cols.push_back(make_dynamic(p.age));
  
  dynamic_expressions values;
  values.push_back(make_dynamic(_, p.id));
  values.push_back(make_dynamic(_, p.first_name));
  values.push_back(make_dynamic(_, p.name));
  values.push_back(make_dynamic(_, p.age));

  BOOST_AUTO(st, db.prepare(insert_into(p)(cols).values(values)));

  dynamic_values params;

  integer id_param;  
  params.push_back(make_dynamic(id_param));

  varchar<30> first_name_param;
  params.push_back(make_dynamic(first_name_param));

  varchar<20> name_param;
  params.push_back(make_dynamic(name_param));

  integer age_param;  
  params.push_back(make_dynamic(age_param));
  
  st.bind_parameters(params);

  id_param = 3;
  first_name_param = "Bart";
  name_param = "Simpson";
  age_param = 9;
  st.execute();

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.first_name, p.age).from(p).where(p.id == 3)),
    "((Bart 9))");

  id_param = 4;
  first_name_param = "Lisa";
  name_param = "Simpson";
  age_param = 7;
  st.execute();

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.first_name, p.age).from(p).where(p.id == 4)),
    "((Lisa 7))");
}

BOOST_FIXTURE_TEST_CASE(prepared_insert_mixed, springfield_fixture) {

  using sql::select;

  person p;

  dynamic_columns cols;
  cols.push_back(make_dynamic(p.id));
  cols.push_back(make_dynamic(p.first_name));
  
  dynamic_expressions values;
  values.push_back(make_dynamic(_, p.id));
  values.push_back(make_dynamic(_, p.first_name));

  BOOST_AUTO(st, db.prepare(insert_into(p)(cols, p.name, p.age).values(values, "Simpson", _)));

  dynamic_values params;

  integer id_param;  
  params.push_back(make_dynamic(id_param));

  varchar<30> first_name_param;
  params.push_back(make_dynamic(first_name_param));

  integer age_param;
  
  st.bind_parameters(params, age_param);

  id_param = 3;
  first_name_param = "Bart";
  age_param = 9;
  st.execute();

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.first_name, p.age).from(p).where(p.id == 3)),
    "((Bart 9))");

  id_param = 4;
  first_name_param = "Lisa";
  age_param = 7;
  st.execute();

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.first_name, p.age).from(p).where(p.id == 4)),
    "((Lisa 7))");
}

BOOST_FIXTURE_TEST_CASE(prepared_update_dynamic, springfield_fixture) {

  using sql::select;

  person p;

  dynamic_updates updates;
  updates.push_back(make_dynamic(p.first_name = _));
  updates.push_back(make_dynamic(p.age = _));
  
  dynamic_boolean predicate = make_dynamic(p.id == _);
  
  BOOST_AUTO(st, db.prepare(update(p).set(p.name = _, updates).where(predicate)));
  
  integer id_param;
  dynamic_values predicate_params;
  predicate_params.push_back(make_dynamic(id_param));

  dynamic_values update_params;
  varchar<30> first_name_param;
  update_params.push_back(make_dynamic(first_name_param));

  integer age_param;  
  update_params.push_back(make_dynamic(age_param));

  varchar<20> name_param;
  name_param = "Bouvier";

  st.bind_parameters(name_param, update_params, predicate_params);
  
  id_param = 1;
  first_name_param = "Patty";
  age_param = 43;
  st.execute();
  
  id_param = 2;
  first_name_param = "Selma";
  age_param = 43;
  st.execute();
  
  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Patty Bouvier 43)"
    " (2 Selma Bouvier 43))"    
    );
}

BOOST_FIXTURE_TEST_CASE(prepared_select_dynamic_bind_results, springfield_fixture) {

  using sql::select;

  person p;
  
  dynamic_expressions exprs;
  exprs.push_back(make_dynamic(p.id));
  exprs.push_back(make_dynamic(p.first_name));
  
  BOOST_AUTO(st, db.prepare(select(exprs).from(p)));
  
  dynamic_values results;
  integer id;
  results.push_back(make_dynamic(id));
  varchar<30> first_name;
  results.push_back(make_dynamic(first_name));
  st.bind_results(results);
  
  BOOST_AUTO(cursor, st.execute());

  cursor.fetch();
  BOOST_CHECK(!id.is_null());
  BOOST_CHECK_EQUAL(id.value(), 1);
  BOOST_CHECK(!first_name.is_null());
  BOOST_CHECK_EQUAL(string(first_name), "Homer");

  cursor.fetch();
  BOOST_CHECK(!id.is_null());
  BOOST_CHECK_EQUAL(id.value(), 2);
  BOOST_CHECK(!first_name.is_null());
  BOOST_CHECK_EQUAL(string(first_name), "Marge");
}

//#include <iostream>
//ostream* init = boost::rdb::trace_stream = &std::cout;

BOOST_FIXTURE_TEST_CASE(prepared_select_dynamic_tables, springfield_fixture) {

  using sql::select;

  person p("husband");
  person spouse("wife");
  partner::qualified link;
  
  dynamic_expressions exprs;
  exprs.push_back(make_dynamic(spouse.first_name));
  
  dynamic_tables tables;
  tables.push_back(make_dynamic(spouse));
  tables.push_back(make_dynamic(link));
  
  dynamic_boolean predicate = make_dynamic(p.id == link.husband && link.wife == spouse.id);
  
  BOOST_TEST_CHECKPOINT("prepare");
  BOOST_AUTO(st, db.prepare(select(p.first_name, exprs).from(p, tables).where(predicate)));
  
  varchar<30> him;

  dynamic_values results;
  varchar<30> her;
  results.push_back(make_dynamic(her));

  BOOST_TEST_CHECKPOINT("bind results");
  st.bind_results(him, results);
  
  BOOST_AUTO(cursor, st.execute());

  BOOST_TEST_CHECKPOINT("before fetch");

  cursor.fetch();
  BOOST_CHECK(!him.is_null());
  BOOST_CHECK_EQUAL(string(him), "Homer");
  BOOST_CHECK(!her.is_null());
  BOOST_CHECK_EQUAL(string(her), "Marge");

  BOOST_TEST_CHECKPOINT("return from test");
}
