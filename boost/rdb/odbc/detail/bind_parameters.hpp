//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

template<BOOST_PP_ENUM_PARAMS(n, class CliType)>
prepared_statement& bind_parameters(BOOST_PP_ENUM_BINARY_PARAMS(n, CliType, &value)) {
  // compilation error here probably indicates type mismatch between placeholders and parameters
  fusion::vector<BOOST_PP_REPEAT(n, BOOST_RDB_ADD_REF, CliType)> params(BOOST_PP_ENUM_PARAMS(n, value));
  bind_parameters_(params);
  return *this;
}
