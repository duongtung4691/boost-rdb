#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>

using namespace std;
using namespace boost::rdb;

template<typename Table>
std::string create() {
  std::ostringstream os;
  create<Table>(os);
  return os.str();
}

template<class SelectList, class FromList, class WhereList>
std::string str(const select_type<SelectList, FromList, WhereList>& select) {
  std::ostringstream os;
  select.str(os);
  return os.str();
}

#define scope

BOOST_RDB_BEGIN_TABLE(person) 
  BOOST_RDB_COLUMN(id, integer)
  BOOST_RDB_COLUMN(name, varchar<20>)
BOOST_RDB_END_TABLE(person) 

int test_main( int, char *[] )
{
  using namespace boost::rdb;

  BOOST_CHECK(
    create<person>()
    == "create table person(id integer, name varchar(20))");

  scope {
    person husband;
    BOOST_CHECK(str(
      select(husband.id).from(husband)
      ) == "select id from person");
  }

  scope {
    person husband;
    person_<1> wife("wife");
    BOOST_CHECK(str(
      select(husband.id)(wife.name).from(husband)(wife)
      ) == "select id, wife.name from person, person as wife");
  }
      
  return 0;
}
