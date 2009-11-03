//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

template<BOOST_PP_ENUM_PARAMS(n, class Expr)>
result_set<select_list, container, false>
execute(BOOST_PP_ENUM_BINARY_PARAMS(n, const Expr, &expr)) {
  executev(fusion::make_vector(BOOST_PP_ENUM_PARAMS(n, expr)));
  return result_set<select_list, container, false>(this->hstmt_);
}
