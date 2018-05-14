#pragma once

#include "types.h"

#pragma warning(push)
#pragma warning(disable: 4702)

namespace agge
{
	class poly_entry
	{
	public:
		template <typename FinalT>
		static void create(poly_entry *&at, const FinalT &from);

		template <typename FinalT>
		static uint16_t size() throw();

		virtual ~poly_entry() {	}
		virtual void clone(poly_entry *&at) const = 0;

		template <typename BaseT>
		BaseT &get() throw();
		poly_entry *next() throw();

	private:
		template <typename FinalT>
		class final_entry;

	private:
		uint16_t _size;
		uint16_t _offset;
	};

	template <typename FinalT>
	class poly_entry::final_entry : poly_entry
	{
	public:
		final_entry(const FinalT &from);
		virtual void clone(poly_entry *&at) const;

	private:
		FinalT _object;
	};


	template <typename BaseT>
	class poly_buffer : noncopyable
	{
	public:
		poly_buffer(count_t byte_size);
		~poly_buffer();

		bool empty() const throw();

		template <typename T>
		void push_back(const T &value);

		void pop_front() throw();

		BaseT &front() throw();

	private:
		void grow_by(count_t sz);
		count_t capacity() const throw();
		void destroy() throw();

	private:
		poly_entry *_write, *_read;
		void *_start, *_limit;
	};



	template <typename BaseT>
	inline poly_buffer<BaseT>::poly_buffer(count_t byte_size)
		: _write(0), _read(0), _start(0), _limit(0)
	{	grow_by(byte_size);	}

	template <typename BaseT>
	inline poly_buffer<BaseT>::~poly_buffer()
	{	destroy();	}

	template <typename BaseT>
	inline bool poly_buffer<BaseT>::empty() const throw()
	{	return _read == _write;	}

	template <typename BaseT>
	template <typename T>
	inline void poly_buffer<BaseT>::push_back(const T &value)
	{
		if (static_cast<uint8_t *>(_limit) - reinterpret_cast<uint8_t *>(_write) < poly_entry::size<T>())
			grow_by(poly_entry::size<T>());
		poly_entry::create(_write, value);
	}

	template <typename BaseT>
	inline void poly_buffer<BaseT>::pop_front() throw()
	{
		poly_entry *current = _read, *next = current->next();

		_read = next == _write ? _write = static_cast<poly_entry *>(_start) : next;
		current->~poly_entry();
	}

	template <typename BaseT>
	inline BaseT &poly_buffer<BaseT>::front() throw()
	{	return _read->get<BaseT>();	}

	template <typename BaseT>
	inline void poly_buffer<BaseT>::grow_by(count_t sz)
	{
		count_t new_size = capacity() + sz;
		void *start = new uint8_t[new_size];
		poly_entry *write = static_cast<poly_entry *>(start);

		for (poly_entry *i = _read; i != _write; i = i->next())
			i->clone(write);
		destroy();
		_start = start;
		_write = write;
		_read = static_cast<poly_entry *>(start);
		_limit = static_cast<uint8_t *>(_start) + new_size;
	}

	template <typename BaseT>
	inline count_t poly_buffer<BaseT>::capacity() const throw()
	{	return static_cast<count_t>(static_cast<uint8_t *>(_limit) - static_cast<uint8_t *>(_start));	}

	template <typename BaseT>
	inline void poly_buffer<BaseT>::destroy() throw()
	{
		while (!empty())
			pop_front();
		delete []static_cast<uint8_t *>(_start);
	}


	template <typename FinalT>
	inline void poly_entry::create(poly_entry *&at, const FinalT &from)
	{
		new(at) final_entry<FinalT>(from);
		at = reinterpret_cast<poly_entry *>(reinterpret_cast<uint8_t *>(at) + size<FinalT>());
	}

	template <typename FinalT>
	inline uint16_t poly_entry::size() throw()
	{	return sizeof(final_entry<FinalT>);	}

	template <typename BaseT>
	inline BaseT &poly_entry::get() throw()
	{	return *reinterpret_cast<BaseT *>(reinterpret_cast<uint8_t *>(this) + _offset);	}

	inline poly_entry *poly_entry::next() throw()
	{	return reinterpret_cast<poly_entry *>(reinterpret_cast<uint8_t *>(this) + _size);	}


	template <typename FinalT>
	inline poly_entry::final_entry<FinalT>::final_entry(const FinalT &from)
		: _object(from)
	{
		this->_size = size<FinalT>();
		this->_offset = static_cast<uint16_t>(reinterpret_cast<uint8_t *>(&_object) - reinterpret_cast<uint8_t *>(this));
	}

	template <typename FinalT>
	inline void poly_entry::final_entry<FinalT>::clone(poly_entry *&at) const
	{	create(at, _object);	}
}

#pragma warning(pop)
