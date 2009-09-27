//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

namespace boost { namespace rdb {

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

  template<class Key, class Map>
  inline typename disable_if<fusion::result_of::has_key<Map, Key>, void>::type
  str_opt_list(std::ostream& os, const char* keyword, const Map& data) {
  }

  template<class Key, class Map>
  inline typename enable_if<fusion::result_of::has_key<Map, Key>, void>::type
  str_opt_list(std::ostream& os, const char* keyword, const Map& data) {
    os << " " << keyword << " ";
    fusion::for_each(fusion::at_key<Key>(data), comma_output(os));
  }

  template<class Key, class Map>
  inline typename disable_if<fusion::result_of::has_key<Map, Key>, void>::type
  str_opt(std::ostream& os, const char* keyword, const Map& data) {
  }

  template<class Key, class Map>
  inline typename enable_if<fusion::result_of::has_key<Map, Key>, void>::type
  str_opt(std::ostream& os, const char* keyword, const Map& data) {
    os << " " << keyword << " ";
    fusion::at_key<Key>(data).str(os);
  }

  template<class Key, class Map>
  inline typename disable_if<fusion::result_of::has_key<Map, Key>, void>::type
  str_opt_kw(std::ostream& os, const char* keyword, const Map& data) {
  }

  template<class Key, class Map>
  inline typename enable_if<fusion::result_of::has_key<Map, Key>, void>::type
  str_opt_kw(std::ostream& os, const char* keyword, const Map& data) {
    os << " " << keyword;
  }

  struct select_impl {

    class cols;
    class distinct;
    class all;
    class tables;
    class where;
    class group_by;
    class order_by;

    template<class Data>
    static void str(std::ostream& os, const Data& data) {
      os << "select";
      
      str_opt_kw<distinct>(os, "distinct", data);
      str_opt_kw<all>(os, "all", data);

      os << " ";
      fusion::for_each(fusion::at_key<cols>(data), comma_output(os));

      str_opt_list<tables>(os, "from", data);
      str_opt<where>(os, "where", data);
    }
  };

  template<class Data>
  struct select_exprs;
  
  template<class Data>
  struct select_begin : select_impl
  {
    Data data_;

#include <boost/preprocessor/iteration/iterate.hpp>
#define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
//#define BOOST_PP_ITERATION_LIMITS (1, 1)
#define BOOST_PP_FILENAME_1       <boost/rdb/detail/select_begin_call.hpp>
#include BOOST_PP_ITERATE()
  };

  template<class Data>
  struct select_exprs : select_impl {

    select_exprs(const Data& data) : data_(data) { }
    Data data_;

    void str(std::ostream& os) const {
      select_impl::str(os, data_);
    }

#include <boost/preprocessor/iteration/iterate.hpp>
#define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
#define BOOST_PP_FILENAME_1       <boost/rdb/detail/select_from.hpp>
#include BOOST_PP_ITERATE()
  };

  struct plain_select : select_begin< fusion::map<> > {
    select_begin< fusion::map<fusion::pair< select_impl::distinct, int> > > distinct;
    select_begin< fusion::map<fusion::pair< select_impl::all, int> > > all;
  };

  extern plain_select select;

  template<class Data>
  struct select_statement : select_impl {

    typedef select_statement_tag tag;
    typedef typename fusion::result_of::value_at_key<Data, select_impl::cols>::type select_list;
    typedef std::deque<typename select_row<select_list>::type> result;

    select_statement(const Data& data) : data_(data) { }
    Data data_;

    const select_list& exprs() const {
      return fusion::at_key<cols>(data_);
    }

    void str(std::ostream& os) const {
      select_impl::str(os, data_);
    }

    template<class Predicate>
    select_statement<
      typename result_of::add_key<Data, select_impl::where, Predicate>::type
    >
    where(const Predicate& predicate) const {
      return select_statement<
        typename result_of::add_key<Data, select_impl::where, Predicate>::type
      >(add_key<select_impl::where>(data_, predicate));
    }
  };

} }

#endif
