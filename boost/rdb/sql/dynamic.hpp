//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP
#define BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  template<class SqlType>
  struct dynamic_expression_wrapper : dynamic_expression {
    typedef SqlType sql_type;
    
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;
    
    placeholder_vector placeholders() const {
      return fusion::make_vector(impl_->placeholders_);
    }

    enum { precedence = precedence_level::lowest };

    dynamic_expression_wrapper(root* p) : dynamic_expression(p) { }

    void str(std::ostream& os) const {
      impl_->str(os);
    }
  };

  template<class Expr>
  struct dynamic_expression_impl : dynamic_expression::root {

    dynamic_expression_impl(const Expr& expr) : dynamic_expression::root(Expr::sql_type::id, Expr::sql_type::length), expr_(expr) {
      fusion::for_each(expr.placeholders(), make_placeholder(this->placeholders_));
    }

    struct make_placeholder {
      make_placeholder(std::vector<dynamic_placeholder>& placeholders) : placeholders_(placeholders) { }
      
      mutable std::vector<dynamic_placeholder>& placeholders_;
      
      template<class Placeholder>
      void operator ()(const Placeholder& p) const {
        placeholders_.push_back(dynamic_placeholder(Placeholder::rdb_type::id, Placeholder::rdb_type::length));
      }
    };

    virtual void str(std::ostream& os) const {
      expr_.str(os);
    }

    Expr expr_;
  };

  template<class Expr>
  dynamic_expression_wrapper<typename Expr::sql_type>
  make_dynamic(const expression<Expr>& expr) {
    return dynamic_expression_wrapper<typename Expr::sql_type>(new dynamic_expression_impl<Expr>(expr));
  }

  // alas no templatized typedefs yet
  // template<class Expr> typedef expression< dynamic_expression_wrapper<typename Expr::sql_type> > dynamic_expression<Expr>;
  typedef expression< dynamic_expression_wrapper<type::integer> > dynamic_integer;
  typedef expression< dynamic_expression_wrapper<type::boolean> > dynamic_boolean;

  typedef std::vector<dynamic_expression> dynamic_expressions;

  struct dynamic_placeholder_impl : dynamic_expression::root {

    dynamic_placeholder_impl(int type, int length) : dynamic_expression::root(type, length) {
    }

    virtual void str(std::ostream& os) const {
      os << "?";
    }
  };

  template<class Expr>
  dynamic_expression_wrapper<typename Expr::sql_type>
  make_dynamic(const placeholder_mark<0>& mark, const expression<Expr>&) {
    return dynamic_expression_wrapper<typename Expr::sql_type>(
      new dynamic_placeholder_impl(Expr::sql_type::id, Expr::sql_type::length));
  }

} } }


#endif
