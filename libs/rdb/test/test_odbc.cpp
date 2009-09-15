#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>
#include <boost/rdb/odbc.hpp>

using namespace std;
using namespace boost::rdb;

#define scope

BOOST_RDB_BEGIN_TABLE(person) 
  BOOST_RDB_COLUMN(id, integer)
  BOOST_RDB_COLUMN(name, varchar<20>)
BOOST_RDB_END_TABLE(person) 

int test_main( int, char *[] )
{
  using namespace boost::rdb;
  using namespace boost::rdb::odbc;

  try {
    //database db("Provider=Microsoft.Jet.OLEDB.4.0; Data Source=test.mdb", "", "");
    //database db("Provider=MSDASQL;Driver={Microsoft Access Driver (*.mdb)};Dbq=c:\\users\\jll\\test.mdb;Uid=;Pwd=;", "", "");
    //database db("Driver={Microsoft Access Driver (*.mdb)}; DBQ=c:\\windows\\system32\\test_odbc\\test.mdb", "", "");
    database db("RDBTest", "", "");
    //database db("Driver={Microsoft Text Driver (*.txt; *.csv)};Dbq=c:\\users\\jll\\test\\;Extensions=asc,csv,tab,txt;", "", "");
  } catch (error& e) {
    cout << e.what() << endl;
  }
      
  return 0;
}
