#include <ut/registry.h>

#include <atlpath.h>
#include <atlstr.h>
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

#if defined(_UNICODE) || defined(UNICODE)
   #define tout wcout
   #define terr wcerr
#else
   #define tout cout
   #define terr cerr
#endif


namespace
{
   struct OutputNode
   {
      virtual ~OutputNode() throw() { }

      virtual shared_ptr<OutputNode> OpenSubnode(const CString& i_name) = 0;
      virtual void CloseNode(bool /*i_ok*/ = true, const CString& /*i_outcomeTitle*/ = CString(),
         const CString& /*i_outcomeMessage*/ = CString()) { }
   };


   struct Statistics
   {
      size_t Passed;
      size_t Failed;
      __int64 TimeSpent;
   };


   class HostedTests
   {
   public:
      HostedTests(LPCTSTR i_testsContainer)
         : m_module(::LoadLibrary(i_testsContainer), &::FreeLibrary)
      {
         ATL::CPath path(i_testsContainer);

         path.StripPath();
         m_containerName = (LPCTSTR)path;
         if (EnumerateTestsF enumerator = reinterpret_cast<EnumerateTestsF>(::GetProcAddress(
            static_cast<HMODULE>(m_module.get()), "utee_enumerate_test_cases")))
         {
            enumerator(this, &HostedTests::acceptTestCase);
            return;
         }
         throw runtime_error("Cannot obtain registry from tests container!");
      }

      void SequentialRun(OutputNode& i_testRunOutput, Statistics& io_statistics) const
      {
         shared_ptr<OutputNode> containerOutput(i_testRunOutput.OpenSubnode(m_containerName));

         for (TestSuites::const_iterator i = m_tests.begin(); i != m_tests.end(); ++i)
         {
            shared_ptr<OutputNode> suiteOutput(containerOutput->OpenSubnode(i->first.c_str()));

            for (SuiteTestCases::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
            {
               shared_ptr<OutputNode> testOutput(suiteOutput->OpenSubnode((*j)->name().c_str()));
               __int64 started, finished;

               ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&started));
               ut::test_result result = (*j)->execute();
               ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&finished));
               bool ok = result.initialized && result.passed && result.terminated;
               CString outcomeTitle(ok ? _T("ok") :
                  !result.initialized ? _T("INIT FAILED") : !result.terminated ? _T("CLEANUP FAILED") : _T("FAILED"));

               testOutput->CloseNode(ok, outcomeTitle, result.outcome.c_str());
               ++(ok ? io_statistics.Passed : io_statistics.Failed);
               io_statistics.TimeSpent += finished - started;
            }
         }
      }

   private:
      typedef vector<ut::test_case*> SuiteTestCases;
      typedef map<string, SuiteTestCases> TestSuites;

   private:
      static void acceptTestCase(void* i_this, ut::test_case* i_test)
      {
         static_cast<HostedTests*>(i_this)->m_tests[i_test->fixture_name().c_str()].push_back(i_test);
      }

   private:
      shared_ptr<void> m_module;
      TestSuites m_tests;
      CString m_containerName;
   };


   namespace Console
   {
      class ColorLock
      {
      public:
         ColorLock(WORD i_color, DWORD i_streamType = STD_OUTPUT_HANDLE)
            : m_hstream(::GetStdHandle(i_streamType))
         {
            CONSOLE_SCREEN_BUFFER_INFO csbi = {};
           
            ::GetConsoleScreenBufferInfo(m_hstream, &csbi);
            m_storedAttrs = csbi.wAttributes;
            ::SetConsoleTextAttribute(m_hstream, i_color);
         }

         ~ColorLock()
         {
            ::SetConsoleTextAttribute(m_hstream, m_storedAttrs);
         }

      private:
         const HANDLE m_hstream;
         WORD m_storedAttrs;
      };


      template <typename CharT>
      inline std::basic_ostream<CharT>& operator <<(std::basic_ostream<CharT>& i_ostream,
         const CString& i_string)
      {
         return i_ostream << (LPCTSTR)i_string;
      }


      struct TestNode : OutputNode
      {
         TestNode(const CString& i_name)
            : Name(i_name)
         {
         }

         const CString Name;

         virtual shared_ptr<OutputNode> OpenSubnode(const CString& /*i_name*/)
         {
            return shared_ptr<OutputNode>();
         }

         virtual void CloseNode(bool i_ok, const CString& i_outcomeTitle, const CString& i_outcomeMessage)
         {
            if (i_ok)
            {
               {  ColorLock cl(FOREGROUND_GREEN); tout << _T(">"); }
            }
            else
            {
               tout << endl;
               {  ColorLock cl(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << _T("   ["); }
               {  ColorLock cl(FOREGROUND_INTENSITY | FOREGROUND_RED); tout << i_outcomeTitle << _T(": "); }
               {  ColorLock cl(FOREGROUND_RED); tout << Name; }
               {  ColorLock cl(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << _T("]") << endl; }
               {  ColorLock cl(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << _T("   --- ") << i_outcomeMessage << _T(" ---") << endl; }
            }
         }
      };


      struct SuiteNode : OutputNode
      {
         virtual ~SuiteNode()
         {
            tout << endl;
         }

         virtual shared_ptr<OutputNode> OpenSubnode(const CString& i_name)
         {
            return shared_ptr<TestNode>(new TestNode(i_name));
         }
      };

      struct ContainerNode : OutputNode
      {
         virtual ~ContainerNode()
         {
            tout << endl;
         }

         virtual shared_ptr<OutputNode> OpenSubnode(const CString& i_name)
         {
            {  ColorLock cl(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << _T(" * "); }
            {  ColorLock cl(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << i_name << _T(" "); }
            return shared_ptr<SuiteNode>(new SuiteNode);
         }
      };

      struct TestRunNode : OutputNode
      {
         virtual shared_ptr<OutputNode> OpenSubnode(const CString& i_name)
         {
            {  ColorLock cl(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << _T("* Container: "); }
            {  ColorLock cl(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); tout << i_name << endl; }
            return shared_ptr<ContainerNode>(new ContainerNode);
         }
      };
   }


   namespace XML
   {
      struct XMLOutputNode
      {
         virtual ~XMLOutputNode() { }
         virtual CString Format(Statistics& io_statistics) const = 0;

         static CString EscapeXML(CString value)
         {
            value.Replace(_T("&"), _T("&amp;"));
            value.Replace(_T("<"), _T("&lt;"));
            value.Replace(_T(">"), _T("&gt;"));
            value.Replace(_T("'"), _T("&apos;"));
            value.Replace(_T("\""), _T("&quot;"));
            return value;
         }
      };

      class TestNode : public OutputNode, public XMLOutputNode
      {
      public:
         TestNode(const CString& i_name)
            : m_name(i_name), m_ok(false)
         {
         }

      private:
         virtual shared_ptr<OutputNode> OpenSubnode(const CString& /*i_name*/)
         {
            return shared_ptr<OutputNode>();
         }

         virtual void CloseNode(bool i_ok, const CString& i_outcomeTitle, const CString& i_outcomeMessage)
         {
            m_ok = i_ok;
            if (!m_ok)
               m_outcome.Format(_T("%s: %s"), i_outcomeTitle, i_outcomeMessage);
            else
               m_outcome = _T("&&&<>'");
         }

         virtual CString Format(Statistics& io_statistics) const
         {
            CString xml;

            ++(m_ok ? io_statistics.Passed : io_statistics.Failed);
            xml.Format(_T("<Leaf><Result exectime=\"0\" name=\"%s\" srcline=\"-1\" srcfile=\"\" message=\"%s\""
               " fail=\"%s\" error=\"N\"/></Leaf>"), EscapeXML(m_name), EscapeXML(m_outcome), m_ok ? _T("N") : _T("Y"));
            return xml;
         }

      private:
         CString m_name;
         CString m_outcome;
         bool m_ok;
      };


      template <typename ChildNodeT>
      class CompositeNode : public OutputNode, public XMLOutputNode
      {
      public:
         CompositeNode(const CString& i_name)
            : m_name(i_name)
         {
         }

      protected:
         virtual CString Format(Statistics& io_statistics) const
         {
            CString innerXml;
            CString xml;
            Statistics innerStatistics = {};

            for (InnerNodes::const_iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
            {
               innerXml += (*i)->Format(innerStatistics);
            }
            io_statistics.Passed += innerStatistics.Passed;
            io_statistics.Failed += innerStatistics.Failed;

            xml.Format(_T("<Node><ResultSet exectime=\"0\" nsucceeded=\"%d\" nerrors=\"0\" nfails=\"%d\" "
               "nresults=\"%d\" name=\"%s\"/>%s</Node>"), innerStatistics.Passed, innerStatistics.Failed,
               innerStatistics.Passed + innerStatistics.Failed, EscapeXML(m_name), innerXml);
            return xml;
         }

      private:
         typedef vector< shared_ptr<XMLOutputNode> > InnerNodes;

      private:
         virtual shared_ptr<OutputNode> OpenSubnode(const CString& i_name)
         {
            shared_ptr<ChildNodeT> child(new ChildNodeT(i_name));
            m_nodes.push_back(child);
            return child;
         }

      private:
         CString m_name;
         InnerNodes m_nodes;
      };

      typedef CompositeNode<TestNode> SuiteNode;
      typedef CompositeNode<SuiteNode> ContainerNode;

      class TestRunNode : public CompositeNode<ContainerNode>
      {
      public:
         TestRunNode()
            : CompositeNode<ContainerNode>(_T("Tests Results"))
         {
         }
         ~TestRunNode()
         {
            try
            {
               Statistics s;

               tout << Format(s);
            }
            catch (...)
            {
            }
         }
      };
   }
}

int run_tests(int argc, const TCHAR *argv[])
{
   bool xml = false;
   __int64 frequency;
   vector<HostedTests> testContainers;

   ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));

   for (int argIndex = 1; argIndex < argc; ++argIndex)
   {
      if (argv[argIndex][0] == _T('-') || argv[argIndex][0] == _T('/'))
      {
         CString option(argv[argIndex] + 1);
         
         xml = xml || option == _T("xml") || option == _T("XMLRunner");
      }
      else
      {
         testContainers.push_back(HostedTests(argv[argIndex]));
      }
   }

   shared_ptr<OutputNode> output = xml ?
      shared_ptr<OutputNode>(new XML::TestRunNode) : shared_ptr<OutputNode>(new Console::TestRunNode);
   Statistics s = { };

   for (vector<HostedTests>::const_iterator i = testContainers.begin(); i != testContainers.end(); ++i)
   {
      i->SequentialRun(*output, s);
   }

   output.reset();

   {  Console::ColorLock cl((s.Failed ? FOREGROUND_RED | FOREGROUND_BLUE : FOREGROUND_INTENSITY) | FOREGROUND_GREEN, STD_ERROR_HANDLE); terr << _T(" PASSED: ") << s.Passed << endl; }
   {  Console::ColorLock cl((s.Failed ? FOREGROUND_INTENSITY : FOREGROUND_GREEN | FOREGROUND_BLUE) | FOREGROUND_RED, STD_ERROR_HANDLE); terr << _T(" FAILED: ") << s.Failed << endl; }
   {  Console::ColorLock cl(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, STD_ERROR_HANDLE); terr << _T(" Time spent: "); }
   {  Console::ColorLock cl(FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, STD_ERROR_HANDLE); terr << 1000 * s.TimeSpent / frequency << _T("ms") << endl; }
   terr << endl;

   return s.Failed == 0 ? 0 : -1;
}

int xcptFilter(PEXCEPTION_POINTERS i_exceptionInfo)
{
   {  Console::ColorLock cl(FOREGROUND_INTENSITY | FOREGROUND_RED, STD_ERROR_HANDLE);
      terr << _T(" Fail to run tests. Exception code: ") << hex << showbase << i_exceptionInfo->ExceptionRecord->ExceptionCode << endl; }
   return EXCEPTION_EXECUTE_HANDLER;
}

int _tmain(int argc, const TCHAR *argv[])
{
   __try
   {
      return run_tests(argc, argv);
   }
   __except (xcptFilter(GetExceptionInformation()))
   {
      return -1;
   }
}
