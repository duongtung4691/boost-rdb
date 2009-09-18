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
  class generic_database {
    Specific& spec() { return static_cast<Specific&>(*this); }
  public:
    template<typename Table>
    void create_table() { spec().execute(rdb::create_table<Table>().str()); }

    template<typename Table>
    void drop_table() { spec().execute(std::string("drop table ") + Table::table_name()); }
  };

  class database : public generic_database<database> {
  public:
    database() { }
    ~database();

    database(const std::string& dsn, const std::string& user, const std::string& password) {
      open(dsn, user, password);
    }

    void open(const std::string& dsn, const std::string& user, const std::string& password);
    void close();
    
    template<class Statement>
    BOOST_CONCEPT_REQUIRES((Statement), void)
    execute(const statement<Statement>& st);

  private:
    std::string dsn_, user_, password_;
    SQLHENV	henv_;
    SQLHDBC hdbc_;
    SQLHSTMT hstmt_;
  };

} } }

#endif // BOOST_ODBC_HPP
