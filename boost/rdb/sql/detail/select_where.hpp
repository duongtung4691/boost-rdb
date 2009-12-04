
    template<class Predicate>
    typename transition<typename Subdialect::where, Predicate>::type
    where(const Predicate& predicate) const {
      BOOST_MPL_ASSERT((allow<Subdialect, State, typename Subdialect::where>));
      BOOST_CONCEPT_ASSERT((BooleanExpression<Predicate>));
      return typename transition<typename Subdialect::where, Predicate>::type(
        ct::static_map_add_key<typename Subdialect::where>(predicate, data_));
    }
