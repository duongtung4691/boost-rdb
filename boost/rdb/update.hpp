//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_UPDATE_HPP
#define BOOST_RDB_UPDATE_HPP

namespace boost { namespace rdb {

  template<class Table, class ColList, class ExprList, class Predicate>
  struct update_type {
  
    ColList cols_;
    ExprList values_;
    Predicate where_;

    typedef ColList col_list_type;
    typedef ExprList value_list_type;
    
    update_type(const ColList& cols, const ExprList& values, const Predicate& where)
      : cols_(cols), values_(values), where_(where) { }

    void str(std::ostream& os) const { str(os, boost::is_same<Predicate, details::none>()); }
    
    void str(std::ostream& os, boost::true_type) const {
      os << "update " << Table::table_name() << " ";
      typedef boost::fusion::vector<const ColList&, const ExprList&> assignment;
      boost::fusion::for_each(boost::fusion::zip_view<assignment>(assignment(cols_, values_)), assign_output(os));
    }

    void str(std::ostream& os, boost::false_type) const {
      os << "update " << Table::table_name() << " ";
      typedef boost::fusion::vector<const ColList&, const ExprList&> assignment;
      boost::fusion::for_each(boost::fusion::zip_view<assignment>(assignment(cols_, values_)), assign_output(os));
      os << " where ";
      where_.str(os);
    }

    template<class Col, class Expr>
    struct with {
      typedef update_type<Table,
        typename boost::fusion::result_of::push_back<const ColList, 
          typename result_of::unwrap<Col>::type
        >::type,
        typename boost::fusion::result_of::push_back<const ExprList,
          typename result_of::make_expression<expression<Col>, Expr>::type
        >::type,
        details::none
      > type;
    };

    template<class Col, class T>
    typename with<Col, T>::type
    set(const expression<Col>& col, const T& expr) const {
      BOOST_MPL_ASSERT((boost::is_same<Predicate, details::none>));
      return typename with<Col, T>::type(
        boost::fusion::push_back(cols_, col.unwrap()),
        boost::fusion::push_back(values_, expression<Col>::make_expression(expr)), details::none());
    }

    template<class Where>
    update_type<Table, ColList, ExprList, Where>
    where(const Where& where) const {
      BOOST_MPL_ASSERT((boost::is_same<Predicate, details::none>));
      return update_type<Table, ColList, ExprList, Where>(cols_, values_, where);
    }
  };

  template<class Table>
  update_type<Table, details::empty, details::empty, details::none>
  update(const Table& table) {
    return update_type<Table, details::empty, details::empty, details::none>(
      details::empty(), details::empty(), details::none());
  }

} }

#endif
