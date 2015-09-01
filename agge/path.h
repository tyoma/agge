#pragma once

namespace agge
{
	enum path_commands {
		path_command_stop = 0x00,
		path_command_move_to = 0x01,
		path_command_line_to = 0x02,
		
		path_command_end_poly = 0x10,

		path_vertex_mask = 0x07,
		path_command_mask = 0x1F
	};

	enum path_flags {
		path_flag_close = 0x20,
		path_flags_mask = 0xE0
	};

	inline bool is_vertex(int c)
	{	return 0 != (path_vertex_mask & c);	}

	inline bool is_end_poly(int c)
	{	return path_command_end_poly == (path_command_mask & c);	}

	inline bool is_close(int c)
	{	return 0 != (path_flag_close & c);	}
}
