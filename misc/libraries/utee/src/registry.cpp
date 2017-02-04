#include <ut/registry.h>

using namespace std;

#if defined(_WIN32) || defined(__CYGWIN__)
   #ifdef __GNUC__
      #define EXPORTED __attribute__((dllexport))
   #else
      #define EXPORTED __declspec(dllexport)
   #endif
#else
   #define EXPORTED
#endif

namespace ut
{
   registry::registry()
   {
#ifdef _MSC_VER
      _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
   }

   registry &registry_instance()
   {
      static registry g_instance;

      return g_instance;
   }

   int registry::tests_count() const
   {
      return static_cast<int>(m_test_cases.size());
   }

   registry::const_iterator registry::tests_begin() const
   {
      return m_test_cases.begin();
   }

   registry::const_iterator registry::tests_end() const
   {
      return m_test_cases.end();
   }
}

extern "C" EXPORTED void utee_enumerate_test_cases(void *i_parameter, AcceptTestF i_acceptor)
{
   ut::registry &r = ut::registry_instance();

   for (ut::registry::const_iterator i = r.tests_begin(); i != r.tests_end(); ++i)
      i_acceptor(i_parameter, i->get());
}
