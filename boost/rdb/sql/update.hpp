//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_UPDATE_HPP
#define BOOST_RDB_UPDATE_HPP

namespace boost { namespace rdb { namespace sql {

  struct update_statement_tag : statement_tag { };

  template<class Context, class Data>
  struct update_set;

  template<class Table, class AssignList>
  struct update_where;

  struct standard_update {
    typedef standard_update this_context;
    template<class Data> struct set { typedef update_set<this_context, Data> type; };
    template<class Data> struct where { typedef update_where<this_context, Data> type; };
  };

  struct update_impl {

    class table;
    class cols;
    class set;
    class where;

    template<class Data>
    static void str(std::ostream& os, const Data& data) {
      os << "update " << fusion::at_key<table>(data)->table();
      str_list_if_has_key<set>(os, " set ", data, "");
      str_obj_if_has_key<where>(os, " where ", data, "");
    }
  };

  template<class Context, class Data>
  struct update_table : update_impl {

    explicit update_table(const Data& data) : data_(data) { }

    Data data_;

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/update_set.hpp>
    #include BOOST_PP_ITERATE()

    void str(std::ostream& os) const { update_impl::str(os, data_); }

  };

  template<class Context, class Data>
  struct update_set : update_impl {

    typedef update_statement_tag tag;
    typedef void result;

    explicit update_set(const Data& data) : data_(data) { }
    Data data_;

    template<class Predicate>
    typename transition::where<
      Context,
      typename result_of::add_key<Data, update_impl::where, Predicate>::type
    >::type
    where(const Predicate& predicate) const {
      return typename transition::where<
        Context,
        typename result_of::add_key<Data, update_impl::where, Predicate>::type
      >::type(add_key<update_impl::where>(data_, predicate));
    }

    void str(std::ostream& os) const { update_impl::str(os, data_); }
  };

  template<class Context, class Data>
  struct update_where : update_impl {

    typedef update_statement_tag tag;
    typedef void result;

    explicit update_where(const Data& data) : data_(data) { }
    Data data_;

    void str(std::ostream& os) const { update_impl::str(os, data_); }
  };

  template<class Table>
  update_table<
    standard_update,
    fusion::map<
      fusion::pair<
      update_impl::table, const Table*
      >
    >
  >
  update(const Table& table) {
    return update_table<
      standard_update,
      fusion::map<
        fusion::pair<
          update_impl::table, const Table*
        >
      >
    >(fusion::make_pair<update_impl::table>(&table));
  }

} } }

#endif
