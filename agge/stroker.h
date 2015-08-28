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

	private:
		struct point_ref;
		
		typedef pod_vector<point_ref> input_vertices;

	private:
		void generate();

	private:
		input_vertices _input;
		points _output;
		points::const_iterator _output_iterator;
		cap *_cap;
		real_t _width;
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
