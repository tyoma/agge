#include "ShellView.h"

#include <Cocoa/Cocoa.h>
#include <memory>

int main(int argc, const char * argv[])
{
	@autoreleasepool
	{
		auto app = [NSApplication sharedApplication];

		[app setActivationPolicy:NSApplicationActivationPolicyRegular];

		auto frame = NSMakeRect(100, 100, 640, 480);
		auto style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;	
		auto window = [[NSWindow alloc] initWithContentRect:frame styleMask:style backing:NSBackingStoreBuffered defer:NO];
		auto shell = [[ShellView alloc] initWithFrame:frame];

		[window setTitle:@"Console-created NSWindow"];
		[window makeKeyAndOrderFront:nil];
		[window setContentView:shell];
		[shell setOnUpdateCaption:^(const char *text) {
			[window setTitle:[NSString stringWithUTF8String:text]];
		}];
		[[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillCloseNotification object:window queue:nil usingBlock:^(NSNotification *note) {
			[app stop:nil];
		}];
		[app activateIgnoringOtherApps:YES];
		[app run];
		[shell setOnUpdateCaption:nil];
	}
	return 0;
}
