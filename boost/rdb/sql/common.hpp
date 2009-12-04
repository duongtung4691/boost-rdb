//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_COMMON_HPP
#define BOOST_RDB_SQL_COMMON_HPP

#include <sstream>
#include <string>

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
#include <boost/mpl/arithmetic.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/make_map.hpp>
#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/value_at_key.hpp>
#include <boost/fusion/include/erase_key.hpp>
#include <boost/fusion/include/size.hpp>
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

#include <boost/rdb/common.hpp>

#include <boost/rdb/detail/static_map.hpp>

#define BOOST_RDB_PP_WITH(z, n, t) ::template with<t##n>::type
#define BOOST_RDB_PP_CALL(z, n, t) (t##n)
#define BOOST_RDB_PP_EXPRESSION(z, n, t) BOOST_PP_COMMA_IF(n) const expression<T##t##n>& t##n
#define BOOST_RDB_PP_REFERENCE(z, n, t) BOOST_PP_COMMA_IF(n) t##n&
#define BOOST_RDB_MAKE_EXPRESSION(z, n, t) BOOST_PP_COMMA_IF(n) make_expression(t##n)
#define BOOST_RDB_RESULT_OF_MAKE_EXPRESSION(z, n, t) \
  BOOST_PP_COMMA_IF(n) typename result_of::make_expression<this_type, t##n>::type
#define BOOST_PP_RDB_CONCEPT_ASSERT(z, n, t) \
  BOOST_CONCEPT_ASSERT((BOOST_PP_TUPLE_ELEM(2, 0, t)<BOOST_PP_TUPLE_ELEM(2, 1, t) ## n>));
#ifdef _MSC_VER
#pragma message("find a solution for g++")
#define BOOST_PP_RDB_MPL_ASSERT(z, n, t) \
  BOOST_MPL_ASSERT((BOOST_PP_TUPLE_ELEM(2, 0, t)<BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, t), n)>));
#else
#define BOOST_PP_RDB_MPL_ASSERT(z, n, t) \
  ;
#endif

namespace boost { namespace rdb { namespace sql {

  namespace detail {
    typedef fusion::list<> empty;
    
    struct none { };
  }

  namespace precedence_level {
    enum level {
      logical_or,
      logical_and,
      logical_not,
      compare,
      add,
      multiply,
      lowest = logical_or,
      highest = multiply
    };
  }
  
  template<class Condition, typename Tag, class Enable = void>
  struct tag_if {
  };
  
  template<class Condition, typename Tag>
  struct tag_if<Condition, Tag, typename enable_if<Condition>::type> {
    typedef Tag tag;
  };

  template<class T>
  struct type_traits;

  struct sql2003 {
    struct select;
    struct distinct;
    struct all;
    struct exprs;
    struct from;
    struct where;
    struct insert;
    struct update;
    struct cols;
    struct values;
    struct set;
  };
  
  template<class Dialect, class State, class Data, class Subdialect>
  struct select_statement;
  
  template<class Dialect, class State, class New>
  struct allow : mpl::false_ { };
  
  #define BOOST_RDB_ALLOW(Dialect, State, New) \
    template<> struct allow<Dialect, Dialect::State, Dialect::New> : mpl::true_ { }
  
  BOOST_RDB_ALLOW(sql2003, from, where);

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

  template<class ExprList>
  void str(std::ostream& os, const ct::static_map_entry<sql2003::exprs, ExprList>& p) {
    os << " ";
    fusion::for_each(p.value, comma_output(os));
  }

  inline void str(std::ostream& os, const ct::static_map_entry<sql2003::distinct, int>& p) {
    os << " distinct";
  }

  inline void str(std::ostream& os, const ct::static_map_entry<sql2003::all, int>& p) {
    os << " all";
  }

  template<class TableList>
  void str(std::ostream& os, const ct::static_map_entry<sql2003::from, TableList>& p) {
    os << " from ";
    fusion::for_each(p.value, comma_output(os));
  }

  template<class Predicate>
  void str(std::ostream& os, const ct::static_map_entry<sql2003::where, Predicate>& p) {
    os << " where ";
    p.value.str(os);
  }

  template<class ExprList>
  void str(std::ostream& os, const fusion::pair<sql2003::exprs, ExprList>& p) {
    os << " ";
    fusion::for_each(p.second, comma_output(os));
  }

  inline void str(std::ostream& os, const fusion::pair<sql2003::distinct, int>& p) {
    os << " distinct";
  }

  inline void str(std::ostream& os, const fusion::pair<sql2003::all, int>& p) {
    os << " all";
  }

  template<class TableList>
  void str(std::ostream& os, const fusion::pair<sql2003::from, TableList>& p) {
    os << " from ";
    fusion::for_each(p.second, comma_output(os));
  }

  template<class Predicate>
  void str(std::ostream& os, const fusion::pair<sql2003::where, Predicate>& p) {
    os << " where ";
    p.second.str(os);
  }

  template<class St>
  struct Statement
  {
    const St& st;
    std::ostream& stream;

    BOOST_CONCEPT_USAGE(Statement) {
      st.str(stream);
      st.placeholders();
    }
  };

  template<class St>
  struct SelectStatement : Statement<St> {
    BOOST_CONCEPT_USAGE(SelectStatement) {
    }
  };
  
  struct any_literal {
    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);
    typedef fusion::vector<> placeholder_vector;
    placeholder_vector placeholders() const { return fusion::make_vector(); }
  };

  template<typename T, class SqlType>
  struct literal : any_literal {
    literal(const T& value) : value_(value) { }
    void str(std::ostream& os) const { os << value_; }
    typedef T cpp_type;
    typedef SqlType sql_type;
    T value_;
  };

  template<int N, class SqlType>
  struct literal<const char[N], SqlType> : any_literal {
    typedef SqlType sql_type;
    literal(const char value[N]) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_); }
    const char* value_;
  };

  template<class SqlType>
  struct literal<const char*, SqlType> : any_literal  {
    typedef SqlType sql_type;
    literal(const char* value) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_); }
    const char* value_;
  };

  template<class SqlType>
  struct literal<std::string, SqlType> : any_literal  {
    typedef SqlType sql_type;
    literal(const char* value) : value_(value) { }
    literal(const std::string& value) : value_(value) { }
    void str(std::ostream& os) const { quote_text(os, value_.begin(), value_.end()); }
    std::string value_;
  };
  
  //template<class T, class SqlType>
  //struct type_traits< literal<T, SqlType> > : type_traits<SqlType> { };

  template<>
  struct literal<long, type::integer> : any_literal  {
  typedef type::integer sql_type;
    literal(long value) : value_(value) { }
    void str(std::ostream& os) const { os << value_; }
    int value_;
  };

  struct num_comparable_type;
  struct numeric_type;
  struct char_type;
  struct boolean_type;
  struct placeholder_type;

  template<>
  struct type_traits<type::integer> {
    static void str(std::ostream& os) { os << "integer"; }
    typedef literal<long, type::integer> literal_type;
    static literal_type make_literal(long val) { return literal_type(val); }
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    typedef long cpp_type;
    typedef long c_type;
  };

  template<>
  struct type_traits<type::real> {
    typedef float cpp_type;
    static void str(std::ostream& os) { os << "real"; }
    typedef literal<cpp_type, type::real> literal_type;
    template<class T>
    static literal_type make_literal(cpp_type val) { return literal_type(val); }
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    typedef cpp_type c_type;
  };

  template<>
  struct type_traits<type::float_> {
    typedef double cpp_type;
    static void str(std::ostream& os) { os << "float"; }
    typedef literal<cpp_type, type::float_> literal_type;
    static literal_type make_literal(cpp_type val) { return literal_type(val); }
    typedef boost::mpl::true_::type is_numeric;
    typedef num_comparable_type comparable_type;
    typedef numeric_type kind;
    typedef cpp_type c_type;
  };

  template<>
  struct type_traits<type::boolean> {
    static void str(std::ostream& os) { os << "boolean"; }
    typedef literal<bool, type::boolean> literal_type;
    static literal_type make_literal(bool val) { return literal_type(val); }
    typedef boolean_type kind;
    typedef bool cpp_type;
  };

  struct char_comparable_type;

  template<size_t N>
  struct type_traits< type::varchar<N> > {
    static void str(std::ostream& os) { os << "varchar(" << N << ")"; }
    typedef literal< std::string, type::varchar<N> > literal_type;
    static literal_type make_literal(const char* str) { return literal_type(str); }
    typedef char_comparable_type comparable_type;
    typedef char_type kind;
    //typedef varchar<N> c_type;
    typedef std::string cpp_type;
  };

  struct comparison {
    typedef type::boolean sql_type;
    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::compare);
  };

  template<class Statement>
  std::string as_string(const Statement& statement) {
    std::ostringstream os;
    statement.str(os);
    return os.str();
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

  namespace result_of {
    template<class Map, class Key, class Value>
    struct replace_value_at_key {
      typedef typename fusion::result_of::as_map<
        typename fusion::result_of::push_back<
          typename fusion::result_of::erase_key<
            Map,
            Key
          >::type,
          typename fusion::result_of::make_pair<
            Key,
            Value
          >::type
        >::type
      >::type type;
    };
  }
  
  template<class Key, class Map, class Value>
  typename result_of::replace_value_at_key<Map, Key, Value>::type
  replace_value_at_key(const Map& m, const Value& value) {
    return fusion::as_map(fusion::push_back(fusion::erase_key<Key>(m), fusion::make_pair<Key>(value)));
  }

  struct extract_sql_kind {

    template<typename Sig>
    struct result;

    template<typename Self, typename Expr>
    struct result<Self(Expr)> {
      typedef typename boost::remove_reference<Expr>::type::sql_type type;
    };
  };

  struct str_clause {
    str_clause(std::ostream& os) : os_(os) { }
    std::ostream& os_;

    template<typename Clause>
    void operator ()(const Clause& clause) const {
      str(os_, clause);
    }
  };
  
  struct universal;

  struct placeholder_type {
  };
  
  template<>
  struct type_traits<placeholder_type> {
    typedef boost::mpl::true_::type is_numeric;
    typedef placeholder_type comparable_type;
    typedef sql::universal kind;
  };

  template<class Expr1, class Expr2>
  struct is_sql_compatible : mpl::or_<
    is_same<
      typename type_traits<typename remove_reference<Expr1>::type::sql_type>::kind,
      typename type_traits<typename remove_reference<Expr2>::type::sql_type>::kind
    >,
    is_same<typename type_traits<typename remove_reference<Expr1>::type::sql_type>::kind, universal>,
    is_same<typename type_traits<typename remove_reference<Expr2>::type::sql_type>::kind, universal>
  > {
  };

  template<class Iter1, class Last, class Iter2, class End>
  struct _sql_lists_compatible : mpl::and_<
    is_sql_compatible<
      typename fusion::result_of::deref<Iter1>::type,
      typename fusion::result_of::deref<Iter2>::type
    >,
    _sql_lists_compatible<
      typename fusion::result_of::next<Iter1>::type,
      Last,
      typename fusion::result_of::next<Iter2>::type,
      typename fusion::result_of::equal_to<
        typename fusion::result_of::next<Iter1>::type,
        Last
      >::type
    >
  > {
  };

  template<class Iter1, class Last, class Iter2>
  struct _sql_lists_compatible<Iter1, Last, Iter2, 
    typename fusion::result_of::equal_to<Last, Last>::type
  > : mpl::true_ {
  };

  template<class ExprList1, class ExprList2>
  struct sql_lists_compatible : _sql_lists_compatible<
    typename fusion::result_of::begin<ExprList1>::type,
    typename fusion::result_of::end<ExprList1>::type,
    typename fusion::result_of::begin<ExprList2>::type,
    typename fusion::result_of::equal_to<
      typename fusion::result_of::begin<ExprList1>::type,
      typename fusion::result_of::end<ExprList1>::type
    >::type
  > {
  };

  // This is /not/ the placeholder type. It's just the marks' type, i.e.
  // the type of `_`, `_1`, etc. Placeholders are typed things, marks are not.
  // The actual type of the placeholder is determined from the context in which
  // it is used.
  template<int N>
  struct placeholder_mark {
    typedef placeholder_type sql_type;
    typedef fusion::vector<> placeholder_vector; // not really used; exists to please mpl::if_ which is not lazy
    placeholder_vector placeholders() const { return fusion::make_vector(); }
    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);
    void str(std::ostream& os) const {
      os << "?";
    }
  };
  
  template<class Col, class Expr>
  struct set_clause {
    set_clause(const Col& col, const Expr& expr) : col_(col), expr_(expr) {
      BOOST_MPL_ASSERT((is_sql_compatible<Col, Expr>));
    }
    
    Col col_;
    Expr expr_;
    
    void str(std::ostream& os) const {
      col_.str(os);
      os << " = ";
      expr_.str(os);
    }
    
    template<class Expr2>
    struct placeholders_for {
      typedef typename Expr2::placeholder_vector placeholder_vector;
      static placeholder_vector make(const set_clause& update) {
        return update.expr_.placeholders();
      }
    };
    
    template<int N>
    struct placeholders_for< placeholder_mark<N> > {
      typedef fusion::vector< type::placeholder<typename Col::sql_type> > placeholder_vector;
      static placeholder_vector make(const set_clause& update) {
        return fusion::make_vector(type::placeholder<typename Col::sql_type>());
      }
    };
    
    typedef typename placeholders_for<Expr>::placeholder_vector placeholder_vector;
    
    placeholder_vector placeholders() const {
      return placeholders_for<Expr>::make(*this);
    }
  };
  
  template<class T>
  struct is_update_container : mpl::false_ {
  };
  
  template<class Col, class Expr>
  struct is_update_container< set_clause<Col, Expr> > : mpl::true_ {
  };

  struct no_tag;
  
  template<class T>
  struct tag_of {
    typedef no_tag type;
  };
  
  template<class T>
  struct is_column_container : mpl::false_ {
  };
  
} } }

#endif // BOOST_RDB_SQL_COMMON_HPP
