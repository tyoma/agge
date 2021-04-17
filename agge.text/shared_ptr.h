#pragma once

#include <agge/config.h>
#include <memory>

namespace agge
{
	using std::shared_ptr;
	using std::weak_ptr;

	template <typename T>
	inline void delete_(T *p)
	{	delete p;	}

	template <typename T, typename RefCounterT = unsigned int>
	class ptr
	{
	public:
		ptr();
		template <typename U>
		explicit ptr(U *from);
		template <typename U, typename DeleterT>
		ptr(U *from, const DeleterT &deleter);
		ptr(const ptr &other);
		~ptr();

		T *get() const {	return _ptr;	}

		T &operator *() {	return *_ptr;	}
		T &operator *() const {	return *_ptr;	}
		T *operator ->() {	return _ptr;	}
		T *operator ->() const {	return _ptr;	}
		ptr &operator =(const ptr &rhs);

	private:
		struct core;

	private:
		template <typename U, typename DeleterT>
		void init(U *from, DeleterT deleter);

	private:
		T *_ptr;
		core *_core;
	};

	template <typename T, typename RefCounterT>
	struct ptr<T, RefCounterT>::core
	{
		virtual ~core() {	}
		virtual void addref() = 0;
		virtual void release() = 0;
	};



	template <typename T, typename RefCounterT>
	inline ptr<T, RefCounterT>::ptr()
		: _ptr(0), _core(0)
	{	}

	template <typename T, typename RefCounterT>
	template <typename U>
	inline ptr<T, RefCounterT>::ptr(U *from)
	{	init(from, &delete_<U>);	}

	template <typename T, typename RefCounterT>
	template <typename U, typename DeleterT>
	inline ptr<T, RefCounterT>::ptr(U *from, const DeleterT &deleter)
	{	init(from, deleter);	}

	template <typename T, typename RefCounterT>
	inline ptr<T, RefCounterT>::ptr(const ptr &other)
		: _ptr(other._ptr), _core(other._core)
	{
		if (_core)
			_core->addref();
	}

	template <typename T, typename RefCounterT>
	inline ptr<T, RefCounterT>::~ptr()
	{
		if (_core)
			_core->release();
	}

	template <typename T, typename RefCounterT>
	inline ptr<T, RefCounterT> &ptr<T, RefCounterT>::operator =(const ptr &rhs)
	{
		core *prev = _core;

		_ptr = rhs._ptr;
		_core = rhs._core;
		if (_core)
			_core->addref();
		if (prev)
			prev->release();
		return *this;
	}

	template <typename T, typename RefCounterT>
	template <typename U, typename DeleterT>
	inline void ptr<T, RefCounterT>::init(U *from, DeleterT deleter)
	{
		class core_impl : public core
		{
		public:
			explicit core_impl(DeleterT deleter, U *from)
				: _deleter(deleter), _ptr(from), _references(1)
			{	}

			virtual void addref()
			{	_references++;	}

			virtual void release()
			{
				if (!--_references)
					destroy();
			}

		private:
			AGGE_AVOID_INLINE void destroy()
			{
				_deleter(_ptr);
				delete this;
			}

		private:
			DeleterT _deleter;
			U *_ptr;
			RefCounterT _references;
		};

		_core = new core_impl(deleter, from);
		_ptr = from;
	}


	template <typename U, typename V>
	inline bool operator ==(const ptr<U> &lhs, const ptr<V> &rhs)
	{	return lhs.get() == rhs.get();	}

	template <typename U, typename V>
	inline bool operator !=(const ptr<U> &lhs, const ptr<V> &rhs)
	{	return lhs.get() != rhs.get();	}

	template <typename U, typename V>
	inline bool operator <(const ptr<U> &lhs, const ptr<V> &rhs)
	{	return lhs.get() < rhs.get();	}
}
