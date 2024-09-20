#include "balls_data.h"

#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <memory>
#include <mt/event.h>
#include <tasker/thread_queue.h>
#include <tasker/task.h>

#include <samples/common/agg_ellipse.h>
#include <samples/common/shell.h>
#include <samples/common/timing.h>

using namespace agge;
using namespace std;
using namespace common;
using namespace tasker;

const int c_render_thread_count = 2;
const int c_balls_number = 1000;

namespace
{
	class thread_pool_queue : public tasker::queue
	{
	public:
		thread_pool_queue(const clock &clock_, unsigned int thread_count)
			: _underlying(clock_), _stop_requested(false)
		{
			while (thread_count--)
				_threads.emplace_back(new mt::thread([this] {	run();	}));
		}

		~thread_pool_queue()
		{
			for (auto i = begin(_threads); i != end(_threads); ++i)
				(*i)->join();
		}

		virtual void schedule(function<void()> &&task, mt::milliseconds defer_by) override
		{	_underlying.schedule(move(task), defer_by);	}

	private:
		void run()
		{
			while (!_stop_requested)
			{
				_underlying.wait();
				_underlying.execute_ready(mt::milliseconds(50));
			}
		}

	private:
		task_queue _underlying;
		bool _stop_requested;
		vector< unique_ptr<mt::thread> > _threads;
	};

	class Balls : public application
	{
	public:
		Balls()
			: _balls(c_balls), _queue([] {	return mt::milliseconds(0);	}, 8), _rasterizers_pool(500)
		{	_balls.resize(c_balls_number);	}

	private:
		template <typename T>
		class pool
		{
		public:
			pool(size_t max_items)
			{
				while (max_items--)
					_ready.push_back(make_shared<T>());
			}

			task< shared_ptr<T> > retrieve()
			{
				shared_ptr<T> item;
				auto completion = make_shared< task_node< shared_ptr<T> > >();
				{
					mt::lock_guard<mt::mutex> l(_mtx);

					if (_ready.empty())
						_pending.push_back(completion);
					else
						item = move(_ready.back()), _ready.pop_back();
				}
				if (item)
					completion->set(move(item));
				return task< shared_ptr<T> >(move(completion));
			}

			void put_back(const shared_ptr<T> &item)
			{
				shared_ptr< task_node< shared_ptr<T> > > completion;

				item->reset();
				{
					mt::lock_guard<mt::mutex> l(_mtx);

					if (_pending.empty())
						_ready.push_back(item);
					else
						completion = move(_pending.back()), _pending.pop_back();
				}

				if (completion)
					completion->set(shared_ptr<T>(item));
			}

		private:
			mt::mutex _mtx;
			vector< shared_ptr< task_node< shared_ptr<T> > > > _pending;
			vector< shared_ptr<T> > _ready;
		};

	private:
		virtual void draw(platform_bitmap &surface, timings &timings)
		{
			long long counter;
			const float dt = 0.3f * (float)stopwatch(_balls_timer);
			const rect_i area = { 0, 0, static_cast<int>(surface.width()), static_cast<int>(surface.height()) };
			mt::event ready;
			auto n = _balls.size();

			stopwatch(counter);
			fill(surface, area, platform_blender_solid_color(color::make(255, 255, 255)));
			timings.clearing += stopwatch(counter);

			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
				move_and_bounce(*i, dt, static_cast<real_t>(surface.width()), static_cast<real_t>(surface.height()));

			stopwatch(counter);
			for (vector<ball>::iterator i = _balls.begin(); i != _balls.end(); ++i)
			{
				platform_blender_solid_color brush(i->color);
				auto b = *i;

				_rasterizers_pool.retrieve().then([this, b] (const auto &rasterizer) {
					add_path(**rasterizer, agg::ellipse(b.x, b.y, b.radius, b.radius));
					return *rasterizer;
				}, _queue).then([&, this] (const async_result< shared_ptr< rasterizer< clipper<int> > > > &ras) {
					(*ras)->sort();
					_rasterizers_pool.put_back(*ras);
					if (!--n)
						ready.set();
				}, _queue);
			}
			ready.wait();
			timings.rendition += stopwatch(counter);
		}

	private:
		long long _balls_timer;
		vector<ball> _balls;
		thread_pool_queue _queue;
		mt::mutex _mtx;
		pool< rasterizer< clipper<int> > > _rasterizers_pool;
	};


}

application *agge_create_application(services &/*s*/)
{	return new Balls;	}
