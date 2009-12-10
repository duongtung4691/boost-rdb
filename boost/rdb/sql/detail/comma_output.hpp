#ifndef BOOST_RDB_DETAIL_OUTPUT_HPP
#define BOOST_RDB_DETAIL_OUTPUT_HPP

namespace boost { namespace rdb { namespace sql { namespace detail {

  struct comma_output {
    comma_output(std::ostream& os) : os_(os), comma_("") { }
    std::ostream& os_;
    mutable const char* comma_;
    
    std::ostream& item() const {
      os_ << comma_;
      comma_ = ", ";
      return os_;
    }
    
    template<typename Item> void operator ()(const Item& i) const {
      str(item(), i);
    }
  };

} } } }

#endif
