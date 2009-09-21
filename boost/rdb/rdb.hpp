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
#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <boost/fusion/container/list.hpp>
#include <boost/fusion/include/make_list.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/join.hpp>
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

#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#define BOOST_RDB_MAX_ARG_COUNT 10
#define BOOST_RDB_PP_WITH(z, n, t) ::template with<t##n>::type
#define BOOST_RDB_PP_CALL(z, n, t) (t##n)
#define BOOST_RDB_PP_EXPRESSION(z, n, t) BOOST_PP_COMMA_IF(n) const expression<T##t##n>& t##n

namespace boost { namespace rdb {

  namespace details {
    typedef boost::fusion::list<> empty;
  }

  namespace precedence_level {
    enum {
      boolean,
      compare,
      add,
      multiply,
      logical_not,
      highest
    };
  }

  template<typename Iter>
  void quote_text(std::ostream& os, Iter iter, Iter last) {
    os << "'";
    while (iter != last) {
      Iter::value_type c = *iter++;
      if (c == '\'')
        os << c;
      os << c;
    }
    os << "'";
  }

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

  struct any_table : boost::noncopyable {
    any_table(const std::string& name) : name_(name) { }
    any_table(const std::string& name, const std::string& alias) : name_(name), alias_(alias) { }
    std::string name_, alias_;

    void str(std::ostream& os) const {
      if (has_alias())
        os << name_ << " as " << alias_;
      else
        os << name_;
    }
    const std::string& alias() const { return alias_; }
    bool has_alias() const { return !alias_.empty(); }
  };

  template<class Expr>
  struct Expression
  {
    Expr expr;
    std::ostream& stream;
    typedef typename Expr::sql_type sql_type;
    enum { precedence = Expr::precedence };

    BOOST_CONCEPT_USAGE(Expression) {
      expr.str(stream);
    }
  };

  template<class Expr>
  struct NumericExpression : Expression<Expr>
  {
    typedef typename Expr::sql_type::is_numeric is_numeric;
  };

  template<class Expr>
  struct ComparableExpression : Expression<Expr>
  {
    typedef typename Expr::sql_type::comparable_type comparable_type;
  };

  template<class Expr, typename T>
  struct CompatibleLiteral
  {
    const T& value;
    BOOST_CONCEPT_USAGE(CompatibleLiteral) {
      Expr::sql_type::make_literal(value);
    }
  };

  template<class Expr>
  struct expression : Expr {
    expression() { }
    template<typename T> expression(const T& arg) : Expr(arg) { }
    template<typename T1, typename T2> expression(const T1& arg1, const T2& arg2) : Expr(arg1, arg2) { }
    const Expr& unwrap() const { return *this; }
  };

  template<class St>
  struct Statement
  {
    const St& st;
    std::ostream& stream;

    BOOST_CONCEPT_USAGE(Statement) {
      st.str(stream);
    }
  };

  template<class Statement>
  struct statement : Statement {
    statement() { }
    template<typename T> statement(const T& arg) : Statement(arg) { }
    const Statement& unwrap() const { return *this; }
  };
  
  struct any_column /*: boost::noncopyable*/ {
    const any_table* table_;
    enum { precedence = precedence_level::highest };

    void initialize(const any_table* table) {
      table_ = table;
    }
  };

  template<class Table, class SqlType, class Base>
  struct column : Base {
    enum { precedence = precedence_level::highest };
    typedef SqlType sql_type;
    typedef Table table_type;
    typedef typename sql_type::cpp_type cpp_type;
    static void str_type(std::ostream& os) { SqlType::str(os); }

    void str(std::ostream& os) const {
      if (table_->has_alias())
        os << table_->alias() << '.' << Base::name();
      else
        os << Base::name();
    }
  };

  template<class Col>
  struct Column
  {
    Col col;
    std::ostream& os;
    typedef typename Col::table_type table_type;

    BOOST_CONCEPT_USAGE(Column) {
      col.str(os);
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

  template<>
  struct literal<const char*> : any_literal  {
    literal(const char* value) : value_(value) { }
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

  template<typename T>
  literal<T> as_expression(const T& value) {
    return literal<T>(value);
  }

  template<class Expr>
  BOOST_CONCEPT_REQUIRES(
    ((Expression<Expr>)),
  (const Expr&))
  as_expression(const expression<Expr>& expr) {
    return expr.unwrap();
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

  template<class Expr>
  struct BooleanExpression : Expression<Expr>
  {
    BOOST_CONCEPT_USAGE(BooleanExpression) {
      BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type, boolean>));
    }
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

  template<class Expr1, class Expr2, int Precedence>
  struct binary_operation {

    enum { precedence = Precedence };

    static void write(std::ostream& os, const Expr1& expr1, const char* op, const Expr2& expr2) {
      write(os, expr1, boost::mpl::bool_<Expr1::precedence < precedence>());
      os << op;
      write(os, expr2, boost::mpl::bool_<Expr2::precedence < precedence>());
    }

    template<class Expr>
    static void write(std::ostream& os, const Expr& expr, boost::mpl::true_) {
      os << "(";
      expr.str(os);
      os << ")";
    }

    template<class Expr>
    static void write(std::ostream& os, const Expr& expr, boost::mpl::false_) {
      expr.str(os);
    }
  };

  #define BOOST_RDB_OPERATOR +
  #define BOOST_RDB_OPERATOR_STRING " + "
  #define BOOST_RDB_OPERATOR_CLASS plus
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::add
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR -
  #define BOOST_RDB_OPERATOR_STRING " - "
  #define BOOST_RDB_OPERATOR_CLASS minus
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::add
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR *
  #define BOOST_RDB__OPERATORSTRING " * "
  #define BOOST_RDB_OPERATOR_CLASS times
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::multiply
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR /
  #define BOOST_RDB_OPERATOR_STRING " / "
  #define BOOST_RDB_OPERATOR_CLASS divide
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::multiply
  #include "boost/rdb/details/arithmetic_operator.hpp"

  #define BOOST_RDB_OPERATOR ==
  #define BOOST_RDB_OPERATOR_STRING " = "
  #define BOOST_RDB_OPERATOR_CLASS eq
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR !=
  #define BOOST_RDB_OPERATOR_STRING " <> "
  #define BOOST_RDB_OPERATOR_CLASS ne
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR <
  #define BOOST_RDB_OPERATOR_STRING " < "
  #define BOOST_RDB_OPERATOR_CLASS lt
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR <=
  #define BOOST_RDB_OPERATOR_STRING " <= "
  #define BOOST_RDB_OPERATOR_CLASS le
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR >
  #define BOOST_RDB_OPERATOR_STRING " > "
  #define BOOST_RDB_OPERATOR_CLASS gt
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR >=
  #define BOOST_RDB_OPERATOR_STRING " >= "
  #define BOOST_RDB_OPERATOR_CLASS ge
  #define BOOST_RDB_OPERATOR_PRECEDENCE precedence_level::compare
  #include "boost/rdb/details/comparison_operator.hpp"

  #define BOOST_RDB_OPERATOR &&
  #define BOOST_RDB_OPERATOR_STRING " and "
  #define BOOST_RDB_OPERATOR_CLASS and
  #include "boost/rdb/details/boolean_operator.hpp"

  #define BOOST_RDB_OPERATOR ||
  #define BOOST_RDB_OPERATOR_STRING " or "
  #define BOOST_RDB_OPERATOR_CLASS or
  #include "boost/rdb/details/boolean_operator.hpp"

  template<class Expr>
  struct not {

    not(const Expr& expr) : expr_(expr) { }

    typedef boolean sql_type;

    enum { precedence = precedence_level::logical_not };
    
    void str(std::ostream& os) const {
      write(os, boost::mpl::bool_<Expr::precedence < precedence>());
    }

    void write(std::ostream& os, boost::mpl::true_) const {
      os << "not (";
      expr_.str(os);
      os << ")";
    }

    void write(std::ostream& os, boost::mpl::false_) const {
      os << "not ";
      expr_.str(os);
    }
    
    Expr expr_;
  };

  template<class Expr>
  BOOST_CONCEPT_REQUIRES(
    ((BooleanExpression<Expr>)),
    (expression< not<Expr> >))
  operator !(const expression<Expr>& expr) {
    return expression< not<Expr> >(expr);
  }

  template<class Table>
  struct initialize_columns {
    initialize_columns(Table* pt) : pt(pt) { }
    template<typename T> void operator ()(T) {
      T::initialize(pt);
    }
    Table* pt;
  };

  template<typename T>
  struct singleton {
    static T _;
  };

  template<class T>
  T singleton<T>::_;

  #define BOOST_RDB_COLUMN(NAME, sql_type) \
  members_before_##NAME;  \
  enum { NAME##_index = boost::mpl::size<members_before_##NAME>::value }; \
  struct NAME##_base : any_column { static const char* name() { return #NAME; } }; \
  typedef expression< column<this_table, sql_type, NAME##_base> > NAME##_type;  \
  NAME##_type NAME;  \
  struct NAME##_member {  \
    typedef std::string type;  \
    static NAME##_type& ref(this_table& obj) { return obj.NAME; }  \
    static const NAME##_type& ref(const this_table& obj) { return obj.NAME; }  \
    static void initialize(this_table* table) { table->NAME.initialize(table); }  \
  };  \
  typedef typename boost::mpl::push_back<members_before_##NAME, NAME##_member>::type

  #define BOOST_RDB_BEGIN_TABLE(NAME)  \
  template<int Alias>  \
  struct NAME##_ : any_table, singleton< NAME##_<Alias> > {  \
    typedef NAME##_<Alias> this_table;  \
    static const char* table_name() { return #NAME; }  \
    NAME##_() : any_table(table_name()) { initialize(); }  \
    NAME##_(const std::string& alias) : any_table(table_name(), alias) { initialize(); }  \
    NAME##_(const this_table& other) { initialize(); }  \
    typedef boost::mpl::vector0<>

  #define BOOST_RDB_END_TABLE(NAME)  \
    column_members; \
    void initialize() { \
      boost::mpl::for_each<this_table::column_members>(initialize_columns<this_table>(this)); \
    } \
  }; \
  typedef NAME##_<0> NAME;

  template<typename Table>
  struct table_column_output : comma_output {
    table_column_output(std::ostream& os, const Table& table) : comma_output(os), table_(table) { }
    
    template<typename Column> void operator ()(Column) {
      std::ostream& os = item();
      os << Column::ref(table_).name() << " ";
      Column::ref(table_).str_type(os);
    }
    
    const Table& table_;
  };

  template<class Statement>
  std::string as_string(const Statement& statement) {
    std::ostringstream os;
    statement.str(os);
    return os.str();
  }

  template<typename Table>
  struct create_table_type {
    void str(std::ostream& os) const {
    os << "create table " << Table::table_name() << "(";
    boost::mpl::for_each<typename Table::column_members>(table_column_output<Table>(os, Table::_));
    os << ")";
    }
    std::string str() const { return as_string(*this); }
  };

  template<typename Table>
  
  create_table_type<Table> create_table() {
    return statement< create_table_type<Table> >();
  }
#if 1
  namespace result_of {
    template<typename T>
    struct make_list {
      typedef typename boost::fusion::result_of::push_back<
        const details::empty,
        T
      >::type type;
    };
  }

  template<typename T>
  typename result_of::make_list<T>::type
  make_list(const T& val) {
    return boost::fusion::push_back(boost::fusion::list<>(), val);
  }
  namespace result_of {
    template<typename T>
    struct make_list2 {
      typedef typename boost::fusion::result_of::make_list<T>::type type;
    };
  }

  template<typename T>
  typename result_of::make_list2<T>::type
  make_list2(const T& val) {
    return boost::fusion::make_list(val);
  }


#else
  namespace result_of {
      using boost::fusion::result_of::make_list;
  }

  using boost::fusion::make_list;


#endif

} }

#include <boost/rdb/expression.hpp>
#include <boost/rdb/insert.hpp>
#include <boost/rdb/select.hpp>
#include <boost/rdb/database.hpp>

#endif // BOOST_RDB_HPP
