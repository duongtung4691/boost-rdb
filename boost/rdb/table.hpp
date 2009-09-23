//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_TABLE_HPP
#define BOOST_RDB_TABLE_HPP

namespace boost { namespace rdb {

  template<typename Table>
  struct create_table_statement {
    void str(std::ostream& os) const {
      os << "create table " << Table::table_name() << "(";
      boost::mpl::for_each<typename Table::column_members>(table_column_output<Table>(os, Table::_));
      os << ")";
    }
    std::string str() const { return as_string(*this); }
  };

  template<class Table>
  create_table_statement<Table> create_table(const Table&) {
    return create_table_statement<Table>();
  }

  template<typename Table>
  struct drop_table_statement {
    void str(std::ostream& os) const {
      os << "drop table " << Table::table_name();
    }
  };

  template<class Table>
  drop_table_statement<Table> drop_table(const Table&) {
    return drop_table_statement<Table>();
  }

} }

#endif
