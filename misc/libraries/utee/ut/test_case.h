#pragma once

#include "exportable.h"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace std
{
   namespace tr1 { }

   using namespace tr1; // To support MSVC 9.0
}

namespace ut
{
   ///   @brief This interface is just a handle to init/term methods that are depending on a specific fixture type.
   struct destructible
   {
      virtual ~destructible() {  }
   };


   class test_result
   {
   public:
      test_result(bool i_initialized, bool i_passed, bool i_terminated, const std::string& i_outcome);

      const bool initialized : 1;
      const bool passed : 1;
      const bool terminated : 1;

      const exportable::immutable_string outcome;

   private:
      const test_result& operator =(const test_result& i_rhs);
   };


   ///   @brief This interface represents an access method to a generic unit test. It allows reaging its metadata
   ///      & execute it.
   struct test_case : destructible
   {
      virtual exportable::immutable_string fixture_name() const = 0;
      virtual exportable::immutable_string name() const = 0;
      virtual test_result execute() = 0;
   };


   ///   @brief The class represents a container for both inititializers and terminators for a fixture of a specific
   ///      class. It is derived from destructible to be uniformely stored within the registry.
   template <typename Fixture>
   class setup_impl : public destructible
   {
      typedef void (Fixture::*_method_t)();
      typedef std::vector<_method_t> _methods_list_t;

      _methods_list_t m_inits, m_teardowns;

   public:
      typedef _method_t method_t;

   public:
      void init(Fixture &fixture) const;
      void teardown(Fixture &fixture) const;

      void add_init_method(const method_t &m);
      void add_teardown_method(const method_t &m);
   };


   ///   @brief The class represents an encapsulation of init/terms and a test method providing a way to execute a test
   ///      case.
   template <typename Fixture>
   class test_case_impl : public test_case
   {
      typedef void (Fixture::*_method_t)();
      typedef std::vector<_method_t> _methods_list_t;
      typedef std::shared_ptr<_methods_list_t> _methods_ptr_t;

      const _method_t m_method;
      const exportable::immutable_string m_name;
      std::shared_ptr< setup_impl<Fixture> > m_setup;

      const test_case_impl &operator =(const test_case_impl &rhs);

   public:
      typedef _method_t method_t;

   public:
      test_case_impl(method_t method, const std::string &name, std::shared_ptr< setup_impl<Fixture> > setup);

      virtual exportable::immutable_string fixture_name() const;
      virtual exportable::immutable_string name() const;

      ///   @brief Tries to execute the test. Creates an instance of a fixture, the tries to apply initializers to it.
      ///      If initialization fails at any step, the test is aborted with no cleanup methods called. If
      ///      initialization is done correctly, the teardown is guaranteed to be called after the test (no matter what
      ///      outcome will be). The test result (passed/failed) is written to the test_results structure.
      virtual test_result execute();
   };



   inline test_result::test_result(bool i_initialized, bool i_passed, bool i_terminated, const std::string& i_outcome)
      : initialized(i_initialized), passed(i_passed), terminated(i_terminated), outcome(i_outcome)
   {
   }



   template <typename Fixture>
   inline test_case_impl<Fixture>::test_case_impl(method_t method, const std::string &name,
      std::shared_ptr< setup_impl<Fixture> > setup)
      : m_method(method), m_name(name), m_setup(setup)
   {	}

   template <typename Fixture>
   inline exportable::immutable_string test_case_impl<Fixture>::fixture_name() const
   {	return std::string(Fixture::__suite_name());	}

   template <typename Fixture>
   inline exportable::immutable_string test_case_impl<Fixture>::name() const
   {	return m_name;	}

   template <typename Fixture>
   inline test_result test_case_impl<Fixture>::execute()
   {
      bool initialized = false;
      bool passed = false;
      bool terminated = false;
      std::string outcome;

      try
      {
         Fixture f;

         try
         {
            m_setup->init(f);
            initialized = true;

            try
            {
               (f.*m_method)();
               passed = true;
            }
            catch (const std::exception &e)
            {
               outcome = e.what();
            }
            catch (...)
            {
               outcome = "Test Failed!";
            }

            try
            {
               m_setup->teardown(f);
               terminated = true;
            }
            catch (const std::exception &e)
            {
               outcome += outcome.empty() ? "" : "\n";
               outcome += std::string("Teardown Failed: ") + e.what();
            }
            catch (...)
            {
               outcome += outcome.empty() ? "" : "\n";
               outcome += "Teardown Failed!";
            }
         }
         catch (const std::exception &e)
         {
            outcome = std::string("Initialization Failed: ") + e.what();
         }
         catch (...)
         {
            outcome = "Initialization Failed!";
         }
      }
      catch (...)
      {
      }
      return test_result(initialized, passed, terminated, outcome);
   }



   template <typename Fixture>
   inline void setup_impl<Fixture>::init(Fixture &fixture) const
   {
      for (typename _methods_list_t::const_iterator i = m_inits.begin(); i != m_inits.end(); ++i)
      {
         (fixture.**i)();
      }
   }

   template <typename Fixture>
   inline void setup_impl<Fixture>::teardown(Fixture &fixture) const
   {
      for (typename _methods_list_t::const_iterator i = m_teardowns.begin(); i != m_teardowns.end(); ++i)
      {
         (fixture.**i)();
      }
   }

   template <typename Fixture>
   inline void setup_impl<Fixture>::add_init_method(const method_t &m)
   {
      if (std::find(m_inits.begin(), m_inits.end(), m) == m_inits.end())
      {
         m_inits.push_back(m);
      }
   }

   template <typename Fixture>
   inline void setup_impl<Fixture>::add_teardown_method(const method_t &m)
   {
      if (std::find(m_teardowns.begin(), m_teardowns.end(), m) == m_teardowns.end())
      {
         m_teardowns.push_back(m);
      }
   }
}
