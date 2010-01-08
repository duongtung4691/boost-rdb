//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

#include <boost/rdb/sql/common.hpp>

#include <deque>

namespace boost { namespace rdb { namespace sql {
  
  extern select_statement<sql2003, sql2003::select, ct::map0, sql2003> select;

  namespace result_of {
    template<class ExprList>
    struct extract_placeholders_from_map_entry< ct::map_entry<sql2003::exprs, ExprList> > {
      typedef typename result_of::placeholders_from_list<ExprList>::type type;
      static type make(const ct::map_entry<sql2003::exprs, ExprList>& p) {
        return sql::placeholders_from_list(p.value);
      }
    };
  }
  
  namespace result_of {
    template<class Predicate>
    struct extract_placeholders_from_map_entry< ct::map_entry<sql2003::where, Predicate> > {
      typedef typename Predicate::placeholder_vector type;
      static type make(const ct::map_entry<sql2003::where, Predicate>& p) {
        return p.value.placeholders();
      }
    };
  }
    
  template<class Dialect, class State, class Data, class Subdialect>
  struct select_statement {

    typedef core::tabular_result_tag tag;

    typedef Data data_type;
    Data data_;

    select_statement(const Data& data) : data_(data) { }

    void str(std::ostream& os) const {
      os << "select";
      data_.for_each(str_clause(os));
    }

    template<class K, class T, class D = Data>
    struct transition {
      typedef select_statement<
        Subdialect,
        K,
        ct::map<K, T, D>,
        Subdialect
      > type;
    };

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_begin_call.hpp>
    #include BOOST_PP_ITERATE()

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_distinct.hpp>
    #include BOOST_PP_ITERATE()

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_all.hpp>
    #include BOOST_PP_ITERATE()

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_from.hpp>
    #include BOOST_PP_ITERATE()
    
    #include "detail/select_where.hpp"

    typedef typename result_of::placeholders_from_map<Data>::type placeholder_vector;
    
    placeholder_vector placeholders() const { return placeholders_from_map(data_); }
  };

  template<class Dialect, class State, class Data, class Subdialect>
  typename ct::result_of::value_at_key<Data, sql::sql2003::exprs>::type
  exprs(const select_statement<Dialect, State, Data, Subdialect>& st) {
    return ct::at_key<sql2003::exprs>(st.data_);
  }

  template<class Dialect, class State, class Data, class Subdialect>
  struct tag_of< select_statement<Dialect, State, Data, Subdialect> > {
    typedef typename select_statement<Dialect, State, Data, Subdialect>::tag type;

  };

} } }

namespace boost { namespace rdb { namespace core {

  template<class Dialect, class State, class Data, class Subdialect>
  struct statement_result_type< sql::select_statement<Dialect, State, Data, Subdialect> > {
    typedef typename ct::result_of::value_at_key<Data, sql::sql2003::exprs>::type type;
  };

} } }

#endif
