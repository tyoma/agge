#include "text.h"

#include "../common/MainDialog.h"
#include "../common/timing.h"

#include <functional>
#include <tchar.h>
#include <windows.h>

using namespace agge;
using namespace std;
using namespace std::placeholders;

namespace
{
	shared_ptr<void> select(HDC hdc, HGDIOBJ hobject)
	{	return shared_ptr<void>(::SelectObject(hdc, hobject), bind(::SelectObject, hdc, _1));	}

	class MemDC : noncopyable
	{
	public:
		MemDC(::bitmap &surface)
			: _dc(::CreateCompatibleDC(NULL), &DeleteDC)
		{	_selector = select(*this, surface.native());	}

		operator HDC() const
		{	return static_cast<HDC>(_dc.get());	}

	private:
		shared_ptr<void> _dc, _selector;
	};

	class TextDrawer : public Drawer
	{
		virtual void draw(::bitmap &surface, Timings &timings)
		{
			const basic_string<TCHAR> &text = c_text2;
			long long timer;
			MemDC dc(surface);
			shared_ptr<void> font(::CreateFont(26, 0, 0, 0, 0, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Georgia")),
				&::DeleteObject);
			shared_ptr<void> font_selector = select(dc, font.get());
			RECT rc = { 0, 0, surface.width(), surface.height() };

			::SetBkColor(dc, RGB(0, 0, 0));
			::SetTextColor(dc, RGB(255, 255, 255));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, "", 0, 0);

			stopwatch(timer);
			::DrawText(dc, text.c_str(), (int)text.size(), &rc, DT_WORDBREAK | DT_CALCRECT);
			timings.stroking += stopwatch(timer);

			::SetBkColor(dc, RGB(100, 100, 100));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, "", 0, 0);

			stopwatch(timer);
			::DrawText(dc, text.c_str(), (int)text.size(), &rc, DT_WORDBREAK);
			double t = stopwatch(timer);
			timings.rasterization += t;
			timings.rendition += t / c_text2.size();
		}

		virtual void resize(int /*width*/, int /*height*/)
		{
		}
	};
}

int main()
{
	TextDrawer d;
	MainDialog dlg(d);

	MainDialog::PumpMessages();
}
