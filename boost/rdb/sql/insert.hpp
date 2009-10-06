//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_INSERT_HPP
#define BOOST_RDB_INSERT_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/expression.hpp>
#include <boost/rdb/sql/select.hpp>

namespace boost { namespace rdb { namespace sql {

  struct insert_impl {

    template<class Data>
    static void str(std::ostream& os, const Data& data) {
      fusion::for_each(data, str_clause(os));
    }
  };

  template<class Table>
  inline void str(std::ostream& os, const fusion::pair<sql2003::insert::table, const Table*>& p) {
    os << "insert into ";
    os << p.second->table();
  }

  template<class ColList>
  inline void str(std::ostream& os, const fusion::pair<sql2003::insert::cols, ColList>& p) {
    os << " (";
    fusion::for_each(p.second, comma_output(os));
    os << ")";
  }

  template<class ValueList>
  inline void str(std::ostream& os, const fusion::pair<sql2003::insert::values, ValueList>& p) {
    os << " values (";
    fusion::for_each(p.second, comma_output(os));
    os << ")";
  }

  template<class Dialect, class State, class Data, class Subdialect>
  struct insert_statement;

  template<class Dialect, class State, class Data, class T>
  struct insert_transition {
    typedef insert_statement<
      Dialect,
      State,
      typename result_of::add_key<
        Data,
        State,
        T
      >::type,
      Dialect
    > type;
  };

  template<class Dialect, class State, class Data, class Subdialect>
  struct insert_statement : State::tags {

    explicit insert_statement(const Data& data) : data_(data) { }

    // todo: conditionally enable the two typedefs
    typedef void result;

    Data data_;

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/insert_cols.hpp>
    #include BOOST_PP_ITERATE()

    template<class ColIter, class ColLast, class ExprIter>
    struct value_list;

    template<class ColLast, class ExprIter>
    struct value_list<ColLast, ColLast, ExprIter> {
      typedef fusion::result_of::make_vector<>::type type;

      template<class T>
      static const fusion::vector<> make(const T&) {
        return fusion::vector<>();
      }
    };  

    template<class ColIter, class ColLast, class ExprIter>
    struct value_list {

      typedef typename remove_reference<
        typename fusion::result_of::deref<ColIter>::type
      >::type col_type;

      typedef value_list<
        typename fusion::result_of::next<ColIter>::type,
        ColLast,
        typename fusion::result_of::next<ExprIter>::type
      > next;

      typedef typename fusion::result_of::push_front<
        const typename next::type,
        typename result_of::make_expression<
          col_type,
          typename remove_reference<
            typename fusion::result_of::deref<ExprIter>::type
          >::type
        >::type
      >::type type;

      template<class T>
      static const type make(const T& iter) { // why doesn't ExprIter work ?
        return fusion::push_front(
          next::make(fusion::next(iter)),
          expression<col_type>::make_expression(*iter)
          );
      }
    };  

    template<class Exprs>
    struct with_values {

      typedef typename fusion::result_of::value_at_key<Data, typename Subdialect::insert::cols>::type cols;
      typedef typename fusion::result_of::end<cols>::type col_last;

      // If this assertion fails the insert list and the value list have different sizes
      BOOST_MPL_ASSERT((mpl::equal_to<
        fusion::result_of::size<cols>,
        fusion::result_of::size<Exprs> >));
      //BOOST_MPL_ASSERT((is_sql_compatible<cols, Exprs>));

      typedef value_list<
        typename fusion::result_of::begin<cols>::type,
        typename fusion::result_of::end<cols>::type,
        typename fusion::result_of::begin<Exprs>::type
      > final_value_list;

      typedef typename insert_transition<
        Subdialect,
        typename Subdialect::insert::values,
        Data,
        typename fusion::result_of::as_vector<
          typename final_value_list::type
        >::type
      >::type type;

      static type make(const Data& data, const Exprs& exprs) {
        return type(add_key<typename Subdialect::insert::values>(
          data, fusion::as_vector(final_value_list::make(fusion::begin(exprs)))));
      }
    };


    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/insert_values.hpp>
    #include BOOST_PP_ITERATE()

#if 0
    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/insert_select.hpp>
    #include BOOST_PP_ITERATE()
    #endif
#endif
    void str(std::ostream& os) const {
      fusion::for_each(data_, str_clause(os));
    }

  };

#if 0
  template<class Dialect, class State, class Data, class Subdialect>
  struct insert_select : insert_impl, select_statement<Dialect, State, Data> {

    typedef void result;
    typedef select_statement<Dialect, State, Data> base;

    insert_select(const Data& data) : base(data) {
      typedef typename fusion::result_of::value_at_key<Data, typename Subdialect::insert::cols>::type insert_list;
      typedef typename fusion::result_of::value_at_key<Data, select_impl::cols>::type select_list;
      BOOST_MPL_ASSERT((mpl::equal_to<
        fusion::result_of::size<insert_list>,
        fusion::result_of::size<select_list> >));
      BOOST_MPL_ASSERT((sql_lists_compatible<insert_list, select_list>));
    }

    void str(std::ostream& os) const {
      typename Subdialect::insert::str(os, base::data_);
      os << " ";
      base::str(os);
    }
  };

  template<class Context, class Data>
  struct insert_select_from : insert_impl, select_statement<Context, Data> {

    typedef insert_statement_tag tag;
    typedef void result;
    typedef select_statement<Context, Data> base;

    insert_select_from(const Data& data) : base(data) { }

    void str(std::ostream& os) const {
      typename Subdialect::insert::str(os, base::data_);
      os << " ";
      base::str(os);
    }
  };
#endif

  template<class Table>
  insert_statement<
    sql2003,
    sql2003::insert::table,
    fusion::map<
      fusion::pair<
      sql2003::insert::table, const Table*
      >
    >,
    sql2003
  >
  insert_into(const Table& table) {
    return insert_statement<
      sql2003,
      sql2003::insert::table,
      fusion::map<
        fusion::pair<
        sql2003::insert::table, const Table*
        >
      >,
      sql2003
    >(fusion::make_pair<sql2003::insert::table>(&table));
  }

} } }
