#include <boost/rdb/sql.hpp>
#include <boost/rdb/odbc.hpp>

#include "test_tables.hpp"

using namespace boost::rdb::sql;
using namespace boost::rdb::sql::test::springfield;

void examples() {
  //[ example_create_table
  create_table(person::_);
  //]
}

#if 1

#include <boost/rdb/sql/select.hpp>

namespace boost { namespace rdb { namespace sql {

  // add a new keyword
  struct mysql5 : sql2003 {
    struct limit;
  };
  
  // allow all sql2003 constructs
  template<class State, class New>
  struct allow<mysql5, State, New> : allow<sql2003, State, New> { };
  
  // augment the standard sql statement
  template<class State, class Data, class Subdialect>
  struct select_statement<mysql5, State, Data, Subdialect> : select_statement<sql2003, State, Data, Subdialect> {

    typedef select_statement<sql2003, State, Data, Subdialect> inherited;

    select_statement(const Data& data) : inherited(data) { }

    typename inherited::template transition<typename Subdialect::limit, int>::type
    limit(int n) const {
      BOOST_MPL_ASSERT((allow<Subdialect, State, Subdialect::limit>));
      return typename inherited::template transition<
        typename Subdialect::limit, int
      >::type(ct::add_key<typename Subdialect::limit>(data_, n));
    }
  };

  // tell how to print the clause
  inline void str(std::ostream& os, const ct::map_entry<mysql5::limit, int>& p) {
    os << " limit " << p.value;
  }
  
  // declare legal transitions
  BOOST_RDB_ALLOW(mysql5, from, limit);
  BOOST_RDB_ALLOW(mysql5, where, limit);

  // grammar entry point
  namespace mysql {
    select_statement<mysql5, mysql5::select, ct::map0, mysql5> select = ct::map0();
  }

  // that's it !

} } }

#include <iostream>

void limit_poc() {
  using namespace std;
  using namespace boost::rdb::sql;
  using mysql::select;
  person p;
  select(p.id).from(p).limit(10);
  select(p.id).from(p).where(p.id > 1).limit(10);
  select(p.id).from(p).limit(10).str(cout);
  cout << endl;
}

void select_size() {
  using namespace std;
  using namespace boost::rdb::sql;
  person p;
  select(p.id).from(p).where(p.age > 18 && p.age < 65).str(cout);
  //0040127A  mov         edi,eax 
  //0040127C  mov         ecx,41h 
  //00401281  lea         esi,[ecx-2Fh] 
  //00401284  mov         dword ptr [esp+28h],edi 
  //00401288  mov         dword ptr [esp+0Ch],edi 
  //0040128C  mov         dword ptr [esp+1Ch],eax 
  //00401290  mov         eax,dword ptr [__imp_std::cout (4060A4h)] 
  //00401295  mov         dword ptr [esp+20h],ecx 
  //00401299  lea         edi,[esp+2Ch] 
  //0040129D  lea         ecx,[esp+0Ch] 
  //004012A1  mov         dword ptr [esp+10h],edi 
  //004012A5  mov         dword ptr [esp+14h],edx 
  //004012A9  mov         dword ptr [esp+18h],esi 
  //004012AD  call             

  //    void str(std::ostream& os) const {
  //00401480  sub         esp,0Ch 
  //00401483  push        esi  
  //00401484  push        edi  
  //00401485  mov         esi,eax 
  //      os << "select";
  //00401487  push        offset string "select" (406610h) 
  //0040148C  push        esi  
  //0040148D  mov         edi,ecx 
  //0040148F  call        std::operator<<<std::char_traits<char> > (401960h) 
  //00401494  add         esp,8 
  //      fusion::for_each(data_, str_clause(os));
  //00401497  push        edi  
  //00401498  lea         eax,[esp+0Ch] 
  //0040149C  mov         dword ptr [esp+0Ch],esi 
  //004014A0  call        boost::rdb::sql::str_clause::operator()<boost::fusion::pair<boost::rdb::sql::sql2003::exprs,boost::fusion::vector<boost::rdb::sql::column<boost::rdb::sql::test::springfield::person_<0>,boost::rdb::sql::integer,boost::rdb::sql::test::springfield::person_<0>::id_base>,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_,boost::fusion::void_> > > (402C70h) 
  //004014A5  push        offset string " from " (40673Ch) 
  //004014AA  push        esi  
  //004014AB  call        std::operator<<<std::char_traits<char> > (401960h) 
  //004014B0  mov         eax,dword ptr [edi+4] 
  //004014B3  add         esp,8 
  //004014B6  lea         ecx,[esp+8] 
  //004014BA  mov         dword ptr [esp+8],esi 
  //004014BE  mov         dword ptr [esp+0Ch],offset string "" (406314h) 
  //004014C6  call        boost::rdb::sql::comma_output::operator()<boost::rdb::sql::test::springfield::person_<0> > (404A60h) 
  //004014CB  push        offset string " where " (40675Ch) 
  //004014D0  push        esi  
  //004014D1  add         edi,8 
  //004014D4  call        std::operator<<<std::char_traits<char> > (401960h) 
  //004014D9  lea         eax,[edi+8] 
  //004014DC  push        eax  
  //004014DD  call        boost::rdb::sql::binary_operation<boost::rdb::sql::gt<boost::rdb::sql::column<boost::rdb::sql::test::springfield::person_<0>,boost::rdb::sql::integer,boost::rdb::sql::test::springfield::person_<0>::age_base>,boost::rdb::sql::literal<long,boost::rdb::sql::integer> >,boost::rdb::sql::lt<boost::rdb::sql::column<boost::rdb::sql::test::springfield::person_<0>,boost::rdb::sql::integer,boost::rdb::sql::test::springfield::person_<0>::age_base>,boost::rdb::sql::literal<long,boost::rdb::sql::integer> >,0>::write (404500h) 
  //004014E2  add         esp,0Ch 
  //    }
  //004014E5  pop         edi  
  //004014E6  pop         esi  
  //004014E7  add         esp,0Ch 
  //004014EA  ret              
  cout << endl;

  cout << sizeof(select(p.id).from(p).where(p.age > 18 && p.age < 65)) << endl;
  // 28 with msvc9
  cout << sizeof(exprs(select(p.id).from(p).where(p.age > 18 && p.age < 65))) << endl;
  // 28 with msvc9
}

int main() {
  limit_poc();
  select_size();
}

#endif
