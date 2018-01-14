#include <samples/common/font_loader.h>
#include <samples/common/serialization.h>
#include <samples/common/truetype.h>

#include <stdlib.h>
#include <strmd/deserializer.h>

using namespace agge;
using namespace std;
using namespace strmd;

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

	string to_string(int value)
	{
		char buffer[20] = { };

		sprintf(buffer, "%d", value);
		return buffer;
	}

	string format_font_name(const string &typeface, int height, bool bold, bool italic, font::key::grid_fit grid_fit)
	{
		return typeface + "-" + to_string(height) + (bold ? "b" : "") + (italic ? "i" : "")
			+ (grid_fit == font::key::gf_strong ? "h" : grid_fit == font::key::gf_vertical ? "v" : "") + ".fnt";
	}
}

font_loader::font_loader(services &s)
	: _services(s)
{
}

font::accessor_ptr font_loader::load(const wchar_t *typeface, int height, bool bold, bool italic,
	font::key::grid_fit grid_fit)
{
	auto_ptr<stream> r(_services.open_file(format_font_name(convert_mb(typeface), height, bold, italic, grid_fit).c_str()));
	deserializer<stream, varint> dser(*r);
	shared_ptr<truetype::font> font(new truetype::font);

	dser(*font);
	return truetype::create_accessor(font);
}
