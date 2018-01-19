#pragma once

#include <agge/types.h>

#include <string.h>

struct CGImage;
typedef struct CGImage *CGImageRef;

struct CGContext;
typedef struct CGContext *CGContextRef;

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

				CGContextRef lock();
				void unlock();

			public:
				const count_t width, height;
				uint8_t *data;

			private:
				CGContextRef _context;
			};



			template <size_t n>
			bool operator ==(const uint8_t (&lhs)[n], const gdi_surface &rhs)
			{	return 0 == memcmp(lhs, rhs.data, n);	}
		}
	}
}
