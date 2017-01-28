#pragma once

#include <agge/types.h>

struct HBITMAP__;
typedef struct HBITMAP__ *HBITMAP;

struct HDC__;
typedef struct HDC__ *HDC;

namespace agge
{
	namespace platform
	{
		namespace tests
		{
			class gdi_surface : noncopyable
			{
			public:
				gdi_surface(count_t width, count_t height);
				~gdi_surface();

				HDC lock();
				void unlock();

				bool is_valid_handle(HBITMAP hbitmap) const;

			public:
				const count_t width, height;
				void *data;

			private:
				HDC _context;
				HBITMAP _bitmap, _previous_bitmap;
			};



			template <size_t n>
			bool operator ==(const uint8_t (&lhs)[n], const gdi_surface &rhs)
			{	return 0 == memcmp(lhs, rhs.data, n);	}
		}
	}
}
