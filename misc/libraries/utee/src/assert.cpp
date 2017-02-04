#include <ut/assert.h>

#include <stdio.h>

using namespace std;

namespace ut
{
   LocationInfo::LocationInfo(const string &i_filename, int i_line)
      : filename(i_filename), line( i_line )
   {  }


   FailedAssertion::FailedAssertion(const string &message, const LocationInfo &i_location)
      : logic_error(ComposeMessage(message, i_location)), location(i_location)
   {  }

   FailedAssertion::~FailedAssertion() throw()
   {  }

   string FailedAssertion::ComposeMessage(const string &message, const LocationInfo &i_location)
   {
      char buffer[20] = { 0 };

      sprintf(buffer, "#%d: ", i_location.line);
      return buffer + message;
   }
}
