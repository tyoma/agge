#include <agge/rasterizer.h>

#include <utee/ut/assert.h>
#include <utee/ut/test.h>
#include <utility>

#define mock_vrasterizer(cells) vector_rasterizer_mockup<decltype(cells), cells>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			template <typename T, size_t N>
			vector<T> mkvector(T (&p)[N])
			{	return vector<T>(p, p + N);	}

			struct cell
			{
				int x, y, area, cover;
			};

			template <typename T>
			struct array_info;

			template <typename T, size_t N>
			struct array_info<T[N]>
			{
				enum { size = N };
				typedef T array_type[N];
			};

			template <typename T, T Array>
			class vector_rasterizer_mockup
			{
				enum { cells_count = array_info<T>::size };

			public:
				typedef unsigned int count_t;
				typedef typename array_info<T>::array_type cell;
				typedef std::pair<int, int> range;
				typedef pair<const cell *, count_t> scanline_cells;

			public:
				vector_rasterizer_mockup()
					: _sorted(false)
				{	}

				void sort()
				{	_sorted = true;	}

				range vrange() const;
				range hrange() const;

				scanline_cells get_scanline_cells(int y) const;


			public:
				bool _sorted;
				vector<cell> _cells;
			};

			struct rendition_block
			{
				int x, y;
				vector<unsigned char> covers_sequence;

				bool operator ==(const rendition_block &rhs) const
				{	return x == rhs.x && y == rhs.y && covers_sequence == rhs.covers_sequence;	}
			};

			class mock_renderer
			{
			public:
				void operator ()(int x, int y, int n, const unsigned char *covers)
				{
					const rendition_block rb = { x, y, vector<unsigned char>(covers, covers + n) };

					rendition_log.push_back(rb);
				}

				vector<rendition_block> rendition_log;
			};


			cell cells_sequence_1[] = {
				{ 1, 1, 0xFF * 0xFF * 2, 0 },
			};
		}

		begin_test_suite( ScanlineRasterizerTests )
			test( Smoke )
			{
			}
		end_test_suite
	}
}
