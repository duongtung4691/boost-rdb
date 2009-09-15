#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>

using namespace std;
using namespace boost::rdb;

template<class Statement>
std::string str(const Statement& statement) {
  std::ostringstream os;
  statement.str(os);
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
  BOOST_RDB_COLUMN(age, integer)
BOOST_RDB_END_TABLE(person) 

int test_main( int, char *[] )
{
  using namespace boost::rdb;

  BOOST_CHECK(str(
    create<person>()
    ) == "create table person(id integer, name varchar(20), age integer)");

  scope {
    person husband;
    BOOST_CHECK(str(
      select(husband.id).from(husband)
      ) == "select id from person");
  }

  scope {
    BOOST_CHECK(str(
      select(person::_.id).from(person::_)
      ) == "select id from person");
  }

  scope {
    person husband;
    person_<1> wife("wife");
    BOOST_CHECK(str(
      select(husband.id)(wife.name).from(husband)(wife)
      ) == "select id, wife.name from person, person as wife");
  }
  
  scope {
    person p;
    
    BOOST_CHECK(str(
      select(p.id).from(p).where(p.name == "Simpson")
      ) == "select id from person where name = 'Simpson'");
    
    BOOST_CHECK(str(
      select(p.id).from(p).where(p.name == "O'Hara")
      ) == "select id from person where name = 'O''Hara'");
  }
      
  return 0;
}
