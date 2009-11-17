//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_COMMON_HPP
#define BOOST_RDB_COMMON_HPP

#include <boost/intrusive_ptr.hpp>
#include <boost/fusion/include/vector.hpp>

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
    
    struct float_ {
      BOOST_STATIC_CONSTANT(int, id = 2);
      BOOST_STATIC_CONSTANT(int, length = 1);
    };
    
    struct real {
      BOOST_STATIC_CONSTANT(int, id = 3);
      BOOST_STATIC_CONSTANT(int, length = 1);
    };
    
    struct boolean {
      BOOST_STATIC_CONSTANT(int, id = 4);
      BOOST_STATIC_CONSTANT(int, length = 1);
    };
    
    template<size_t N> struct varchar {
      BOOST_STATIC_CONSTANT(int, id = 5);
      BOOST_STATIC_CONSTANT(size_t, length = N);
    };
    
    struct dynamic_expressions;

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
  
  namespace detail {

    class ref_counted {

    public:
      ref_counted() : ref_count_(0) { }
      virtual ~ref_counted() { }

    private:
      int ref_count_;

      friend void intrusive_ptr_add_ref(ref_counted* p) {
        ++p->ref_count_;
      }

      friend void intrusive_ptr_release(ref_counted* p) {
        if (--p->ref_count_ == 0)
          delete p;
      }
    };
  }

  struct abstract_dynamic_value : detail::ref_counted {
    abstract_dynamic_value(int type, int length) : type_(type), length_(length) { }
    virtual ~abstract_dynamic_value() { }
    int type() const { return type_; }
    int length() const { return length_; }
    int type_;
    int length_;
  };
  
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

    struct root : detail::ref_counted {
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

    intrusive_ptr<root> impl_;

    void str(std::ostream& os) const {
      impl_->str(os);
    }
  };
  
  struct dynamic_expressions : std::vector<dynamic_expression> {
  
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;

    placeholder_vector placeholders() const {
      int size = 0;
      std::vector<dynamic_expression>::const_iterator in = begin();

      while (in != end()) {
        size += in++->placeholders().size();
      }

      std::vector<dynamic_placeholder> result(size);
      std::vector<dynamic_placeholder>::iterator out = result.begin();
      in = begin();

      while (in != end()) {
        out = std::copy(in->placeholders().begin(), in->placeholders().end(), out);
        ++in;
      }

      return result;
    }

    typedef type::dynamic_expressions sql_type;
    
    void str(std::ostream& os) const;
  };

  struct dynamic_column : dynamic_expression {
    dynamic_column(root* impl) : dynamic_expression(impl) { }
  };

} }

#endif
