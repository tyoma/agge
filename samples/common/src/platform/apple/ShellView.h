#pragma once

#import <Cocoa/Cocoa.h>

@interface ShellView : NSView
	- (void) dealloc;
	- (void) createApp;
	- (void) setFrameSize:(NSSize)newSize;
	- (void) drawRect:(NSRect)dirtyRect;
	- (void) setOnUpdateCaption:(void (^)(const char *text))callback;
@end

