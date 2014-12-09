#pragma once

#pragma pack(push, 1)

namespace aggx
{
	namespace pixel_format
	{
		union bgra32
		{
			struct quad
			{
				unsigned int b : 8;
				unsigned int g : 8;
				unsigned int r : 8;
				unsigned int a : 8;
			} quad;
			unsigned int value;
		};

		union rgba32
		{
			struct quad
			{
				unsigned int r : 8;
				unsigned int g : 8;
				unsigned int b : 8;
				unsigned int a : 8;
			} quad;
			unsigned int value;
		};
	}
}

#pragma pack(pop)
