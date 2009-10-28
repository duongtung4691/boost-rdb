
template<class Expr1, class Expr2, class SqlType>
struct BOOST_RDB_OPERATOR_CLASS : binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE> {

  BOOST_RDB_OPERATOR_CLASS(const Expr1& expr1, const Expr2& expr2) :
    binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE>(expr1, expr2) { }

  typedef SqlType sql_type;
  typedef typename type_traits<sql_type>::cpp_type cpp_type;
  
  void str(std::ostream& os) const {
    this->write(os, this->expr1_, BOOST_RDB_OPERATOR_STRING, this->expr2_);
  }
};

template<class Expr, typename T>
BOOST_CONCEPT_REQUIRES(
  ((NumericExpression<Expr>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename type_traits<typename Expr::sql_type>::literal_type, typename Expr::sql_type> >))
operator BOOST_RDB_OPERATOR(const expression<Expr>& expr, const T& val) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename type_traits<typename Expr::sql_type>::literal_type, typename Expr::sql_type> >(expr, type_traits<typename Expr::sql_type>::make_literal(val));
}

template<class Expr, typename T>
BOOST_CONCEPT_REQUIRES(
  ((NumericExpression<Expr>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<typename type_traits<typename Expr::sql_type>::literal_type, Expr, typename Expr::sql_type> >))
operator BOOST_RDB_OPERATOR(const T& val, const expression<Expr>& expr) {
  return expression< BOOST_RDB_OPERATOR_CLASS<typename type_traits<typename Expr::sql_type>::literal_type, Expr, typename Expr::sql_type> >(type_traits<typename Expr::sql_type>::make_literal(val), expr);
}

template<class Expr1, class Expr2>
BOOST_CONCEPT_REQUIRES(
  ((NumericExpression<Expr1>))
  ((NumericExpression<Expr2>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2, typename Expr1::sql_type> >))
operator BOOST_RDB_OPERATOR(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2, typename Expr1::sql_type > >(expr1, expr2);
}

#undef BOOST_RDB_OPERATOR
#undef BOOST_RDB_OPERATOR_STRING
#undef BOOST_RDB_OPERATOR_CLASS
#undef BOOST_RDB_OPERATOR_PRECEDENCE
