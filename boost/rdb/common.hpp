//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_COMMON_HPP
#define BOOST_RDB_COMMON_HPP

#include <boost/intrusive_ptr.hpp>

namespace boost { namespace rdb {

  struct statement_tag { };
  struct insert_statement_tag : statement_tag { };
  struct select_statement_tag : statement_tag { };
  struct update_statement_tag : statement_tag { };

  namespace type {
    // types as they exist independantly of any implementation
    // name `type` is already used in namespace `boost` but
    // namespace `rdb` is not meant to be imported as a whole
    // so it should do no harm
    struct integer {
      BOOST_STATIC_CONSTANT(int, id = 1);
      BOOST_STATIC_CONSTANT(int, length = 1);
    };
    
    struct boolean {
      BOOST_STATIC_CONSTANT(int, id = 2);
      BOOST_STATIC_CONSTANT(int, length = 1);
    };
    
    template<int N> struct varchar {
      BOOST_STATIC_CONSTANT(int, id = 3);
      BOOST_STATIC_CONSTANT(int, length = N);
    };

    // metafunction that returns a type for holding a value of a given (rdb) type
    // for a specific database
    template<class T, class Tag>
    struct cli_type;

    template<class Type>
    struct placeholder {
      // Empty for statically typed placeholders, but dynamic placeholders
      // contain type and length information
      typedef Type rdb_type;
    };
  }

  struct abstract_dynamic_value {
    abstract_dynamic_value(int type, int length) : type_(type), length_(length), ref_count_(0) { }
    virtual ~abstract_dynamic_value() { }
    int type() const { return type_; }
    int length() const { return length_; }
    int type_;
    int length_;
    int ref_count_;
  };

  inline void intrusive_ptr_add_ref(abstract_dynamic_value* p) {
    ++p->ref_count_;
  }

  inline void intrusive_ptr_release(abstract_dynamic_value* p) {
    if (--p->ref_count_ == 0)
      delete p;
  }
  
  struct dynamic_placeholder { // make it a specialization of placeholder<> ? but what for ?
    dynamic_placeholder() : type_(0), length_(0) { }
    dynamic_placeholder(int type, int length) : type_(type), length_(length) { }
    int type() const { return type_; }
    int length() const { return length_; }
    int type_;
    int length_;
  };

  typedef std::vector<dynamic_placeholder> dynamic_placeholders;

  struct dynamic_expression {

    struct root {
      root(int type, int length) : type_(type), length_(length) { }
      virtual void str(std::ostream& os) const = 0;
      dynamic_placeholders placeholders_;
      int type_;
      int length_;
    };
  
    dynamic_expression(root* impl) : impl_(impl) { }
    
    int type() const { return impl_->type_; }
    int length() const { return impl_->length_; }
    const dynamic_placeholders& placeholders() const { return impl_->placeholders_; }

    shared_ptr<root> impl_;

    void str(std::ostream& os) const {
      impl_->str(os);
    }
  };

  struct dynamic_column : dynamic_expression {
    dynamic_column(root* impl) : dynamic_expression(impl) { }
  };

} }

#endif
