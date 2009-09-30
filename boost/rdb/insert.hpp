//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_INSERT_HPP
#define BOOST_RDB_INSERT_HPP

namespace boost { namespace rdb {

  struct insert_statement_tag : statement_tag { };

  template<class Table, class AssignList>
  struct insert_assign;

  template<class Context, class Data>
  struct insert_cols;

  template<class Context, class Data>
  struct insert_values;

  template<class Context, class Data>
  struct insert_select;

  template<class Context, class Data>
  struct insert_select_from;

  struct standard_insert {
    typedef standard_insert this_context;
    template<class Data> struct cols { typedef insert_cols<this_context, Data> type; };
    template<class Data> struct values { typedef insert_values<this_context, Data> type; };
    template<class Data> struct select { typedef insert_select<this_context, Data> type; };
    template<class Data> struct from { typedef insert_select_from<this_context, Data> type; };
    template<class Data> struct where { typedef insert_select_from<this_context, Data> type; };
  };

  struct insert_impl {

    class table;
    class cols;
    class values;

    template<class Data>
    static void str(std::ostream& os, const Data& data) {
      os << "insert into " << fusion::at_key<table>(data)->table_name();
      str_list_if_has_key<cols>(os, " (", data, ")");
      str_list_if_has_key<values>(os, " values (", data, ")");
    }
  };

  template<class Context, class Data>
  struct insert_table : insert_impl {

    explicit insert_table(const Data& data) : data_(data) { }

    Data data_;

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/detail/insert_cols.hpp>
    #include BOOST_PP_ITERATE()

    void str(std::ostream& os) const { insert_impl::str(os, data_); }

  };

  template<class Context, class Data>
  struct insert_cols : insert_impl {

    explicit insert_cols(const Data& data) : data_(data) { }

    Data data_;

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/detail/insert_values.hpp>
    #include BOOST_PP_ITERATE()

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/detail/insert_select.hpp>
    #include BOOST_PP_ITERATE()

    void str(std::ostream& os) const { insert_impl::str(os, data_); }

  };

  template<class Context, class Data>
  struct insert_values: insert_impl {

    explicit insert_values(const Data& data) : data_(data) { }

    typedef insert_statement_tag tag;
    typedef void result;

    Data data_;

    void str(std::ostream& os) const { insert_impl::str(os, data_); }

  };

  template<class Context, class Data>
  struct insert_select : insert_impl, select_projection<Context, Data> {

    typedef void result;
    typedef select_projection<Context, Data> base;

    insert_select(const Data& data) : base(data) {
      typedef typename fusion::result_of::value_at_key<Data, insert_impl::cols>::type insert_list;
      typedef typename fusion::result_of::value_at_key<Data, select_impl::cols>::type select_list;
      BOOST_MPL_ASSERT((mpl::equal_to<
        fusion::result_of::size<insert_list>,
        fusion::result_of::size<select_list> >));
      BOOST_MPL_ASSERT((sql_compatible<insert_list, select_list>));
    }

    void str(std::ostream& os) const {
      insert_impl::str(os, base::data_);
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
      insert_impl::str(os, base::data_);
      os << " ";
      base::str(os);
    }
  };
  template<class Table>
  insert_table<
    standard_insert,
    fusion::map<
      fusion::pair<
      insert_impl::table, const Table*
      >
    >
  >
  insert_into(const Table& table) {
    return insert_table<
      standard_insert,
      fusion::map<
        fusion::pair<
          insert_impl::table, const Table*
        >
      >
    >(fusion::make_pair<insert_impl::table>(&table));
  }

} }

#endif
