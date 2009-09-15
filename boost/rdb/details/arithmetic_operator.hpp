
template<class Expr1, class Expr2, class SqlType>
struct BOOST_RDB_OPERATOR_CLASS {

  BOOST_RDB_OPERATOR_CLASS(const Expr1& expr1, const Expr2& expr2) : expr1_(expr1), expr2_(expr2) { }

  typedef SqlType sql_type;
  
  void str(std::ostream& os) const {
    expr1_.str(os);
    os << BOOST_RDB_OPERATOR_STRING;
    expr2_.str(os);
  }
  
  Expr1 expr1_;
  Expr2 expr2_;
};

template<class Expr, typename T>
expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename Expr::sql_type::literal_type, typename Expr::sql_type> >
operator BOOST_RDB_OPERATOR(const expression<Expr>& expr, const T& val) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr, typename Expr::sql_type::literal_type, integer> >(expr, Expr::sql_type::make_literal(val));
}

template<class Expr1, class Expr2>
expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2, typename Expr1::sql_type> >
operator BOOST_RDB_OPERATOR(const expression<Expr1>& expr1, const expression<Expr2>& expr2) {
  return expression< BOOST_RDB_OPERATOR_CLASS<Expr1, Expr2, typename Expr1::sql_type > >(expr1, expr2);
}

#undef BOOST_RDB_OPERATOR
#undef BOOST_RDB_OPERATOR_STRING
#undef BOOST_RDB_OPERATOR_CLASS
