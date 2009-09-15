#ifndef BOOST_RDB_HPP
#define BOOST_RDB_HPP

#include <iostream>
#include <boost/format.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/ref.hpp>
#include <boost/concept_check.hpp>
#include <boost/concept/requires.hpp>

namespace boost { namespace rdb {

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

struct any_table {
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

struct any_column {
  const any_table* table_;
  std::string name_;
  
  void str(std::ostream& os) const {
    if (table_->has_alias())
      os << table_->alias() << '.' << name_;
    else
      os << name_;
  }
  
  void initialize(const any_table* table, const char* name) {
    table_ = table;
    name_ = name;
  }
  
  const std::string& name() const { return name_; }
};

template<class X>
struct Expression
{
  X expr;
  std::ostream& stream;
  
  BOOST_CONCEPT_USAGE(Expression) {
    expr.str(stream);
  }
};

template<class Expr>
struct expression : Expr {
  expression() { }
  template<typename T> expression(const T& arg) : Expr(arg) { }
  template<typename T1, typename T2> expression(const T1& arg1, const T2& arg2) : Expr(arg1, arg2) { }
};

template<class Table, class SqlType>
struct column : any_column {
  typedef SqlType sql_type;
  static void str_type(std::ostream& os) { SqlType::str(os); }
};

template<typename T>
struct literal {
  literal(const T& value) : value_(value) { }
  T value_;
};

template<>
struct literal<const char*> {
  literal(const char* value) : value_(value) { }
  const char* value_;
};

template<>
struct literal<std::string> {
  literal(const char* value) : value_(value) { }
  literal(const std::string& value) : value_(value) { }
  void str(std::ostream& os) const { quote_text(os, value_.begin(), value_.end()); }
  std::string value_;
};

template<>
struct literal<int> {
  literal(int value) : value_(value) { }
  void str(std::ostream& os) const { os << value_; }
  int value_;
};

struct integer
{
  static void str(std::ostream& os) { os << "integer"; }
  typedef literal<int> literal_type;
  static literal_type make_literal(int val) { return literal_type(val); }
};

template<int N>
struct varchar
{
  static void str(std::ostream& os) { os << "varchar(" << N << ")"; }
  typedef literal<std::string> literal_type;
  static literal_type make_literal(const char* str) { return literal_type(str); }
};

template<class Expr1, class Expr2>
struct equal {
  equal(const Expr1& expr1, const Expr2& expr2) : expr1_(expr1), expr2_(expr2) { }
  
  void str(std::ostream& os) const {
    expr1_.str(os);
    os << " = ";
    expr2_.str(os);
  }
  
  Expr1 expr1_;
  Expr2 expr2_;
};

template<class Expr, typename T>
expression< equal<Expr, typename Expr::sql_type::literal_type> >
operator ==(const expression<Expr>& expr, const T& val) {
  return expression<equal<Expr, typename Expr::sql_type::literal_type> >(expr, Expr::sql_type::make_literal(val));
}

#define BOOST_RDB_OPERATOR +
#define BOOST_RDB_OPERATOR_STRING " + "
#define BOOST_RDB_OPERATOR_CLASS plus
#include "boost/rdb/details/arithmetic_operator.hpp"

#define BOOST_RDB_OPERATOR -
#define BOOST_RDB_OPERATOR_STRING " - "
#define BOOST_RDB_OPERATOR_CLASS minus
#include "boost/rdb/details/arithmetic_operator.hpp"

#define BOOST_RDB_OPERATOR *
#define BOOST_RDB__OPERATORSTRING " * "
#define BOOST_RDB_OPERATOR_CLASS times
#include "boost/rdb/details/arithmetic_operator.hpp"

#define BOOST_RDB_OPERATOR /
#define BOOST_RDB_OPERATOR_STRING " / "
#define BOOST_RDB_OPERATOR_CLASS divide
#include "boost/rdb/details/arithmetic_operator.hpp"

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

#define BOOST_RDB_COLUMN(name, sql_type) \
members_before_##name;  \
typedef expression< column<this_table, sql_type> > name##_type;  \
name##_type name;  \
struct name##_member {  \
  typedef std::string type;  \
  static name##_type& ref(this_table& obj) { return obj.name; }  \
  static const name##_type& ref(const this_table& obj) { return obj.name; }  \
  static void initialize(this_table* table) { table->name.initialize(table, #name); }  \
};  \
typedef typename boost::mpl::push_back<members_before_##name, name##_member>::type

#define BOOST_RDB_BEGIN_TABLE(NAME)  \
template<int Alias>  \
struct NAME##_ : any_table, singleton< NAME##_<Alias> > {  \
  typedef NAME##_<Alias> this_table;  \
  static const char* table_name() { return #NAME; }  \
  person_() : any_table(table_name()) { initialize(); }  \
  person_(const std::string& alias) : any_table(table_name(), alias) { initialize(); }  \
  person_(const this_table& other) { initialize(); }  \
  typedef boost::mpl::vector0<>

#define BOOST_RDB_END_TABLE(NAME)  \
  column_members; \
  void initialize() { \
    boost::mpl::for_each<this_table::column_members>(initialize_columns<this_table>(this)); \
  } \
}; \
typedef NAME##_<0> NAME;

template<class SelectList, class FromList, class WhereList>
struct select_type;

typedef boost::fusion::vector<> empty_vector;

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

template<class SelectList>
struct select_type<SelectList, void, void>;

template<class SelectList>
struct select_type<SelectList, void, void> {
  select_type(const SelectList& exprs) : exprs(exprs) { }
  SelectList exprs;

  void str(std::ostream& os) const {
    os << "select ";
    boost::fusion::for_each(exprs, comma_output(os));
  }

  template<typename Expr>
  BOOST_CONCEPT_REQUIRES(
    ((Expression<Expr>)),
    (select_type<
      typename boost::fusion::result_of::push_back< const SelectList, boost::reference_wrapper<const Expr> >::type,
      void, void
    >)) operator ()(const Expr& expr) const {
    return select_type<
      typename boost::fusion::result_of::push_back< const SelectList, boost::reference_wrapper<const Expr> >::type,
      void, void
    >(boost::fusion::push_back(exprs, boost::cref(expr)));
  }

  template<typename Table>
  select_type<
    SelectList,
    typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Table> >::type,
    void
  >
  from(const Table& table) const {
    return select_type<
      SelectList,
      typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Table> >::type,
      void
    >(exprs, boost::fusion::make_vector(boost::cref(table)));
  }
};

template<class SelectList, class FromList>
struct select_type<SelectList, FromList, void> : select_type<SelectList, void, void> {
  typedef select_type<SelectList, void, void> just_select;
  select_type(const SelectList& exprs, const FromList& tables) : just_select(exprs), tables(tables) { }

  FromList tables;

  template<class Table>
  select_type<
    SelectList,
    typename boost::fusion::result_of::push_back< const FromList, boost::reference_wrapper<const Table> >::type,
    void
  >
  operator ()(const Table& table) const {
    return select_type<
      SelectList,
      typename boost::fusion::result_of::push_back< const FromList, boost::reference_wrapper<const Table> >::type,
      void
    >(exprs, boost::fusion::push_back(tables, boost::cref(table)));
  }
  
  template<class Pred>
  BOOST_CONCEPT_REQUIRES(
    ((Expression<Pred>)),
    (select_type<
      SelectList,
      FromList,
      Pred>)
    )
  where(const Pred& pred) const {
    return select_type<
      SelectList,
      FromList,
      Pred
    >(exprs, tables, pred);
  }

  void str(std::ostream& os) const {
    just_select::str(os);
    os << " from ";
    boost::fusion::for_each(tables, comma_output(os));
  }
};

template<class SelectList, class FromList, class Predicate>
struct select_type : select_type<SelectList, FromList, void> {
  typedef select_type<SelectList, FromList, void> select_from;
  select_type(const SelectList& exprs, const FromList& tables, const Predicate& pred)
    : select_from(exprs, tables), pred(pred) { }

  const Predicate& pred;

  void str(std::ostream& os) const {
    select_from::str(os);
    os << " where ";
    pred.str(os);
  }
};

template<class Expr>
BOOST_CONCEPT_REQUIRES(
  ((Expression<Expr>)),
  (select_type<
    typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Expr> >::type,
    void, void>))
select(const Expr& expr) {
  return select_type<
    typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Expr> >::type,
    void, void
  >(boost::fusion::make_vector(boost::cref(expr)));
}

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

template<typename Table>
struct create_table_type {
  void str(std::ostream& os) const {
  os << "create table " << Table::table_name() << "(";
  boost::mpl::for_each<typename Table::column_members>(table_column_output<Table>(os, Table::_));
  os << ")";
  }
};

template<typename Table>
create_table_type<Table> create() {
  return create_table_type<Table>();
}

} }

#endif // BOOST_RDB_HPP
