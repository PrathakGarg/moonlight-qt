#include "darwin_gestures.h"

#include "SDL_compat.h"
#include <SDL_syswm.h>

#import <Cocoa/Cocoa.h>

namespace {

enum PinchPhase {
    PinchPhaseBegin = 0,
    PinchPhaseUpdate = 1,
    PinchPhaseEnd = 2,
};

id g_monitor = nil;
DarwinPinchCallback g_callback = nullptr;
void *g_userdata = nullptr;
NSWindow *g_nsWindow = nullptr;

}  // namespace

void installDarwinPinchMonitor(SDL_Window *window, DarwinPinchCallback callback, void *userdata)
{
    removeDarwinPinchMonitor();

    if (!window || !callback) {
        return;
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window, &info) || info.subsystem != SDL_SYSWM_COCOA) {
        return;
    }

    g_nsWindow = info.info.cocoa.window;
    g_callback = callback;
    g_userdata = userdata;

    g_monitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskMagnify
                                                        handler:^NSEvent *(NSEvent *event) {
        if (g_nsWindow != nullptr && [event window] != g_nsWindow) {
            return event;
        }

        switch ([event phase]) {
        case NSEventPhaseBegan:
            g_callback(PinchPhaseBegin, 0.0f, g_userdata);
            break;
        case NSEventPhaseChanged:
            g_callback(PinchPhaseUpdate, static_cast<float>([event magnification]), g_userdata);
            break;
        case NSEventPhaseEnded:
        case NSEventPhaseCancelled:
            g_callback(PinchPhaseEnd, 0.0f, g_userdata);
            break;
        default:
            break;
        }

        // Consume magnify so macOS does not also deliver it as scroll to SDL.
        return nil;
    }];
}

void removeDarwinPinchMonitor()
{
    if (g_monitor != nil) {
        [NSEvent removeMonitor:g_monitor];
        g_monitor = nil;
    }

    g_callback = nullptr;
    g_userdata = nullptr;
    g_nsWindow = nullptr;
}
