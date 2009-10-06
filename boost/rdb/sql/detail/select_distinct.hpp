//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

    template<BOOST_PP_ENUM_PARAMS(n, class Expr)>
    select_statement<
      Subdialect,
      typename Subdialect::select::distinct,
      typename result_of::add_key<
        typename result_of::add_key<
          Data,
          typename Subdialect::select::exprs,
          fusion::vector<
            BOOST_PP_REPEAT(n, BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION, Expr)
          >
        >::type,
        typename Subdialect::select::distinct,
        int
      >::type,
      Subdialect
    >
    distinct(BOOST_PP_ENUM_BINARY_PARAMS(n, const Expr, &expr)) {
      return select_statement<
        Subdialect,
        typename Subdialect::select::distinct,
        typename result_of::add_key<
          typename result_of::add_key<
            Data,
            typename Subdialect::select::exprs,
            fusion::vector<
              BOOST_PP_REPEAT(n, BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION, Expr)
            >
          >::type,
          typename Subdialect::select::distinct,
          int
        >::type,
        Subdialect
      >(
        add_key<typename Subdialect::select::distinct>(
          add_key<typename Subdialect::select::exprs>(
            data_,
            fusion::vector<
              BOOST_PP_REPEAT(n, BOOST_RDB_PP_RESULT_OF_AS_EXPRESSION, Expr)
            >(
              BOOST_PP_REPEAT(n, BOOST_RDB_PP_AS_EXPRESSION, expr)
            )),
          0));
    }
