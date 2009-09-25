//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SELECT_HPP
#define BOOST_RDB_SELECT_HPP

namespace boost { namespace rdb {

  struct select_statement_tag : statement_tag { };

  struct make_row {

    template<typename T>
    struct extract_type;

    template<typename T>
    struct extract_type<const expression<T>&> {
      typedef T type;
    };

    template<typename T>
    struct extract_type< expression<T> > {
      typedef T type;
    };

    template<typename Sig>
    struct result;

    template<typename Self, typename Expr>
    struct result<Self(Expr)>
    {
      typedef typename boost::remove_reference<Expr>::type::cpp_type type;
    };
  };

  template<class SelectList>
  struct select_row {
    typedef typename fusion::result_of::as_vector<
      typename fusion::result_of::transform<SelectList, make_row>::type
    >::type type;
  };

  template<class SelectList, class FromList, class WhereList>
  struct select_statement;

  template<class SelectList>
  struct select_statement<SelectList, void, void>;

  template<class SelectList>
  struct select_statement<SelectList, void, void> {
    select_statement(const SelectList& exprs) : exprs(exprs) { }
    SelectList exprs;
    typedef select_statement<SelectList, void, void> this_type;

    void str(std::ostream& os) const {
      os << "select ";
      fusion::for_each(exprs, comma_output(os));
    }

    template<class T>
    struct with {
      typedef select_statement<
        typename fusion::result_of::push_back< const SelectList, literal<T> >::type,
        void, void
      > type;
    };

    template<class Expr>
    struct with< expression<Expr> > {
      typedef select_statement<
        typename fusion::result_of::push_back< const SelectList, Expr>::type,
        void, void
      > type;
    };

    template<class Expr> struct with< expression<Expr>& > : with< expression<Expr> > { };
    template<class Expr> struct with< const expression<Expr> > : with< expression<Expr> > { };
    template<class Expr> struct with< const expression<Expr>& > : with< expression<Expr> > { };

    template<typename T>
    typename with<T>::type
    operator ()(const T& expr) const {
      return typename with<T>::type(fusion::push_back(exprs, as_expression(expr)));
    }
        
#define BOOST_RDB_PP_SELECT_VALUES(z, n, unused) \
    template<BOOST_PP_ENUM_PARAMS(n, typename T)> \
    typename this_type BOOST_PP_REPEAT(n, BOOST_RDB_PP_WITH, T) \
    operator ()(BOOST_PP_ENUM_BINARY_PARAMS(n, const T, expr)) const { \
      return (*this)BOOST_PP_REPEAT(n, BOOST_RDB_PP_CALL, expr); \
    }

BOOST_PP_REPEAT_FROM_TO(2, BOOST_RDB_MAX_ARG_COUNT, BOOST_RDB_PP_SELECT_VALUES, ~)

    template<class ExprList>
    select_statement<
      typename fusion::result_of::join<const SelectList, const ExprList>::type,
      void, void
    >
    operator ()(const expression_list<ExprList>& more) const {
      return select_statement<
        typename fusion::result_of::join<const SelectList, const ExprList>::type,
        void, void
      >(fusion::join(exprs, more.unwrap()));
    }

    template<class Table>
    struct with_table {
      typedef select_statement<
        SelectList,
        typename result_of::make_list< boost::reference_wrapper<const Table> >::type,
        void
      > type;
    };

    template<class Table>
    typename with_table<Table>::type
    from(const Table& table) const {
      return typename with_table<Table>::type(exprs, make_list(boost::cref(table)));
    }

    template<class Table0, class Table1>
    typename with_table<Table0>::type::template with<Table1>::type
    from(const Table0& table0, const Table1& table1) const {
      return from(table0)(table1);
    }
  };

  template<class SelectList, class FromList>
  struct select_statement<SelectList, FromList, void> : select_statement<SelectList, void, void> {
    typedef select_statement<SelectList, void, void> just_select;
    select_statement(const SelectList& exprs, const FromList& tables) : just_select(exprs), tables(tables) { }

    typedef select_statement_tag tag;
    typedef SelectList select_type;

    FromList tables;
    typedef std::deque<typename select_row<SelectList>::type> result;

    template<class Table>
    struct with {
      typedef select_statement<
        SelectList,
        typename fusion::result_of::push_back<
          const FromList, boost::reference_wrapper<const Table>
        >::type,
        void
      > type;
    };

    template<class Table>
    typename with<Table>::type
    operator ()(const Table& table) const {
      return typename with<Table>::type(just_select::exprs, fusion::push_back(tables, boost::cref(table)));
    }
    
    template<class Pred>
    BOOST_CONCEPT_REQUIRES(
      ((Expression<Pred>)),
      (select_statement<
        SelectList,
        FromList,
        Pred>)
      )
    where(const Pred& pred) const {
      return select_statement<
        SelectList,
        FromList,
        Pred
      >(just_select::exprs, tables, pred);
    }

    void str(std::ostream& os) const {
      just_select::str(os);
      os << " from ";
      fusion::for_each(tables, comma_output(os));
    }
  };

  template<class SelectList, class FromList, class Predicate>
  struct select_statement : select_statement<SelectList, FromList, void> {
    typedef select_statement<SelectList, FromList, void> select_from;
    select_statement(const SelectList& exprs, const FromList& tables, const Predicate& pred)
      : select_from(exprs, tables), pred(pred) { }

    typedef select_statement_tag tag;
    typedef SelectList select_type;

    const Predicate& pred;

    void str(std::ostream& os) const {
      select_from::str(os);
      os << " where ";
      pred.str(os);
    }
  };
  
  extern select_statement<details::empty, void, void> select;

  namespace comma {

    template<class Expr1, class Expr2>
    typename result_of::make_expression_list<Expr1, Expr2>::type
      operator ,(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
      return make_expression_list(expr1, expr2);
    }

    template<class ExprList, class Expr>
    typename result_of::extend_expression_list<ExprList, Expr>::type
    operator ,(const expression_list<ExprList>& exprs, const expression<Expr>& expr) {
        return extend_expression_list(exprs, expr);
    }
  }

} }

#endif
