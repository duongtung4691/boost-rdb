#ifndef BOOST_ODBC_HPP
#define BOOST_ODBC_HPP

#include <sql.h>
#include <sqlext.h>

namespace boost { namespace rdb { namespace odbc {

      struct error : std::exception {
        error(SQLSMALLINT handle_type, SQLHANDLE handle, long rc);
        virtual const char* what() const throw();
        long rc;
        SQLCHAR stat[10]; // Status SQL
        SQLINTEGER err;
        char msg[200];
      };

  template<class Specific>
  struct generic_database {
  };

  class database : public generic_database<database> {
  public:
    database() { }

    database(const std::string& dsn, const std::string& user, const std::string& password) {
      connect(dsn, user, password);
    }

    void connect(const std::string& dsn, const std::string& user, const std::string& password);

  private:
    std::string dsn_, user_, password_;
    SQLHENV	henv_;
    SQLHDBC hdbc_;
    SQLHSTMT hstmt_;
  };

} } }

#endif // BOOST_ODBC_HPP
