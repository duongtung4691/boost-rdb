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

    struct mysql5;
    
    namespace key {
      struct limit;
    }

    template<class Dialect>
    struct select_limit_context {
    };

    namespace transition {
      BOOST_RDB_DEFINE_TRANSITION(limit)
    }

    template<class Context, class Data>
    struct mysql_select_statement : select_statement<Context, Data> {

      mysql_select_statement(const Data& data) : select_statement<Context, Data>(data) { }

      typename transition::limit<
        Context,
        typename result_of::add_key<Data, key::limit, int>::type
      >::type
      limit(int n) const {
        return typename transition::limit<
          Context,
          typename result_of::add_key<Data, key::limit, int>::type
        >::type(add_key<key::limit>(data_, n));
      }
    };
  
  }

  template<>
  struct select_where_context<mysql::mysql5> {
    template<class Data> struct limit { typedef mysql::mysql_select_statement<select_where_context<mysql::mysql5>, Data> type; };
    //template<class Data> struct limit { typedef mysql::mysql_select_statement<mysql::select_limit_context<mysql::mysql5>, Data> type; };
  };

  template<>
  struct select_from_context<mysql::mysql5> {
    template<class Data> struct where { typedef mysql::mysql_select_statement<select_where_context<mysql::mysql5>, Data> type; };
    template<class Data> struct limit { typedef mysql::mysql_select_statement<select_where_context<mysql::mysql5>, Data> type; };
    //template<class Data> struct limit { typedef mysql::mysql_select_statement<mysql::select_limit_context<mysql::mysql5>, Data> type; };
  };

  template<>
  struct select_projection_context<mysql::mysql5> {
    template<class Data> struct from { typedef mysql::mysql_select_statement<select_from_context<mysql::mysql5>, Data> type; };
  };

  namespace mysql {

    extern select_begin< select_context<mysql5>, fusion::map<> > select;


    void test() {
      person p;
      select(p.id).from(p).limit(10);
      select(p.id).from(p).where(p.id > 1).limit(10);
    }
  }

} } }

#endif
