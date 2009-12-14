//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DYNAMIC_HPP
#define BOOST_RDB_SQL_DYNAMIC_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/dynamic.hpp>

#include <boost/any.hpp>

#include <numeric>
#include <functional>

namespace boost { namespace rdb { namespace sql {

  using namespace dynamic;
    
  inline void str(std::ostream& os, const dynamic_expressions& exprs) {
    std::for_each(exprs.begin(), exprs.end(), detail::comma_output(os));
  }    

  struct make_dynamic_placeholders {
    make_dynamic_placeholders(std::vector<dynamic::dynamic_placeholder>& placeholders) : placeholders_(placeholders) { }
    
    mutable std::vector<dynamic::dynamic_placeholder>& placeholders_;
    
    template<class Placeholder>
    void operator ()(const Placeholder& p) const {
      placeholders_.push_back(dynamic::dynamic_placeholder(Placeholder::rdb_type::id, Placeholder::rdb_type::length));
    }
  };

  template<class SqlType>
  struct dynamic_expression_wrapper : dynamic::dynamic_expression {
    typedef SqlType sql_type;
    
    typedef fusion::vector< const std::vector<dynamic::dynamic_placeholder> > placeholder_vector;
    
    placeholder_vector placeholders() const {
      return fusion::make_vector(impl_->placeholders_);
    }

    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::lowest);

    dynamic_expression_wrapper(root* p) : dynamic::dynamic_expression(p) { }
  };

  template<class Expr>
  struct dynamic_expression_impl : dynamic::dynamic_expression::root {

    dynamic_expression_impl(const Expr& expr) : dynamic::dynamic_expression::root(Expr::sql_type::id, Expr::sql_type::length), expr_(expr) {
      fusion::for_each(expr.placeholders(), make_dynamic_placeholders(this->placeholders_));
    }

    virtual void str(std::ostream& os) const {
      expr_.str(os);
    }

    Expr expr_;
  };

  template<class Expr>
  expression< dynamic_expression_wrapper<typename Expr::sql_type> >
  make_dynamic(const expression<Expr>& expr) {
    return expression< dynamic_expression_wrapper<typename Expr::sql_type> >(new dynamic_expression_impl<Expr>(expr));
  }

  // alas no templatized typedefs yet
  // template<class Expr> typedef expression< dynamic_expression_wrapper<typename Expr::sql_type> > dynamic_expression<Expr>;
  typedef expression< dynamic_expression_wrapper<core::integer> > dynamic_integer;
  typedef expression< dynamic_expression_wrapper<core::boolean> > dynamic_boolean;

  struct dynamic_placeholder_impl : dynamic::dynamic_expression::root {

    dynamic_placeholder_impl(int type, int length) : dynamic::dynamic_expression::root(type, length) {
      placeholders_.push_back(dynamic::dynamic_placeholder(type, length));
    }

    virtual void str(std::ostream& os) const {
      os << "?";
    }
  };

  template<class Expr>
  dynamic_expression_wrapper<typename Expr::sql_type>
  make_dynamic(const placeholder_mark<0>& mark, const expression<Expr>&) {
    return dynamic_expression_wrapper<typename Expr::sql_type>(
      new dynamic_placeholder_impl(Expr::sql_type::id, Expr::sql_type::length));
  }

  template<class SqlType>
  struct dynamic_column_wrapper : dynamic_expression_wrapper<SqlType> {
    dynamic_column_wrapper(typename dynamic_expression_wrapper<SqlType>::root* p) : dynamic_expression_wrapper<SqlType>(p) { }
  };

  template<class Table, class SqlType, class Base>
  dynamic_column_wrapper<SqlType>
  make_dynamic(const expression< column<Table, SqlType, Base> >& col) {
    return dynamic_column_wrapper<SqlType>(new dynamic_expression_impl< column<Table, SqlType, Base> >(col));
  }
  
  class dynamic_columns {
  
  private:
    std::vector<dynamic::dynamic_expression> cols_;
  
  public:
  
    typedef fusion::vector<> placeholder_vector;

    placeholder_vector placeholders() const {
      return placeholder_vector();
    }
    
    template<class SqlType>
    void push_back(const dynamic_column_wrapper<SqlType>& col) {
      cols_.push_back(col);
    }

    typedef void sql_type;
    
    void str(std::ostream& os) const {
      std::for_each(cols_.begin(), cols_.end(), detail::comma_output(os));
    }
  };

  template<>
  struct is_column_container<dynamic_columns> : mpl::true_ {
  };
  
  namespace result_of {
  
    template<>
    struct make_expression<dynamic_columns, dynamic::dynamic_expressions> {
      typedef dynamic::dynamic_expressions type;
      static const type& make(const dynamic::dynamic_expressions& exprs) { return exprs; }
    };

    template<>
    struct make_expression<dynamic::dynamic_expressions, dynamic::dynamic_expressions> {
      typedef dynamic::dynamic_expressions type;
      static const dynamic::dynamic_expressions& make(const dynamic::dynamic_expressions& exprs) { return exprs; }
    };
  }

  template<>
  struct is_placeholder_mark<dynamic::dynamic_expressions> : false_type {
  };
  
  struct dynamic_update {

    struct root : rdb::detail::ref_counted {
      dynamic::dynamic_placeholders placeholders_;
      virtual void str(std::ostream& os) const = 0;
    };
    
    dynamic_update(root* impl) : impl_(impl) { }

    //typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;
    
    const dynamic::dynamic_placeholders& placeholders() const {
      return impl_->placeholders_;
    }
    
    void str(std::ostream& os) const {
      impl_->str(os);
    }
    
    intrusive_ptr<root> impl_;
    
    template<class Col, class Expr>
    struct wrapper : root {

      wrapper(const set_clause<Col, Expr>& update) : update_(update) {
        fusion::for_each(update_.placeholders(), make_dynamic_placeholders(this->placeholders_));
      }
      
      set_clause<Col, Expr> update_;
      
      virtual void str(std::ostream& os) const {
        update_.str(os);
      }
    };
  };
    
  template<class Col, class Expr>
  dynamic_update make_dynamic(const set_clause<Col, Expr>& update) {
    return dynamic_update(new dynamic_update::wrapper<Col, Expr>(update));
  }
  
  class dynamic_updates {
  
  private:
    std::vector<dynamic_update> updates_;
  
  public:
  
    typedef fusion::vector< const std::vector<dynamic::dynamic_placeholder> > placeholder_vector;

    placeholder_vector placeholders() const {
      int size = 0;
      std::vector<dynamic_update>::const_iterator in = updates_.begin();

      while (in != updates_.end()) {
        in->placeholders().size();
        size += in++->placeholders().size();
      }

      std::vector<dynamic::dynamic_placeholder> result(size);
      std::vector<dynamic::dynamic_placeholder>::iterator out = result.begin();
      in = updates_.begin();

      while (in != updates_.end()) {
        out = std::copy(in->placeholders().begin(), in->placeholders().end(), out);
        ++in;
      }

      return result;
    }
    
    void push_back(const dynamic_update& update) {
      updates_.push_back(update);
    }

    typedef void sql_type;
    
    void str(std::ostream& os) const {
      std::for_each(updates_.begin(), updates_.end(), detail::comma_output(os));
    }
  };

  template<>  
  struct is_update_container<dynamic_updates> : mpl::true_ {
  };
  
  struct dynamic_table {

    struct root : rdb::detail::ref_counted {
      root() { }
      virtual void str(std::ostream& os) const = 0;
    };
  
    dynamic_table(root* impl) : impl_(impl) { }

    intrusive_ptr<root> impl_;

    void str(std::ostream& os) const {
      impl_->str(os);
    }
    
    template<class Table>
    struct wrapper : root {

      wrapper(const Table& table) : table_(table) {
      }
      
      const Table& table_;
      
      virtual void str(std::ostream& os) const {
        table_.str(os);
      }
    };
  };
    
  template<class Table>
  typename enable_if<is_table_container<Table>, dynamic_table>::type
  make_dynamic(const Table& table) {
    return dynamic_table(new dynamic_table::wrapper<Table>(table));
  }
  
  class dynamic_tables {
  
  private:
    std::vector<dynamic_table> tables_;
  
  public:
  
    typedef void table_container_tag;
  
    typedef fusion::vector<> placeholder_vector;

    placeholder_vector placeholders() const {
      return placeholder_vector();
    }
    
    void push_back(const dynamic_table& table) {
      tables_.push_back(table);
    }
    
    void str(std::ostream& os) const {
      std::for_each(tables_.begin(), tables_.end(), detail::comma_output(os));
    }
  };
  
} } }


#endif
