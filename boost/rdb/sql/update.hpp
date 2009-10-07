//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_UPDATE_HPP
#define BOOST_RDB_UPDATE_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  struct update_statement_tag : statement_tag { };

  template<class Dialect, class State, class Data, class Subdialect>
  struct update_statement :
    tag_if<fusion::result_of::has_key<Data, typename Subdialect::update::set>, update_statement_tag> {

    explicit update_statement(const Data& data) : data_(data) { }

    typedef void result;

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

    template<class Predicate>
    typename transition<typename Subdialect::update::where, Predicate>::type
    where(const Predicate& predicate) const {
      BOOST_MPL_ASSERT((allow<Subdialect, State, typename Subdialect::update::where>));
      return typename transition<typename Subdialect::update::where, Predicate>::type(
        add_key<typename Subdialect::update::where>(this->data_, predicate));
    }

    void str(std::ostream& os) const {
      fusion::for_each(data_, str_clause(os));
    }
  };

  BOOST_RDB_ALLOW(sql2003, update::set, update::where);

  template<class Table>

  inline void str(std::ostream& os, const fusion::pair<sql2003::update::table, const Table*>& p) {
    os << "update ";
    os << p.second->table();
  }

  template<class SetList>
  inline void str(std::ostream& os, const fusion::pair<sql2003::update::set, SetList>& p) {
    os << " set ";
    fusion::for_each(p.second, comma_output(os));
  }

  template<class Table>
  update_statement<
    sql2003,
    sql2003::update::table,
    fusion::map<
      fusion::pair<
      sql2003::update::table, const Table*
      >
    >,
    sql2003
  >
  update(const Table& table) {
    return update_statement<
      sql2003,
      sql2003::update::table,
      fusion::map<
        fusion::pair<
        sql2003::update::table, const Table*
        >
      >,
      sql2003
    >(fusion::make_pair<sql2003::update::table>(&table));
  }

} } }

#endif
