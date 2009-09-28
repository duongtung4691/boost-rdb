//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_INSERT_HPP
#define BOOST_RDB_INSERT_HPP

namespace boost { namespace rdb {

  struct insert_statement_tag : statement_tag { };

  template<class Table, class AssignList>
  struct insert_assign;

  template<class Table, class ColList>
  struct insert_cols;

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_values;

  template<class Context, class Data>
  struct insert_select;

  template<class Context, class Data>
  struct insert_select_from;

  struct standard_insert_context {
    typedef standard_insert_context this_context;

    template<class Data>
    struct call {
      typedef insert_select<this_context, Data> type;
    };

    template<class Data>
    struct from {
      typedef insert_select_from<this_context, Data> type;
    };

    template<class Data>
    struct where {
      typedef insert_select_from<this_context, Data> type;
    };

  };

  template<class Table>
  struct insert_statement {

    template<class Col>
    typename insert_cols<Table, detail::empty>::template with<Col>::type
    operator ()(const expression<Col>& col) const {
      return insert_cols<Table, detail::empty>(detail::empty())(col);
    }

    template<class Col, class T>
    typename insert_assign<Table, detail::empty>::template with<Col, T>::type
    set(const expression<Col>& col, const T& expr) const {
      return insert_assign<Table, detail::empty>(detail::empty())(col, expr);
    }

#define BOOST_RDB_PP_INSERT_COLS(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, class Tcol)> \
    typename insert_cols<Table, typename result_of::make_list< Tcol0 >::type \
      >BOOST_PP_REPEAT_FROM_TO(1, n, BOOST_RDB_PP_WITH, Tcol) \
    operator ()(BOOST_PP_REPEAT(n, BOOST_RDB_PP_EXPRESSION, col)) const { \
      return (*this)BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, col); \
    }

    BOOST_PP_REPEAT_FROM_TO(2, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_COLS, ~)
  };

  struct insert_impl {
    class table;
    class cols;

    template<class Data>
    static void str(std::ostream& os, const Data& data) {
      os << "insert into " << fusion::at_key<table>(data)->table_name() << " (";
      fusion::for_each(fusion::at_key<cols>(data), comma_output(os));
      os << ")";
    }
  };

  template<class Table, class ColList>
  struct insert_cols : insert_impl {

    typedef ColList col_list_type;

    typedef fusion::map<
      fusion::pair<table, const Table*>,
      fusion::pair<cols, ColList>
    > Data;

    Data data_;
    typedef standard_insert_context Context;

    insert_cols(const ColList& kols) : cols_(kols),
      data_(fusion::make_map<table, cols>(&Table::_, kols)) { }

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    //#define BOOST_PP_ITERATION_LIMITS (1, 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/detail/insert_select.hpp>
    #include BOOST_PP_ITERATE()

    template<class Col>
    struct with {
      typedef insert_cols<
        Table,
        typename fusion::result_of::push_back<const ColList, Col>::type
      > type;
    };

    template<class Col>
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
    (typename with<Col>::type))
    operator ()(const expression<Col>& col) const {
      BOOST_MPL_ASSERT((boost::is_same<Table, typename Col::table_type>));
      return insert_cols<
        Table,
        typename fusion::result_of::push_back<const ColList, Col>::type
      >(fusion::push_back(cols_, col.unwrap()));
    }

  BOOST_PP_REPEAT_FROM_TO(2, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_COLS, ~)

    ColList cols_;

    typedef insert_values<Table, ColList, detail::empty,
      typename fusion::result_of::begin<ColList>::type
      > insert_values0;
    
#define BOOST_RDB_PP_INSERT_VALUES(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, typename T)> \
    typename insert_values0 BOOST_PP_REPEAT(n, BOOST_RDB_PP_WITH, T) \
    values(BOOST_PP_ENUM_BINARY_PARAMS(n, T, arg)) const { \
      return insert_values0(cols_, detail::empty())BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, arg); \
    }

BOOST_PP_REPEAT_FROM_TO(1, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_VALUES, ~)

  };

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_values {

    typedef insert_statement_tag tag;
    typedef void result;

    typedef insert_values type;
    insert_values(const ColList& cols, const ExprList& values) : cols_(cols), values_(values) { }

    ColList cols_;
    ExprList values_;

    template<typename T>
    struct with {
    
      typedef typename boost::remove_reference<
        typename fusion::result_of::deref<ColIter>::type
      >::type col_type;
    
      typedef typename result_of::make_expression<expression<col_type>, T>::type value_type;
      
      typedef typename fusion::result_of::push_back<
        const ExprList, value_type>::type values_list_type;

      typedef typename fusion::result_of::next<ColIter>::type next_col_iter;
        
      typedef insert_values<Table, ColList, values_list_type, next_col_iter> type;
    };

    template<typename T>
    typename with<T>::type
    operator ()(const T& expr) const {
      typedef typename with<T>::type result_type;
      typedef typename boost::remove_reference<typename fusion::result_of::deref<ColIter>::type>::type col_type;
      return result_type(cols_, fusion::push_back(values_, expression<col_type>::make_expression(expr)));
    }

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " (";
      fusion::for_each(cols_, comma_output(os));
      os << ") values (";
      fusion::for_each(values_, comma_output(os));
      os << ")";
    }
  };

  template<class Table, class AssignList>
  struct insert_assign {

    typedef insert_statement_tag tag;
    typedef void result;

    insert_assign(const AssignList& assign) : assigns_(assign) { }

    AssignList assigns_;

    template<class Col, class Expr>
    struct with {
      typedef insert_assign<
        Table,
        typename fusion::result_of::push_back<
          const AssignList, 
          typename fusion::result_of::make_vector<
            typename result_of::unwrap<Col>::type,
            typename result_of::make_expression<expression<Col>, Expr>::type
          >::type
        >::type
      > type;
    };

    template<class Col, class T>
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
      (typename with<Col, T>::type))
    operator ()(const expression<Col>& col, const T& expr) const {
      return typename with<Col, T>::type(
        fusion::push_back(assigns_,
          fusion::make_vector(
            col.unwrap(),
            expression<Col>::make_expression(expr))));
    }

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " set ";
      fusion::for_each(assigns_, assign_output(os));
    }
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
      insert_impl::str(os, data_);
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
      insert_impl::str(os, data_);
      os << " ";
      base::str(os);
    }
  };

  template<class Table>
  insert_statement<Table>
  insert_into(const Table& table) {
    return insert_statement<Table>();
  }

} }

#endif
