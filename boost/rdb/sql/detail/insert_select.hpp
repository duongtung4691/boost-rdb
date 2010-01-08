//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

    template<BOOST_PP_ENUM_PARAMS(n, class Expr)>
    typename transition<
      typename Subdialect::insert_select,
      fusion::vector<
        BOOST_PP_ENUM_PARAMS(n, Expr)
      >
    >::type
    select(BOOST_PP_ENUM_BINARY_PARAMS(n, const Expr, &expr)) {
      BOOST_MPL_ASSERT((allow<Subdialect, State, typename Subdialect::insert_select>));
      return typename transition<
        typename Subdialect::insert_select,
        fusion::vector<
          BOOST_PP_ENUM_PARAMS(n, Expr)
        >
      >::type(ct::add_key<typename Subdialect::insert_select>(this->data_,
        fusion::vector<
          BOOST_PP_ENUM_PARAMS(n, Expr)
        >(
          BOOST_PP_ENUM_PARAMS(n, expr)
        )));
    }
