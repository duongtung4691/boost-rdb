//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

    template<BOOST_PP_ENUM_PARAMS(n, class Expr)>
    typename transition<
      typename Subdialect::exprs,
      fusion::vector<
        BOOST_PP_ENUM_PARAMS(n, Expr)
      >,
      static_map<typename Subdialect::all, int, Data>
    >::type
    all(BOOST_PP_ENUM_BINARY_PARAMS(n, const Expr, &expr)) {
      return typename transition<
        typename Subdialect::exprs,
        fusion::vector<
          BOOST_PP_ENUM_PARAMS(n, Expr)
        >,
        static_map<typename Subdialect::all, int, Data>
      >::type(static_map_add_key<typename Subdialect::exprs>(
          fusion::make_vector(BOOST_PP_ENUM_PARAMS(n, expr)),
          static_map_add_key<typename Subdialect::all, int>(0, data_)));
    }
