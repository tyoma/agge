#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace ut
{
   class LocationInfo
   {
      const LocationInfo &operator =(const LocationInfo &rhs);

   public:
      LocationInfo(const std::string &i_filename, int i_line);

      const std::string filename;
      const int line;
   };

   class FailedAssertion : public std::logic_error
   {
      static std::string ComposeMessage(const std::string &message, const LocationInfo &i_location);

   public:
      FailedAssertion(const std::string &message, const LocationInfo &i_location);
      virtual ~FailedAssertion() throw();

      LocationInfo location;
   };


   template <typename T1, typename T2>
   inline void are_equal(const T1 &i_lhs, const T2 &i_rhs, const LocationInfo &i_location)
   {
      if (!(i_lhs == i_rhs))
         throw FailedAssertion("Values are not equal!", i_location);
   }

   template <typename T1, typename T2>
   inline void are_not_equal(const T1 &i_lhs, const T2 &i_rhs, const LocationInfo &i_location)
   {
      if (!(i_lhs != i_rhs))
         throw FailedAssertion("Values are equal!", i_location);
   }

   template <typename T1, typename T2>
   inline void are_equivalent(const T1& i_reference, const T2& i_actual, const LocationInfo &i_location)
   {
      using namespace std;

      vector<typename T1::value_type> reference(i_reference.begin(), i_reference.end());
      vector<typename T2::value_type> actual(i_actual.begin(), i_actual.end());

      sort(reference.begin(), reference.end());
      sort(actual.begin(), actual.end());
      if (lexicographical_compare(reference.begin(), reference.end(), actual.begin(), actual.end())
         != lexicographical_compare(actual.begin(), actual.end(), reference.begin(), reference.end()))
         throw FailedAssertion("The sets are not equivalent!", i_location);
   }

   template <typename T1, size_t n, typename T2>
   inline void are_equivalent(T1 (&i_reference)[n], const T2& i_actual, const LocationInfo &i_location)
   {
      are_equivalent(std::vector<T1>(i_reference, i_reference + n), i_actual, i_location);
   }

   inline void is_true(bool i_value, const LocationInfo &i_location)
   {
      if (!i_value)
         throw FailedAssertion("Value is not 'true'!", i_location);
   }

   inline void is_false(bool i_value, const LocationInfo &i_location)
   {
      if (i_value)
         throw FailedAssertion("Value is not 'false'!", i_location);
   }

   template <typename T>
   inline void is_empty(const T& i_container, const LocationInfo &i_location)
   {
      if (!i_container.empty())
         throw FailedAssertion("The container is not empty!", i_location);
   }

   template <typename T>
   inline void is_null(const T &i_value, const LocationInfo &i_location)
   {
      if (!(i_value == NULL))
         throw FailedAssertion("Value is not null!", i_location);
   }

   template <typename T>
   inline void is_not_null(const T &i_value, const LocationInfo &i_location)
   {
      if (!(i_value != NULL))
         throw FailedAssertion("Value is null!", i_location);
   }
}

#define assert_equal(_mp_lhs, _mp_rhs)  ut::are_equal(_mp_lhs, _mp_rhs, ut::LocationInfo(__FILE__, __LINE__))
#define assert_equivalent(_mp_reference, _mp_actual)  ut::are_equivalent(_mp_reference, _mp_actual, ut::LocationInfo(__FILE__, __LINE__))
#define assert_not_equal(_mp_lhs, _mp_rhs)  ut::are_not_equal(_mp_lhs, _mp_rhs, ut::LocationInfo(__FILE__, __LINE__))
#define assert_is_true(_mp_value)  ut::is_true(_mp_value, ut::LocationInfo(__FILE__, __LINE__))
#define assert_is_false(_mp_value)  ut::is_false(_mp_value, ut::LocationInfo(__FILE__, __LINE__))
#define assert_is_empty(_mp_container)  ut::is_empty(_mp_container, ut::LocationInfo(__FILE__, __LINE__))
#define assert_null(_mp_value) ut::is_null(_mp_value, ut::LocationInfo(__FILE__, __LINE__))
#define assert_not_null(_mp_value) ut::is_not_null(_mp_value, ut::LocationInfo(__FILE__, __LINE__))

#define assert_throws(_mp_fragment, expected_exception)\
   try { _mp_fragment; throw ut::FailedAssertion("Expected exception was not thrown!", ut::LocationInfo( __FILE__, __LINE__ )); }\
   catch (const ut::FailedAssertion &) { throw; }\
   catch (const expected_exception &) { } \
   catch (...) { throw ut::FailedAssertion("Exception of unexpected type was thrown!", ut::LocationInfo( __FILE__, __LINE__ )); }
