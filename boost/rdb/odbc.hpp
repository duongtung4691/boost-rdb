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

  inline bool sql_fail(long rc) {
    return rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO;
  }

  inline void sql_check(SQLSMALLINT handle_type, SQLHANDLE handle, long rc) {
    if (sql_fail(rc)) {
      error ex(handle_type, handle, rc);
      //TR << "ODBC Exception: " << ex.what() << endl;
      throw ex;
    }
  }

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
    
    template<class Insert>
    BOOST_CONCEPT_REQUIRES(((Statement<Insert>)), (void))
    execute(const insert_statement<Insert>& st) { execute(as_string(st)); }

    template<class SelectList, class FromList, class Predicate>
    std::deque<typename select_row<SelectList>::type>
    execute(const select_type<SelectList, FromList, Predicate>& select)
    {
      std::deque<typename select_row<SelectList>::type> results;
      execute(select, results);
      return results; // optimize later
    }

    struct read_row {
      read_row(database& db) : db_(db), i_(1) { }
      database& db_;
      mutable int i_;

      template<class Expr>
      void operator ()(boost::fusion::vector<Expr, long&> value) const {
        using namespace boost::fusion;
        SQLLEN n;
        SQLGetData(db_.hstmt_, i_++, SQL_C_LONG, &at_c<1>(value), 0, &n);
      }

      template<class Expr>
      void operator ()(boost::fusion::vector<Expr, std::string&> value) const {
        using namespace boost::fusion;
        char buf[Expr::sql_type::size];
        SQLLEN n;
        SQLGetData(db_.hstmt_, i_++, SQL_C_CHAR, buf, sizeof buf, &n);
        &at_c<1>(value).assign(buf, buf + n);
      }
    };
    
    template<class SelectList, class FromList, class Predicate, class ResultSet>
    void execute(const select_type<SelectList, FromList, Predicate>& select, ResultSet& results)
    {
      execute(as_string(select));
      typedef typename select_row<SelectList>::type row_type;

      while (true) {
        int rc = SQLFetch(hstmt_);

        if (rc == SQL_NO_DATA)
          break;

        if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
          throw error(SQL_HANDLE_STMT, hstmt_, rc);

        row_type row;
        typedef boost::fusion::vector<const SelectList&, row_type&> zip;
        boost::fusion::for_each(boost::fusion::zip_view<zip>(zip(select.exprs, row)), read_row(*this));
        results.push_back(row);
      }
    }
    
    void execute(const std::string& sql);

  private:
    std::string dsn_, user_, password_;
    SQLHENV	henv_;
    SQLHDBC hdbc_;
    SQLHSTMT hstmt_;
  };

} } }

#endif // BOOST_ODBC_HPP
