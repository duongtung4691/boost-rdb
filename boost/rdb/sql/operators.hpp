//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_OPERATORS_HPP
#define BOOST_RDB_OPERATORS_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/expression.hpp>

namespace boost { namespace rdb { namespace sql {

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
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::logical_and
  #include "boost/rdb/sql/detail/boolean_operator.hpp"

  #define BOOST_RDB_OPERATOR ||
  #define BOOST_RDB_OPERATOR_STRING " or "
  #define BOOST_RDB_OPERATOR_CLASS or_
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::logical_or
  #include "boost/rdb/sql/detail/boolean_operator.hpp"

  template<class Expr>
  struct not_ {

    not_(const Expr& expr) : expr_(expr) { }

    typedef core::boolean sql_type;
    
    typedef typename Expr::placeholder_vector placeholder_vector;

    placeholder_vector placeholders() const {
      return expr_.placeholders();
    }

    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::logical_not);
    
    void str(std::ostream& os) const {
      this->write(os, boost::mpl::bool_<precedence_of<Expr>::value < precedence>());
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

  namespace detail {
    template<class Expr>
    struct test_null : binary_operation<Expr, null_expr, precedence_level::compare> {
      
      test_null(const Expr& expr, const char* op) :
        binary_operation<Expr, null_expr, precedence_level::compare>(expr, null),
        op_(op) { }
      
      const char* op_;
      
      typedef core::boolean sql_type;
      
      void str(std::ostream& os) const {
        this->write(os, this->expr1_, this->op_, this->expr2_);
      }
    };
  }

  template<class Expr>  
  expression< detail::test_null<Expr> >
  operator ==(const expression<Expr>& expr, const expression<null_expr>&) {
    return expression< detail::test_null<Expr> >(expr, " is ");
  }

  template<class Expr>  
  expression< detail::test_null<Expr> >
  operator !=(const expression<Expr>& expr, const expression<null_expr>&) {
    return expression< detail::test_null<Expr> >(expr, " is not ");
  }

  template<class Select>
  struct op_exists {
    
    op_exists(const Select& select) : select_(select) { }

    typedef core::boolean sql_type;

    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::logical_not);

    typedef typename Select::placeholder_vector placeholder_vector;

    placeholder_vector placeholders() const {
      return select_.placeholders();
    }
    
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

} } }

#endif
