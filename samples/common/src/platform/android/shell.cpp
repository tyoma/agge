#include <samples/common/services.h>
#include <samples/common/shell.h>

#include "../../shell-inline.h"
#include "bitmap.h"

#include "sys/timerfd.h"

#include <android/log.h>
#include <android/native_activity.h>
#include <memory>
#include <time.h>
#include <unistd.h>

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "agge_sample", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "agge_sample", __VA_ARGS__))

using namespace std;

namespace
{
	const int c_averaging_n = 100;
	application::timings c_zero_timings = { };

	class shell : services
	{
	public:
		shell(ANativeActivity *activity)
			: _activity(activity), _application(agge_create_application(*this)), _window(0), _timings(c_zero_timings),
				_timings_averaging(0)
		{	}

		void onWindowCreated(ANativeWindow *window)
		{
			ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBX_8888);
			_window = window;
			LOGI("Native window created.");
		}

		void onWindowDestroyed()
		{
			_window = nullptr;
			LOGI("Native window destroyed.");
		}

		void redrawWindow(ANativeWindow *window)
		{
			platform_bitmap surface(*window);
			application::timings t = { };

			_application->draw(surface, t);
			_timings.clearing += t.clearing;
			_timings.stroking += t.stroking;
			_timings.rasterization += t.rasterization;
			_timings.rendition += t.rendition;
			_timings.blitting += t.blitting;
			if (++_timings_averaging == c_averaging_n)
			{
				LOGI("Render timings: clear=%gms, total=%gms, stroking=%gms, rasterization=%gms, rendition=%gms",
					_timings.clearing / c_averaging_n,
					(_timings.stroking + _timings.rasterization + _timings.rendition) / c_averaging_n,
					_timings.stroking / c_averaging_n,
					_timings.rasterization / c_averaging_n,
					_timings.rendition / c_averaging_n);
				_timings_averaging = 0;
				_timings = c_zero_timings;
			}
		}

		void onResized(ANativeWindow *window)
		{	_application->resize(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));	}

		void onInputCreated(AInputQueue *queue)
		{
			itimerspec ts = {};
			auto looper = ALooper_prepare(0);
			auto timer = timerfd_create(CLOCK_MONOTONIC, 0);

			ts.it_value.tv_nsec = ts.it_interval.tv_nsec = 10000000;
			timerfd_settime(timer, 0, &ts, nullptr);

			LOGI("Timer device created: fd=%d", timer);
			
			ALooper_addFd(looper, timer, 0, ALOOPER_EVENT_INPUT, &onTimer, this);
			AInputQueue_attachLooper(queue, looper, 1, &onMessageS, this);
			ALooper_acquire(looper);
			_queue.reset(queue, [looper, timer] (AInputQueue *queue) {
				AInputQueue_detachLooper(queue);
				ALooper_removeFd(looper, timer);
				ALooper_release(looper);
				close(timer);
				LOGI("Timer device destroyed: fd=%d", timer);
			});

			LOGI("Input intialized.");
		}

		void onInputDestroyed()
		{
			_queue.reset();
			LOGI("Input destroyed.");
		}

	private:
		virtual stream *open_file(const char *path)
		{
			class asset_stream : public stream
			{
			public:
				asset_stream(AAssetManager *amanager, const char *path)
					: _stream(AAssetManager_open(amanager, path, AASSET_MODE_STREAMING), &AAsset_close)
				{
					LOGI("Loaded %s at %X...", path, _stream.get());
				}

				virtual void read(void *buffer, size_t size)
				{	AAsset_read(_stream.get(), buffer, size);	}

			private:
				shared_ptr<AAsset> _stream;
			};

			return new asset_stream(_activity->assetManager, path);
		}

	private:
		static int onMessageS(int fd, int events, void* data)
		{	return static_cast<shell *>(data)->onMessage(fd, events);	}

		int onMessage(int /*fd*/, int /*events*/)
		{
			AInputEvent *event = 0;

			while (AInputQueue_getEvent(_queue.get(), &event) >= 0)
			{
				if (AInputQueue_preDispatchEvent(_queue.get(), event))
					continue;
				int32_t handled = 0;
				AInputQueue_finishEvent(_queue.get(), event, handled);
			}
			return 1;
		}

		static int onTimer(int /*fd*/, int /*events*/, void* data)
		{
			auto self = static_cast<shell *>(data);

			if (self->_window)
				self->redrawWindow(self->_window);
			return 1;
		}

	private:
		ANativeActivity *_activity;
		unique_ptr<application> _application;
		shared_ptr<AInputQueue> _queue;
		ANativeWindow *_window;
		application::timings _timings;
		int _timings_averaging;
	};

	void onDestroy(ANativeActivity *activity)
	{	delete static_cast<shell *>(activity->instance);	}

	void onInputCreated(ANativeActivity *activity, AInputQueue *queue)
	{	static_cast<shell *>(activity->instance)->onInputCreated(queue);	}

	void onInputDestroyed(ANativeActivity *activity, AInputQueue * /*queue*/)
	{	static_cast<shell *>(activity->instance)->onInputDestroyed();	}

	void onWindowCreated(ANativeActivity *activity, ANativeWindow *window)
	{	static_cast<shell *>(activity->instance)->onWindowCreated(window);	}

	void onWindowRedrawNeeded(ANativeActivity *activity, ANativeWindow *window)
	{	static_cast<shell *>(activity->instance)->redrawWindow(window);	}

	void onWindowResized(ANativeActivity *activity, ANativeWindow *window)
	{	static_cast<shell *>(activity->instance)->onResized(window);	}

	void onWindowDestroyed(ANativeActivity *activity, ANativeWindow * /*window*/)
	{	static_cast<shell *>(activity->instance)->onWindowDestroyed();	}
}

extern "C" void ANativeActivity_onCreate(ANativeActivity *activity, void * /*savedState*/, size_t /*savedStateSize*/)
{
	unique_ptr<shell> s(new shell(activity));

	activity->instance = s.release();
	activity->callbacks->onDestroy = &onDestroy;
	activity->callbacks->onInputQueueCreated = &onInputCreated;
	activity->callbacks->onInputQueueDestroyed = &onInputDestroyed;
	activity->callbacks->onNativeWindowCreated = &onWindowCreated;
	activity->callbacks->onNativeWindowRedrawNeeded = &onWindowRedrawNeeded;
	activity->callbacks->onNativeWindowResized = &onWindowResized;
	activity->callbacks->onNativeWindowDestroyed = &onWindowDestroyed;
}
