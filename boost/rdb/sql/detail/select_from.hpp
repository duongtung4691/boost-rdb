    template<BOOST_PP_ENUM_PARAMS(n, class Table)>
    typename transition<
      typename Subdialect::from,
      fusion::vector<BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)>
    >::type
    from(BOOST_PP_ENUM_BINARY_PARAMS(n, const Table, &table)) {
      return typename transition<
        typename Subdialect::from,
        fusion::vector<BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)>
      >::type(ct::static_map_add_key<typename Subdialect::from>(
        fusion::vector<
          BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
        >(BOOST_PP_ENUM_PARAMS(n, table)),
        data_)
        
        //static_map<
        //  typename Subdialect::from,
        //  BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table),
        //  Data
        //>(fusion::vector<
        //    BOOST_PP_REPEAT(n, BOOST_RDB_PP_REFERENCE, const Table)
        //  >(BOOST_PP_ENUM_PARAMS(n, table)),
        //  data_)
        );
    }
