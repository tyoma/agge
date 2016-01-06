#pragma once

#include "vertex_sequence.h"

namespace agge
{
	class dash : private vertex_sequence
	{
	public:
		dash();

		void reset();
		void add_dash(real_t dash_length, real_t gap_length);

		void remove_all();
		using vertex_sequence::move_to;
		using vertex_sequence::line_to;
		void close_polygon();

		int vertex(real_t *x, real_t *y);

	private:
		enum state { initial = 0, move = 1, generate = 2, complete = 3 };

		struct dash_gap
		{
			real_t dash_length, gap_length;
		};

	private:
		vertex_sequence::const_iterator _i;
		dash_gap _dashes;
		real_t _remainder;
		point_r _previous;
		int _state;
	};
}
