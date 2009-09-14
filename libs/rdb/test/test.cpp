#include <iostream>
#include <boost/test/minimal.hpp>
#include <boost/rdb/rdb.hpp>

using namespace std;
using namespace boost::rdb;

#ifdef _WIN32

#include <windows.h>
#include <boost/iostreams/stream.hpp>

using namespace boost::iostreams;

class debug_output_sink : public boost::iostreams::sink {
public:
    debug_output_sink(int) { }
    std::streamsize write(const char* s, std::streamsize n);
};

streamsize debug_output_sink::write(const char* s, streamsize n)
{
  TCHAR chunk[1000 + 1];
  streamsize remain(n);
  
  while (remain)
  {
    streamsize chunk_size(min(remain, sizeof chunk - 1));
    *copy(s, s + chunk_size, chunk) = 0;
    OutputDebugString(chunk);
    remain -= chunk_size;
  }
  
  return n;
}

#endif

int test_main( int, char *[] )
{
#ifdef _WIN32
  static stream_buffer<debug_output_sink> buf(0);
  cout.rdbuf(&buf);
#endif
      
  return 0;
}
