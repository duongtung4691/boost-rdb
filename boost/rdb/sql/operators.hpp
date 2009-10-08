//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_OPERATORS_HPP
#define BOOST_RDB_OPERATORS_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/expression.hpp>

namespace boost { namespace rdb { namespace sql {

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

  template<class Expr>
  inline expression< sql::like<Expr> > expression<Expr>::like(const std::string& pattern) const {
    BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type::kind, char_type>));
    return sql::like<Expr>(*this, pattern);
  }

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

  template<class Expr>
  BOOST_CONCEPT_REQUIRES(
    ((BooleanExpression<Expr>)),
    (expression< not_<Expr> >))
  operator !(const expression<Expr>& expr) {
    return expression< not_<Expr> >(expr);
  }

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

} } }

#endif
