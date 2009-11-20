//[ prologue
#include <iostream>
#include <boost/fusion/include/io.hpp>
#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>

using namespace boost::rdb;
using namespace boost::rdb::sql;
using namespace boost::rdb::odbc;

using namespace std;
//]

#include <fstream>
#include <list>

#include "test_tables.hpp"

using namespace boost::rdb::sql::test::springfield;

ofstream file("output.txt");
ofstream osql("sql.txt");
#define cout file
ostream* init = boost::rdb::odbc::trace_stream = &osql;

//[ output_deque
template<typename T>
std::ostream& operator <<(std::ostream& os, const std::deque<T>& seq) {
  const char* sep = "";
  os << "(";
  typename std::deque<T>::const_iterator iter = seq.begin(), last = seq.end();
  while (iter != last) {
    os << sep;
    os << *iter;
    sep = " ";
    ++iter;
  }
  return os << ")";
}
//]

struct markup_t {
  markup_t(const char* str) : n(0) {
    cout << "//[" << str << "_result\n";
    osql << "//[" << str << "_sql\n";
  }

  ~markup_t() {
    cout << "//]\n";
    osql << "//]\n";
  }

  int n;
};

#define markup(id) for (markup_t t(#id); t.n == 0; ++t.n)

int main() {

  //[connect
  database db("boost", "boost", "boost");
  //]

  try {
    db.execute(drop_table(person::_));
  } catch (odbc_error) {
  }

  try {
    db.execute(drop_table(partner::_));
  } catch (odbc_error) {
  }

  try {
    //[ create_tables
    db.execute(create_table(person::_));
    db.execute(create_table(partner::_));
    //]

    //[ alias
    person p;
    //]

    osql << "//" "[ " "insert_sql" << endl;
    //[ insert_homer
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
    //]
    osql << "//" "]\n";

    /*
    //[ insert_marge_error
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", 34, "Simpson")); // compilation error !
    //]
    */

    //[ insert_marge_corrected
    db.execute(insert_into(p)(p.id, p.first_name, p.name).values(2, "Marge", "Simpson"));
    //]

    markup(select_print_all) {
      //[ select_print_all
      cout << db.execute(select(p.id, p.first_name, p.name, p.age).from(p)) << endl;
      //]
    }

    markup(select_into_vector) {
      //[ select_into_vector
      typedef nullable< boost::fusion::vector<string, double> > row_type;
      std::deque<row_type> results;
      db.execute(select(p.first_name, p.age).from(p)).all(results);
      for (size_t i = 0; i < results.size(); i++) {
        row_type row = results[i];
        cout << row.get<0>() << ", age: ";
        if (row.is_null<1>())
          cout << "unknown";
        else
          cout << row.get<1>();
        cout << endl;
      }
      //]
    }

    markup(select_fetch) {
      //[ select_fetch
      BOOST_AUTO(results, db.execute(select(p.first_name, p.age).from(p)));
      BOOST_TYPEOF(results)::value_type row;
      while (results.fetch(row)) {
        cout << row.get<0>() << ", age: ";
        if (row.is_null<1>())
          cout << "unknown";
        else
          cout << row.get<1>();
        cout << endl;
      }
      //]
    }

    {
      //[ select_with_nested_row_type
      BOOST_AUTO(query, select(p.id, p.first_name, p.name, p.age).from(p));
      typedef BOOST_TYPEOF(query)::row row;
      std::vector<row> results;
      db.execute(query).all(results);
      //]
    }

    markup(aliases) {
    //[aliases
    partner assoc;
    db.execute(insert_into(partner::_)(assoc.husband, assoc.wife).values(1, 2));

    person him("husband"), her("wife");
    cout << db.execute(
      select(him.id, him.first_name, him.name, her.id, her.first_name)
        .from(him, her, assoc)
        .where(him.id == assoc.husband && her.id == assoc.wife))
      << "\n";
    //]
    }

    person p1("p1"), p2("p2"), p3("p3");
    partner l;

    //[jumbo_select
    select(p1.id, (p1.age + p2.age) / 2)                      // expressions
      .from(p1, p2)
      .where(p1.age > 18
        && p1.first_name.like("%r%")                          // like
        && p1.name.in("Simpson", "Bouvier")                   // in (values)
        && p1.id.in(                                          // in (subquery)
          select(l.husband)
            .from(l)
            .where(l.husband == p1.id && l.wife == p2.id))
        && !exists(                                           // exists (subquery)
          select(p3.id)
            .from(p3)
            .where(p1.name == p3.name)
            )
          )
    //]
    .str(osql);
  } catch (exception& e) {
      #undef cout
      cout << e.what();
    }
}
