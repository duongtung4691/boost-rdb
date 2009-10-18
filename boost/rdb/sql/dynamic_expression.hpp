//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP
#define BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  struct dynamic_placeholder { // make it a specialization of placeholder<> ? but what for ?
    dynamic_placeholder(int type, int length) : type_(type), length_(length) { }
    int type() const { return type_; }
    int length() const { return length_; }
    int type_;
    int length_;
  };

  template<class SqlType>
  struct dynamic_expression_wrapper {
    typedef SqlType sql_type;
    
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;
    
    placeholder_vector placeholders() const {
      std::vector<dynamic_placeholder> t1 = impl_->placeholders_;
      fusion::vector< const std::vector<dynamic_placeholder> > t2 = fusion::make_vector(t1);
      return fusion::make_vector(impl_->placeholders_);
    }
    
    enum { precedence = precedence_level::lowest };

    struct root {
      virtual void str(std::ostream& os) const = 0;
      std::vector<dynamic_placeholder> placeholders_;
    };

    shared_ptr<root> impl_;

    dynamic_expression_wrapper(root* p) : impl_(p) { }

    void str(std::ostream& os) const {
      impl_->str(os);
    }
  };

  template<class Expr>
  struct dynamic_expression_impl : dynamic_expression_wrapper<typename Expr::sql_type>::root {
    typedef typename dynamic_expression_wrapper<typename Expr::sql_type>::root root;

    dynamic_expression_impl(const Expr& expr) : expr_(expr) {
      fusion::for_each(expr.placeholders(), make_placeholder(this->placeholders_));
    }

    struct make_placeholder {
      make_placeholder(std::vector<dynamic_placeholder>& placeholders) : placeholders_(placeholders) { }
      
      mutable std::vector<dynamic_placeholder>& placeholders_;
      
      template<class Placeholder>
      void operator ()(const Placeholder& p) const {
        placeholders_.push_back(dynamic_placeholder(Placeholder::rdb_type::id, 1));
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


} } }


#endif
