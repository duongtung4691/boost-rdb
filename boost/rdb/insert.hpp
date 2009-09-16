//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_INSERT_HPP
#define BOOST_RDB_INSERT_HPP

namespace boost { namespace rdb {

  template<class Table, class ColList, class ValueList>
  struct insert_type;

  template<class Table, class ColList>
  struct insert_type<Table, ColList, void> {

    insert_type(const ColList& cols) : cols_(cols) { }

    void str(std::ostream& os) const {
      os << "insert into " << Table::table_name() << " (";
      boost::fusion::for_each(cols_, comma_output(os));
      os << ")";
    }

    template<typename Col>
    BOOST_CONCEPT_REQUIRES(
      ((Column<Col>)),
      (insert_type<
        Table,
        typename boost::fusion::result_of::push_back< const ColList, boost::reference_wrapper<const Col> >::type,
        void
      >))
    operator ()(const Col& col) const {
      return insert_type<
        Table,
        typename boost::fusion::result_of::push_back< const ColList, boost::reference_wrapper<const Col> >::type,
        void
      >(boost::fusion::push_back(cols_, boost::cref(col)));
    }

    ColList cols_;
  };

  template<class Table, class Col>
  BOOST_CONCEPT_REQUIRES(
    ((Column<Col>)),
    (insert_type<Table,
      typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Col> >::type,
      void
      >))
  insert_into(const expression<Col>& col) {
    BOOST_MPL_ASSERT((boost::is_same<Table, typename Col::table_type>));
    return insert_type<Table,
      typename boost::fusion::result_of::make_vector< boost::reference_wrapper<const Col> >::type,
      void
      >(boost::fusion::make_vector(boost::cref(col)));
  }

} }

#endif