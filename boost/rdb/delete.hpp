//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_DELETE_HPP
#define BOOST_RDB_DELETE_HPP

namespace boost { namespace rdb {

  struct delete_tag { };

  template<class Table, class Predicate>
  struct delete_statement {

    typedef delete_tag statement_tag;

    delete_statement(const Predicate& where) : where_(where) { }

    Predicate where_;

    void str(std::ostream& os) const { str(os, boost::is_same<Predicate, details::none>()); }
    
    void str(std::ostream& os, boost::true_type) const {
      os << "delete from " << Table::table_name();
    }

    void str(std::ostream& os, boost::false_type) const {
      os << "delete from " << Table::table_name();
      os << " where ";
      where_.str(os);
    }

    template<class Where>
    delete_statement<Table, Where>
    where(const Where& where) const {
      BOOST_MPL_ASSERT((boost::is_same<Predicate, details::none>));
      return delete_statement<Table, Where>(where);
    }
  };

  template<class Table>
  delete_statement<Table, details::none>
  delete_from(const Table& table) {
    return delete_statement<Table, details::none>(details::none());
  }

} }

#endif
