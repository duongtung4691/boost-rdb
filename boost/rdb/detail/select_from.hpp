    template<BOOST_PP_ENUM_PARAMS(n, class Table)>
    typename after_projection<
      Context,
      typename result_of::add_key<
        Data,
        tables,
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
        >
      >::type
    >::type
    from(BOOST_PP_ENUM_BINARY_PARAMS(n, const Table, &table)) {
      return typename after_projection<
        Context,
        typename result_of::add_key<
          Data,
          tables,
          fusion::vector<
            BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
          >
        >::type
      >::type(add_key<tables>(
        data_,
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
        >(BOOST_PP_ENUM_PARAMS(n, table))
      ));
    }
