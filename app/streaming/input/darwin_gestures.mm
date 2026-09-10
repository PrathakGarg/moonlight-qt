#import "darwin_gestures.h"

#import <AppKit/AppKit.h>
#include "SDL_compat.h"
#include <SDL_syswm.h>

namespace {

id gPinchMonitor = nil;
void (*gPinchCallback)(int phase, float magnificationDelta, void *userdata) = nullptr;
void *gPinchUserdata = nullptr;

}  // namespace

void installDarwinPinchMonitor(void *sdlWindow, void (*callback)(int phase, float magnificationDelta, void *userdata), void *userdata)
{
    removeDarwinPinchMonitor();

    if (!sdlWindow || !callback) {
        return;
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo((SDL_Window *) sdlWindow, &info) || info.subsystem != SDL_SYSWM_COCOA) {
        return;
    }

    gPinchCallback = callback;
    gPinchUserdata = userdata;

    NSWindow *window = info.info.cocoa.window;
    gPinchMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMagnify
                                                          handler:^NSEvent *(NSEvent *event) {
        if (!gPinchCallback || event.window != window) {
            return event;
        }

        switch (event.type) {
        case NSEventTypeMagnify:
            if (event.phase == NSEventPhaseBegan) {
                gPinchCallback(0, event.magnification, gPinchUserdata);
            } else if (event.phase == NSEventPhaseChanged) {
                gPinchCallback(1, event.magnification, gPinchUserdata);
            } else if (event.phase == NSEventPhaseEnded || event.phase == NSEventPhaseCancelled) {
                gPinchCallback(2, event.magnification, gPinchUserdata);
            }
            return nil;
        default:
            return event;
        }
    }];
}

void removeDarwinPinchMonitor()
{
    if (gPinchMonitor) {
        [NSEvent removeMonitor:gPinchMonitor];
        gPinchMonitor = nil;
    }

    gPinchCallback = nullptr;
    gPinchUserdata = nullptr;
}
