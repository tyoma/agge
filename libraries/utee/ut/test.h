///   @file test.h
///   @brief This is the main file of the UT framework. It includes the definition of macros that enable writing of UTs.
///   The sample UTs written via the famework goes below. This code just registers two test cases. You can later read
///   them and execute each (or only one).
///
///   Glossary (xUnit terminology)
///      Test fixture (aka test context) - a set of preconditions or state needed to run a test. The developer should
///         set up a known good state before the tests, and after the tests return to the original state.
///
///      Test suite - a set of tests that all share the same fixture. The order of the tests shouldn't matter.
///
///      Test execution - an execution of an individual unit test that proceeds as follows:
///         1. initializers(); // First, we should prepare our 'world' to make an isolated environment for testing
///         2. <body of a test> // Here we make all the tests
///         3. teardown(); // In the end, whether succeed or fail we should clean up our 'world' to not disturb other
///            tests or code
///
///   Sample test file (cpp, no need in a separate header):
///
///   #include <ut/test.h>
///   
///   begin_test_suite( AcmeTests )
///      init( AcmeTestsInit )
///      {
///         // Fuel up our rocket
///      }
///
///      teardown( AcmeTestsCleanup )
///      {
///         // Drain the fuel, assert if anything goes wrong
///      }
///
///
///      test( LaunchTheRocket )
///      {
///         // Test if the rocket can be launched
///      }
///
///
///      test( LaunchAndCheckRocketGauges )
///      {
///         // Check that the rocket gauges are showing that we are in flight
///      }
///   end_test_suite

#pragma once

#include "registry.h"

namespace ut
{
   registry &registry_instance();

   ///   @brief Automatic registrar class for an initalization method.
   template <class FixtureT, void (FixtureT::*metadata_provider())()>
   struct test_init_registrar
   {
      test_init_registrar()
      {
         registry_instance().add_init<FixtureT>(metadata_provider());
      }
   };


   ///   @brief Automatic registrar class for a cleanup method.
   template <class FixtureT, void (FixtureT::*metadata_provider())()>
   struct test_teardown_registrar
   {
      test_teardown_registrar()
      {
         registry_instance().add_teardown<FixtureT>(metadata_provider());
      }
   };


   ///   @brief Automatic registrar class for a test method.
   template <class FixtureT, void (FixtureT::*metadata_provider(const char *&))()>
   struct test_case_registrar
   {
      test_case_registrar()
      {
         typedef void (FixtureT::*test_case_method_t)();

         const char *name = 0;
         test_case_method_t method = metadata_provider(name);

         registry_instance().add_test<FixtureT>(method, name);
      }
   };
}

///   @brief The set of macros to enable tests notation as described above.
#define begin_test_suite( __test_suite )\
   class __test_suite\
   {\
      typedef __test_suite this_suite_class;\
   public:\
      static const char *__suite_name()\
      {  return #__test_suite; }

#define init( __test_init )\
   static void (this_suite_class::*__##__test_init##_meta())()\
   {  return &this_suite_class::__test_init; }\
   ut::test_init_registrar<this_suite_class, &this_suite_class::__##__test_init##_meta>   __##__test_init##_registrar;\
   void __test_init()

#define teardown( __test_tdwn )\
   static void (this_suite_class::*__##__test_tdwn##_meta())()\
   {  return &this_suite_class::__test_tdwn; }\
   ut::test_teardown_registrar<this_suite_class, &this_suite_class::__##__test_tdwn##_meta>  __##__test_tdwn##_registrar;\
   void __test_tdwn()

#define test( __test )\
   static void (this_suite_class::*__##__test##_meta(const char *&name))()\
   {  return name = #__test, &this_suite_class::__test;  }\
   ut::test_case_registrar<this_suite_class, &this_suite_class::__##__test##_meta>  __##__test##_registrar;\
   void __test()

#define obsolete_test( __test )

#define end_test_suite\
   } static g_suite##__LINE__;
