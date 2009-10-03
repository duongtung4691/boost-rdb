//[ prologue
#include <iostream>
#include <boost/fusion/include/io.hpp>
#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>

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
ostream* init = boost::rdb::trace_stream = &osql;

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

struct markup {
  markup(const char* str) { cout << "//[" << str << "\n"; }
  ~markup() { cout << "//]\n"; }
};

#define with_markup(str, code) { markup _markup_(##str); code }

int main() {

  //[connect
  database db("boost", "boost", "boost");
  //]

  try {
    db.execute(drop_table(person::_));
  } catch (error) {
  }

  try {
    db.execute(drop_table(partner::_));
  } catch (error) {
  }

  try {
    //[ create_tables
    db.execute(create_table(person::_));
    db.execute(create_table(partner::_));
    //]

    //[ alias
    person p;
    //]

    //[ insert_homer
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37));
    //]

    with_markup("insert_sql",
      insert_into(p)(p.id, p.first_name, p.name, p.age).values(1, "Homer", "Simpson", 37).str(cout);
    )

    /*
    //[ insert_marge_error
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", 34, "Simpson")); // compilation error !
    //]
    */

    //[ insert_marge_corrected
    db.execute(insert_into(p)(p.id, p.first_name, p.name, p.age).values(2, "Marge", "Simpson", 34));
    //]

    with_markup("select_1_result",
      //[ select_1
      cout << db.execute(select(p.id, p.first_name, p.name, p.age).from(p).where(p.id == 1)).all()[0] << endl;
      //]
    );

    //[ select_2
    typedef nullable_row< boost::fusion::vector<long, string, string, long> > row_type;
    std::list<row_type> results;
    db.execute(select(p.id, p.first_name, p.name, p.age).from(p)).all(results);
    /*<-*/ with_markup("select_2_result", /*->*/copy(results.begin(), results.end(), ostream_iterator<row_type>(cout, "\n"));
    //]
    );

    {
      //[ select_with_nested_row_type
      BOOST_AUTO(query, select(p.id, p.first_name, p.name, p.age).from(p));
      typedef BOOST_TYPEOF(query)::row row;
      std::vector<row> results;
      db.execute(query).all(results);
      //]
    }

    osql << "//" "[ " "alias_sql" << endl;
    //[alias_test
    partner assoc;
    db.execute(insert_into(partner::_)(assoc.husband, assoc.wife).values(1, 2));

    person him("husband"), her("wife");
    /*<-*/ with_markup("alias_result", /*->*/cout << db.execute(
      select(him.id, him.first_name, him.name, her.id, her.first_name)
        .from(him, her, assoc)
        .where(him.id == assoc.husband && her.id == assoc.wife))
      << "\n";
    //]
    osql << "//" "]\n";
    );

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
