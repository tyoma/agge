#include <samples/common/font_loader.h>
#include <samples/common/serialization.h>
#include <samples/common/truetype.h>

#include <cstdio>
#include <strmd/deserializer.h>

using namespace agge;
using namespace std;
using namespace strmd;

namespace strmd
{
	template <> struct is_arithmetic<wchar_t> { static const bool value = true; };
}

namespace
{
	string convert_mb(const wstring &from)
	{
		const size_t required_length = wcstombs(0, from.c_str(), ~(size_t()));
		vector<char> buffer;

		if (required_length && required_length != static_cast<size_t>(-1))
		{
			buffer.resize(required_length);
			wcstombs(&buffer[0], from.c_str(), required_length);
		}
		return string(buffer.begin(), buffer.end());
	}

	class reader
	{
	public:
		reader(const string &path)
			: _file(fopen(path.c_str(), "rb"), &fclose)
		{	}

		void read(void *buffer, size_t size)
		{	fread(buffer, 1, size, _file.get());	}

	private:
		shared_ptr<FILE> _file;
	};

	string format_font_name(const string &typeface, int height, bool bold, bool italic, font::key::grid_fit grid_fit)
	{
		char buffer[100] = { 0 };

		_snprintf(buffer, sizeof(buffer), "%s-%d%s%s%s.fnt", typeface.c_str(), height, bold ? "b" : "",
			italic ? "i" : "", grid_fit == font::key::gf_strong ? "h" : grid_fit == font::key::gf_vertical ? "v" : "");
		return buffer;
	}
}

font::accessor_ptr font_loader::load(const wchar_t *typeface, int height, bool bold, bool italic,
	font::key::grid_fit grid_fit)
{
	reader r(format_font_name(convert_mb(typeface), height, bold, italic, grid_fit));
	deserializer<reader, varint> dser(r);
	shared_ptr<truetype::font> font(new truetype::font);

	dser(*font);
	return truetype::create_accessor(font);
}
