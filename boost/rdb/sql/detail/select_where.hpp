
    template<class Predicate>
    typename transition<typename Subdialect::where, Predicate>::type
    where(const Predicate& predicate) const {
      BOOST_MPL_ASSERT((allow<Subdialect, State, typename Subdialect::where>));
      BOOST_CONCEPT_ASSERT((BooleanExpression<Predicate>));
      return typename transition<typename Subdialect::where, Predicate>::type(
        ct::add_key<typename Subdialect::where>(this->data_, predicate));
    }
