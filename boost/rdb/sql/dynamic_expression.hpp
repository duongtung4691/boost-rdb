//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP
#define BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  struct dynamic_placeholder { // make it a specialization of placeholder<> ? but what for ?
    int type_;
    int length_;
  };

  template<class SqlType>
  struct dynamic_expression_wrapper {
    typedef SqlType sql_type;
    typedef fusion::vector< std::vector<dynamic_placeholder> > placeholders;
    enum { precedence = precedence_level::lowest };
    struct root {
      virtual void str(std::ostream& os) const = 0;
    };
    shared_ptr<root> impl_;

    dynamic_expression_wrapper(root* p) : impl_(p) { }

    void str(std::ostream& os) const {
      impl_->str(os);
    }
  };

  template<class Expr>
  struct dynamic_expression_impl : dynamic_expression_wrapper<typename Expr::sql_type>::root {
    dynamic_expression_impl(const Expr& expr) : expr_(expr) { }
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


} } }


#endif
