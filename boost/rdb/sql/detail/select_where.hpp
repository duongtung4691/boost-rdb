
    template<class Predicate>
    typename transition<typename Subdialect::select::where, Predicate>::type
    where(const Predicate& predicate) const {
      BOOST_MPL_ASSERT((allow<Subdialect, State, typename Subdialect::select::where>));
      return typename transition<typename Subdialect::select::where, Predicate>::type(
        add_key<typename Subdialect::select::where>(data_, predicate));
    }
