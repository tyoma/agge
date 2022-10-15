#pragma once

#include "types.h"

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


	template <typename SourceT, typename GeneratorT>
	class path_generator_adapter
	{
	public:
		path_generator_adapter(const SourceT &source, GeneratorT &generator);

		void rewind(int /*path_id*/) { /*not implemented*/ }
		int vertex(real_t *x, real_t *y);

	private:
		enum state { initial = 0, accumulate = 1, generate = 2, stage_mask = 3, complete = 4 };

	private:
		void set_stage(state stage, bool force_complete = false);

	private:
		SourceT _source;
		GeneratorT &_generator;
		real_t _start_x, _start_y;
		int _state;
	};


	class path_close
	{
	public:
		path_close();

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		unsigned int _step;
	};


	template <typename PathIterator1T, typename PathIterator2T = void>
	class join
	{
	public:
		join(const PathIterator1T &lhs, const PathIterator2T &rhs);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		PathIterator1T _lhs;
		PathIterator2T _rhs;
	};

	template <typename PathIterator1T>
	class join<PathIterator1T, void>
	{
	public:
		join(const PathIterator1T &lhs);

		void rewind(unsigned id);
		int vertex(real_t *x, real_t *y);

	private:
		PathIterator1T _lhs;
	};



	template <typename SourceT, typename GeneratorT>
	path_generator_adapter<SourceT, GeneratorT> assist(const SourceT &source, GeneratorT &generator)
	{	return path_generator_adapter<SourceT, GeneratorT>(source, generator);	}

	//template <typename PathIterator1T, typename PathIterator2T>
	//join<PathIterator1T, PathIterator2T> join(const PathIterator1T &path1, const PathIterator2T &path2)
	//{	return join<PathIterator1T, PathIterator2T>(path1, path2);	}

	inline bool is_vertex(int c)
	{	return 0 != (path_vertex_mask & c);	}

	inline bool is_end_poly(int c)
	{	return path_command_end_poly == (path_command_mask & c);	}

	inline bool is_close(int c)
	{	return 0 != (path_flag_close & c);	}

	template <typename AcceptorT>
	inline void add_polyline_vertex(AcceptorT &acceptor, real_t x, real_t y, int command)
	{
		if (path_command_move_to == (path_vertex_mask & command))
			acceptor.move_to(x, y);
		else if (path_command_line_to == (path_vertex_mask & command))
			acceptor.line_to(x, y);
		
		if (is_close(command))
			acceptor.close_polygon();
	}

	template <typename SinkT, typename PathIteratorT>
	inline void add_path(SinkT &sink, PathIteratorT path)
	{
		real_t x, y;

		path.rewind(0);
		for (int command; command = path.vertex(&x, &y), path_command_stop != command; )
			add_polyline_vertex(sink, x, y, command);
	}


	template <typename SourceT, typename GeneratorT>
	inline path_generator_adapter<SourceT, GeneratorT>::path_generator_adapter(const SourceT &source, GeneratorT &generator)
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


	inline path_close::path_close()
		: _step(0)
	{	}

	inline void path_close::rewind(unsigned)
	{	_step = 0;	}

	inline int path_close::vertex(real_t *, real_t *)
	{	return _step ? path_command_stop : (_step++, path_command_end_poly | path_flag_close);	}


	template <typename PathIterator1T, typename PathIterator2T>
	inline join<PathIterator1T, PathIterator2T>::join(const PathIterator1T &lhs, const PathIterator2T &rhs)
		: _lhs(lhs), _rhs(rhs)
	{	}

	template <typename PathIterator1T, typename PathIterator2T>
	inline void join<PathIterator1T, PathIterator2T>::rewind(unsigned /*id*/)
	{
		_lhs.rewind(0);
		_rhs.rewind(0);
	}

	template <typename PathIterator1T, typename PathIterator2T>
	inline int join<PathIterator1T, PathIterator2T>::vertex(real_t *x, real_t *y)
	{
		switch (int command = _lhs.vertex(x, y))
		{
		default:
			return command;

		case path_command_stop:
			switch (command = _rhs.vertex(x, y))
			{
			default:
				return command;

			case path_command_move_to:
				return path_command_line_to;
			}
		}
	}


	template <typename PathIterator1T>
	inline join<PathIterator1T, void>::join(const PathIterator1T &lhs)
		: _lhs(lhs)
	{	}

	template <typename PathIterator1T>
	inline void join<PathIterator1T, void>::rewind(unsigned id)
	{	_lhs.rewind(id);	}

	template <typename PathIterator1T>
	inline int join<PathIterator1T, void>::vertex(real_t *x, real_t *y)
	{	return _lhs.vertex(x, y);	}


	template <typename T>
	inline join<T, void> joining(const T &lhs)
	{	return join<T, void>(lhs);	}

	template <typename T11, typename T12, typename T2>
	inline join<join<T11, T12>, T2> operator &(const join<T11, T12> &lhs, const T2 &rhs)
	{	return join<join<T11, T12>, T2>(lhs, rhs);	}
}
