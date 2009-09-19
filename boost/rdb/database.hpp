//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_DATABASE_HPP
#define BOOST_RDB_DATABASE_HPP

namespace boost { namespace rdb {

  template<class Specific>
  class generic_database {
    Specific& spec() { return static_cast<Specific&>(*this); }
  public:
    template<typename Table>
    void create_table() { spec().exec_str(rdb::create_table<Table>().str()); }

    template<typename Table>
    void drop_table() { spec().exec_str(std::string("drop table ") + Table::table_name()); }
    
    template<class St>
    BOOST_CONCEPT_REQUIRES(((Statement<St>)), (void))
    execute(const statement<St>& st) { spec().exec_str(as_str(st)); }
  };

} }

#endif