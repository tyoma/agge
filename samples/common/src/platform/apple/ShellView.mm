#import "ShellView.h"

@implementation ShellView
	- (void) createApp
	{
		_services.reset(new shell_services);
		_surface.reset(new platform_bitmap(1, 1));
		_application.reset(agge_create_application(*_services));
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
		application::timings t;
		
		_application->draw(*_surface, t);
		_surface->blit([[NSGraphicsContext currentContext]CGContext], 0, 0, _surface->width(), _surface->height());
	}
@end

application::~application()
{	}

void application::resize(int /*width*/, int /*height*/)
{	}

stream *shell_services::open_file(const char *path)
{
	throw 0;
}

double stopwatch(long long&)
{
	return 0;
}
