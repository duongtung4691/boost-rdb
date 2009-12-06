#include "test_odbc.hpp"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace boost;
using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;
using namespace boost::rdb::sql::test::springfield;

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_integer_param, homer_marge_fixture) {
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

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_varchar_param, homer_marge_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(select(p.id).from(p).where(p.first_name == _)));
  
  varchar<30> param;
  st.bind_parameters(param);
  
  param = "Homer";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((1))");
  
  param = "Marge";
  BOOST_RDB_CHECK_SELECT_RESULTS(st.execute(), "((2))");
}

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_integer_param_in_exprs, homer_marge_fixture) {
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

BOOST_FIXTURE_TEST_CASE(prepared_select_bind_results, homer_marge_fixture) {
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

BOOST_FIXTURE_TEST_CASE(prepared_insert_bind_params, schema_fixture) {
  using boost::rdb::sql::select;
  person p;
  BOOST_AUTO(st, db.prepare(insert_into(p)(p.id, p.first_name, p.name, p.age).values(_, _, _, _)));
  
  integer id_param;
  varchar<30> first_name_param;
  varchar<20> name_param;
  float_ age_param;
  
  st.bind_parameters(id_param, first_name_param, name_param, age_param);
  
  id_param = 1;
  first_name_param = "Bart";
  name_param = "Simpson";
  age_param = 9;
  st.execute();  
  
  id_param = 2;
  first_name_param = "Lisa";
  name_param = "Simpson";
  age_param = 7;
  st.execute();  
  
  id_param = 3;
  first_name_param = "Maggie";
  name_param = "Simpson";
  age_param = 0;
  st.execute();  
  
  id_param = 4;
  first_name_param = "Montgomery";
  name_param = "Burns";
  age_param.set_null();
  st.execute();  

  BOOST_RDB_CHECK_SELECT_RESULTS(
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)),
    "((1 Bart Simpson 9)"
    " (2 Lisa Simpson 7)"
    " (3 Maggie Simpson 0)"
    " (4 Montgomery Burns null))"
    );
}
