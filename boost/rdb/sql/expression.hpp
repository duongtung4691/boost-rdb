//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_EXPRESSION_HPP
#define BOOST_RDB_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

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
      // TODO improve compile error when T is not compatible
      static const type make(const T& value) {
        return Expr::sql_type::make_literal(value);
      }
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
  struct like;

  template<class Expr, class Subquery>
  struct in_subquery {
    in_subquery(const Expr& expr, const Subquery& subquery) : expr_(expr), subquery_(subquery) { }
    const Expr& expr_;
    const Subquery& subquery_;
    typedef boolean sql_type;
    enum { precedence = precedence_level::highest };
    void str(std::ostream& os) const {
      if (Expr::precedence < precedence) {
        os << "(";
        expr_.str(os);
        os << ")";
      } else {
        expr_.str(os);
      }
      os << " in (";
      subquery_.str(os);
      os << ")";
    }
  };

  template<class Expr, class ExprList>
  struct in_values {
    in_values(const Expr& expr, const ExprList& alt) : expr_(expr), alt_(alt) { }
    Expr expr_;
    ExprList alt_;
    typedef boolean sql_type;
    enum { precedence = precedence_level::highest };
    void str(std::ostream& os) const {
      if (Expr::precedence < precedence) {
        os << "(";
        expr_.str(os);
        os << ")";
      } else {
        expr_.str(os);
      }
      os << " in (";
      fusion::for_each(alt_, comma_output(os));
      os << ")";
    }
  };

  template<class Expr>
  struct expression : Expr {
    typedef expression this_type;
    expression() { }
    template<typename T> expression(const T& arg) : Expr(arg) { }
    template<typename T1, typename T2> expression(const T1& arg1, const T2& arg2) : Expr(arg1, arg2) { }
    const Expr& unwrap() const { return *this; }
    
    template<typename T>
    typename result_of::make_expression<Expr, T>::type
    static make_expression(const T& any) {
      return result_of::make_expression<Expr, T>::make(any);
    }
    
    expression< sql::like<Expr> > like(const std::string& pattern) const;
    
    template<class Tag, class T>
    struct dispatch_in {
      typedef typename result_of::make_expression<Expr, T>::type value_type;
      typedef typename fusion::result_of::make_vector<value_type>::type value_list_type;
      typedef expression< sql::in_values<Expr, value_list_type> > return_type;
      static return_type make(const Expr& expr, const T& value) {
        return return_type(expr, fusion::make_vector(make_expression(value)));
      }
    };

    template<class Subquery>
    struct dispatch_in<select_statement_tag, Subquery> {
      typedef expression< sql::in_subquery<Expr, Subquery> > return_type;
      static return_type make(const Expr& expr, const Subquery& subquery) { return return_type(expr, subquery); }
    };

    template<class T>
    typename dispatch_in<typename tag_of<T>::type, T>::return_type
    in(const T& any) const {
      return dispatch_in<typename tag_of<T>::type, T>::make(*this, any);
    }
    
    #define BOOST_PP_ITERATION_LIMITS (2, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/in_values.hpp>
    #include BOOST_PP_ITERATE()

    using Expr::operator =; // for set col = value
  };

  namespace result_of {
    template<class Expr>
    struct as_expression< const expression<Expr> > {
      typedef Expr type;
    };

    template<class Expr>
    struct as_expression< expression<Expr> > {
      typedef Expr type;
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
  struct BooleanExpression : Expression<Expr> {
    BOOST_CONCEPT_USAGE(BooleanExpression) {
      BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type, boolean>));
    }
  };
  
  const expression<null_type> null = expression<null_type>();

} } }

#include <boost/rdb/sql/operators.hpp>

#endif
