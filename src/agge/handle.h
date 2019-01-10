#pragma once

namespace agge
{
	class handle
	{
	public:
		enum { storage_size = 32 };

	public:
		template <typename NativeHandleT>
		handle(NativeHandleT native_handle);

		template <typename NativeHandleT>
		operator NativeHandleT() const;

		template <typename NativeHandleT>
		const NativeHandleT *address_of() const;

		template <typename NativeHandleT>
		NativeHandleT *address_of();

	private:
		unsigned char _handle[storage_size];
	};



	template <typename NativeHandleT>
	inline handle::handle(NativeHandleT native_handle)
	{	*address_of<NativeHandleT>() = native_handle;	}

	template <typename NativeHandleT>
	inline handle::operator NativeHandleT() const
	{	return *address_of<NativeHandleT>();	}

	template <typename NativeHandleT>
	inline const NativeHandleT *handle::address_of() const
	{
		const void *validating[sizeof(NativeHandleT) <= storage_size ? 1 : -1] = { _handle };
		return static_cast<const NativeHandleT *>(validating[0]);
	}

	template <typename NativeHandleT>
	inline NativeHandleT *handle::address_of()
	{
		void *validating[sizeof(NativeHandleT) <= storage_size ? 1 : -1] = { _handle };
		return static_cast<NativeHandleT *>(validating[0]);
	}
}
