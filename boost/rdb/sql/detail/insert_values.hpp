//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

    template<BOOST_PP_ENUM_PARAMS(n, class Expr)>
    typename transition::values<
      Context,
      typename result_of::add_key<
        Data,
        insert_impl::values,
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION, Expr)
        >
      >::type
    >::type
    values(BOOST_PP_ENUM_BINARY_PARAMS(n, const Expr, &expr)) {
      return typename transition::values<
        Context,
        typename result_of::add_key<
          Data,
          insert_impl::values,
          fusion::vector<
            BOOST_PP_REPEAT(n, BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION, Expr)
          >
        >::type
      >::type(add_key<insert_impl::values>(data_,
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION, Expr)
        >(
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_AS_EXPRESSION, expr)
        )));
    }
