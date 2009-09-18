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

  template<class Table, class ColList>
  struct insert_cols_type;
  
  template<class Table, class ColList, class ValueList>
  struct insert_type : insert_cols_type<Table, ColList> {

    typedef insert_cols_type<Table, ColList> just_cols;
    typedef ColList col_list_type;
    typedef ValueList value_list_type;
    
    insert_type(const ColList& cols, const ValueList& values) : insert_cols_type(cols), values_(values) { }

    ValueList values_;

    void str(std::ostream& os) const {
      just_cols::str(os);
      os << " values (";
      boost::fusion::for_each(values_, comma_output(os));
      os << ")";
    }
  };

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_values_expr_type;

  namespace result_of {
    template<class Table, class ColList, class ColIter, class ValueList, class Expr>
    struct insert_values_expr {
      typedef typename boost::fusion::result_of::next<ColIter>::type next_col_iter;
      
      typedef typename boost::fusion::result_of::push_back<
        const ValueList, Expr>::type values_type;
        
      typedef typename boost::mpl::if_<
        boost::is_same<
          next_col_iter,
          typename boost::fusion::result_of::end<ColList>::type
        >,
        insert_statement< insert_type<Table, ColList, values_type> >,
        insert_values_expr_type<Table, ColList, values_type, next_col_iter>
      >::type type;
    };

    template<class Table, class ColList, class ColIter, class ValueList, typename T>
    struct insert_values_literal : insert_values_expr<Table, ColList, ColIter, ValueList,
      typename boost::remove_reference<
        typename boost::fusion::result_of::deref<ColIter>::type
      >::type::sql_type::literal_type
    > {
    };
  }

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_values_expr_type : insert_cols_type<Table, ColList> {

    typedef insert_cols_type<Table, ColList> just_cols;

    insert_values_expr_type(const ColList& cols, const ExprList& values) : just_cols(cols), values_(values) { }

    ExprList values_;

    void str(std::ostream& os) const {
      just_cols::str(os);
      os << " values (";
      boost::fusion::for_each(values_, comma_output(os));
      os << ")";
    }

    template<typename T>
    typename result_of::insert_values_literal<Table, ColList, ColIter, ExprList, T>::type
    operator ()(const T& expr) const {
      typedef typename result_of::insert_values_literal<Table, ColList, ColIter, ExprList, T>::type result_type;
      typedef typename boost::remove_reference<boost::fusion::result_of::deref<ColIter>::type>::type col_type;
      return result_type(cols_, boost::fusion::push_back(values_, col_type::sql_type::make_literal(expr)));
    }

    template<class Expr>
    BOOST_CONCEPT_REQUIRES(
      ((Expression<Expr>)),
      (typename result_of::insert_values_expr<Table, ColList, ColIter, ExprList, Expr>::type))
    operator ()(const expression<Expr>& expr) const {
      typedef typename result_of::insert_values_expr<Table, ColList, ColIter, ExprList, Expr>::type result_type;
      return result_type(cols_, boost::fusion::push_back(values_, expr.unwrap()));
    }
  };

  template<class Table, class ColList>
  struct insert_cols_type {

    insert_cols_type(const insert_cols_type&);
    insert_cols_type(const ColList& cols) : cols_(cols) { }

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " (";
      boost::fusion::for_each(cols_, comma_output(os));
      os << ")";
    }

    template<class Col>
    struct with {
      typedef typename insert_cols_type<
        Table,
        typename boost::fusion::result_of::push_back< const ColList, boost::reference_wrapper<const Col> >::type
      > type;
    };

    template<class Col>
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
      (insert_cols_type<
        Table,
        typename boost::fusion::result_of::push_back< const ColList, boost::reference_wrapper<const Col> >::type
      >))
    operator ()(const expression<Col>& col) const {
      return insert_cols_type<
        Table,
        typename boost::fusion::result_of::push_back< const ColList, boost::reference_wrapper<const Col> >::type
      >(boost::fusion::push_back(cols_, boost::cref(col.unwrap())));
    }

    template<typename T>
    typename result_of::insert_values_literal<Table, ColList, 
      typename boost::fusion::result_of::begin<ColList>::type,
      details::empty, T
    >::type
    values(const T& expr) const {
      typedef typename result_of::insert_values_literal<Table, ColList, 
        typename boost::fusion::result_of::begin<ColList>::type,
        details::empty, T
      >::type result_type;
      typedef typename boost::remove_reference<boost::fusion::result_of::front<ColList>::type>::type col_type;
      return result_type(cols_, boost::fusion::push_back(details::empty(),
        col_type::sql_type::make_literal(expr)));
    }

    template<class Expr>
    BOOST_CONCEPT_REQUIRES(
      ((Expression<Expr>)),
      (typename result_of::insert_values_expr<Table, ColList, 
        typename boost::fusion::result_of::begin<ColList>::type,
        details::empty, Expr>::type))
    values(const expression<Expr>& expr) const {
      typedef typename result_of::insert_values_expr<Table, ColList, 
        typename boost::fusion::result_of::begin<ColList>::type,
        details::empty, Expr>::type result_type;
      typedef typename boost::remove_reference<boost::fusion::result_of::front<ColList>::type>::type col_type;
      BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type::kind, typename col_type::sql_type::kind>));
      return result_type(cols_, boost::fusion::push_back(details::empty(), expr.unwrap()));
    }

    ColList cols_;
  };

  template<class Table, class Col>
  BOOST_CONCEPT_REQUIRES(
    ((Column<Col>)),
    (insert_cols_type<Table,
      typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Col> >::type
      >))
  insert_into(const expression<Col>& col) {
    BOOST_MPL_ASSERT((boost::is_same<Table, typename Col::table_type>));
    return insert_cols_type<Table,
      typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Col> >::type
      >(boost::fusion::make_vector(boost::cref(col.unwrap())));
  }

  template<class Table, class Col1, class Col2>
  typename insert_cols_type<Table,
    typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Col1> >::type
  >::with<Col2>::type
  insert_into(const expression<Col1>& col1, const expression<Col2>& col2) {
    return insert_into<Table>(col1)(col2);
  }

} }

#endif