//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_TABLE_HPP
#define BOOST_RDB_TABLE_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/expression.hpp>

namespace boost { namespace rdb { namespace sql {

  template<class Table, class SqlType, class Base>
  struct column : Base {
    enum { precedence = precedence_level::highest };
    typedef SqlType rdb_type;
    static void str_type(std::ostream& os) { SqlType::str(os); }
    
    template<class T>
    set_clause<column, typename result_of::make_expression<column, T>::type>
    operator =(const T& expr) const {
      return set_clause<column, typename result_of::make_expression<column, T>::type>(
        *this, expression<column>::make_expression(expr));
    }
    
    template<class Expr>
    set_clause<column, Expr>
    operator =(const expression<Expr>& expr) const {
      return set_clause<column, Expr>(*this, expr.unwrap());
    }
    
    template<class T>
    set_clause<column, typename result_of::make_expression<column, T>::type>
    operator <<(const T& expr) const {
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
  
  template<class Table, class SqlType, class Base>
  struct is_column_container< column<Table, SqlType, Base> > : mpl::true_ {
  };

  template<class Col>
  struct ColumnContainer
  {
    Col col;
    std::ostream& os;

    BOOST_CONCEPT_USAGE(ColumnContainer) {
      BOOST_MPL_ASSERT((is_column_container<Col>));
      col.str(os);
    }
  };

  template<class T, class Enable = void>
  struct is_table_container : mpl::false_ {
  };

  template<class T>
  struct is_table_container<T, typename T::table_container_tag> : mpl::true_ {
  };

  namespace detail {

    struct any_table : boost::noncopyable {
      any_table() { }
      any_table(const std::string& alias) : alias_(alias) { }
      std::string alias_;
      const std::string& alias() const { return alias_; }
      bool has_alias() const { return !alias_.empty(); }
      typedef fusion::vector<> placeholder_vector;
      placeholder_vector placeholders() const { return fusion::make_vector(); }
      typedef void table_container_tag;
    };
    
    struct any_column /*: boost::noncopyable*/ {
      const any_table* table_;
      BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);

      void initialize(const any_table* table) {
        table_ = table;
      }

      typedef fusion::vector<> placeholder_vector;
      placeholder_vector placeholders() const { return fusion::make_vector(); }
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
    
    template<class Base, bool IsSelfQualified>
    struct table_;
    
    template<class Base>
    struct table_<Base, false> : Base, any_table {
      table_() { }
      table_(const std::string& alias) : any_table(alias) { }
      
      void str(std::ostream& os) const {
        if (has_alias())
          os << Base::name() << " as " << alias_;
        else
          os << Base::name();
      }

      // following function name chosen because it won't conflict with column names :-P
      static const char* table() { return Base::name(); }
    };
    
    template<class Base>
    struct table_<Base, true> : Base, any_table {

      table_() : any_table(Base::name()) { }
      
      void str(std::ostream& os) const {
        os << Base::name();
      }
    };
  }
  
  #define BOOST_RDB_BEGIN_TABLE(NAME)  \
  struct NAME##_base { static const char* name() { return #NAME; } }; \
  template<int Alias> \
  struct NAME##_ : ::boost::rdb::sql::detail::table_<NAME##_base, Alias == -1>, ::boost::rdb::sql::detail::singleton< NAME##_<Alias> > {  \
    typedef NAME##_<Alias> this_table;  \
    typedef NAME##_<1> _1; typedef NAME##_<2> _2; typedef NAME##_<3> _3;  \
    NAME##_() { initialize(); }  \
    NAME##_(const std::string& alias) : ::boost::rdb::sql::detail::table_<NAME##_base, Alias == -1>(alias) { initialize(); }  \
    NAME##_(const this_table& other) { initialize(); }  \
    typedef NAME##_<-1> qualified;  \
    typedef ::boost::mpl::vector0<>

  #define BOOST_RDB_END_TABLE(NAME)  \
    column_members; \
    void initialize() { \
      ::boost::mpl::for_each<this_table::column_members>(::boost::rdb::sql::detail::initialize_columns<this_table>(this)); \
    } \
  }; \
  typedef NAME##_<0> NAME;

  #define BOOST_RDB_COLUMN(NAME, rdb_type) \
  members_before_##NAME;  \
  enum { NAME##_index = boost::mpl::size<members_before_##NAME>::value }; \
  struct NAME##_base : ::boost::rdb::sql::detail::any_column { static const char* name() { return #NAME; } }; \
  typedef ::boost::rdb::sql::expression< ::boost::rdb::sql::column<this_table, ::boost::rdb::core::rdb_type, NAME##_base> > NAME##_type;  \
  NAME##_type NAME;  \
  struct NAME##_member {  \
    typedef std::string type;  \
    static NAME##_type& ref(this_table& obj) { return obj.NAME; }  \
    static const NAME##_type& ref(const this_table& obj) { return obj.NAME; }  \
    static void initialize(this_table* table) { table->NAME.initialize(table); }  \
  };  \
  typedef typename ::boost::mpl::push_back<members_before_##NAME, NAME##_member>::type

  template<typename Table>
  struct table_column_output : detail::comma_output {
    table_column_output(std::ostream& os, const Table& table) : comma_output(os), table_(table) { }
    
    template<typename Column> void operator ()(Column) {
      std::ostream& os = item();
      os << Column::ref(table_).name() << " ";
      Column::ref(table_).str_type(os);
    }
    
    const Table& table_;
  };

  struct create_table_statement_tag : core::statement_tag { };

  template<typename Table>
  struct create_table_statement {
    typedef create_table_statement_tag tag;
    typedef void result;
    void str(std::ostream& os) const {
      os << "create table " << Table::table() << "(";
      boost::mpl::for_each<typename Table::column_members>(table_column_output<Table>(os, Table::_));
      os << ")";
    }
    std::string str() const { return as_string(*this); }
  };

  template<class Table>
  create_table_statement<Table> create_table(const Table&) {
    return create_table_statement<Table>();
  }

  struct drop_table_statement_tag : core::statement_tag { };

  template<typename Table>
  struct drop_table_statement {
    typedef drop_table_statement_tag tag;
    typedef void result;
    void str(std::ostream& os) const {
      os << "drop table " << Table::table();
    }
  };

  template<class Table>
  drop_table_statement<Table> drop_table(const Table&) {
    return drop_table_statement<Table>();
  }

} } }

#endif
