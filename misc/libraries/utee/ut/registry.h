#pragma once

#include "test_case.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

namespace ut
{
   struct type_info_less : std::binary_function<const std::type_info *, const std::type_info *, bool>
   {
      bool operator ()(const std::type_info *lhs, const std::type_info *rhs) const;
   };

   ///   @brief This is a central registry for unit tests based on this famework. It supports adding non-static test
   ///      methods from fixtures (tests cannot go outside of any fixture), as well as initialization and teardown
   ///      methods. For a definition of a 'fixture', please, refer to http://en.wikipedia.org/wiki/Test_fixture
   class registry
   {
      typedef std::map<const std::type_info *, std::shared_ptr<destructible>, type_info_less> _setups_map_t;

      std::vector< std::shared_ptr<test_case> > m_test_cases;
      std::set<std::string> m_registered_names;
      _setups_map_t m_setups;

      template <typename FixtureT>
      std::shared_ptr< setup_impl<FixtureT> > get_setup();

   public:
      typedef std::vector< std::shared_ptr<test_case> >::const_iterator const_iterator;

   public:

      registry();

      ///   @brief Registers a test cases which can be run on properly initialized/terminated instance of a fixture.
      template <typename FixtureT>
      void add_test(void (FixtureT::*method)(), const char *name);

      ///   @brief Registers an initializer method. All registered initializers will be called for an instance of
      ///      a fixture prior each single test.
      template <typename FixtureT>
      void add_init(void (FixtureT::*method)());

      ///   @brief Registers a teardown method. All registered terminators will be called for an instance of a fixture
      ///      after each single test. Terminators can throw exceptions/assertions in the case cleanup cannot be done.
      template <typename FixtureT>
      void add_teardown(void (FixtureT::*method)());

      ///   @brief This group of methods provides access to the test cases registered. Dereferencing an iterator gives
      ///      shared_ptr to a test case interface (@see test_case).
      int tests_count() const;
      const_iterator tests_begin() const;
      const_iterator tests_end() const;
   };



   inline bool type_info_less::operator ()(const std::type_info *lhs, const std::type_info *rhs) const
   {
      return !!lhs->before(*rhs);
   }



   template <typename FixtureT>
   inline std::shared_ptr< setup_impl< FixtureT > > registry::get_setup()
   {
      typedef setup_impl<FixtureT> fixture_setup;

      const std::type_info *t = &typeid(FixtureT);
      _setups_map_t::const_iterator s = m_setups.find(t);

      s = s == m_setups.end() ?
         m_setups.insert(std::make_pair(t, std::shared_ptr<destructible>(new fixture_setup))).first : s;
      return std::shared_ptr<fixture_setup>( std::static_pointer_cast< fixture_setup >( s->second ) );
   }

   template <typename FixtureT>
   inline void registry::add_test(void (FixtureT::*method)(), const char *name)
   {
      typedef test_case_impl<FixtureT> test_case;
      typedef setup_impl<FixtureT> fixture_setup;

      std::shared_ptr<fixture_setup> setup(get_setup<FixtureT>());
      std::shared_ptr<test_case> tc(new test_case_impl<FixtureT>(method, name, setup));

      if (m_registered_names.insert(std::string(tc->fixture_name().c_str()) + tc->name().c_str()).second)
         m_test_cases.push_back(tc);
   }

   template <typename FixtureT>
   inline void registry::add_init(void (FixtureT::*method)())
   {
      get_setup<FixtureT>()->add_init_method(method);
   }

   template <typename FixtureT>
   inline void registry::add_teardown(void (FixtureT::*method)())
   {
      get_setup<FixtureT>()->add_teardown_method(method);
   }
}

typedef void (*AcceptTestF)(void *i_parameter, ut::test_case *i_test);
typedef void (*EnumerateTestsF)(void *i_parameter, AcceptTestF i_acceptor);
