//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_EXPRESSION_HPP
#define BOOST_RDB_EXPRESSION_HPP

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
  struct like {
    like(const Expr& expr, const std::string& pattern) : expr_(expr), pattern_(pattern) { }
    const Expr& expr_;
    std::string pattern_;
    typedef boolean sql_type;
    enum { precedence = precedence_level::compare };
    void str(std::ostream& os) const {
      expr_.str(os);
      os << " like ";
      quote_text(os, pattern_);
    }
  };

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
    
    expression< sql::like<Expr> > like(const std::string& pattern) const {
      BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type::kind, char_type>));
      return sql::like<Expr>(*this, pattern);
    }
    
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
  #include "boost/rdb/sql/detail/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR -
  #define BOOST_RDB_OPERATOR_STRING " - "
  #define BOOST_RDB_OPERATOR_CLASS minus
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::add
  #include "boost/rdb/sql/detail/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR *
  #define BOOST_RDB_OPERATOR_STRING " * "
  #define BOOST_RDB_OPERATOR_CLASS times
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::multiply
  #include "boost/rdb/sql/detail/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR /
  #define BOOST_RDB_OPERATOR_STRING " / "
  #define BOOST_RDB_OPERATOR_CLASS divide
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::multiply
  #include "boost/rdb/sql/detail/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR ==
  #define BOOST_RDB_OPERATOR_STRING " = "
  #define BOOST_RDB_OPERATOR_CLASS eq
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/sql/detail/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR !=
  #define BOOST_RDB_OPERATOR_STRING " <> "
  #define BOOST_RDB_OPERATOR_CLASS ne
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/sql/detail/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR <
  #define BOOST_RDB_OPERATOR_STRING " < "
  #define BOOST_RDB_OPERATOR_CLASS lt
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/sql/detail/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR <=
  #define BOOST_RDB_OPERATOR_STRING " <= "
  #define BOOST_RDB_OPERATOR_CLASS le
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/sql/detail/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR >
  #define BOOST_RDB_OPERATOR_STRING " > "
  #define BOOST_RDB_OPERATOR_CLASS gt
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/sql/detail/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR >=
  #define BOOST_RDB_OPERATOR_STRING " >= "
  #define BOOST_RDB_OPERATOR_CLASS ge
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/sql/detail/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR &&
  #define BOOST_RDB_OPERATOR_STRING " and "
  #define BOOST_RDB_OPERATOR_CLASS and_
  #include "boost/rdb/sql/detail/boolean_operator.hpp"

  #define BOOST_RDB_OPERATOR ||
  #define BOOST_RDB_OPERATOR_STRING " or "
  #define BOOST_RDB_OPERATOR_CLASS or_
  #include "boost/rdb/sql/detail/boolean_operator.hpp"

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
  
  const expression<null_type> null = expression<null_type>();

  namespace detail {
    template<class Expr>
    struct test_null : binary_operation<Expr, null_type, precedence_level::compare> {
      
      test_null(const Expr& expr, const char* op) : expr_(expr), op_(op) { }
      
      const char* op_;
      
      typedef boolean sql_type;
      
      void str(std::ostream& os) const {
        this->write(os, expr_, op_, null);
      }
      
      Expr expr_;
    };
  }

  template<class Expr>  
  expression< detail::test_null<Expr> >
  operator ==(const expression<Expr>& expr, const expression<null_type>&) {
    return expression< detail::test_null<Expr> >(expr, " is ");
  }

  template<class Expr>  
  expression< detail::test_null<Expr> >
  operator !=(const expression<Expr>& expr, const expression<null_type>&) {
    return expression< detail::test_null<Expr> >(expr, " is not ");
  }

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
        typename fusion::result_of::make_list<
          boost::reference_wrapper<const Expr1>,
          boost::reference_wrapper<const Expr2>
        >::type
      > type;
    };

    template<class ExprList, class Expr>
    struct extend_expression_list {
      typedef expression_list<
        typename fusion::result_of::push_back<
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
    return type(fusion::make_list(boost::cref(expr1), boost::cref(expr2)));
  }

  template<class ExprList, class Expr>
  typename result_of::extend_expression_list<ExprList, Expr>::type
  extend_expression_list(const expression_list<ExprList>& exprs, const expression<Expr>& expr) {
    typedef typename result_of::extend_expression_list<ExprList, Expr>::type result_type;
    return fusion::push_back(exprs.unwrap(), boost::cref(expr.unwrap()));
  }

  template<class Select>
  struct op_exists {
    
    op_exists(const Select& select) : select_(select) { }

    typedef boolean sql_type;

    enum { precedence = precedence_level::logical_not };
    
    void str(std::ostream& os) const {
      os << "exists (";
      select_.str(os);
      os << ")";
    }
    
    Select select_;
  };

  template<class Select>
  BOOST_CONCEPT_REQUIRES(
    ((SelectStatement<Select>)),
    (expression< op_exists<Select> >))
  exists(const Select& select) {
    return expression< op_exists<Select> >(select);
  }

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


} } }

#endif
