#ifndef BOOST_RDB_HPP
#define BOOST_RDB_HPP

#include <iostream>
#include <sstream>
#include <deque>

#include <boost/format.hpp>
#include <boost/ref.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <boost/fusion/container/list.hpp>
#include <boost/fusion/include/make_list.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/make_map.hpp>
#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/value_at_key.hpp>
#include <boost/fusion/include/erase_key.hpp>
#include <boost/fusion/include/size.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/join.hpp>
#include <boost/fusion/include/replace_if.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/end.hpp>
#include <boost/fusion/include/next.hpp>
#include <boost/fusion/include/deref.hpp>
#include <boost/fusion/include/front.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/include/zip_view.hpp>

#include <boost/concept_check.hpp>
#include <boost/concept/requires.hpp>
#include <boost/noncopyable.hpp>

#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#define BOOST_RDB_MAX_SIZE FUSION_MAX_VECTOR_SIZE
#define BOOST_RDB_MAX_ARG_COUNT 10
#define BOOST_RDB_PP_WITH(z, n, t) ::template with<t##n>::type
#define BOOST_RDB_PP_CALL(z, n, t) (t##n)
#define BOOST_RDB_PP_EXPRESSION(z, n, t) BOOST_PP_COMMA_IF(n) const expression<T##t##n>& t##n
#define BOOST_RDB_PP_AS_EXPRESSION(z, n, t) BOOST_PP_COMMA_IF(n) as_expression(t##n)
#define BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION(z, n, t) BOOST_PP_COMMA_IF(n) typename result_of::as_expression<t##n>::type
#define BOOST_RDB_PP_REFERENCE(z, n, t) BOOST_PP_COMMA_IF(n) t##n&

namespace boost { namespace rdb {

  namespace detail {
    typedef fusion::list<> empty;
    
    struct none { };
  }

  namespace precedence_level {
    enum level {
      boolean,
      compare,
      add,
      multiply,
      logical_not,
      highest
    };
  }

  template<class Context, class Data>
  struct select_statement;

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

  struct comma_output {
    comma_output(std::ostream& os) : os_(os), comma_("") { }
    std::ostream& os_;
    mutable const char* comma_;
    std::ostream& item() const {
      os_ << comma_;
      comma_ = ", ";
      return os_;
    }
    template<typename Item> void operator ()(const Item& i) const {
      i.str(item());
    }
  };

  struct assign_output : comma_output {
    assign_output(std::ostream& os) : comma_output(os) { }

    template<typename First, typename Second>
    void operator ()(const fusion::vector<First, Second>& p) const {
      std::ostream& os = this->item();
      using namespace fusion;
      at_c<0>(p).str(os);
      os_ << " = ";
      at_c<1>(p).str(os);
    }
  };

  //namespace result_of {
  //  template<class Key, class Value, class Sequence>
  //  struct replace_value {
  //    typedef typename result_of::replace_if<
  //      Sequence,
  //    >::type;
  //  };
  //}
  //    
  //    
  //    replace_if(const Sequence& seq



  struct statement_tag { };

  template<class St>
  struct Statement
  {
    const St& st;
    std::ostream& stream;

    BOOST_CONCEPT_USAGE(Statement) {
      statement_tag* p = static_cast<typename St::tag*>(0);
      st.str(stream);
    }
  };

  struct select_statement_tag : statement_tag { };

  template<class St>
  struct SelectStatement : Statement<St> {
    St& st;
    std::ostream& stream;
    BOOST_CONCEPT_USAGE(SelectStatement) {
      select_statement_tag* p = static_cast<typename St::tag*>(0);
      st.str(stream);
    }
  };

  struct any_literal {
    enum { precedence = precedence_level::highest };
  };

  template<typename T>
  struct literal : any_literal {
    literal(const T& value) : value_(value) { }
    void str(std::ostream& os) const { os << value_; }
    typedef T cpp_type;
    T value_;
  };

  template<int N>
  struct literal<const char[N]> : any_literal {
    literal(const char value[N]) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_); }
    const char* value_;
  };

  template<>
  struct literal<const char*> : any_literal  {
    literal(const char* value) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_); }
    const char* value_;
  };

  template<>
  struct literal<std::string> : any_literal  {
    literal(const char* value) : value_(value) { }
    literal(const std::string& value) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_.begin(), value_.end()); }
    std::string value_;
  };

  template<>
  struct literal<long> : any_literal  {
    literal(long value) : value_(value) { }
    void str(std::ostream& os) const { os << value_; }
    int value_;
  };

  namespace result_of {
    template<typename T>
    struct as_expression {
      typedef literal<T> type;
    };
    template<typename T> struct as_expression<T&> : as_expression<T> { };
    template<typename T> struct as_expression<const T&> : as_expression<T> { };
  }

  template<typename T>
  literal<T> as_expression(const T& value) {
    return literal<T>(value);
  }

  struct num_comparable_type;
  struct numeric_type;
  struct char_type;
  struct boolean_type;

  struct integer
  {
    static void str(std::ostream& os) { os << "integer"; }
    typedef literal<long> literal_type;
    static literal_type make_literal(long val) { return literal_type(val); }
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    typedef long cpp_type;
  };

  struct boolean
  {
    static void str(std::ostream& os) { os << "boolean"; }
    typedef literal<bool> literal_type;
    static literal_type make_literal(bool val) { return literal_type(val); }
    typedef boolean_type kind;
    typedef bool cpp_type;
  };

  struct char_comparable_type;

  template<int N>
  struct varchar
  {
    static void str(std::ostream& os) { os << "varchar(" << N << ")"; }
    typedef literal<std::string> literal_type;
    static literal_type make_literal(const char* str) { return literal_type(str); }
    typedef char_comparable_type comparable_type;
    typedef char_type kind;
    typedef std::string cpp_type;
    enum { size = N };
  };

  struct comparison {
    typedef boolean sql_type;
    enum { precedence = precedence_level::compare };
  };

  template<class Statement>
  std::string as_string(const Statement& statement) {
    std::ostringstream os;
    statement.str(os);
    return os.str();
  }

  namespace result_of {
    template<typename T>
    struct make_list {
      typedef typename fusion::result_of::push_back<
        const detail::empty,
        T
      >::type type;
    };
  }

  template<typename T>
  typename result_of::make_list<T>::type
  make_list(const T& val) {
    return fusion::push_back(fusion::list<>(), val);
  }
  namespace result_of {
    template<typename T>
    struct make_list2 {
      typedef typename fusion::result_of::make_list<T>::type type;
    };
  }

  template<typename T>
  typename result_of::make_list2<T>::type
  make_list2(const T& val) {
    return fusion::make_list(val);
  }
    namespace result_of {
      template<class Map, class Key, class Value>
      struct add_key {
        typedef typename fusion::result_of::as_map<
          typename fusion::result_of::push_back<
            Map,
            typename fusion::result_of::make_pair<
              Key,
              Value
            >::type
          >::type
        >::type type;
      };
    }
    
    template<class Key, class Map, class Value>
    typename result_of::add_key<Map, Key, Value>::type
    add_key(const Map& m, const Value& value) {
      return fusion::as_map(fusion::push_back(m, fusion::make_pair<Key>(value)));
    }

} }

#include <boost/rdb/expression.hpp>
#include <boost/rdb/table.hpp>
#include <boost/rdb/insert.hpp>
#include <boost/rdb/update.hpp>
#include <boost/rdb/select.hpp>
#include <boost/rdb/delete.hpp>
#include <boost/rdb/database.hpp>

#endif // BOOST_RDB_HPP
