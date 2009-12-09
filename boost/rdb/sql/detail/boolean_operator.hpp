
template<class Expr1, class Expr2>
struct BOOST_RDB_OPERATOR_CLASS : binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE> {

  BOOST_RDB_OPERATOR_CLASS(const Expr1& expr1, const Expr2& expr2) :
    binary_operation<Expr1, Expr2, BOOST_RDB_OPERATOR_PRECEDENCE>(expr1, expr2) { }

  typedef core::boolean sql_type;
  
  void str(std::ostream& os) const {
    this->write(os, this->expr1_, BOOST_RDB_OPERATOR_STRING, this->expr2_);
  }
};

template<class Expr1, class Expr2>
BOOST_CONCEPT_REQUIRES(
  ((BooleanExpression<Expr1>))
  ((BooleanExpression<Expr2>)),
  (expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2> >))
operator BOOST_RDB_OPERATOR(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2> >(expr1, expr2);
}

#undef BOOST_RDB_OPERATOR
#undef BOOST_RDB_OPERATOR_STRING
#undef BOOST_RDB_OPERATOR_CLASS
#undef BOOST_RDB_OPERATOR_PRECEDENCE
