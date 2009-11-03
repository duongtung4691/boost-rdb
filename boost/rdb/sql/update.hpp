//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_UPDATE_HPP
#define BOOST_RDB_UPDATE_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/expression.hpp>

namespace boost { namespace rdb { namespace sql {

  struct extract_placeholders_from_assign {

    template<typename Sig>
    struct result;
    
    typedef extract_placeholders_from_assign Self;
    
    template<class Update, class Placeholders>
    struct result<Self(Update&, Placeholders&)> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::join<
          const Placeholders,
          const typename Update::placeholder_vector
        >::type
      >::type type;
      
      static type make(const Update& update, const Placeholders& placeholders) {
        return fusion::join(placeholders, update.placeholders());
      }
    };

    // I think that the reason why we need both const and non-const version is
    // inconsistencies in fusion's functions and result_of meta-functions
    template<class Update, class Placeholders>
    struct result<Self(const Update&, const Placeholders&)> : result<Self(Update&, Placeholders&)> {
    };

    template<class Update, class Placeholders>
    typename result<Self(Update&, Placeholders&)>::type
    operator ()(const Update& update, const Placeholders& placeholders) {
      using namespace fusion;
      return result<Self(const Update&, const Placeholders&)>::make(update, placeholders);
    }
  };
  
  namespace result_of {
    template<class AssignList>
    struct extract_placeholders_from_pair<sql2003::set, AssignList> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::accumulate<AssignList, fusion::vector<>, extract_placeholders_from_assign>::type
      >::type type;
      static type make(const fusion::pair<sql2003::set, AssignList>& p) {
        return fusion::accumulate(p.second, fusion::vector<>(), extract_placeholders_from_assign());
      }
    };
  }

  template<class Dialect, class State, class Data, class Subdialect>
  struct update_statement :
    tag_if<fusion::result_of::has_key<Data, typename Subdialect::set>, update_statement_tag> {

    explicit update_statement(const Data& data) : data_(data) { }

    typedef void result;
    typedef typename result_of::placeholders_from_pair_list<Data>::type placeholder_vector;

    placeholder_vector placeholders() const {
      using namespace fusion;
      return placeholders_from_pair_list(data_);
    }

    Data data_;

    template<class K, class T, class D = Data>
    struct transition {
      typedef update_statement<
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
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/update_set.hpp>
    #include BOOST_PP_ITERATE()
    
    #include "detail/select_where.hpp"

    void str(std::ostream& os) const {
      fusion::for_each(data_, str_clause(os));
    }
  };

  BOOST_RDB_ALLOW(sql2003, set, where);

  template<class Table>

  inline void str(std::ostream& os, const fusion::pair<sql2003::update, const Table*>& p) {
    os << "update ";
    os << p.second->table();
  }

  template<class AssignList>
  inline void str(std::ostream& os, const fusion::pair<sql2003::set, AssignList>& p) {
    os << " set ";
    fusion::for_each(p.second, comma_output(os));
  }
  
  template<class Table>
  update_statement<
    sql2003,
    sql2003::update,
    fusion::map<
      fusion::pair<
      sql2003::update, const Table*
      >
    >,
    sql2003
  >
  update(const Table& table) {
    return update_statement<
      sql2003,
      sql2003::update,
      fusion::map<
        fusion::pair<
        sql2003::update, const Table*
        >
      >,
      sql2003
    >(fusion::make_pair<sql2003::update>(&table));
  }

} } }

#endif
