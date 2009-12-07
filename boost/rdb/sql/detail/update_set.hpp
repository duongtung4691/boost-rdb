//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#define n BOOST_PP_ITERATION()

template<BOOST_PP_ENUM_PARAMS(n, class Update)>
typename transition<
  typename Subdialect::set,
  fusion::vector<
    BOOST_PP_ENUM_PARAMS(n, Update)
  >
>::type
set(BOOST_PP_ENUM_BINARY_PARAMS(n, const Update, &update)) {
  // failure here probably indicates that you are trying to pass something else than
  // assignments like `column = expression` or dynamic_updates
  BOOST_PP_REPEAT(n, BOOST_PP_RDB_MPL_ASSERT, (is_update_container, Update))
  return typename transition<
    typename Subdialect::set,
    fusion::vector<
      BOOST_PP_ENUM_PARAMS(n, Update)
    >
  >::type(ct::add_key<typename Subdialect::set>(data_, fusion::make_vector(BOOST_PP_ENUM_PARAMS(n, update))));
}
