//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_INSERT_HPP
#define BOOST_RDB_INSERT_HPP

namespace boost { namespace rdb {

  struct insert_statement_tag : statement_tag { };

  template<class Table, class ColList, class ExprList>
  struct insert_assign;

  template<class Table, class ColList>
  struct insert_cols;

  template<class Table, class ColList, class ExprList, class ColIter>
  struct insert_values;

  template<class Table, class ColList, class Select>
  struct insert_select;

  template<class Table>
  struct insert_statement {

    template<class Col>
    typename insert_cols<Table, details::empty>::template with<Col>::type
    operator ()(const expression<Col>& col) const {
      return insert_cols<Table, details::empty>(details::empty())(col);
    }

    template<class Col, class T>
    typename insert_assign<Table, details::empty, details::empty>::template with<Col, T>::type
    set(const expression<Col>& col, const T& expr) const {
      return insert_assign<Table, details::empty, details::empty>().set(col, expr);
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

  template<class Table, class ColList>
  struct insert_cols {

    typedef ColList col_list_type;

    insert_cols(const ColList& cols) : cols_(cols) { }

    template<class Col>
    struct with {
      typedef insert_cols<
        Table,
        typename boost::fusion::result_of::push_back<const ColList, Col>::type
      > type;
    };

    template<class SelectList, class FromList, class WhereList>
    insert_select< Table, ColList, select_statement<SelectList, FromList, WhereList> >
    operator ()(const select_statement<SelectList, FromList, WhereList>& select) const {
      return insert_select< Table, ColList, select_statement<SelectList, FromList, WhereList> >(cols_, select);
    }

    template<class Col>
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
    (typename with<Col>::type))
    operator ()(const expression<Col>& col) const {
      BOOST_MPL_ASSERT((boost::is_same<Table, typename Col::table_type>));
      return insert_cols<
        Table,
        typename boost::fusion::result_of::push_back<const ColList, Col>::type
      >(boost::fusion::push_back(cols_, col.unwrap()));
    }

  BOOST_PP_REPEAT_FROM_TO(2, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_COLS, ~)

    ColList cols_;

    typedef insert_values<Table, ColList, details::empty,
      typename boost::fusion::result_of::begin<ColList>::type
      > insert_values0;
    
#define BOOST_RDB_PP_INSERT_VALUES(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, typename T)> \
    typename insert_values0 BOOST_PP_REPEAT(n, BOOST_RDB_PP_WITH, T) \
    values(BOOST_PP_ENUM_BINARY_PARAMS(n, T, arg)) const { \
      return insert_values0(cols_, details::empty())BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, arg); \
    }

BOOST_PP_REPEAT_FROM_TO(1, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_INSERT_VALUES, ~)

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " ";
      typedef boost::fusion::vector<const ColList&, const ExprList&> assignment;
      boost::fusion::for_each(boost::fusion::zip_view<assignment>(assignment(just_cols::cols_, values_)), assign_output(os));
    }
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
        typename boost::fusion::result_of::deref<ColIter>::type
      >::type col_type;
    
      typedef typename result_of::make_expression<expression<col_type>, T>::type value_type;
      
      typedef typename boost::fusion::result_of::push_back<
        const ExprList, value_type>::type values_list_type;

      typedef typename boost::fusion::result_of::next<ColIter>::type next_col_iter;
        
      typedef insert_values<Table, ColList, values_list_type, next_col_iter> type;
    };

    template<typename T>
    typename with<T>::type
    operator ()(const T& expr) const {
      typedef typename with<T>::type result_type;
      typedef typename boost::remove_reference<typename boost::fusion::result_of::deref<ColIter>::type>::type col_type;
      return result_type(cols_, boost::fusion::push_back(values_, expression<col_type>::make_expression(expr)));
    }

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " (";
      boost::fusion::for_each(cols_, comma_output(os));
      os << ") values (";
      boost::fusion::for_each(values_, comma_output(os));
      os << ")";
    }
  };

  template<class Table, class ColList, class ExprList>
  struct insert_assign {

    insert_assign(const ColList& cols, const ExprList& values) : cols_(cols), values_(values) { }

    ColList cols_;
    ExprList values_;

    template<class Col, class Expr>
    struct with {
      typedef insert_assign<
        Table,
        typename boost::fusion::result_of::push_back<
          const ColList, 
          typename result_of::unwrap<Col>::type
        >::type,
        typename boost::fusion::result_of::push_back<
          const ExprList,
          typename result_of::make_expression<expression<Col>, Expr>::type
        >::type
      > insert_assign;
    };

    template<class Col, class T>
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
      (typename with<Col, T>::type))
      set(const expression<Col>& col, const T& expr) const {
      return typename with<Col, T>::type(
        boost::fusion::push_back(cols_, col.unwrap()),
        boost::fusion::push_back(values_, expression<Col>::make_expression(expr)));
    }
  };
  
  template<class Table, class ColList, class Select>
  struct insert_select {

    typedef insert_statement_tag tag;
    typedef void result;

    insert_select(const ColList& cols, const Select& select) : cols_(cols), select_(select) { }

    Select select_;
    ColList cols_;

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " (";
      boost::fusion::for_each(cols_, comma_output(os));
      os << ") ";
      select_.str(os);
    }
  };


  template<class Table>
  insert_statement<Table>
  insert_into(const Table& table) {
    return insert_statement<Table>();
  }

} }

#endif
