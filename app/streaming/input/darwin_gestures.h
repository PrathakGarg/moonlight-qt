#pragma once

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void installDarwinPinchMonitor(void *nsWindow, void (*callback)(int phase, float magnificationDelta, void *userdata), void *userdata);
void removeDarwinPinchMonitor();

#ifdef __cplusplus
}
#endif
