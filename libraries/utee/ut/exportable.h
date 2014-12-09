#pragma once

#include <ostream>
#include <string>

namespace ut
{
   namespace exportable
   {
      template <typename CharT>
      class immutable_basic_string
      {
      public:
         immutable_basic_string(const std::basic_string<CharT>& i_from);
         immutable_basic_string(const immutable_basic_string& i_other);
         ~immutable_basic_string();

         const CharT* c_str() const throw();

      private:
         typedef void (*DeleterF)(const CharT* i_charsBuffer);

      private:
         const immutable_basic_string& operator =(const immutable_basic_string& i_rhs);

         void construct(const std::basic_string<CharT>& i_from);

         CharT* m_text;
         DeleterF m_deleter;
      };

      typedef immutable_basic_string<char> immutable_string;



      template <typename CharT>
      inline immutable_basic_string<CharT>::immutable_basic_string(const std::basic_string<CharT>& i_from)
      {
         construct(i_from);
      }

      template <typename CharT>
      inline immutable_basic_string<CharT>::immutable_basic_string(const immutable_basic_string& i_other)
      {
         construct(i_other.c_str());
      }

      template <typename CharT>
      inline immutable_basic_string<CharT>::~immutable_basic_string()
      {
         m_deleter(m_text);
      }

      template <typename CharT>
      inline const CharT* immutable_basic_string<CharT>::c_str() const throw()
      {
         return m_text;
      }

      template <typename CharT>
      inline void immutable_basic_string<CharT>::construct(const std::basic_string<CharT>& i_from)
      {
         struct LocalDeleter
         {
            void static deleterFunction(const CharT* i_charsBuffer)
            {
               delete[] i_charsBuffer;
            }
         };

         m_text = new CharT[i_from.size() + 1];
         std::copy(i_from.begin(), i_from.end(), m_text);
         m_text[i_from.size()] = CharT();
         m_deleter = &LocalDeleter::deleterFunction;
      }
   }
}

template <typename CharT>
inline std::basic_ostream<CharT>& operator <<(std::basic_ostream<CharT>& i_ostream,
   const ut::exportable::immutable_basic_string<CharT>& i_string)
{
   return i_ostream << i_string.c_str();
}
