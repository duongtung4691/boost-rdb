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

  namespace mysql {

    struct mysql5 : sql2003 {
    struct select : sql2003::select {
      struct limit;
    };
  };

  template<class Data>
  struct select_statement<mysql::mysql5, mysql::mysql5::select::begin, Data, mysql::mysql5>
    : select_statement<sql2003, mysql::mysql5::select::begin, Data, mysql::mysql5> {

    select_statement(const Data& data) : select_statement<sql2003, sql2003::select::begin, Data, mysql::mysql5>(data) { }
  };
  
  template<class State, class Data, class Subdialect>
  struct select_statement<mysql::mysql5, State, Data, Subdialect>
    : select_statement<sql2003, State, Data, Subdialect> {

    select_statement(const Data& data) : select_statement<sql2003, State, Data, Subdialect>(data) { }

    select_statement<
      Subdialect,
      typename Subdialect::select::limit,
      typename result_of::add_key<Data, typename Subdialect::select::limit, int>::type,
      Subdialect
    >
    limit(int n) const {
      return select_statement<
        Subdialect,
        typename Subdialect::select::limit,
        typename result_of::add_key<Data, mysql::mysql5::select::limit, int>::type,
        Subdialect
      >(add_key<typename Subdialect::select::limit>(data_, n));
    }
  };
  
  }

  namespace mysql {

    extern select_statement<mysql::mysql5, mysql::mysql5::select::begin, fusion::map<>, mysql::mysql5> select;


    void test() {
      person p;
      //select++;
      //select(p.id)++;
      select(p.id).from(p).limit(10);
      select(p.id).from(p).where(p.id > 1).limit(10);
    }
  }

} } }

#endif
