
template<class Expr1, class Expr2, class SqlType>
struct BOOST_RDB_OPERATOR_CLASS : binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE> {

  BOOST_RDB_OPERATOR_CLASS(const Expr1& expr1, const Expr2& expr2) :
    binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE>(expr1, expr2) { }

  typedef SqlType rdb_type;
  
  void str(std::ostream& os) const {
    this->write(os, this->expr1_, BOOST_RDB_OPERATOR_STRING, this->expr2_);
  }
};

template<class Expr, typename T>
BOOST_CONCEPT_REQUIRES(
  ((NumericExpression<Expr>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename core::make_literal<typename Expr::rdb_type, T>::type, typename Expr::rdb_type> >))
operator BOOST_RDB_OPERATOR(const expression<Expr>& expr, const T& val) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename core::make_literal<typename Expr::rdb_type, T>::type, typename Expr::rdb_type> >(expr, core::make_literal<typename Expr::rdb_type, T>::value(val));
}

template<class Expr, typename T>
BOOST_CONCEPT_REQUIRES(
  ((NumericExpression<Expr>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<typename core::make_literal<typename Expr::rdb_type, T>::type, Expr, typename Expr::rdb_type> >))
operator BOOST_RDB_OPERATOR(const T& val, const expression<Expr>& expr) {
  return expression< BOOST_RDB_OPERATOR_CLASS<typename core::make_literal<typename Expr::rdb_type, T>::type, Expr, typename Expr::rdb_type> >(core::make_literal<typename Expr::rdb_type, T>::value(val), expr);
}

template<class Expr1, class Expr2>
BOOST_CONCEPT_REQUIRES(
  ((NumericExpression<Expr1>))
  ((NumericExpression<Expr2>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2, typename Expr1::rdb_type> >))
operator BOOST_RDB_OPERATOR(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2, typename Expr1::rdb_type > >(expr1, expr2);
}

#undef BOOST_RDB_OPERATOR
#undef BOOST_RDB_OPERATOR_STRING
#undef BOOST_RDB_OPERATOR_CLASS
#undef BOOST_RDB_OPERATOR_PRECEDENCE
