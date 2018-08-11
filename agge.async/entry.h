#pragma once

namespace agge
{
	typedef unsigned char uint8_t;
	typedef unsigned char byte;
	typedef unsigned short int uint16_t;
	typedef unsigned short int int16_t;

	template <typename T>
	class static_entry
	{
	public:
		static void create(uint8_t *&at, uint8_t *start, uint8_t *end, const T &value);
		static void destroy(uint8_t *&at) throw();
		static T &get(uint8_t *&at, uint8_t *start, uint8_t *end) throw();
	};


	struct poly_entry_descriptor
	{
		uint16_t size;
		int16_t base_offset;
	};

	template <typename T>
	class poly_entry
	{
	public:
		template <typename FinalT>
		static void create(uint8_t *&at, uint8_t *start, uint8_t *end, const FinalT &value);
		template <typename FinalT>
		static void create(uint8_t *&at, uint8_t *start, uint8_t *end, FinalT &value);
		static void destroy(uint8_t *&at) throw();
		static T &get(uint8_t *&at, uint8_t *start, uint8_t *end) throw();

	private:
		template <typename FinalT>
		static poly_entry_descriptor *prepare_slot(uint8_t *&at, uint8_t *start, uint8_t *end, const FinalT &value);

		template <typename FinalT>
		static void post_construct(uint8_t *&at, poly_entry_descriptor *d, const T *object);
	};



	template <typename T>
	inline void static_entry<T>::create(uint8_t *&at, uint8_t *start, uint8_t *end, const T &value)
	{
		new(at) T(value);
		if (at + 2 * sizeof(T) > end)
			at = start;
		else
			at += sizeof(T);
	}

	template <typename T>
	inline void static_entry<T>::destroy(uint8_t *&at) throw()
	{
		reinterpret_cast<T *>(at)->~T();
		at += sizeof(T);
	}

	template <typename T>
	inline T &static_entry<T>::get(uint8_t *&at, uint8_t *start, uint8_t *end) throw()
	{
		if (end - at < sizeof(T))
			at = start;
		return *reinterpret_cast<T *>(at);
	}


	template <typename T>
	template <typename FinalT>
	inline poly_entry_descriptor *poly_entry<T>::prepare_slot(uint8_t *&at, uint8_t *start, uint8_t *end,
		const FinalT &value)
	{
		struct type_check_t { type_check_t(const T *) {	} } type_check(&value);

		if (end - at < sizeof(poly_entry_descriptor))
		{
			at = start;
		}
		else if (end - at < sizeof(poly_entry_descriptor) + sizeof(FinalT))
		{
			const poly_entry_descriptor zero = {};

			*reinterpret_cast<poly_entry_descriptor *>(at) = zero;
			at = start;
		}

		return reinterpret_cast<poly_entry_descriptor *>(at);
	}

	template <typename T>
	template <typename FinalT>
	inline void poly_entry<T>::post_construct(uint8_t *&at, poly_entry_descriptor *d, const T *object)
	{
		d->base_offset = static_cast<int16_t>(reinterpret_cast<const byte *>(object) - reinterpret_cast<byte *>(d + 1));
		at += d->size = sizeof(poly_entry_descriptor) + sizeof(FinalT);
	}

#define POLY_ENTRY_CREATE_DEF(cv)\
	template <typename T>\
	template <typename FinalT>\
	inline void poly_entry<T>::create(uint8_t *&at, uint8_t *start, uint8_t *end, FinalT cv value)\
	{\
		poly_entry_descriptor *d = prepare_slot(at, start, end, value);\
		post_construct<FinalT>(at, d, new(d + 1) FinalT(value));\
	}

	POLY_ENTRY_CREATE_DEF(const &);
	POLY_ENTRY_CREATE_DEF(&);

#undef POLY_ENTRY_CREATE_DEF

	template <typename T>
	inline void poly_entry<T>::destroy(uint8_t *&at) throw()
	{
		poly_entry_descriptor *d = reinterpret_cast<poly_entry_descriptor *>(at);

		reinterpret_cast<T *>(reinterpret_cast<byte *>(d + 1) + d->base_offset)->~T();
		at += d->size;
	}

	template <typename T>
	inline T &poly_entry<T>::get(uint8_t *&at, uint8_t *start, uint8_t *end) throw()
	{
		if (end - at < sizeof(poly_entry_descriptor) || !reinterpret_cast<const poly_entry_descriptor *>(at)->size)
			at = start;

		poly_entry_descriptor *d = reinterpret_cast<poly_entry_descriptor *>(at);
		return *reinterpret_cast<T *>(reinterpret_cast<byte *>(d + 1) + d->base_offset);
	}
}
