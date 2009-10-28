//  Copyright Jean-Louis Leroy 2009.
// Use, modification, and distribution are subject to the Boost Software License, Version 1.0.

#ifndef BOOST_RDB_UPDATE_HPP
#define BOOST_RDB_UPDATE_HPP

#include <boost/rdb/sql/common.hpp>
#include <boost/rdb/sql/expression.hpp>

namespace boost { namespace rdb { namespace sql {

  
  struct dynamic_update {

    struct root : rdb::detail::ref_counted {
      dynamic_placeholders placeholders_;
      virtual void str(std::ostream& os) const = 0;
    };
    
    dynamic_update(root* impl) : impl_(impl) { }

    //typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;
    
    const dynamic_placeholders& placeholders() const {
      return impl_->placeholders_;
    }
    
    void str(std::ostream& os) const {
      impl_->str(os);
    }
    
    intrusive_ptr<root> impl_;
    
    template<class Col, class Expr>
    struct wrapper : root {

      wrapper(const set_clause<Col, Expr>& update) : update_(update) {
        fusion::for_each(update_.placeholders(), make_dynamic_placeholders(this->placeholders_));
      }
      
      set_clause<Col, Expr> update_;
      
      virtual void str(std::ostream& os) const {
        update_.str(os);
      }
    };
  };
    
  template<class Col, class Expr>
  dynamic_update make_dynamic(const set_clause<Col, Expr>& update) {
    return dynamic_update(new dynamic_update::wrapper<Col, Expr>(update));
  }
  
  class dynamic_updates {
  
  private:
    std::vector<dynamic_update> updates_;
  
  public:
  
    typedef fusion::vector< const std::vector<dynamic_placeholder> > placeholder_vector;

    placeholder_vector placeholders() const {
      int size = 0;
      std::vector<dynamic_update>::const_iterator in = updates_.begin();

      while (in != updates_.end()) {
        in->placeholders().size();
        size += in++->placeholders().size();
      }

      std::vector<dynamic_placeholder> result(size);
      std::vector<dynamic_placeholder>::iterator out = result.begin();
      in = updates_.begin();

      while (in != updates_.end()) {
        out = std::copy(in->placeholders().begin(), in->placeholders().end(), out);
        ++in;
      }

      return result;
    }
    
    void push_back(const dynamic_update& update) {
      updates_.push_back(update);
    }

    typedef void sql_type;
    
    void str(std::ostream& os) const {
      std::for_each(updates_.begin(), updates_.end(), comma_output(os));
    }
  };

  template<>  
  struct is_update_container<dynamic_updates> : mpl::true_ {
  };

  struct extract_placeholders_from_assign {

    template<typename Sig>
    struct result;
    
    typedef extract_placeholders_from_assign Self;

    template<class Col, class Expr, class Placeholders>
    struct result<Self(set_clause<Col, Expr>&, Placeholders&)> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::join<
          const Placeholders,
          const typename set_clause<Col, Expr>::placeholder_vector
        >::type
      >::type type;
      
      static type make(const set_clause<Col, Expr>& update, const Placeholders& placeholders) {
        return fusion::join(placeholders, update.placeholders());
      }
    };

    template<class Col, class Expr, class Placeholders>
    struct result<Self(const set_clause<Col, Expr>&, Placeholders&)> : result<Self(set_clause<Col, Expr>&, Placeholders&)> {
    };

    template<class Placeholders>
    struct result<Self(dynamic_updates&, const Placeholders&)> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::join<
          const Placeholders,
          const dynamic_updates::placeholder_vector
        >::type
      >::type type;
      
      static type make(const dynamic_updates& updates, const Placeholders& placeholders) {
        return fusion::join(placeholders, updates.placeholders());
      }
    };

    // why are both needed ?
    template<class Placeholders>
    struct result<Self(const dynamic_updates&, const Placeholders&)> : result<Self(dynamic_updates&, const Placeholders&)> {
    };

    template<class T, class Placeholders>
    typename result<Self(T&, Placeholders&)>::type
    operator ()(T& update, Placeholders& placeholders) {
      using namespace fusion;
      return result<Self(T&, Placeholders&)>::make(update, placeholders);
    }
  };
  
  namespace result_of {
    template<class AssignList>
    struct extract_placeholders_from_pair<sql2003::set, AssignList> {
      typedef typename fusion::result_of::as_vector<
        typename fusion::result_of::accumulate<AssignList, fusion::vector<>, extract_placeholders_from_assign>::type
      >::type type;
      static type make(const fusion::pair<sql2003::set, AssignList>& p) {
        return fusion::accumulate(p.second, fusion::vector<>(), extract_placeholders_from_assign());
      }
    };
  }

  template<class Dialect, class State, class Data, class Subdialect>
  struct update_statement :
    tag_if<fusion::result_of::has_key<Data, typename Subdialect::set>, update_statement_tag> {

    explicit update_statement(const Data& data) : data_(data) { }

    typedef void result;
    typedef typename result_of::placeholders_from_pair_list<Data>::type placeholder_vector;

    placeholder_vector placeholders() const {
      using namespace fusion;
      return placeholders_from_pair_list(data_);
    }

    Data data_;

    template<class K, class T, class D = Data>
    struct transition {
      typedef update_statement<
        Subdialect,
        K,
        typename result_of::add_key<
          D,
          K,
          T
        >::type,
        Subdialect
      > type;
    };

    #define BOOST_PP_ITERATION_LIMITS (1, BOOST_RDB_MAX_SIZE - 1)
    #define BOOST_PP_FILENAME_1       <boost/rdb/sql/detail/update_set.hpp>
    #include BOOST_PP_ITERATE()
    
    #include "detail/select_where.hpp"

    void str(std::ostream& os) const {
      fusion::for_each(data_, str_clause(os));
    }
  };

  BOOST_RDB_ALLOW(sql2003, set, where);

  template<class Table>

  inline void str(std::ostream& os, const fusion::pair<sql2003::update, const Table*>& p) {
    os << "update ";
    os << p.second->table();
  }

  template<class AssignList>
  inline void str(std::ostream& os, const fusion::pair<sql2003::set, AssignList>& p) {
    os << " set ";
    fusion::for_each(p.second, comma_output(os));
  }
  
  template<class Table>
  update_statement<
    sql2003,
    sql2003::update,
    fusion::map<
      fusion::pair<
      sql2003::update, const Table*
      >
    >,
    sql2003
  >
  update(const Table& table) {
    return update_statement<
      sql2003,
      sql2003::update,
      fusion::map<
        fusion::pair<
        sql2003::update, const Table*
        >
      >,
      sql2003
    >(fusion::make_pair<sql2003::update>(&table));
  }

} } }

#endif
