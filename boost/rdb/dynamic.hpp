//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_DYNAMIC_HPP
#define BOOST_RDB_DYNAMIC_HPP

#include <boost/rdb/detail/output.hpp>

#include <boost/intrusive_ptr.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <bitset>
#include <ostream>
#include <algorithm>

#define BOOST_RDB_MAX_SIZE FUSION_MAX_VECTOR_SIZE
#define BOOST_RDB_MAX_ARG_COUNT 10

namespace boost { namespace rdb { namespace dynamic {

  struct dynamic_expressions;
  
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

    typedef dynamic_expressions sql_type;
    
    //void str(std::ostream& os) const {
    //  std::for_each(begin(), end(), detail::comma_output(os));
    //}    
  };

  struct dynamic_column : dynamic_expression {
    dynamic_column(root* impl) : dynamic_expression(impl) { }
  };

} } }

#endif
