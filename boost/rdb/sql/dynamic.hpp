//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP
#define BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/intrusive_ptr.hpp>

namespace boost { namespace rdb { namespace sql {

  struct dynamic_value {
    dynamic_value(int type, int length) : type_(type), length_(length), ref_count_(0) { }
    virtual ~dynamic_value() { }
    int type() const { return type_; }
    int length() const { return length_; }
    int type_;
    int length_;
    int ref_count_;
  };

  inline void intrusive_ptr_add_ref(dynamic_value* p) {
    ++p->ref_count_;
  }

  inline void intrusive_ptr_release(dynamic_value* p) {
    if (--p->ref_count_ == 0)
      delete p;
  }

  typedef std::vector< intrusive_ptr<dynamic_value> > dynamic_values;
  
  struct dynamic_placeholder { // make it a specialization of placeholder<> ? but what for ?
    dynamic_placeholder(int type, int length) : type_(type), length_(length) { }
    int type() const { return type_; }
    int length() const { return length_; }
    int type_;
    int length_;
  };

  typedef std::vector<dynamic_placeholder> dynamic_placeholders;

  template<class SqlType>
  struct dynamic_expression_wrapper {
    typedef SqlType sql_type;
    
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;
    
    placeholder_vector placeholders() const {
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


} } }


#endif
