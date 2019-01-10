#pragma once

namespace agge
{
	namespace tests
	{
		template <typename T>
		class scoped_ptr
		{
		public:
			scoped_ptr(T *from);
			~scoped_ptr();

			void reset(T *other = 0) throw();

			T *operator ->() const throw();
			T &operator *() const throw();

		private:
			scoped_ptr(const scoped_ptr &other);
			const scoped_ptr operator =(const scoped_ptr &rhs);

		private:
			T *_ptr;
		};



		template <typename T>
		inline scoped_ptr<T>::scoped_ptr(T *from)
			: _ptr(from)
		{	}

		template <typename T>
		inline scoped_ptr<T>::~scoped_ptr()
		{
			T *ptr[sizeof(T) > 0 ? 1 : 0] = { _ptr };
			delete ptr[0];
		}

		template <typename T>
		inline void scoped_ptr<T>::reset(T *other) throw()
		{
			delete _ptr;
			_ptr = other;
		}

		template <typename T>
		inline T *scoped_ptr<T>::operator ->() const throw()
		{	return _ptr;	}

		template <typename T>
		inline T &scoped_ptr<T>::operator *() const throw()
		{	return *_ptr;	}
	}
}
