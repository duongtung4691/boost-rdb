//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

    template<BOOST_PP_ENUM_PARAMS(n, class Col)>
    typename transition::cols<
      Context,
      typename result_of::add_key<
        Data,
        cols,
        fusion::vector<
          BOOST_PP_ENUM_PARAMS(n, Col)
        >
      >::type
    >::type
    operator ()(BOOST_PP_ENUM_BINARY_PARAMS(n, const Col, &col)) {
      return typename transition::cols<
        Context,
        typename result_of::add_key<
          Data,
          cols,
          fusion::vector<
            BOOST_PP_ENUM_PARAMS(n, Col)
          >
        >::type
      >::type(add_key<cols>(data_, fusion::make_vector(BOOST_PP_ENUM_PARAMS(n, col))));
    }
