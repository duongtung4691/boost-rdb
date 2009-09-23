//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_EXPRESSION_HPP
#define BOOST_RDB_EXPRESSION_HPP

namespace boost { namespace rdb {

  template<class Expr>
  struct Expression
  {
    Expr expr;
    std::ostream& stream;
    typedef typename Expr::sql_type sql_type;
    enum { precedence = Expr::precedence };

    BOOST_CONCEPT_USAGE(Expression) {
      expr.str(stream);
    }
  };

  template<class Expr>
  struct NumericExpression : Expression<Expr>
  {
    typedef typename Expr::sql_type::is_numeric is_numeric;
  };

  template<class Expr>
  struct ComparableExpression : Expression<Expr>
  {
    typedef typename Expr::sql_type::comparable_type comparable_type;
  };

  template<class Expr, typename T>
  struct CompatibleLiteral
  {
    const T& value;
    BOOST_CONCEPT_USAGE(CompatibleLiteral) {
      Expr::sql_type::make_literal(value);
    }
  };

  template<class Expr>
  struct expression;
    
  namespace result_of {
    template<class Expr, typename T>
    struct make_expression_ {
      typedef typename Expr::sql_type::literal_type type;
      static const type make(const T& value) { return Expr::sql_type::make_literal(value); }
    };
  
    template<class Expr, class Expr2>
    struct make_expression_< Expr, expression<Expr2> > {
      typedef Expr2 type;
      static Expr2 make(const Expr2& expr) { return expr; }
    };
  
    template<class Expr, typename T>
    struct make_expression : make_expression_<
      Expr,
      typename boost::remove_const<
        typename boost::remove_reference<T>::type
      >::type
    > {
    };
  }

  template<class Expr>
  struct expression : Expr {
    expression() { }
    template<typename T> expression(const T& arg) : Expr(arg) { }
    template<typename T1, typename T2> expression(const T1& arg1, const T2& arg2) : Expr(arg1, arg2) { }
    const Expr& unwrap() const { return *this; }
    
    template<typename T>
    typename result_of::make_expression<Expr, T>::type
    static make_expression(const T& any) {
      return result_of::make_expression<Expr, T>::make(any);
    }
  };

  template<typename T>
  literal<T> as_expression(const T& value) {
    return literal<T>(value);
  }

  namespace result_of {
    template<class Expr>
    struct as_expression< const expression<Expr> > {
      typedef const Expr& type;
    };

    template<class Expr>
    struct as_expression< expression<Expr> > {
      typedef const Expr& type;
    };

    // remove the expression<> decorator from a Column or an Expression
    template<class Content>
    struct unwrap {
      typedef typename boost::remove_cv<Content>::type type;
    };

  }

  template<class Expr>
  BOOST_CONCEPT_REQUIRES(
    ((Expression<Expr>)),
  (const Expr&))
  as_expression(const expression<Expr>& expr) {
    return expr.unwrap();
  }

  template<class Expr>
  struct BooleanExpression : Expression<Expr>
  {
    BOOST_CONCEPT_USAGE(BooleanExpression) {
      BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type, boolean>));
    }
  };

  template<class Expr1, class Expr2, int Precedence>
  struct binary_operation {

    enum { precedence = Precedence };

    static void write(std::ostream& os, const Expr1& expr1, const char* op, const Expr2& expr2) {
      write(os, expr1, boost::mpl::bool_<static_cast<int>(Expr1::precedence) < precedence>());
      os << op;
      write(os, expr2, boost::mpl::bool_<static_cast<int>(Expr2::precedence) < precedence>());
    }

    template<class Expr>
    static void write(std::ostream& os, const Expr& expr, boost::mpl::true_) {
      os << "(";
      expr.str(os);
      os << ")";
    }

    template<class Expr>
    static void write(std::ostream& os, const Expr& expr, boost::mpl::false_) {
      expr.str(os);
    }
  };

  #define BOOST_RDB_OPERATOR +
  #define BOOST_RDB_OPERATOR_STRING " + "
  #define BOOST_RDB_OPERATOR_CLASS plus
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::add
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR -
  #define BOOST_RDB_OPERATOR_STRING " - "
  #define BOOST_RDB_OPERATOR_CLASS minus
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::add
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR *
  #define BOOST_RDB_OPERATOR_STRING " * "
  #define BOOST_RDB_OPERATOR_CLASS times
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::multiply
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR /
  #define BOOST_RDB_OPERATOR_STRING " / "
  #define BOOST_RDB_OPERATOR_CLASS divide
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::multiply
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR ==
  #define BOOST_RDB_OPERATOR_STRING " = "
  #define BOOST_RDB_OPERATOR_CLASS eq
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR !=
  #define BOOST_RDB_OPERATOR_STRING " <> "
  #define BOOST_RDB_OPERATOR_CLASS ne
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR <
  #define BOOST_RDB_OPERATOR_STRING " < "
  #define BOOST_RDB_OPERATOR_CLASS lt
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR <=
  #define BOOST_RDB_OPERATOR_STRING " <= "
  #define BOOST_RDB_OPERATOR_CLASS le
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR >
  #define BOOST_RDB_OPERATOR_STRING " > "
  #define BOOST_RDB_OPERATOR_CLASS gt
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR >=
  #define BOOST_RDB_OPERATOR_STRING " >= "
  #define BOOST_RDB_OPERATOR_CLASS ge
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR &&
  #define BOOST_RDB_OPERATOR_STRING " and "
  #define BOOST_RDB_OPERATOR_CLASS and_
  #include "boost/rdb/details/boolean_operator.hpp"

  #define BOOST_RDB_OPERATOR ||
  #define BOOST_RDB_OPERATOR_STRING " or "
  #define BOOST_RDB_OPERATOR_CLASS or_
  #include "boost/rdb/details/boolean_operator.hpp"

  template<class Expr>
  struct not_ {

    not_(const Expr& expr) : expr_(expr) { }

    typedef boolean sql_type;

    enum { precedence = precedence_level::logical_not };
    
    void str(std::ostream& os) const {
      this->write(os, boost::mpl::bool_<static_cast<int>(Expr::precedence) < static_cast<int>(precedence)>());
    }

    void write(std::ostream& os, boost::mpl::true_) const {
      os << "not (";
      expr_.str(os);
      os << ")";
    }

    void write(std::ostream& os, boost::mpl::false_) const {
      os << "not ";
      expr_.str(os);
    }
    
    Expr expr_;
  };

  template<class Expr>
  BOOST_CONCEPT_REQUIRES(
    ((BooleanExpression<Expr>)),
    (expression< not_<Expr> >))
  operator !(const expression<Expr>& expr) {
    return expression< not_<Expr> >(expr);
  }

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
      typedef select_statement<ExprList, void, void> type;
    };
  }
  */

} }

#endif
