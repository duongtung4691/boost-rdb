    template<BOOST_PP_ENUM_PARAMS(n, class Table)>
    select_statement<
      Subdialect,
      typename Subdialect::select::from,
      typename result_of::add_key<
        Data,
        typename Subdialect::select::from,
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
        >
      >::type,
      Subdialect
    >
    from(BOOST_PP_ENUM_BINARY_PARAMS(n, const Table, &table)) {
      return select_statement<
      Subdialect,
      typename Subdialect::select::from,
        typename result_of::add_key<
          Data,
          typename Subdialect::select::from,
          fusion::vector<
            BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
          >
        >::type,
        Subdialect
      >(add_key<typename Subdialect::select::from>(
        data_,
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
        >(BOOST_PP_ENUM_PARAMS(n, table))
      ));
    }
