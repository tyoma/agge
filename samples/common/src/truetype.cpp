#include <samples/common/truetype.h>

using namespace agge;
using namespace std;

namespace truetype
{
	namespace
	{
		real_t fixed2real(int value)
		{	return value / 65536.0f;	}

		agge::glyph::path_point path_point(int command, real_t x, real_t y)
		{
			agge::glyph::path_point p = { command, x, y };
			return p;
		}

		void qbezier(agge::glyph::outline_storage &outline, real_t x2, real_t y2, real_t x3, real_t y3, real_t d = 0.03f)
		{
			const real_t x1 = (outline.end() - 1)->x, y1 = (outline.end() - 1)->y;
			
			for (real_t t = d; t < 1.0f; t += d)
			{
				real_t t_ = 1.0f - t;
				
				outline.push_back(path_point(path_command_line_to,
					t_ * t_ * x1 + 2.0f * t_ * t * x2 + t * t * x3,
					t_ * t_ * y1 + 2.0f * t_ * t * y2 + t * t * y3));
			}
			outline.push_back(path_point(path_command_line_to, x3, y3));
		}
	}

	agge::font::accessor_ptr create_accessor(const shared_ptr<font> &tt_font, const agge::font_descriptor &d)
	{
		class accessor : public agge::font::accessor
		{
		public:
			accessor(shared_ptr<font> tt_font, const agge::font_descriptor &d)
				: _tt_font(tt_font), _descriptor(d)
			{	}

		private:
			virtual font_descriptor get_descriptor() const
			{	return _descriptor;	}

			virtual agge::font_metrics get_metrics() const
			{
				agge::font_metrics m = {
					fixed2real(_tt_font->metrics.ascent),
					fixed2real(_tt_font->metrics.descent),
					fixed2real(_tt_font->metrics.leading)
				};

				return m;
			}

			virtual agge::uint16_t get_glyph_index(wchar_t character) const
			{
				unordered_map<wchar_t, agge::uint16_t>::const_iterator i = _tt_font->char_to_glyph.find(character);
				return i != _tt_font->char_to_glyph.end() ? i->second : 0;
			}

			virtual agge::glyph::outline_ptr load_glyph(agge::uint16_t index, agge::glyph::glyph_metrics &m) const
			{
				agge::glyph::outline_ptr o(new agge::glyph::outline_storage);
				agge::glyph::glyph_metrics empty = { };

				m = empty;
				if (index < _tt_font->glyphs.size())
				{
					glyph &g = _tt_font->glyphs[index];

					m.advance_x = fixed2real(g.metrics.advance_x);
					m.advance_y = fixed2real(g.metrics.advance_y);
					for (vector<poly>::const_iterator p = g.polygons.begin(); p != g.polygons.end(); ++p)
					{
						o->push_back(path_point(path_command_move_to, fixed2real(p->start.x), fixed2real(p->start.y)));
						for (vector<segment>::const_iterator seg = p->segments.begin(); seg != p->segments.end(); ++seg)
							switch (seg->type)
							{
							case segment::line:
								for (vector< point<int> >::const_iterator i = seg->points.begin(); i != seg->points.end(); ++i)
									o->push_back(path_point(path_command_line_to, fixed2real(i->x), fixed2real(i->y)));
								break;

							case segment::qspline:
								for (size_t i = 0, count = seg->points.size(); i != count - 1; ++i)
								{
									const point<int> &pnt_b = seg->points[i]; // B is always the current point
									point<int> pnt_c = seg->points[i + 1];

									if (i < count - 2) // If not on last spline, compute C
									{
										// midpoint (x,y)
										*(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
										*(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
									}
									qbezier(*o, fixed2real(pnt_b.x), fixed2real(pnt_b.y), fixed2real(pnt_c.x), fixed2real(pnt_c.y));
								}
								break;

							case segment::cspline:
								break;
							}
						(o->end() - 1)->command |= path_flag_close;
					}
				}
				return o;
			}

		private:
			shared_ptr<font> _tt_font;
			font_descriptor _descriptor;
		};

		return agge::font::accessor_ptr(new accessor(tt_font, d));
	}
}
