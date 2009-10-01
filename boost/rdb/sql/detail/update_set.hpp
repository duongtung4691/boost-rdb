//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

    template<BOOST_PP_ENUM_PARAMS(n, class Set)>
    typename transition::set<
      Context,
      typename result_of::add_key<
        Data,
        update_impl::set,
        fusion::vector<
          BOOST_PP_ENUM_PARAMS(n, Set)
        >
      >::type
    >::type
    set(BOOST_PP_ENUM_BINARY_PARAMS(n, const Set, &set)) {
      return typename transition::set<
        Context,
        typename result_of::add_key<
          Data,
          update_impl::set,
          fusion::vector<
            BOOST_PP_ENUM_PARAMS(n, Set)
          >
        >::type
      >::type(add_key<update_impl::set>(data_, fusion::make_vector(BOOST_PP_ENUM_PARAMS(n, set))));
    }
