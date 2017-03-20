#pragma once

#include "helpers.h"

#include <agge/path.h>
#include <agge/tools.h>
#include <tests/common/helpers.h>

#include <ut/assert.h>
#include <utility>

namespace agge
{
	namespace tests
	{
		namespace mocks
		{
			struct cell
			{
				int x;
				int area;
				int cover;
			};

			class path
			{
			public:
				struct point
				{
					real_t x, y;
					int command;

					bool operator ==(const point &rhs) const
					{
						if (command != rhs.command)
							return false;
						if (command == path_command_stop)
							return true;
						return equal(x, rhs.x) && equal(y, rhs.y);
					}
				};

			public:
				path()
					: position(0)
				{	}

				template <typename T, size_t n>
				path(T (&points_)[n])
					: points(points_, points_ + n), position(0)
				{	}

				void rewind(unsigned /*path_id*/)
				{	position = 0;	}

				int vertex(real_t *x, real_t *y)
				{
					if (position < points.size())
					{
						*x = points[position].x, *y = points[position].y;
						return points[position++].command;
					}
					return path_command_stop;
				}

			public:
				std::vector<point> points;
				size_t position;
			};

			template <typename T>
			struct coords_pair
			{
				T x1, y1;
				T x2, y2;

				bool operator ==(const coords_pair<T> &rhs) const
				{	return equal(x1, rhs.x1) && equal(y1, rhs.y1) && equal(x2, rhs.x2) && equal(y2, rhs.y2);	}
			};


			template <typename T>
			class vector_rasterizer
			{
			public:
				void line(T x1, T y1, T x2, T y2)
				{
					coords_pair<T> segment = { x1, y1, x2, y2 };

					if (!segments.empty())
					{
						coords_pair<T> &last = segments.back();

						if (segment.x1 == segment.x2 && last.x1 == last.x2 && segment.x1 == last.x1)
						{
							last.y2 = y2;
							return;
						}
						else if (segment.y1 == segment.y2 && last.y1 == last.y2 && segment.y1 == last.y1)
						{
							last.x2 = x2;
							return;
						}
					}
					segments.push_back(segment);
				}

			public:
				std::vector< coords_pair<T> > segments;
			};

			template <size_t precision>
			class mask
			{
			public:
				typedef std::pair<const cell * /*begin*/, const cell * /*end*/> scanline_cells;

				enum { _1_shift = precision };

			public:
				template <typename T, int n>
				mask(const T (&cells)[n], int y0)
					: _min_y(y0), _height(n)
				{
					int min_x = 0x7FFFFFFF, max_x = -0x7FFFFFFF;

					for (int i = 0; i != n; ++i)
					{
						_cells.push_back(cells[i]);
						for (int j = 0; j != cells[i].second - cells[i].second; ++j)
						{
							min_x = agge_min(min_x, (cells[i].first + j)->x);
							max_x = agge_max(min_x, (cells[i].first + j)->x);
						}
					}
					_width = agge_max(max_x - min_x, -1) + 1;
				}

				scanline_cells operator [](int y) const
				{	return _cells.at(y - _min_y);	}

				int min_y() const
				{	return _min_y;	}

				int height() const
				{	return _height;	}

			protected:
				int width() const
				{	return _width;	}

			private:
				int _width, _min_y, _height;
				std::vector<scanline_cells> _cells;
			};


			template <size_t precision>
			class mask_full : public mask<precision>
			{
			public:
				template <typename T, int n>
				mask_full(const T (&cells)[n], int y0)
					: mask<precision>(cells, y0)
				{	}

				using mask<precision>::width;
			};


			template <typename CoverT = uint8_t>
			class renderer_adapter
			{
			public:
				typedef CoverT cover_type;

				struct render_log_entry
				{
					int x;
					std::vector<cover_type> covers;

					bool operator ==(const render_log_entry &rhs) const
					{	return x == rhs.x && covers == rhs.covers;	}
				};

			public:
				bool set_y(int y)
				{
					current_y = y;
					return set_y_result;
				}

				void operator ()(int x, count_t length, const cover_type *covers)
				{
					using namespace std;

					render_log_entry e = { x, vector<cover_type>(covers, covers + length) };
					render_log.push_back(e);
					raw_render_log.push_back(make_pair(covers, length));
				}

			public:
				int current_y;
				bool set_y_result;
				std::vector<render_log_entry> render_log;
				std::vector< std::pair<const cover_type *, count_t> > raw_render_log;
			};


			template <typename PixelT, size_t x_guard = 0, size_t y_guard = 0>
			class bitmap
			{
			public:
				typedef PixelT pixel;

			public:
				bitmap(count_t width, count_t height)
					: _width(width), _height(height), data((width + x_guard) * (height + y_guard))
				{	}

				pixel *row_ptr(count_t y)
				{	return &data[y * (_width + x_guard)];	}

				count_t width() const
				{	return _width;	}

				count_t height() const
				{	return _height;	}

			public:
				std::vector<pixel> data;

			private:
				count_t _width, _height;
			};


			template <typename PixelT, typename CoverT>
			class blender
			{
			public:
				typedef PixelT pixel;
				typedef CoverT cover_type;

				struct fill_log_entry;

			public:
				void operator ()(PixelT *pixels, int x, int y, unsigned int length) const
				{
					fill_log_entry entry = { pixels, x, y, length };

					filling_log.push_back(entry);
				}

				void operator ()(PixelT *pixels, int x, int y, unsigned int length, const cover_type *covers) const
				{
					assert_not_equal(0u, length);

					int offset = sizeof(cover_type) * 8;
					int mask_x = 0x000000FF << offset;
					int mask_y = 0x0000FF00 << offset;

					for (; length; --length, ++pixels, ++covers)
						*pixels = static_cast<pixel>(static_cast<int>(*covers)
							+ ((x << offset) & mask_x)
							+ ((y << (offset + 8)) & mask_y));
				}

				mutable std::vector<fill_log_entry> filling_log;
			};


			template <typename PixelT, typename CoverT>
			struct blender<PixelT, CoverT>::fill_log_entry
			{
				pixel *pixels;
				int x;
				int y;
				unsigned int length;

				bool operator ==(const fill_log_entry &rhs) const
				{	return pixels == rhs.pixels && x == rhs.x && y == rhs.y && length == rhs.length;	}
			};


			template <typename T, size_t precision>
			struct simple_alpha
			{
				T operator ()(int area) const
				{
					int v = area >> (precision + 1);

					if (v >= 1 << precision)
						v = (1 << precision) - 1;
					return static_cast<T>(v);
				}
			};
		}
	}
}
