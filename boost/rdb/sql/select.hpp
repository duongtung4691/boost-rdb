//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

#include <boost/rdb/sql/common.hpp>

#include <deque>

namespace boost { namespace rdb { namespace sql {

  extern select_statement<sql2003, sql2003::select, fusion::map<>, sql2003> select;
  
  template<class Data, class Key, class Enable = void>
  struct select_result_if {
    select_result_if(const Data& data) : data_(data) { }
    Data data_;
  };
  
  template<class Data, class Key>
  struct select_result_if<
    Data,
    Key,
    typename enable_if<
      fusion::result_of::has_key<Data, Key>
    >::type
  > {

    typedef typename fusion::result_of::value_at_key<Data, Key>::type select_list;

    select_result_if(const Data& data) : data_(data) { }
    Data data_;

    select_list exprs() const {
      return fusion::at_key<Key>(data_);
    }
  };

  namespace result_of {
    template<class ExprList>
    struct extract_placeholders_from_pair<sql2003::exprs, ExprList> {
      typedef typename result_of::placeholders_from_list<ExprList>::type type;
      static type make(const fusion::pair<sql2003::exprs, ExprList>& p) {
        return sql::placeholders_from_list(p.second);
      }
    };
  }
  
  namespace result_of {
    template<class Predicate>
    struct extract_placeholders_from_pair<sql2003::where, Predicate> {
      typedef typename Predicate::placeholder_vector type;
      static type make(const fusion::pair<sql2003::where, Predicate>& p) {
        return p.second.placeholders();
      }
    };
  }
    
  template<class Dialect, class State, class Data, class Subdialect>
  struct select_statement :
    select_result_if<Data, typename Subdialect::exprs>,
    tag_if<fusion::result_of::has_key<Data, typename Subdialect::from>, select_statement_tag> {

    select_statement(const Data& data) : select_result_if<Data, typename Subdialect::exprs>(data) { }

    void str(std::ostream& os) const {
      os << "select";
      fusion::for_each(this->data_, str_clause(os));
    }

    template<class K, class T, class D = Data>
    struct transition {
      typedef select_statement<
        Subdialect,
        K,
        typename result_of::add_key<
          D,
          K,
          T
        >::type,
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

    typedef typename result_of::placeholders_from_pair_list<Data>::type placeholder_vector;
    
    placeholder_vector placeholders() const {
      return placeholders_from_pair_list(this->data_);
    }
  };

  template<class Dialect, class State, class Data, class Subdialect>
  struct tag_of< select_statement<Dialect, State, Data, Subdialect> > {
    typedef typename select_statement<Dialect, State, Data, Subdialect>::tag type;

  };

} } }

#endif
