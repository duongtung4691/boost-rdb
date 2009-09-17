//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

namespace boost { namespace rdb {

  template<class ExprList>
  struct expression_list : ExprList {
    typedef ExprList expression_list_type;
    expression_list(const ExprList& exprs) : ExprList(exprs) { }
    const ExprList& unwrap() const { return *this; }
  };

  namespace result_of {
    template<class Expr1, class Expr2>
    struct make_expression_list {
      typedef typename expression_list<
        typename boost::fusion::result_of::make_vector<
          boost::reference_wrapper<const Expr1>,
          boost::reference_wrapper<const Expr2>
        >::type
      > type;
    };

    template<class ExprList, class Expr>
    struct extend_expression_list {
      typedef typename expression_list<
        typename boost::fusion::result_of::push_back<
          const ExprList,
          boost::reference_wrapper<const Expr>
        >::type
      > type;
    };
  }

  template<class Expr1, class Expr2>
  typename result_of::make_expression_list<Expr1, Expr2>::type
  make_expression_list(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
    typedef typename result_of::make_expression_list<Expr1, Expr2>::type type;
    return type(boost::fusion::make_vector(boost::cref(expr1), boost::cref(expr2)));
  }

  template<class ExprList, class Expr>
  typename result_of::extend_expression_list<ExprList, Expr>::type
  extend_expression_list(const expression_list<ExprList>& exprs, const expression<Expr>& expr) {
    typedef typename result_of::extend_expression_list<ExprList, Expr>::type result_type;
    return boost::fusion::push_back(exprs.unwrap(), boost::cref(expr.unwrap()));
  }

  template<class SelectList, class FromList, class WhereList>
  struct select_type;

  template<class SelectList>
  struct select_type<SelectList, void, void>;

  template<class SelectList>
  struct select_type<SelectList, void, void> {
    select_type(const SelectList& exprs) : exprs(exprs) { }
    SelectList exprs;

    void str(std::ostream& os) const {
      os << "select ";
      boost::fusion::for_each(exprs, comma_output(os));
    }

    template<typename Expr>
    BOOST_CONCEPT_REQUIRES(
      ((Expression<Expr>)),
      (select_type<
        typename boost::fusion::result_of::push_back< const SelectList, boost::reference_wrapper<const Expr> >::type,
        void, void
      >)) operator ()(const expression<Expr>& expr) const {
      return select_type<
        typename boost::fusion::result_of::push_back< const SelectList, boost::reference_wrapper<const Expr> >::type,
        void, void
      >(boost::fusion::push_back(exprs, boost::cref(expr.unwrap())));
    }
    
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

    template<typename Table>
    select_type<
      SelectList,
      typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Table> >::type,
      void
    >
    from(const Table& table) const {
      return select_type<
        SelectList,
        typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Table> >::type,
        void
      >(exprs, boost::fusion::make_vector(boost::cref(table)));
    }
  };

  template<class SelectList, class FromList>
  struct select_type<SelectList, FromList, void> : select_type<SelectList, void, void> {
    typedef select_type<SelectList, void, void> just_select;
    select_type(const SelectList& exprs, const FromList& tables) : just_select(exprs), tables(tables) { }

    FromList tables;

    template<class Table>
    select_type<
      SelectList,
      typename boost::fusion::result_of::push_back< const FromList, boost::reference_wrapper<const Table> >::type,
      void
    >
    operator ()(const Table& table) const {
      return select_type<
        SelectList,
        typename boost::fusion::result_of::push_back< const FromList, boost::reference_wrapper<const Table> >::type,
        void
      >(exprs, boost::fusion::push_back(tables, boost::cref(table)));
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
  
  extern select_type<details::empty_vector, void, void> select;

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