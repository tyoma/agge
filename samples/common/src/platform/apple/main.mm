#import <samples/common/src/platform/apple/ShellView.h>

#import <Cocoa/Cocoa.h>
#include <memory>

@interface AppDelegate : NSObject<NSApplicationDelegate>
	{
		NSWindow *_window;
		ShellView *_view;
	}
@end

@implementation AppDelegate
	- (id)init
	{
		if (self = [super init])
		{
			NSRect content = NSMakeRect(0.0f, 0.0f, 600.0f, 400.0f);
			
			_window = [[NSWindow alloc] initWithContentRect:content styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskResizable)
				backing:NSBackingStoreBuffered defer:YES];
			_view = [[ShellView alloc] initWithFrame:content];
		}
		return self;
	}

	- (void)dealloc
	{
		[_view release];
		[_window release];
		[super dealloc];
	}

	- (void)applicationWillFinishLaunching:(NSNotification *)notification
	{	[_window setContentView:_view];	}

	- (void)applicationDidFinishLaunching:(NSNotification *)notification
	{	[_window makeKeyAndOrderFront:self];	}
@end

int main(int /*argc*/, const char * /*argv*/[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSApplication *application = [NSApplication sharedApplication];
	AppDelegate *applicationDelegate =[[[AppDelegate alloc] init] autorelease];

	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	[NSApp activateIgnoringOtherApps:YES];

	[application setDelegate:applicationDelegate];

	[application run];

	[pool drain];

	return 0;
}
