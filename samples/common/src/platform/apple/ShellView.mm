#include "ShellView.h"

#include "../../../services.h"
#include "../../../shell.h"
#include "../../../smoothing.h"
#include "../../../timing.h"

#include <chrono>
#include <tasker/ui_queue.h>

using namespace std;

class shell_services : public services
{
	virtual stream *open_file(const char *path)
	{
		class file_stream : public stream
		{
		public:
			file_stream(const char *path)
				: _stream(fopen(path, "rb"), &fclose)
			{	}

			virtual void read(void *buffer, size_t size)
			{	fread(buffer, 1, size, _stream.get());	}

		private:
			shared_ptr<FILE> _stream;
		};

		return new file_stream(path);
	}
};

@implementation ShellView
	{
		std::unique_ptr<shell_services> _services;
		std::unique_ptr<platform_bitmap> _surface;
		std::unique_ptr<application> _application;
		smoothing<application::timings> _smoothing;
		std::unique_ptr<tasker::ui_queue> _ui_queue;
		void (^_onUpdateCaption)(const char *text);
	}

	- (void) dealloc
	{
		_ui_queue.reset();
		_application.reset();
		_surface.reset();
		_services.reset();
		[super dealloc];
	}

	- (void) createApp
	{
		_services.reset(new shell_services());
		_surface.reset(new platform_bitmap(1, 1));
		_application.reset(agge_create_application(*_services));
		_ui_queue.reset(new tasker::ui_queue([] {
			return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch());
		}));

		[self schedule_redraw];
	}

	- (id) initWithFrame:(NSRect)frameRect
	{
		self = [super initWithFrame:frameRect];
		[self scaleUnitSquareToSize:NSMakeSize(0.5, 0.5)];
		[self createApp];
		return self;
	}

	- (void) setFrameSize:(NSSize)newSize
	{
		_surface->resize(2 * static_cast<unsigned>(newSize.width), 2 * static_cast<unsigned>(newSize.height));
		_application->resize(2 * static_cast<unsigned>(newSize.width), 2 * static_cast<unsigned>(newSize.height));
		[super setFrameSize:newSize];
	}

	- (void) drawRect:(NSRect)dirtyRect
	{
		application::timings t = {};
		long long c;
		
		_application->draw(*_surface, t);
		stopwatch(c);
			_surface->blit([[NSGraphicsContext currentContext]CGContext], 0, 0, _surface->width(), _surface->height());
		t.blitting = stopwatch(c);
		_smoothing.add(t);
	}

	- (void) setOnUpdateCaption:(void (^)(const char *text))callback;
	{
		_onUpdateCaption = callback;
	}

	- (void) schedule_redraw
	{
		[self update_text];
		[self setNeedsDisplay:YES];
		_ui_queue->schedule([self] {
			[self schedule_redraw];
		}, chrono::milliseconds(20));
	}

	- (void) update_text
	{
		auto t = _smoothing.get();
		char caption[256];
		snprintf(caption, sizeof(caption), "Total: %.2fms, clear: %.2fms, stroking: %.2fms, raster: %.2fms, render: %.2fms, blitting: %.2fms",
		t.clearing + t.stroking + t.rasterization + t.rendition, t.clearing, t.stroking, t.rasterization, t.rendition, t.blitting);
		if (_onUpdateCaption != nil)
			_onUpdateCaption(caption);

	}

@end

application::~application()
{	}

void application::resize(int /*width*/, int /*height*/)
{	}

double stopwatch(long long &last_time)
{
	auto now = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
	auto elapsed = now - last_time;

	last_time = now;
	return elapsed * 0.001;
}
