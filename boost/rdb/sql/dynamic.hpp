//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP
#define BOOST_RDB_SQL_DYNAMIC_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>
#include <numeric>
#include <functional>

namespace boost { namespace rdb { namespace sql {

  template<class SqlType>
  struct dynamic_expression_wrapper : dynamic_expression {
    typedef SqlType sql_type;
    
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;
    
    placeholder_vector placeholders() const {
      return fusion::make_vector(impl_->placeholders_);
    }

    enum { precedence = precedence_level::lowest };

    dynamic_expression_wrapper(root* p) : dynamic_expression(p) { }
  };

  template<class Expr>
  struct dynamic_expression_impl : dynamic_expression::root {

    dynamic_expression_impl(const Expr& expr) : dynamic_expression::root(Expr::sql_type::id, Expr::sql_type::length), expr_(expr) {
      fusion::for_each(expr.placeholders(), make_placeholder(this->placeholders_));
    }

    struct make_placeholder {
      make_placeholder(std::vector<dynamic_placeholder>& placeholders) : placeholders_(placeholders) { }
      
      mutable std::vector<dynamic_placeholder>& placeholders_;
      
      template<class Placeholder>
      void operator ()(const Placeholder& p) const {
        placeholders_.push_back(dynamic_placeholder(Placeholder::rdb_type::id, Placeholder::rdb_type::length));
      }
    };

    virtual void str(std::ostream& os) const {
      expr_.str(os);
    }

    Expr expr_;
  };

  template<class Expr>
  dynamic_expression_wrapper<typename Expr::sql_type>
  make_dynamic(const expression<Expr>& expr) {
    return dynamic_expression_wrapper<typename Expr::sql_type>(new dynamic_expression_impl<Expr>(expr));
  }

  // alas no templatized typedefs yet
  // template<class Expr> typedef expression< dynamic_expression_wrapper<typename Expr::sql_type> > dynamic_expression<Expr>;
  typedef expression< dynamic_expression_wrapper<type::integer> > dynamic_integer;
  typedef expression< dynamic_expression_wrapper<type::boolean> > dynamic_boolean;

  struct dynamic_expressions : std::vector<dynamic_expression> {
  
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;

    placeholder_vector placeholders() const {

      int size = 0;
      std::vector<dynamic_expression>::const_iterator in = begin();

      while (in != end()) {
        size += in++->placeholders().size();
      }

      std::vector<dynamic_placeholder> result(size);
      std::vector<dynamic_placeholder>::iterator out = result.begin();
      in = begin();

      while (in != end()) {
        out = std::copy(in->placeholders().begin(), in->placeholders().end(), out);
        ++in;
      }

      return result;
    }

    typedef void sql_type;
    
    void str(std::ostream& os) const {
      std::for_each(begin(), end(), comma_output(os));
    }
  };

  struct dynamic_placeholder_impl : dynamic_expression::root {

    dynamic_placeholder_impl(int type, int length) : dynamic_expression::root(type, length) {
      placeholders_.push_back(dynamic_placeholder(type, length));
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
    dynamic_column_wrapper(root* p) : dynamic_expression_wrapper<SqlType>(p) { }
  };

  template<class Table, class SqlType, class Base>
  dynamic_column_wrapper<SqlType>
  make_dynamic(const expression< column<Table, SqlType, Base> >& col) {
    return dynamic_column_wrapper<SqlType>(new dynamic_expression_impl< column<Table, SqlType, Base> >(col));
  }
  
  class dynamic_columns {
  
  private:
    std::vector<dynamic_expression> cols_;
  
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
      std::for_each(cols_.begin(), cols_.end(), comma_output(os));
    }
  };

  template<>
  struct is_column_container<dynamic_columns> : mpl::true_ {
  };
  
  namespace result_of {
  
    template<>
    struct make_expression<dynamic_columns, dynamic_expressions> {
      typedef dynamic_expressions type;
      static const dynamic_expressions& make(const dynamic_expressions& exprs) { return exprs; }
    };

    template<>
    struct make_expression<dynamic_expressions, dynamic_expressions> {
      typedef dynamic_expressions type;
      static const dynamic_expressions& make(const dynamic_expressions& exprs) { return exprs; }
    };
  }

  template<>
  struct is_placeholder_mark<dynamic_expressions> : false_type {
  };
  
} } }


#endif
