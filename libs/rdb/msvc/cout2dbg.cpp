
#ifdef _WIN32

#include <windows.h>
#include <boost/iostreams/stream.hpp>
#include <iostream>

using namespace std;
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

static stream_buffer<debug_output_sink> buf(0);
streambuf* old = cout.rdbuf(&buf);


#endif
