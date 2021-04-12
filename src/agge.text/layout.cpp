#include <agge.text/layout.h>

#include <agge/config.h>

using namespace std;

namespace agge
{
	void layout::commit_glyph_run(text_line &current_line, glyph_run *&current)
	{
		if (!current->empty())
		{
			current_line.extend_end();
			current = &duplicate_last(_glyph_runs);
			current->set_end();
		}
		current->offset = zero();
	}

	pair<real_t, real_t> layout::setup_line_metrics(text_line &text_line_)
	{
		real_t descent = real_t();
		pair<real_t, real_t> m((real_t()), (real_t()));

		for (text_line::const_iterator i = text_line_.begin(), end = text_line_.end(); i != end; ++i)
		{
			const font_metrics grm = i->font_->get_metrics();

			if (m.first < grm.ascent)
				m.first = grm.ascent;
			if (m.second < grm.descent + grm.leading)
				m.second = grm.descent + grm.leading;
			if (descent < grm.descent)
				descent = grm.descent;
		}
		text_line_.descent = descent;
		return m;
	}

	pair<real_t, real_t> layout::setup_line_metrics(text_line &text_line_, const font_metrics &m)
	{	return text_line_.empty() ? make_pair(m.ascent, m.descent + m.leading) : setup_line_metrics(text_line_);	}
}
