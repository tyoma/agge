#include <samples/common/font_loader.h>
#include <samples/common/serialization.h>
#include <samples/common/truetype.h>

#include <stdlib.h>
#include <strmd/deserializer.h>
#include <strmd/container_ex.h>
#include <tests/common/scoped_ptr.h>

using namespace agge;
using namespace std;
using namespace strmd;

namespace
{
	string to_string(int value)
	{
		char buffer[20] = { };

		sprintf(buffer, "%d", value);
		return buffer;
	}

	string format_font_name(const string &family, int height, font_weight weight, bool italic, font_hinting grid_fit)
	{
		return family + "-" + to_string(height) + (weight >= bold ? "b" : "") + (italic ? "i" : "")
			+ (grid_fit == hint_strong ? "h" : grid_fit == hint_vertical ? "v" : "") + ".fnt";
	}
}

font_loader::font_loader(services &s)
	: _services(s)
{
}

font::accessor_ptr font_loader::load(const font_descriptor &descriptor)
{
	tests::scoped_ptr<stream> r(_services.open_file(format_font_name(descriptor.family, descriptor.height,
		descriptor.weight, descriptor.italic, descriptor.hinting).c_str()));
	deserializer<stream, varint> dser(*r);
	shared_ptr<truetype::font> font(new truetype::font);

	dser(*font);
	return truetype::create_accessor(font, descriptor);
}
