#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>

#include "test_tables.hpp"

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

void examples() {
  //[ example_create_table
  create_table(person::_);
  //]
}

#if 1

#include <boost/rdb/sql/select.hpp>

namespace boost { namespace rdb { namespace sql {

  // add a new keyword
  struct mysql5 : sql2003 {
    struct limit;
  };
  
  // allow all sql2003 constructs
  template<class State, class New>
  struct allow<mysql5, State, New> : allow<sql2003, State, New> { };
  
  // augment the standard sql statement
  template<class State, class Data, class Subdialect>
  struct select_statement<mysql5, State, Data, Subdialect> : select_statement<sql2003, State, Data, Subdialect> {

    typedef select_statement<sql2003, State, Data, Subdialect> inherited;

    select_statement(const Data& data) : inherited(data) { }

    typename inherited::template transition<typename Subdialect::limit, int>::type
    limit(int n) const {
      BOOST_MPL_ASSERT((allow<Subdialect, State, Subdialect::limit>));
      return typename inherited::template transition<
        typename Subdialect::limit, int
      >::type(add_key<typename Subdialect::limit>(data_, n));
    }
  };

  // tell how to print the clause
  inline void str(std::ostream& os, const fusion::pair<mysql5::limit, int>& p) {
    os << " limit " << p.second;
  }
  
  // declare legal transitions
  BOOST_RDB_ALLOW(mysql5, from, limit);
  BOOST_RDB_ALLOW(mysql5, where, limit);

  // grammar entry point
  namespace mysql {
    select_statement<mysql5, mysql5::select, fusion::map<>, mysql5> select = fusion::map<>();
  }

  // that's it !

} } }

#include <iostream>

int main() {
  using namespace std;
  using namespace boost::rdb::sql;
  using mysql::select;
  person p;
  select(p.id).from(p).limit(10);
  select(p.id).from(p).where(p.id > 1).limit(10);
  select(p.id).from(p).limit(10).str(cout);
  cout << endl;
}

#endif
