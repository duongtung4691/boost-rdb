//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

namespace boost { namespace rdb {

  struct make_row {

    template<typename T>
    struct extract_type;

    template<typename T>
    struct extract_type<const expression<T>&> {
      typedef T type;
    };

    template<typename T>
    struct extract_type< expression<T> > {
      typedef T type;
    };

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
    typedef typename boost::fusion::result_of::as_vector<
      typename boost::fusion::result_of::transform<SelectList, make_row>::type
    >::type type;
  };

  
  /*
  struct make_result_vector {
    template<class Vector, class Expr>
    typename boost::fusion::result_of::push_back<
      Vector,
      typename Expr::result_type
    >::type
    operator ()(const Expr&, const Vector& v) {
      typedef typename Expr::result_type result_type;
      return boost::fusion::push_back(result_type());
    }

  };

  template<class SelectList>
  typename boost::fusion::result_of::accumulate<
    SelectList, 
    boost::fusion::vector<>, 
    make_result_vector
  >::type
  result_vector(const SelectList&) {
    boost::fusion::accumulate(SelectList, boost::fusion::vector<>(), make_result_vector());
  }
*/

  template<class SelectList, class FromList, class WhereList>
  struct select_type;

  template<class SelectList>
  struct select_type<SelectList, void, void>;

  template<class SelectList>
  struct select_type<SelectList, void, void> {
    select_type(const SelectList& exprs) : exprs(exprs) { }
    SelectList exprs;
    typedef select_type<SelectList, void, void> this_type;

    void str(std::ostream& os) const {
      os << "select ";
      boost::fusion::for_each(exprs, comma_output(os));
    }

    template<class T>
    struct with {
      typedef typename select_type<
        typename boost::fusion::result_of::push_back< const SelectList, literal<T> >::type,
        void, void
      > type;
    };

    template<class Expr>
    struct with< expression<Expr> > {
      typedef typename select_type<
        typename boost::fusion::result_of::push_back< const SelectList, Expr>::type,
        void, void
      > type;
    };

    template<class Expr> struct with< expression<Expr>& > : with< expression<Expr> > { };
    template<class Expr> struct with< const expression<Expr> > : with< expression<Expr> > { };
    template<class Expr> struct with< const expression<Expr>& > : with< expression<Expr> > { };

    template<typename T>
    typename with<T>::type
    operator ()(const T& expr) const {
      return typename with<T>::type(boost::fusion::push_back(exprs, as_expression(expr)));
    }
        
#define BOOST_RDB_PP_SELECT_VALUES(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, typename T)> \
    typename this_type BOOST_PP_REPEAT(n, BOOST_RDB_PP_WITH, T) \
    operator ()(BOOST_PP_ENUM_BINARY_PARAMS(n, const T, expr)) const { \
      return (*this)BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, expr); \
    }

BOOST_PP_REPEAT_FROM_TO(2, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_SELECT_VALUES, ~)

    template<class ExprList>
    select_type<
      typename boost::fusion::result_of::join<const SelectList, const ExprList>::type,
      void, void
    >
    operator ()(const expression_list<ExprList>& more) const {
      return select_type<
        typename boost::fusion::result_of::join<const SelectList, const ExprList>::type,
        void, void
      >(boost::fusion::join(exprs, more.unwrap()));
    }

    template<class Table>
    struct with_table {
      typedef select_type<
        SelectList,
        typename result_of::make_list< boost::reference_wrapper<const Table> >::type,
        void
      > type;
    };

    template<class Table>
    typename with_table<Table>::type
    from(const Table& table) const {
      return typename with_table<Table>::type(exprs, make_list(boost::cref(table)));
    }

    template<class Table0, class Table1>
    typename with_table<Table0>::type::with<Table1>::type
    from(const Table0& table0, const Table1& table1) const {
      return from(table0)(table1);
    }
  };

  template<class SelectList, class FromList>
  struct select_type<SelectList, FromList, void> : select_type<SelectList, void, void> {
    typedef select_type<SelectList, void, void> just_select;
    select_type(const SelectList& exprs, const FromList& tables) : just_select(exprs), tables(tables) { }

    FromList tables;
    typedef typename select_row<SelectList>::type row_type;

    template<class Table>
    struct with {
      typedef select_type<
        SelectList,
        typename boost::fusion::result_of::push_back<
          const FromList, boost::reference_wrapper<const Table>
        >::type,
        void
      > type;
    };

    template<class Table>
    typename with<Table>::type
    operator ()(const Table& table) const {
      return typename with<Table>::type(exprs, boost::fusion::push_back(tables, boost::cref(table)));
    }
    
    template<class Pred>
    BOOST_CONCEPT_REQUIRES(
      ((Expression<Pred>)),
      (select_type<
        SelectList,
        FromList,
        Pred>)
      )
    where(const Pred& pred) const {
      return select_type<
        SelectList,
        FromList,
        Pred
      >(exprs, tables, pred);
    }

    void str(std::ostream& os) const {
      just_select::str(os);
      os << " from ";
      boost::fusion::for_each(tables, comma_output(os));
    }
  };

  template<class SelectList, class FromList, class Predicate>
  struct select_type : select_type<SelectList, FromList, void> {
    typedef select_type<SelectList, FromList, void> select_from;
    select_type(const SelectList& exprs, const FromList& tables, const Predicate& pred)
      : select_from(exprs, tables), pred(pred) { }

    const Predicate& pred;

    void str(std::ostream& os) const {
      select_from::str(os);
      os << " where ";
      pred.str(os);
    }
  };
  
  extern select_type<details::empty, void, void> select;

  namespace comma {

    template<class Expr1, class Expr2>
    typename result_of::make_expression_list<Expr1, Expr2>::type
      operator ,(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
      return make_expression_list(expr1, expr2);
    }

    template<class ExprList, class Expr>
    typename result_of::extend_expression_list<ExprList, Expr>::type
    operator ,(const expression_list<ExprList>& exprs, const expression<Expr>& expr) {
        return extend_expression_list(exprs, expr);
    }
  }

} }

#endif
