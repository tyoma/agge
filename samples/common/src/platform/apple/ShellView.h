#ifndef ShellView_h
#define ShellView_h

#import <Cocoa/Cocoa.h>
#include <memory>
#include <samples/common/services.h>
#include <samples/common/shell.h>

class shell_services : public services
{
	virtual stream *open_file(const char *path);
};

@interface ShellView : NSView
{
	std::auto_ptr<shell_services> _services;
	std::auto_ptr<platform_bitmap> _surface;
	std::auto_ptr<application> _application;
}

	- (void) createApp;
	- (id) init;
	- (void) setFrameSize:(NSSize)newSize;
	- (void) drawRect:(NSRect)dirtyRect;
@end

#endif /* ShellView_h */
