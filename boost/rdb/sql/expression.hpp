//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_EXPRESSION_HPP
#define BOOST_RDB_EXPRESSION_HPP

#include <boost/rdb/sql/common.hpp>

namespace boost { namespace rdb { namespace sql {

  struct extract_placeholders_from_list {

    template<typename Sig>
    struct result;

    template<class Self, class Expr, class Placeholders>
    struct result<Self(Expr&, Placeholders&)> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::join<
          const Placeholders,
          const typename Expr::placeholder_vector
        >::type
      >::type type;
    };

    template<class Expr, class Placeholders>
    typename result<extract_placeholders_from_list(Expr&, Placeholders&)>::type
    operator ()(Expr& expr, Placeholders& placeholders) {
      return fusion::as_vector(fusion::join(placeholders, expr.placeholders()));
    }
  };

  namespace result_of {
    template<class ExprList>
    struct placeholders_from_list {
      typedef typename fusion::result_of::accumulate<
        ExprList,
        fusion::vector<>,
        extract_placeholders_from_list
      >::type type;
    };
  }
  
  template<class ExprList>
  typename result_of::placeholders_from_list<ExprList>::type
  placeholders_from_list(const ExprList& exprs) {
    return fusion::accumulate(exprs, fusion::make_vector(), extract_placeholders_from_list());
  }
  
  template<class Expr>
  struct expression;

  template<class Expr>
  struct Expression
  {
    Expr expr;
    std::ostream& stream;
    typedef typename Expr::sql_type sql_type;
    typedef typename Expr::placeholder_vector placeholder_vector;
    BOOST_STATIC_CONSTANT(int, precedence = Expr::precedence);

    BOOST_CONCEPT_USAGE(Expression) {
      expr.str(stream);
      expr.placeholders();
    }
  };

  template<class Expr>
  struct NumericExpression : Expression<Expr>
  {
    //typedef typename type_traits<typename Expr::sql_type>::is_numeric is_numeric;
  };

  template<class Expr>
  struct ComparableExpression : Expression<Expr>
  {
    typedef typename type_traits<typename Expr::sql_type>::comparable_type comparable_type;
  };

  template<class Expr>
  struct BooleanExpression : Expression<Expr> {
    BOOST_CONCEPT_USAGE(BooleanExpression) {
      BOOST_MPL_ASSERT((boost::is_same<typename Expr::sql_type, type::boolean>));
    }
  };

  template<class Expr, typename T>
  struct CompatibleLiteral
  {
    const T& value;
    BOOST_CONCEPT_USAGE(CompatibleLiteral) {
      type_traits<typename Expr::sql_type>::make_literal(value);
    }
  };

  template<class Expr>
  struct is_placeholder_mark : is_same<typename Expr::sql_type, placeholder_type> {
  };
    
  namespace result_of {
    template<class Expr, typename T>
    struct make_expression_ {
      typedef typename type_traits<typename Expr::sql_type>::literal_type type;
      // TODO improve compile error when T is not compatible
      static const type make(const T& value) {
        return type_traits<typename Expr::sql_type>::make_literal(value);
      }
    };
  
    template<class Expr, class Expr2>
    struct make_expression_< Expr, expression<Expr2> > {
      typedef Expr2 type;
      static Expr2 make(const Expr2& expr) { return expr; }
    };
  
    template<class Expr, typename T>
    struct make_expression : make_expression_<
      Expr,
      typename boost::remove_const<
        typename boost::remove_reference<T>::type
      >::type
    > {
    };
  }

  template<class Expr, class Subquery>
  struct in_subquery {
    in_subquery(const Expr& expr, const Subquery& subquery) : expr_(expr), subquery_(subquery) { }
    const Expr& expr_;
    const Subquery& subquery_;
    typedef type::boolean sql_type;
    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);
    
    typedef typename fusion::result_of::join<
      const typename Expr::placeholder_vector,
      const typename Subquery::placeholder_vector
    >::type placeholder_vector;

    placeholder_vector placeholders() const {
      return fusion::join(expr_.placeholders(), subquery_.placeholders());
    }

    void str(std::ostream& os) const {
      if (Expr::precedence < precedence) {
        os << "(";
        expr_.str(os);
        os << ")";
      } else {
        expr_.str(os);
      }
      os << " in (";
      subquery_.str(os);
      os << ")";
    }
  };

  template<class Expr>
  struct make_in_values_placeholders {
    template<typename Sig>
    struct result;
    
    template<class Value, class IsPlaceholder, class Placeholders>
    struct discriminate;
    
    template<class Value, class Placeholders>
    struct discriminate<Value, true_type, Placeholders> {
      typedef typename fusion::result_of::push_back<
        Placeholders, type::placeholder<typename Expr::sql_type>
      >::type type;
      
      static type make(Value& value, Placeholders& placeholders) {
        return fusion::push_back(placeholders, value.placeholders());
      }
    };
    
    template<class Value, class Placeholders>
    struct discriminate<Value, false_type, Placeholders> {
      typedef Placeholders type;
      
      static type make(Value& value, Placeholders& placeholders) {
        return placeholders;
      }
    };

    template<class Self, class Value, class Placeholders>
    struct result<Self(Value&, const Placeholders&)> {
      typedef typename discriminate<Value, typename is_placeholder_mark<Value>::type, Placeholders>::type type;
    };

    template<class Value, class Placeholders>
    typename discriminate<Value, typename is_placeholder_mark<Value>::type, Placeholders>::type
    operator ()(Value& value, Placeholders& placeholders) {
      return discriminate<Value, typename is_placeholder_mark<Value>::type, Placeholders>::make(value, placeholders);
    }
  };

  template<class Expr, class ExprList>
  struct in_values {
    in_values(const Expr& expr, const ExprList& alt) : expr_(expr), alt_(alt) { }
    Expr expr_;
    ExprList alt_;
    typedef type::boolean sql_type;
    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);

    typedef typename fusion::result_of::join<
      const typename Expr::placeholder_vector,
      const typename fusion::result_of::accumulate<
        ExprList,
        fusion::vector<>,
        make_in_values_placeholders<Expr>
      >::type
    >::type placeholder_vector;

    placeholder_vector
    //fusion::joint_view< fusion::vector<>, fusion::vector<> >
    placeholders() const {
      return fusion::join(
        expr_.placeholders(),
        fusion::accumulate(alt_, fusion::make_vector(), make_in_values_placeholders<Expr>()));
    }

    void str(std::ostream& os) const {
      if (Expr::precedence < precedence) {
        os << "(";
        expr_.str(os);
        os << ")";
      } else {
        expr_.str(os);
      }
      os << " in (";
      fusion::for_each(alt_, comma_output(os));
      os << ")";
    }
  };

  namespace detail {

    template<class Expr1, class IsPlaceHolder1, class Expr2, class IsPlaceHolder2>
    struct binary_operation_placeholders_impl;

    template<class Expr1, class Expr2>
    struct binary_operation_placeholders : binary_operation_placeholders_impl<
      Expr1, typename is_placeholder_mark<Expr1>::type,
      Expr2, typename is_placeholder_mark<Expr2>::type
    > {
    };

    template<class Expr1, class Expr2>
    struct binary_operation_placeholders_impl<Expr1, false_type, Expr2, false_type> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::join<
          const typename Expr1::placeholder_vector,
          const typename Expr2::placeholder_vector
        >::type
      >::type type;
      static type make(const Expr1& expr1, const Expr2& expr2) {
        return fusion::as_vector(fusion::join(expr1.placeholders(), expr2.placeholders()));
      }
    };

    template<class Expr1, class Expr2>
    struct binary_operation_placeholders_impl<Expr1, false_type, Expr2, true_type> {
      typedef typename fusion::result_of::push_back<
        const typename Expr1::placeholder_vector,
        type::placeholder<typename Expr1::sql_type>
      >::type type;
      static type make(const Expr1& expr1, const Expr2& expr2) {
        return fusion::push_back(expr1.placeholders(), rdb::type::placeholder<typename Expr1::sql_type>());
      }
    };

    template<class Expr1, class Expr2>
    struct binary_operation_placeholders_impl<Expr1, true_type, Expr2, false_type> {
      typedef typename fusion::result_of::push_front<
        typename Expr2::placeholder_vector,
        type::placeholder<typename Expr2::sql_type>
      >::type type;
      static type make(const Expr1& expr1, const Expr2& expr2) {
        return fusion::push_front(expr2.placeholders(), rdb::type::placeholder<typename Expr2::sql_type>());
      }
    };
  }

  template<class Expr1, class Expr2, int Precedence>
  struct binary_operation {

    BOOST_STATIC_CONSTANT(int, precedence = Precedence);

    binary_operation(const Expr1& expr1, const Expr2& expr2) : expr1_(expr1), expr2_(expr2) { }
    
    Expr1 expr1_;
    Expr2 expr2_;

    static void write(std::ostream& os, const Expr1& expr1, const char* op, const Expr2& expr2) {
      write(os, expr1, boost::mpl::bool_<static_cast<int>(Expr1::precedence) < precedence>());
      os << op;
      write(os, expr2, boost::mpl::bool_<static_cast<int>(Expr2::precedence) < precedence>());
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

    //typedef typename mpl::if_<
    //  is_placeholder_mark<Expr1>,
    //  typename fusion::result_of::push_front<
    //    typename Expr2::placeholder_vector,
    //    type::placeholder<typename Expr2::sql_type>
    //  >::type,
    //  typename mpl::if_<
    //    is_placeholder_mark<Expr2>,
    //    typename fusion::result_of::push_back<
    //      typename Expr1::placeholder_vector,
    //      type::placeholder<typename Expr1::sql_type>
    //    >::type,
    //    typename fusion::result_of::join<
    //      typename Expr1::placeholder_vector,
    //      typename Expr2::placeholder_vector
    //    >::type
    //  >::type
    //>::type placeholder_vector;

    typedef detail::binary_operation_placeholders<Expr1, Expr2> placeholder_helper;

    typedef typename placeholder_helper::type placeholder_vector;

    placeholder_vector placeholders() const { return placeholder_helper::make(expr1_, expr2_); }
  };

  template<class Expr1, class Expr2>
  struct like : binary_operation<Expr1, Expr2, precedence_level::compare> {
    like(const Expr1& expr1, const Expr2& expr2) :
      binary_operation<Expr1, Expr2, precedence_level::compare>(expr1, expr2) { }
    typedef type::boolean sql_type;
    void str(std::ostream& os) const {
      this->expr1_.str(os);
      os << " like ";
      this->expr2_.str(os);
    }
  };

  template<class Expr>
  struct expression : Expr {
    typedef expression this_type;
    expression() { }
    template<typename T> explicit expression(const T& arg) : Expr(arg) { }
    template<typename T1, typename T2> expression(const T1& arg1, const T2& arg2) : Expr(arg1, arg2) { }
    const Expr& unwrap() const { return *this; }
    
    template<typename T>
    typename result_of::make_expression<Expr, T>::type
    static make_expression(const T& any) {
      return result_of::make_expression<Expr, T>::make(any);
    }
    
    template<class Pattern>
    expression< sql::like<Expr, typename result_of::make_expression<Expr, Pattern>::type> >
    like(const Pattern& pattern) const {
      BOOST_MPL_ASSERT((boost::is_same<typename type_traits<typename Expr::sql_type>::kind, char_type>));
      return expression< sql::like<Expr, typename result_of::make_expression<Expr, Pattern>::type> >(*this, make_expression(pattern));
    }
    
    template<class Tag, class T>
    struct dispatch_in {
      typedef typename result_of::make_expression<Expr, T>::type value_type;
      typedef typename fusion::result_of::make_vector<value_type>::type value_list_type;
      typedef expression< sql::in_values<Expr, value_list_type> > return_type;
      static return_type make(const Expr& expr, const T& value) {
        return return_type(expr, fusion::make_vector(make_expression(value)));
      }
    };

    template<class Subquery>
    struct dispatch_in<select_statement_tag, Subquery> {
      typedef expression< sql::in_subquery<Expr, Subquery> > return_type;
      static return_type make(const Expr& expr, const Subquery& subquery) { return return_type(expr, subquery); }
    };

    template<class T>
    typename dispatch_in<typename tag_of<T>::type, T>::return_type
    in(const T& any) const {
      return dispatch_in<typename tag_of<T>::type, T>::make(*this, any);
    }
    
    #define BOOST_PP_ITERATION_LIMITS (2, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/in_values.hpp>
    #include BOOST_PP_ITERATE()

    using Expr::operator =; // for set col = value
  };
  
  struct null_type { };

  template<>
  struct type_traits<null_type> {
    typedef null_type comparable_type;
    typedef universal kind;
  };
  
  struct null_expr {
    typedef null_type sql_type;
    typedef fusion::vector<> placeholder_vector;
    placeholder_vector placeholders() const { return fusion::make_vector(); }
    BOOST_STATIC_CONSTANT(int, precedence = precedence_level::highest);
    void str(std::ostream& os) const {
      os << "null";
    }
  };
  
  const expression<null_expr> null = expression<null_expr>();

  const expression< placeholder_mark<0> > _;
  
  namespace result_of {
    template<class Key, class Value>
    struct extract_placeholders_from_pair {
      typedef fusion::vector<> type;
      static type make(const fusion::pair<Key, Value>&) { return type(); }
    };
  }
  
  template<class Key, class Value>
  typename result_of::extract_placeholders_from_pair<Key, Value>::type
  extract_placeholders_from_pair(const fusion::pair<Key, Value>& p) {
    return result_of::extract_placeholders_from_pair<Key, Value>::make(p);
  }
    
  struct extract_placeholders_from_pair_list {

    template<typename Sig>
    struct result;

    template<class Self, class Key, class Value, class Placeholders>
    struct result<Self(const fusion::pair<Key, Value>&, const Placeholders&)> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::join<
          const Placeholders,
          const typename result_of::extract_placeholders_from_pair<Key, Value>::type
        >::type
      >::type type;
    };
    
    template<class Key, class Value, class Placeholders>
    typename result<extract_placeholders_from_pair_list(const fusion::pair<Key, Value>&, const Placeholders&)>::type
    operator ()(const fusion::pair<Key, Value>& p, const Placeholders& placeholders) {
      return fusion::as_vector(fusion::join(placeholders, extract_placeholders_from_pair(p)));
    }
  };

  namespace result_of {
    template<class PairList>
    struct placeholders_from_pair_list {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::accumulate<
          const PairList,
          fusion::vector<>,
          extract_placeholders_from_pair_list
        >::type
      >::type type;
    };
  }
  
  template<class Map>
  typename result_of::placeholders_from_pair_list<Map>::type
  placeholders_from_pair_list(const Map& map) {
    return fusion::as_vector(fusion::accumulate(map, fusion::make_vector(), extract_placeholders_from_pair_list()));
  }
  
  template<class T>
  struct is_column_container< expression<T> > : is_column_container<T> {
  };

} } }

#include <boost/rdb/sql/operators.hpp>

#endif
