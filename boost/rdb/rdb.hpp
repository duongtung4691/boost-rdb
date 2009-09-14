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

namespace boost { namespace rdb {

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

template<class Table, typename SqlType>
struct column : any_column {
  static void sql_type(std::ostream& os) { SqlType::str(os); }
};

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

struct integer
{
  static void str(std::ostream& os) { os << "integer"; }
};

template<int N>
struct varchar
{
  static void str(std::ostream& os) { os << "varchar(" << N << ")"; }
};

#define BOOST_RDB_COLUMN(name, sql_type) \
members_before_##name;  \
typedef column<this_table, sql_type> name##_type;  \
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
  select_type<
    typename boost::fusion::result_of::push_back< const SelectList, boost::reference_wrapper<const Expr> >::type,
    void, void
  > operator ()(const Expr& expr) const {
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

  template<typename Table>
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

  void str(std::ostream& os) const {
    just_select::str(os);
    os << " from ";
    boost::fusion::for_each(tables, comma_output(os));
  }
};

template<class Expr>
select_type<typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Expr> >::type, void, void>
select(const Expr& expr) {
  return select_type<
    typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Expr> >::type,
    void, void
  >(boost::fusion::make_vector(boost::cref(expr)));
}

template<typename Table>
struct table_column_output : comma_output  {
  table_column_output(std::ostream& os, const Table& table) : comma_output(os), table_(table) { }
  
  template<typename Column> void operator ()(Column) {
    std::ostream& os = item();
    os << Column::ref(table_).name() << " ";
    Column::ref(table_).sql_type(os);
  }
  
  const Table& table_;
};

template<typename Table>
void create(std::ostream& os) {
  os << "create table " << Table::table_name() << "(";
  boost::mpl::for_each<typename Table::column_members>(table_column_output<Table>(os, Table::_));
  os << ")";
}

} }

#endif // BOOST_RDB_HPP
