#pragma once

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*DarwinPinchCallback)(int phase, float magnificationDelta, void *userdata);

void installDarwinPinchMonitor(SDL_Window *window, DarwinPinchCallback callback, void *userdata);
void removeDarwinPinchMonitor();

#ifdef __cplusplus
}
#endif
