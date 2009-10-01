//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_TABLE_HPP
#define BOOST_RDB_TABLE_HPP

namespace boost { namespace rdb { namespace sql {

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
    
    template<class T>
    set_clause<column, typename result_of::make_expression<column, T>::type>
    operator =(const T& expr) const {
      return set_clause<column, typename result_of::make_expression<column, T>::type>(
        *this, expression<column>::make_expression(expr));
    }

    void str(std::ostream& os) const {
      if (this->table_->has_alias())
        os << this->table_->alias() << '.' << Base::name();
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

  struct create_table_statement_tag : statement_tag { };

  template<typename Table>
  struct create_table_statement {
    typedef create_table_statement_tag tag;
    typedef void result;
    void str(std::ostream& os) const {
      os << "create table " << Table::table_name() << "(";
      boost::mpl::for_each<typename Table::column_members>(table_column_output<Table>(os, Table::_));
      os << ")";
    }
    std::string str() const { return as_string(*this); }
  };

  template<class Table>
  create_table_statement<Table> create_table(const Table&) {
    return create_table_statement<Table>();
  }

  struct drop_table_statement_tag : statement_tag { };

  template<typename Table>
  struct drop_table_statement {
    typedef drop_table_statement_tag tag;
    typedef void result;
    void str(std::ostream& os) const {
      os << "drop table " << Table::table_name();
    }
  };

  template<class Table>
  drop_table_statement<Table> drop_table(const Table&) {
    return drop_table_statement<Table>();
  }

} } }

#endif
