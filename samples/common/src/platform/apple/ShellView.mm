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
    class file_stream : public stream
    {
    public:
        file_stream(const char *path)
            : _stream(fopen(path, "rb"), &fclose)
        {    }

        virtual void read(void *buffer, size_t size)
        {    fread(buffer, 1, size, _stream.get());    }

    private:
        std::shared_ptr<FILE> _stream;
    };

    return new file_stream(path);
}

double stopwatch(long long&)
{
	return 0;
}
