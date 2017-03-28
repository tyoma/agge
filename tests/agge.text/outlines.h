#pragma once

#include <agge.text/font.h>

namespace agge
{
	namespace tests
	{
		const glyph::path_point c_outline_1[] = {
			{ path_command_move_to, 1.1f, 2.4f },
			{ path_command_line_to, 4.1f, 7.5f },
			{ path_command_line_to | path_flag_close, 5.5f, 1.1f },
		};

		const glyph::path_point c_outline_2[] = {
			{ path_command_move_to, 1.1f, 2.4f },
			{ path_command_line_to, 4.1f, 7.5f },
			{ path_command_line_to, 4.1f, 4.5f },
			{ path_command_line_to | path_flag_close, 5.5f, 1.1f },
		};

		const glyph::path_point c_outline_diamond[] = {
			{ path_command_move_to, -2.1f, 0.0f },
			{ path_command_line_to, 0.0f, -2.1f },
			{ path_command_line_to, 2.1f, 0.0f },
			{ path_command_line_to | path_flag_close, 0.0f, 2.1f },
		};

	}
}
