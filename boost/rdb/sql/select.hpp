//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  struct make_row {

    template<typename Sig>
    struct result;

    template<typename Self, typename Expr>
    struct result<Self(Expr)>
    {
      typedef typename boost::remove_reference<Expr>::type::cpp_type type;
    };
  };

  template<class SelectList>
  struct select_row {
    typedef typename fusion::result_of::as_vector<
      typename fusion::result_of::transform<SelectList, make_row>::type
    >::type type;
  };

  struct sql2003;

  template<class Dialect>
  struct select_where_context {
  };

  template<class Dialect>
  struct select_from_context {
    template<class Data> struct where { typedef select_statement<select_where_context<Dialect>, Data> type; };
  };

  template<class Dialect>
  struct select_projection_context {
    template<class Data> struct from { typedef select_statement<select_from_context<Dialect>, Data> type; };
  };

  template<class Dialect>
  struct select_context {
    template<class Data> struct select { typedef select_statement<select_projection_context<Dialect>, Data> type; };
    template<class Data> struct distinct { typedef select_statement<select_projection_context<Dialect>, Data> type; };
    template<class Data> struct all { typedef select_statement<select_projection_context<Dialect>, Data> type; };
  };
  
  template<class Context, class Data>
  struct select_begin : select_impl
  {
    Data data_;

#define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
//#define BOOST_PP_ITERATION_LIMITS (1, 1)
#define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_begin_call.hpp>
#include BOOST_PP_ITERATE()

#define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
//#define BOOST_PP_ITERATION_LIMITS (1, 1)
#define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_distinct.hpp>
#include BOOST_PP_ITERATE()

#define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
//#define BOOST_PP_ITERATION_LIMITS (1, 1)
#define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_all.hpp>
#include BOOST_PP_ITERATE()

  };

  extern select_begin< select_context<sql2003>, fusion::map<> > select;

  template<class Context, class Data>
  struct select_statement : select_impl {

    typedef select_statement_tag tag;
    typedef typename fusion::result_of::value_at_key<Data, select_impl::cols>::type select_list;
    typedef nullable<typename select_row<select_list>::type> row;
    //typedef std::deque<row> raw_result;
    typedef std::deque<row> result;

    select_statement(const Data& data) : data_(data) { }
    Data data_;

    const select_list& exprs() const {
      return fusion::at_key<cols>(data_);
    }

    void str(std::ostream& os) const {
      select_impl::str<Context>(os, data_);
    }

    #include <boost/preprocessor/iteration/iterate.hpp>
    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/select_from.hpp>
    #include BOOST_PP_ITERATE()

    template<class Predicate>
    typename transition::where<
      Context,
      typename result_of::add_key<Data, select_impl::where, Predicate>::type
    >::type
    where(const Predicate& predicate) const {
      return typename transition::where<
        Context,
        typename result_of::add_key<Data, select_impl::where, Predicate>::type
      >::type(add_key<select_impl::where>(data_, predicate));
    }
  };

  template<class Context, class Data>
  struct tag_of< select_statement<Context, Data> > {
    typedef typename select_statement<Context, Data>::tag type;
  };

} } }

#endif
