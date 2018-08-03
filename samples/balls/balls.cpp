#include "balls_data.h"

#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <memory>
#include <poly-queue/circular.h>
#include <src/agge/hybrid_event.h>
#include <src/agge/thread.h>
#include <tests/agge/mt.h>

#include <misc/experiments/common/ellipse.h>

#include <samples/common/shell.h>
#include <samples/common/timing.h>

#pragma warning(disable:4355)

using namespace agge;
using namespace std;
using namespace common;

const int c_thread_count = 4;
const int c_balls_number = 2000;

namespace
{
	template <typename RasterizerT>
	class async
	{
	public:
		typedef RasterizerT rasterizer_type;

	public:
		async(count_t n);

		rasterizer_type* acquire();

		template <typename SurfaceT, typename BlenderT>
		void submit(rasterizer_type *rasterizer_, SurfaceT &surface, rect_i *window, const BlenderT &blender);

		void wait_completion();

	private:
		class rendition_pack;

		typedef renderer_parallel renderer_type;

	private:
		static void sorter_proc(void *p);
		static void rendition_proc(void *p);

	private:
		renderer_type _renderer;
		pq::circular_buffer<rendition_pack> _unsorted;
		hybrid_event _unsorted_ready;
		pq::circular_buffer<rendition_pack> _sorted;
		hybrid_event _sorted_ready;
		pq::circular_buffer<rasterizer_type *> _free;
		hybrid_event _free_ready;
		hybrid_event _complete;
		thread _sorter_thread;
		thread _renderer_thread;
		int _n;
	};

#pragma pack(1)

	template <typename RasterizerT>
	class async<RasterizerT>::rendition_pack
	{
	public:
		rendition_pack(rasterizer_type *rasterizer_, platform_bitmap &surface, rect_i * /*window*/, const platform_blender_solid_color &blender)
			: _surface(surface), _blender(blender)
		{	this->rasterizer = move(rasterizer_);	}

		void render(renderer_type &ren)
		{	ren(_surface, 0, *this->rasterizer, _blender, winding<>());	}

	public:
		rasterizer_type * rasterizer;

	private:
		void operator =(const rendition_pack&);

	private:
		platform_bitmap & _surface;
		char b[8];
		const platform_blender_solid_color _blender;
	};

	template <typename RasterizerT>
	async<RasterizerT>::async(count_t n)
		: _renderer(2), _unsorted(10000), _sorted(10000), _sorter_thread(&sorter_proc, this), _renderer_thread(&rendition_proc, this), _n(n)
	{
		while (n--)
		{
			auto p = new rasterizer_type;
			unique_ptr<rasterizer_type> ras(p);

			_free.produce(p, [](int) {});
			ras.release();
		}
		_complete.signal();
	}

	template <typename RasterizerT>
	RasterizerT *async<RasterizerT>::acquire()
	{
		rasterizer_type *ras = 0;

		_free.consume([&](rasterizer_type *o) { ras = o; }, [this](int n) {
			if (n == 0)
				_free_ready.wait();
			if (n == _n)
				_complete.wait();
			return true;
		});
		return ras;
	}

	template <typename RasterizerT>
	template <typename SurfaceT, typename BlenderT>
	void async<RasterizerT>::submit(rasterizer_type *rasterizer_, SurfaceT &surface, rect_i *window, const BlenderT &blender)
	{
		rendition_pack pack(rasterizer_, surface, window, blender);

		_unsorted.produce(pack, [this](int n) {
			if (-1 == n)
				_unsorted_ready.signal();
		});
	}

	template <typename RasterizerT>
	void async<RasterizerT>::wait_completion()
	{
		_complete.wait();
		_complete.signal(); // acquire & wait_completion must be called from the same thread/queue.
	}

	template <typename RasterizerT>
	void async<RasterizerT>::sorter_proc(void *p)
	{
		async *self = static_cast<async *>(p);
		for (;;)
		{
			self->_unsorted.consume([self](rendition_pack &o) {
				o.rasterizer->sort();
				self->_sorted.produce(o, [self](int n) {
					if (-1 == n)
						self->_sorted_ready.signal();
				});
			}, [self](int n) {
				if (n == 0)
					self->_unsorted_ready.wait();
				return true;
			});
		}
	}

	template <typename RasterizerT>
	void async<RasterizerT>::rendition_proc(void *p)
	{
		async *self = static_cast<async *>(p);
		for (;;)
		{
			self->_sorted.consume([self](rendition_pack &o) {
				o.render(self->_renderer);
				self->_free.produce(o.rasterizer, [self](int n) {
					if (-1 == n)
						self->_free_ready.signal();
					if (self->_n - 1 == n)
						self->_complete.signal();
				});
			}, [self](int n) {
				if (n == 0)
					self->_sorted_ready.wait();
				return true;
			});
		}
	}


	class Balls : public application
	{
	public:
		Balls()
			: _async(30), _balls(c_balls)
		{	_balls.resize(c_balls_number);	}

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const float dt = 0.3f * (float)stopwatch(_balls_timer);
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };

			stopwatch(counter);
			fill(surface, area, platform_blender_solid_color(255, 255, 255));
			timings.clearing += stopwatch(counter);

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
				move_and_bounce(*i, dt, static_cast<real_t>(surface.width()), static_cast<real_t>(surface.height()));

			stopwatch(counter);
			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
			{
				ellipse e(i->x, i->y, i->radius, i->radius);
				platform_blender_solid_color brush(i->color.r, i->color.g, i->color.b, i->color.a);
				unique_ptr<async_t::rasterizer_type> ras(_async.acquire());

				ras->reset();

				add_path(*ras, e);
				_async.submit(ras.get(), surface, 0, brush);
				ras.release();
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
{
	return new Balls;
}
