//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_CORE_HPP
#define BOOST_RDB_CORE_HPP

#include <boost/intrusive_ptr.hpp>

#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
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

  struct num_comparable_type;
  struct char_comparable_type;
  struct universal;
  struct numeric_type;
  struct char_type;
  struct boolean_type;

  // types as they exist independantly of any implementation
  // name `type` is already used in namespace `boost` but
  // namespace `rdb` is not meant to be imported as a whole
  // so it should do no harm
    
  struct null_type {
    typedef null_type comparable_type;
    typedef universal kind;
  };
  
  struct placeholder_type {
    typedef boost::mpl::true_::type is_numeric;
    typedef placeholder_type comparable_type;
    typedef core::universal kind;
  };

  struct integer {
    BOOST_STATIC_CONSTANT(int, id = 1);
    BOOST_STATIC_CONSTANT(int, length = 1);
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    static void str(std::ostream& os) { os << "integer"; }
  };

  struct float_ {
    BOOST_STATIC_CONSTANT(int, id = 3);
    BOOST_STATIC_CONSTANT(int, length = 1);
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    static void str(std::ostream& os) { os << "float"; }
  };
  
  struct boolean {
    BOOST_STATIC_CONSTANT(int, id = 4);
    BOOST_STATIC_CONSTANT(int, length = 1);
    typedef boost::mpl::false_::type is_numeric;
    typedef boolean_type kind;
    static void str(std::ostream& os) { os << "boolean"; }
  };
  
  template<size_t N> struct varchar {
    BOOST_STATIC_CONSTANT(int, id = 5);
    BOOST_STATIC_CONSTANT(size_t, length = N);
    typedef boost::mpl::false_::type is_numeric;
    typedef char_comparable_type comparable_type;
    typedef char_type kind;
    static void str(std::ostream& os) { os << "varchar(" << N << ")"; }
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
  
  struct any_literal {
    //BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);
    typedef fusion::vector<> placeholder_vector;
    placeholder_vector placeholders() const { return fusion::make_vector(); }
  };

  template<typename T, class RdbType>
  struct literal : any_literal {
    literal(const T& value) : value_(value) { }
    void str(std::ostream& os) const { os << value_; }
    typedef RdbType rdb_type;
    T value_;
  };

  template<int N, class RdbType>
  struct literal<const char[N], RdbType> : any_literal {
    typedef RdbType rdb_type;
    literal(const char value[N]) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_); }
    const char* value_;
  };

  template<class RdbType>
  struct literal<const char*, RdbType> : any_literal  {
    typedef RdbType rdb_type;
    literal(const char* value) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_); }
    const char* value_;
  };

  template<typename Iter>
  void quote_text(std::ostream& os, Iter iter, Iter last) {
    os << "'";
    while (iter != last) {
      typename Iter::value_type c = *iter++;
      if (c == '\'')
        os << c;
      os << c;
    }
    os << "'";
  }

  inline void quote_text(std::ostream& os, const std::string& str) {
    quote_text(os, str.begin(), str.end());
  }
  
  void quote_text(std::ostream& os, const char* str);

  template<class RdbType>
  struct literal<std::string, RdbType> : any_literal  {
    typedef RdbType rdb_type;
    literal(const char* value) : value_(value) { }
    literal(const std::string& value) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_.begin(), value_.end()); }
    std::string value_;
  };

  template<>
  struct literal<long, integer> : any_literal  {
    typedef integer rdb_type;
    literal(long value) : value_(value) { }
    void str(std::ostream& os) const { os << value_; }
    int value_;
  };
  
  template<class RdbType, class CppType>
  struct make_literal;
  
  template<>
  struct make_literal<integer, long> {
    typedef literal<long, integer> type;
    static type value(long val) { return type(val); }
  };
  
  template<>
  struct make_literal<integer, int> : make_literal<integer, long> { };
  
  template<class T>
  struct make_literal<float_, T> {
    typedef literal<double, float_> type;
    static type value(double val) { BOOST_MPL_ASSERT((is_arithmetic<T>)); return type(val); }
  };
  
  template<size_t N>
  struct make_literal<varchar<N>, const char*> {
    typedef literal<std::string, varchar<N>> type;
    static type value(const std::string& val) { return type(val); }
  };
  
  template<size_t N, int M>
  struct make_literal<varchar<N>, const char[M]> {
    typedef literal<std::string, varchar<N>> type;
    static type value(const std::string& val) { BOOST_STATIC_ASSERT(N >= M); return type(val); }
  };
  
  template<size_t N, int M>
  struct make_literal<varchar<N>, char[M]> {
    typedef literal<std::string, varchar<N>> type;
    static type value(const std::string& val) { BOOST_STATIC_ASSERT(N >= M); return type(val); }
  };

} } }

#endif
