#include <samples/common/shell.h>

#include "../../shell-inline.h"
#include "bitmap.h"

#include <android/native_activity.h>

namespace
{
	void onDestroy(ANativeActivity *activity)
	{	delete static_cast<application *>(activity->instance);	}

	int onMessage(int /*fd*/, int /*events*/, void* data)
	{
		AInputQueue *queue = static_cast<AInputQueue *>(data);
		AInputEvent *event = 0;
		while (AInputQueue_getEvent(queue, &event) >= 0)
		{
			if (AInputQueue_preDispatchEvent(queue, event))
				continue;
			int32_t handled = 0;
			AInputQueue_finishEvent(queue, event, handled);
		}
	}

	void onInputCreated(ANativeActivity *activity, AInputQueue *queue)
	{	AInputQueue_attachLooper(queue, ALooper_prepare(0), 1, &onMessage, queue);	}

	void onInputDestroyed(ANativeActivity * /*activity*/, AInputQueue *queue)
	{	AInputQueue_detachLooper(queue);	}

	void onWindowCreated(ANativeActivity * /*activity*/, ANativeWindow *window)
	{	ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBX_8888);	}

	void onWindowRedrawNeeded(ANativeActivity *activity, ANativeWindow *window)
	{
		platform_bitmap surface(*window);
		application *app = static_cast<application *>(activity->instance);
		application::timings t;

		app->draw(surface, t);
	}

	void onWindowResized(ANativeActivity *activity, ANativeWindow *window)
	{
		application *app = static_cast<application *>(activity->instance);

		app->resize(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
	}
}

extern "C" void ANativeActivity_onCreate(ANativeActivity *activity, void * /*savedState*/, size_t /*savedStateSize*/)
{
	activity->instance = agge_create_application();
	activity->callbacks->onDestroy = &onDestroy;
	activity->callbacks->onInputQueueCreated = &onInputCreated;
	activity->callbacks->onInputQueueDestroyed = &onInputDestroyed;
	activity->callbacks->onNativeWindowCreated = &onWindowCreated;
	activity->callbacks->onNativeWindowRedrawNeeded = &onWindowRedrawNeeded;
	activity->callbacks->onNativeWindowResized = &onWindowResized;
}
