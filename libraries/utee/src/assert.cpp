#include <ut/assert.h>

#include <stdlib.h>

namespace ut
{
   LocationInfo::LocationInfo( const std::string &i_filename, int i_line )
      : filename( i_filename ), line( i_line )
   {  }


   FailedAssertion::FailedAssertion( const std::string &message, const LocationInfo &i_location )
      : logic_error( ComposeMessage( message, i_location) ), location( i_location )
   {  }

   FailedAssertion::~FailedAssertion() throw()
   {  }

   std::string FailedAssertion::ComposeMessage( const std::string &message, const LocationInfo &i_location )
   {
      char buffer[ 20 ] = { 0 };

      sprintf_s(buffer, sizeof(buffer), "#%d: ", i_location.line);
      return buffer + message;
   }
}
