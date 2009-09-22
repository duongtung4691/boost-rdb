//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_INSERT_HPP
#define BOOST_RDB_INSERT_HPP

namespace boost { namespace rdb {

  template<class Insert>
  struct insert_statement : Insert {
    insert_statement(const typename Insert::col_list_type& cols, const typename Insert::value_list_type& values) 
      : Insert(cols, values) { }
  };

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_vals;

  template<class Table, class ColList, class ValueList>
  struct insert_type;

  namespace result_of {
    template<class Table, class ColList, class ColIter, class ValueList, class Expr>
    struct insert_expr {
      typedef typename boost::fusion::result_of::next<ColIter>::type next_col_iter;
      
      typedef typename boost::fusion::result_of::push_back<
        const ValueList, Expr>::type values_type;
        
      typedef typename boost::mpl::if_<
        boost::is_same<
          next_col_iter,
          typename boost::fusion::result_of::end<ColList>::type
        >,
        insert_statement< insert_type<Table, ColList, values_type> >,
        insert_vals<Table, ColList, values_type, next_col_iter>
      >::type type;
    };

    template<class Table, class ColList, class ColIter, class ValueList, typename T>
    struct insert_literal : insert_expr<Table, ColList, ColIter, ValueList,
      typename boost::remove_reference<
        typename boost::fusion::result_of::deref<ColIter>::type
      >::type::sql_type::literal_type
    > {
    };
  }


  template<class Table, class ColList = details::empty>
  struct insert_cols {

    typedef ColList col_list_type;

    insert_cols(const ColList& cols) : cols_(cols) { }

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " (";
      boost::fusion::for_each(cols_, comma_output(os));
      os << ")";
    }

    template<class Col>
    struct with {
      typedef insert_cols<
        Table,
        typename boost::fusion::result_of::push_back<const ColList, Col>::type
      > type;
    };

    template<class Col>
    /*
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
      (*/insert_cols<
        Table,
        typename boost::fusion::result_of::push_back<const ColList, Col>::type
      >
      //))
    operator ()(const expression<Col>& col) const {
    BOOST_MPL_ASSERT((boost::is_same<Table, typename Col::table_type>));
      return insert_cols<
        Table,
        typename boost::fusion::result_of::push_back<const ColList, Col>::type
      >(boost::fusion::push_back(cols_, col.unwrap()));
    }

#define BOOST_RDB_PP_INSERT_COLS(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, class Tcol)> \
    typename insert_cols<Table, typename result_of::make_list< Tcol0 >::type \
      >BOOST_PP_REPEAT_FROM_TO(1, n, BOOST_RDB_PP_WITH, Tcol) \
    operator ()(BOOST_PP_REPEAT(n, BOOST_RDB_PP_EXPRESSION, col)) { \
      return (*this)BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, col); \
    }

  BOOST_PP_REPEAT_FROM_TO(2, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_COLS, ~)

    ColList cols_;

    typedef insert_vals<Table, ColList, details::empty,
      typename boost::fusion::result_of::begin<ColList>::type
      > insert_values0;
    
#define BOOST_RDB_PP_INSERT_VALUES(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, typename T)> \
    typename insert_values0 BOOST_PP_REPEAT(n, BOOST_RDB_PP_WITH, T) \
    values(BOOST_PP_ENUM_BINARY_PARAMS(n, T, arg)) const { \
      return insert_values0(cols_, details::empty())BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, arg); \
    }

BOOST_PP_REPEAT_FROM_TO(1, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_VALUES, ~)
  };

  template<class Table, class ColList, class ValueList>
  struct insert_type : insert_cols<Table, ColList> {

    typedef insert_cols<Table, ColList> just_cols;
    typedef ColList col_list_type;
    typedef ValueList value_list_type;
    
    insert_type(const ColList& cols, const ValueList& values) : just_cols(cols), values_(values) { }

    ValueList values_;

    void str(std::ostream& os) const {
      just_cols::str(os);
      os << " values (";
      boost::fusion::for_each(values_, comma_output(os));
      os << ")";
    }
  };

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_vals {

    //typedef insert_cols<Table, ColList> just_cols;
    typedef insert_vals type;
    insert_vals(const ColList& cols, const ExprList& values) : cols_(cols), values_(values) { }

    ColList cols_;
    ExprList values_;

    template<typename T>
    struct with {
      typedef typename result_of::insert_literal<Table, ColList, ColIter, ExprList, T>::type type;
    };

    template<typename T>
    typename with<T>::type
    operator ()(const T& expr) const {
      typedef typename with<T>::type result_type;
      typedef typename boost::remove_reference<typename boost::fusion::result_of::deref<ColIter>::type>::type col_type;
      return result_type(cols_, boost::fusion::push_back(values_, col_type::sql_type::make_literal(expr)));
    }

    template<class Expr>
    struct with< const expression<Expr>& > {
      typedef typename result_of::insert_expr<Table, ColList, ColIter, ExprList, Expr>::type type;
    };

    template<class Expr>
    struct with< expression<Expr>& > {
      typedef typename result_of::insert_expr<Table, ColList, ColIter, ExprList, Expr>::type type;
    };

    template<class Expr>
    struct with< expression<Expr> > {
      typedef typename result_of::insert_expr<Table, ColList, ColIter, ExprList, Expr>::type type;
    };

    template<class Expr>
    struct with< const expression<Expr> > {
      typedef typename result_of::insert_expr<Table, ColList, ColIter, ExprList, Expr>::type type;
    };

    template<class Expr>
    BOOST_CONCEPT_REQUIRES(
      ((Expression<Expr>)),
      (typename with< const expression<Expr>& >::type))
    operator ()(const expression<Expr>& expr) const {
      typedef typename with< const expression<Expr>& >::type result_type;
      return result_type(cols_, boost::fusion::push_back(values_, expr.unwrap()));
    }
  };

  template<class Table>
  insert_cols<Table>
  insert_into(const Table& table) {
    return insert_cols<Table>(details::empty());
  }

} }

#endif
