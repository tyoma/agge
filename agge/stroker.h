#pragma once

#include "pod_vector.h"

namespace agge
{
	typedef pod_vector<point_r> points;

	class stroke : noncopyable
	{
	public:
		struct cap;
		struct join;

	public:
		stroke();
		~stroke();

		void remove_all();
		void add_vertex(real_t x, real_t y, int command);
		
		int vertex(real_t *x, real_t *y);
		
		void width(real_t w);

		template <typename CapT>
		void set_cap(const CapT &c);

		template <typename JoinT>
		void set_join(const JoinT &j);

	private:
		enum state {
			start_cap = 0,
			outline_forward = 1,
			end_cap = 2,
			outline_backward = 3,
			end_poly = 4,
			stop = 5,
		};
		struct point_ref;
		typedef pod_vector<point_ref> input_vertices;

	private:
		void generate();

	private:
		input_vertices _input;
		points _output;
		input_vertices::const_iterator _input_iterator;
		points::const_iterator _output_iterator;
		cap *_cap;
		join *_join;
		real_t _width;
		int _state;
		bool _initial;
	};

	struct stroke::cap
	{
		virtual ~cap() { }
		virtual void calc(points &output, real_t w, const point_r &v0, real_t d, const point_r &v1) const = 0;
	};

	struct stroke::join
	{
		virtual ~join() { }
		virtual void calc(points &output, real_t w, const point_r &v0, real_t d01, const point_r &v1, real_t d12, const point_r &v2) const = 0;
	};


	template <typename SourceT, typename GeneratorT>
	class path_generator_adapter : noncopyable
	{
	public:
		path_generator_adapter(SourceT &source, GeneratorT &generator);

		void rewind(int /*path_id*/) { /*not implemented*/ }
		int vertex(real_t *x, real_t *y);

	private:
		enum state { initial = 0, accumulate = 1, generate = 2, stage_mask = 3, complete = 4 };

	private:
		void set_stage(state stage, bool force_complete = false);

	private:
		SourceT &_source;
		GeneratorT &_generator;
		real_t _start_x, _start_y;
		int _state;
	};



	template <typename CapT>
	inline void stroke::set_cap(const CapT &/*c*/)
	{
		_cap = new CapT;
	}

	template <typename JoinT>
	inline void stroke::set_join(const JoinT &/*j*/)
	{
		_join = new JoinT;
	}


	template <typename SourceT, typename GeneratorT>
	inline path_generator_adapter<SourceT, GeneratorT>::path_generator_adapter(SourceT &source, GeneratorT &generator)
		: _source(source), _generator(generator), _state(initial)
	{	}

	template <typename SourceT, typename GeneratorT>
	inline int path_generator_adapter<SourceT, GeneratorT>::vertex(real_t *x, real_t *y)
	{
		int command;

		for (;;)
			switch (_state & stage_mask)
			{
			case initial:
				command = _source.vertex(&_start_x, &_start_y);
				set_stage(accumulate, path_command_stop == command);

			case accumulate:
				if (_state & complete)
					return path_command_stop;

				_generator.remove_all();
				_generator.add_vertex(_start_x, _start_y, path_command_move_to);

				for (;;)
				{
					real_t xx, yy;

					command = _source.vertex(&xx, &yy);
					if (path_command_move_to == command)
					{
						_start_x = xx;
						_start_y = yy;
					}
					else if (path_command_stop != command)
					{
						_generator.add_vertex(xx, yy, command);
						continue;
					}
					break;
				}
				set_stage(generate, path_command_stop == command);

			case generate:
				command = _generator.vertex(x, y);
				if (path_command_stop != command)
					return command;
				set_stage(accumulate);
			}
	}

	template <typename SourceT, typename GeneratorT>
	inline void path_generator_adapter<SourceT, GeneratorT>::set_stage(state stage, bool force_complete)
	{	_state = (stage & stage_mask) | (force_complete ? complete : (_state & complete));	}
}
