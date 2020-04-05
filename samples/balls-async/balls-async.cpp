#include "balls_data.h"

#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <agge.async/worker.h>
#include <memory>

#include <samples/common/agg_ellipse.h>
#include <samples/common/shell.h>
#include <samples/common/timing.h>

using namespace agge;
using namespace std;
using namespace common;

const int c_render_thread_count = 2;
const int c_balls_number = 1000;

namespace
{
	template <typename T, unsigned align_by_order = 4>
	class aligned
	{
	public:
		aligned(const T &from)
		{	new(address()) T(from);	}

		aligned(aligned &other)
		{	new(address()) T(*other);	}

		~aligned()
		{	(**this).~T();	}

		T &operator *()
		{	return *static_cast<T *>(address());	}

	private:
		enum {
			align_by = 1 << align_by_order,
			align_mask = align_by - 1,
		};

	private:
		void operator =(const aligned &rhs);

		void *address()
		{	return _buffer + align_by - (((unsigned)(size_t)_buffer) & align_mask);	}

	private:
		agge::uint8_t _buffer[sizeof(T) + align_by];
	};

	template <typename RasterizerT>
	class async : noncopyable
	{
	public:
		typedef RasterizerT rasterizer_type;

	public:
		async(count_t n);

		unique_ptr<rasterizer_type> acquire();

		template <typename SurfaceT, typename BlenderT>
		void submit(unique_ptr<rasterizer_type> &rasterizer_, SurfaceT &surface, const BlenderT &blender);

		void wait_completion();

	private:
		typedef renderer_parallel renderer_type;
		typedef agge::worker< unique_ptr<rasterizer_type> > render_worker;
		typedef typename render_worker::work_in render_work;
		typedef agge::worker<render_work> sorter_worker;
		typedef typename sorter_worker::work_in sort_work;

		template <typename SurfaceT, typename BlenderT>
		struct sorter_work_impl;

		template <typename SurfaceT, typename BlenderT>
		struct renderer_no_window_work_impl;

	private:
		renderer_type _renderer;

		hybrid_event _filled_ready, _sorted_ready, _complete_ready, _all_ready;
		typename sorter_worker::in_queue_type _filled_rasterizers;
		typename sorter_worker::out_queue_type _sorted_rasterizers;
		typename render_worker::out_queue_type _complete_rasterizers;
		sorter_worker _sorter_w;
		render_worker _renderer_w;
	};

	template <typename RasterizerT>
	template <typename SurfaceT, typename BlenderT>
	struct async<RasterizerT>::renderer_no_window_work_impl : async<RasterizerT>::render_work
	{
		renderer_no_window_work_impl(renderer_no_window_work_impl &&other)
			: ras(move(other.ras)), surface(other.surface), blender(other.blender), ren(other.ren)
		{	}

		renderer_no_window_work_impl(unique_ptr<typename async::rasterizer_type> &&ras_, SurfaceT &surface_,
				const BlenderT &blender_, typename async::renderer_type &ren_)
			: ras(move(ras_)), surface(surface_), blender(blender_), ren(ren_)
		{	}

		virtual void run(typename async::render_worker::out_queue_type &output)
		{
			ren(surface, 0, *ras, *blender, winding<>());
			output.produce(ras);
		}

		unique_ptr<typename async::rasterizer_type> ras;
		SurfaceT &surface;
		aligned<BlenderT> blender;
		typename async::renderer_type &ren;
	};

	template <typename RasterizerT>
	template <typename SurfaceT, typename BlenderT>
	struct async<RasterizerT>::sorter_work_impl : async<RasterizerT>::sort_work
	{
		sorter_work_impl(sorter_work_impl &&other)
			: inner(move(other.inner))
		{	}

		sorter_work_impl(unique_ptr<typename async::rasterizer_type> &&ras, SurfaceT &surface, const BlenderT &blender,
				typename async::renderer_type &ren)
			: inner(move(ras), surface, blender, ren)
		{	}

		virtual void run(typename async::sorter_worker::out_queue_type &output)
		{
			inner.ras->sort();
			output.produce(move(inner));
		}

		typename async<RasterizerT>::renderer_no_window_work_impl<SurfaceT, BlenderT> inner;
	};


	template <typename RasterizerT>
	async<RasterizerT>::async(count_t n)
		: _renderer(c_render_thread_count), _filled_rasterizers(_filled_ready), _sorted_rasterizers(_sorted_ready),
			_complete_rasterizers(_complete_ready, (int)n, &_all_ready),
			_sorter_w(_filled_rasterizers, _sorted_rasterizers), _renderer_w(_sorted_rasterizers, _complete_rasterizers)
	{
		while (n--)
		{
			unique_ptr<rasterizer_type> r(new rasterizer_type);
			_complete_rasterizers.produce(r);
		}
	}

	template <typename RasterizerT>
	unique_ptr<RasterizerT> async<RasterizerT>::acquire()
	{
		unique_ptr<RasterizerT> ras;

		_complete_rasterizers.consume([&](unique_ptr<RasterizerT> &o) {
			ras = move(o);
		});
		ras->reset();
		return move(ras);
	}

	template <typename RasterizerT>
	template <typename SurfaceT, typename BlenderT>
	void async<RasterizerT>::submit(unique_ptr<rasterizer_type> &rasterizer_, SurfaceT &surface, const BlenderT &blender)
	{
		sorter_work_impl<SurfaceT, BlenderT> w(move(rasterizer_), surface, blender, _renderer);
		_filled_rasterizers.produce(move(w));
	}

	template <typename RasterizerT>
	void async<RasterizerT>::wait_completion()
	{
		_all_ready.wait();
		_all_ready.signal(); // acquire & wait_completion must be called from the same thread/queue.
	}


	class Balls : public application
	{
	public:
		Balls()
			: _async(10), _balls(c_balls)
		{	_balls.resize(c_balls_number);	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const float dt = 0.3f * (float)stopwatch(_balls_timer);
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };

			stopwatch(counter);
			fill(surface, area, platform_blender_solid_color(color::make(255, 255, 255)));
			timings.clearing += stopwatch(counter);

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
				move_and_bounce(*i, dt, static_cast<real_t>(surface.width()), static_cast<real_t>(surface.height()));

			stopwatch(counter);
			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
			{
				platform_blender_solid_color brush(i->color);
				unique_ptr<async_t::rasterizer_type> ras(_async.acquire().release());

				add_path(*ras, agg::ellipse(i->x, i->y, i->radius, i->radius));
				_async.submit(ras, surface, brush);
			}
			_async.wait_completion();
			timings.rendition += stopwatch(counter);
		}

	private:
		typedef async< rasterizer< clipper<int> > > async_t;

	private:
		async_t _async;
		long long _balls_timer;
		vector<ball> _balls;
	};
}

application *agge_create_application(services &/*s*/)
{	return new Balls;	}
