
template<class Expr1, class Expr2>
struct BOOST_RDB_OPERATOR_CLASS : binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE> {

  BOOST_RDB_OPERATOR_CLASS(const Expr1& expr1, const Expr2& expr2) :
    binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE>(expr1, expr2) { }

  typedef type::boolean sql_type;
  
  void str(std::ostream& os) const {
    this->write(os, this->expr1_, BOOST_RDB_OPERATOR_STRING, this->expr2_);
  }
};

template<class Expr, typename T>
typename disable_if<
  is_expression<T>,
  expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename make_literal<typename Expr::sql_type, T>::type> >
>::type
operator BOOST_RDB_OPERATOR(const expression<Expr>& expr, const T& val) {
  //BOOST_CONCEPT_ASSERT((ComparableExpression<Expr>));
  return expression< BOOST_RDB_OPERATOR_CLASS<
    Expr,
    typename make_literal<typename Expr::sql_type, T>::type
  > >(expr, make_literal<typename Expr::sql_type, T>::value(val));
}

template<class Expr, typename T>
typename disable_if<
  is_expression<T>,
  expression< BOOST_RDB_OPERATOR_CLASS<typename make_literal<typename Expr::sql_type, T>::type, Expr> >
>::type
operator BOOST_RDB_OPERATOR(const T& val, const expression<Expr>& expr) {
  //BOOST_CONCEPT_ASSERT((ComparableExpression<Expr>));
  return expression< BOOST_RDB_OPERATOR_CLASS<
    typename make_literal<typename Expr::sql_type, T>::type,
    Expr
  > >(make_literal<typename Expr::sql_type, T>::value(val), expr);
}

template<class Expr1, class Expr2>
expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2> >
operator BOOST_RDB_OPERATOR(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
  //BOOST_CONCEPT_ASSERT((ComparableExpression<Expr1>));
  //BOOST_CONCEPT_ASSERT((ComparableExpression<Expr2>));
  BOOST_MPL_ASSERT((is_sql_compatible<Expr1, Expr2>));
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2> >(expr1, expr2);
}

#undef BOOST_RDB_OPERATOR
#undef BOOST_RDB_OPERATOR_STRING
#undef BOOST_RDB_OPERATOR_CLASS
#undef BOOST_RDB_OPERATOR_PRECEDENCE
