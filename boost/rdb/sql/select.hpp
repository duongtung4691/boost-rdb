//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

#include <boost/rdb/sql/common.hpp>

#include <deque>

namespace boost { namespace rdb { namespace sql {

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

    namespace result_of {
      template<class Entry>
      struct extract_placeholders_from_static_map_entry {
        typedef fusion::vector<> type;
        static type make(const Entry&) { return type(); }
      };
    }
    
    template<class Entry>
    typename result_of::extract_placeholders_from_static_map_entry<Entry>::type
    extract_placeholders_from_static_map_entry(const Entry& p) {
      return result_of::extract_placeholders_from_static_map_entry<Entry>::make(p);
    }
    
    struct extract_placeholders_from_static_map {

      template<class Entry, class Seq>
      struct result {
        typedef typename fusion::result_of::as_vector<
          typename fusion::result_of::join<
            Seq,
            typename result_of::extract_placeholders_from_static_map_entry<Entry>::type
          >::type
        >::type type;
      };
      
      template<class Entry, class Seq>
      typename result<Entry, Seq>::type
      operator ()(const Entry& entry, const Seq& seq) const {
        return fusion::as_vector(fusion::join(seq,
          result_of::extract_placeholders_from_static_map_entry<Entry>::make(entry.value)));
          //extract_placeholders_from_static_map_entry(val)));
      }
    };

    namespace result_of {
      template<class Map>
      struct placeholders_from_static_map {
        typedef typename ct::result_of::accumulate<
          Map,
          extract_placeholders_from_static_map,
          fusion::vector<>
        >::type type;
      };
    }
  
    template<class Map>
    typename result_of::placeholders_from_static_map<Map>::type
    placeholders_from_static_map(const Map& map) {
      return ct::accumulate(map, extract_placeholders_from_static_map(), fusion::make_vector());
    }
  
  extern select_statement<sql2003, sql2003::select, ct::static_map0, sql2003> select;

  namespace result_of {
    template<class ExprList>
    struct extract_placeholders_from_static_map_entry< ct::static_map_entry<sql2003::exprs, ExprList> > {
      typedef typename result_of::placeholders_from_list<ExprList>::type type;
      static type make(const ct::static_map_entry<sql2003::exprs, ExprList>& p) {
        return sql::placeholders_from_list(p.value);
      }
    };
  }
  
  namespace result_of {
    template<class Predicate>
    struct extract_placeholders_from_static_map_entry< ct::static_map_entry<sql2003::where, Predicate> > {
      typedef typename Predicate::placeholder_vector type;
      static type make(const ct::static_map_entry<sql2003::where, Predicate>& p) {
        return p.value.placeholders();
      }
    };
  }
    
  template<class Dialect, class State, class Data, class Subdialect>
  struct select_statement {

    typedef select_statement_tag tag;

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
        ct::static_map<K, T, D>,
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

    typedef typename result_of::placeholders_from_static_map<Data>::type placeholder_vector;
    
    placeholder_vector placeholders() const { return placeholders_from_static_map(data_); }
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

namespace boost { namespace rdb {

  template<class Dialect, class State, class Data, class Subdialect>
  struct statement_result_type< sql::select_statement<Dialect, State, Data, Subdialect> > {
    typedef typename ct::result_of::value_at_key<Data, sql::sql2003::exprs>::type type;
  };

} }

#endif
