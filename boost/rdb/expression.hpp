//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_EXPRESSION_HPP
#define BOOST_RDB_EXPRESSION_HPP

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
      typedef expression_list<
        typename boost::fusion::result_of::make_list<
          boost::reference_wrapper<const Expr1>,
          boost::reference_wrapper<const Expr2>
        >::type
      > type;
    };

    template<class ExprList, class Expr>
    struct extend_expression_list {
      typedef expression_list<
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
    return type(boost::fusion::make_list(boost::cref(expr1), boost::cref(expr2)));
  }

  template<class ExprList, class Expr>
  typename result_of::extend_expression_list<ExprList, Expr>::type
  extend_expression_list(const expression_list<ExprList>& exprs, const expression<Expr>& expr) {
    typedef typename result_of::extend_expression_list<ExprList, Expr>::type result_type;
    return boost::fusion::push_back(exprs.unwrap(), boost::cref(expr.unwrap()));
  }

  /*
  namespace result_of {
    template<class ExprList>
    struct select {
      typedef select_type<ExprList, void, void> type;
    };
  }
  */

} }

#endif
