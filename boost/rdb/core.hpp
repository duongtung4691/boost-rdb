//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_CORE_HPP
#define BOOST_RDB_CORE_HPP

#include <boost/intrusive_ptr.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <bitset>
#include <ostream>

#define BOOST_RDB_MAX_SIZE FUSION_MAX_VECTOR_SIZE
#define BOOST_RDB_MAX_ARG_COUNT 10

namespace boost { namespace rdb { namespace detail {

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

} } }

namespace boost { namespace rdb { namespace core {

  struct statement_tag { };
  struct insert_statement_tag : statement_tag { };
  struct tabular_result_tag : statement_tag { };
  struct update_statement_tag : statement_tag { };

  template<class Statement>
  struct statement_result_type {
    typedef void type;
  };

  // types as they exist independantly of any implementation
  // name `type` is already used in namespace `boost` but
  // namespace `rdb` is not meant to be imported as a whole
  // so it should do no harm
  struct integer {
    BOOST_STATIC_CONSTANT(int, id = 1);
    BOOST_STATIC_CONSTANT(int, length = 1);
  };

  struct real {
    BOOST_STATIC_CONSTANT(int, id = 2);
    BOOST_STATIC_CONSTANT(int, length = 1);
  };

  struct float_ {
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
  
  struct datetime {
    BOOST_STATIC_CONSTANT(int, id = 6);
    BOOST_STATIC_CONSTANT(size_t, length = 1);
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
  
  template<class Seq>
  struct nullable {
    Seq values_;
    typedef std::bitset<fusion::result_of::size<Seq>::value> status_vector_type;
    typedef Seq value_vector_type;
    status_vector_type status_;
    bool is_null(int pos) const { return !status_[pos]; }
    template<int I> bool is_null() const { return !status_[I]; }
    void set_null(int pos, bool to_null) { status_[pos] = !to_null; }
    template<int I> typename fusion::result_of::at_c<const Seq, I>::type get() const {
      return fusion::at_c<I>(values_);
    }
    template<int I> typename fusion::result_of::at_c<Seq, I>::type ref() {
      return fusion::at_c<I>(values_);
    }
    const Seq& values() const { return values_; }
    Seq& values() { return values_; }
    const status_vector_type& status() const { return status_; }
    status_vector_type& status() { return status_; }
    nullable& operator =(const Seq& values) { values_ = values; return *this; }
  };

  template<class Row>
  struct print_row_element {

    print_row_element(std::ostream& os, const Row& r) : os_(os), r_(r), bit_(0) { }
    
    std::ostream& os_;
    const Row& r_;
    mutable int bit_;
    
    template<class T>
    void operator ()(const T& value) const {
      if (bit_)
        os_ << " ";
      if (r_.is_null(bit_++))
        os_ << "null";
      else
        os_ << value;
    }
  };

  template<class Seq>
  std::ostream& operator <<(std::ostream& os, const nullable<Seq>& r) {
    os << "(";
    fusion::for_each(r.values(), print_row_element< nullable<Seq> >(os, r));
    os << ")";
    return os;
  }

} } }

#endif
