#include "input.h"

#include <QtGlobal>
#include <Limelight.h>

#include "streaming/streamutils.h"

namespace {
constexpr float kPinchBaseSpan = 0.10f;
constexpr float kPinchMinSpan = 0.04f;
constexpr float kPinchMaxSpan = 1.0f;
// macOS reports per-event magnification deltas; accumulate them for span mapping.
constexpr float kPinchSensitivity = 2.5f;
}  // namespace

bool SdlInputHandler::getNormalizedVideoCursorPosition(float &normX, float &normY) const
{
    if (!m_Window) {
        return false;
    }

    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_Window, &windowWidth, &windowHeight);

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    SDL_Rect src, dst;
    src.x = src.y = 0;
    src.w = m_StreamWidth;
    src.h = m_StreamHeight;

    dst.x = dst.y = 0;
    dst.w = windowWidth;
    dst.h = windowHeight;

    StreamUtils::scaleSourceToDestinationSurface(&src, &dst);

    if (dst.w <= 0 || dst.h <= 0) {
        return false;
    }

    mouseX = qMin(qMax(mouseX - dst.x, 0), dst.w);
    mouseY = qMin(qMax(mouseY - dst.y, 0), dst.h);

    normX = static_cast<float>(mouseX) / dst.w;
    normY = static_cast<float>(mouseY) / dst.h;
    return true;
}

void SdlInputHandler::sendPinchEvent(uint8_t eventType)
{
    LiSendPinchEvent(eventType, m_PinchSpan, m_PinchCenterX, m_PinchCenterY);
}

void SdlInputHandler::abortPinchGesture()
{
    if (!m_PinchActive) {
        return;
    }

    if (LiGetHostFeatureFlags() & LI_FF_PINCH_EVENTS) {
        LiSendPinchEvent(LI_PINCH_EVENT_END, m_PinchSpan, m_PinchCenterX, m_PinchCenterY);
    }

    m_PinchActive = false;
}

void SdlInputHandler::handlePinchGesture(int phase, float magnificationDelta)
{
    if (!isCaptureActive()) {
        return;
    }

    if (!(LiGetHostFeatureFlags() & LI_FF_PINCH_EVENTS)) {
        return;
    }

    switch (phase) {
    case 0:
        if (!getNormalizedVideoCursorPosition(m_PinchCenterX, m_PinchCenterY)) {
            m_PinchCenterX = 0.5f;
            m_PinchCenterY = 0.5f;
        }
        m_PinchActive = true;
        m_PinchAccumulatedMagnification = 0.0f;
        m_PinchSpan = kPinchBaseSpan;
        sendPinchEvent(LI_PINCH_EVENT_BEGIN);
        break;
    case 1: {
        if (!m_PinchActive) {
            return;
        }

        m_PinchAccumulatedMagnification += magnificationDelta;
        m_PinchSpan = qBound(
            kPinchMinSpan,
            kPinchBaseSpan * (1.0f + (m_PinchAccumulatedMagnification * kPinchSensitivity)),
            kPinchMaxSpan);
        sendPinchEvent(LI_PINCH_EVENT_UPDATE);
        break;
    }
    case 2:
        if (!m_PinchActive) {
            return;
        }

        sendPinchEvent(LI_PINCH_EVENT_END);
        m_PinchActive = false;
        break;
    default:
        break;
    }
}

#ifdef Q_OS_DARWIN
void SdlInputHandler::darwinPinchCallback(int phase, float magnificationDelta, void *userdata)
{
    static_cast<SdlInputHandler *>(userdata)->handlePinchGesture(phase, magnificationDelta);
}
#endif
