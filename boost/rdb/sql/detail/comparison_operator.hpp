
template<class Expr1, class Expr2>
struct BOOST_RDB_OPERATOR_CLASS : binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE> {

  BOOST_RDB_OPERATOR_CLASS(const Expr1& expr1, const Expr2& expr2) :
    binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE>(expr1, expr2) { }

  typedef core::boolean rdb_type;
  
  void str(std::ostream& os) const {
    this->write(os, this->expr1_, BOOST_RDB_OPERATOR_STRING, this->expr2_);
  }
};

template<class Expr, typename T>
expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename core::make_literal<typename Expr::rdb_type, T>::type> >
operator BOOST_RDB_OPERATOR(const expression<Expr>& expr, const T& val) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename core::make_literal<typename Expr::rdb_type, T>::type> >(expr, core::make_literal<typename Expr::rdb_type, T>::value(val));
}

template<class Expr, typename T>
BOOST_CONCEPT_REQUIRES(
  ((ComparableExpression<Expr>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<typename core::make_literal<typename Expr::rdb_type, T>::type, Expr> >))
operator BOOST_RDB_OPERATOR(const T& val, const expression<Expr>& expr) {
  return expression< BOOST_RDB_OPERATOR_CLASS<typename core::make_literal<typename Expr::rdb_type, T>::type, Expr> >(core::make_literal<typename Expr::rdb_type, T>::value(val), expr);
}

template<class Expr1, class Expr2>
BOOST_CONCEPT_REQUIRES(
  ((ComparableExpression<Expr1>))
  ((ComparableExpression<Expr2>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2> >))
operator BOOST_RDB_OPERATOR(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
  BOOST_MPL_ASSERT((is_sql_compatible<Expr1, Expr2>));
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2> >(expr1, expr2);
}

#undef BOOST_RDB_OPERATOR
#undef BOOST_RDB_OPERATOR_STRING
#undef BOOST_RDB_OPERATOR_CLASS
#undef BOOST_RDB_OPERATOR_PRECEDENCE
