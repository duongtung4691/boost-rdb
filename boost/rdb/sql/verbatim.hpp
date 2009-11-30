//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_SQL_VERBATIM_HPP
#define BOOST_RDB_SQL_VERBATIM_HPP

#include <boost/rdb/sql/expression.hpp>

namespace boost { namespace rdb { namespace sql {

  template<class Type, class PlaceHolders>
  struct verbatim_expression {
  
    typedef Type sql_type;
    
    verbatim_expression(const std::string& expr, const PlaceHolders placeholders)
      : expr_(expr), placeholders_(placeholders) { }
    
    std::string expr_;
    
    void str(std::ostream& os) const {
      os << expr_;
    }
    
    PlaceHolders placeholders_;
    typedef PlaceHolders placeholder_vector;
    placeholder_vector placeholders() const { return placeholders_; }
  };
  
  template<class Type>
  verbatim_expression< Type, fusion::vector<> >
  verbatim(const std::string& sql) {
    return verbatim_expression< Type, fusion::vector<> >(sql, fusion::make_vector());
  }

} } }

#endif
