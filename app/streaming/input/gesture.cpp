#include "input.h"

#include <QtGlobal>
#include <Limelight.h>

namespace {
constexpr float kPinchBaseSpan = 0.10f;
constexpr float kPinchMinSpan = 0.04f;
constexpr float kPinchMaxSpan = 0.5f;
constexpr float kPinchMagnifyScale = 0.25f;
constexpr float kPinchMaxSpanDelta = 0.01f;
constexpr float kPinchCenterX = 0.5f;
constexpr float kPinchCenterY = 0.5f;
}  // namespace

void SdlInputHandler::sendPinchEvent(uint8_t eventType)
{
    LiSendPinchEvent(eventType, m_PinchSpan, kPinchCenterX, kPinchCenterY);
}

void SdlInputHandler::abortPinchGesture()
{
    if (!m_PinchActive) {
        return;
    }

    if (LiGetHostFeatureFlags() & LI_FF_PINCH_EVENTS) {
        LiSendPinchEvent(LI_PINCH_EVENT_END, m_PinchSpan, kPinchCenterX, kPinchCenterY);
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
        m_PinchActive = true;
        m_PinchSpan = kPinchBaseSpan;
        sendPinchEvent(LI_PINCH_EVENT_BEGIN);
        break;
    case 1: {
        if (!m_PinchActive) {
            return;
        }

        const float target_span = m_PinchSpan * (1.0f + (magnificationDelta * kPinchMagnifyScale));
        const float clamped_span = qBound(m_PinchSpan - kPinchMaxSpanDelta, target_span, m_PinchSpan + kPinchMaxSpanDelta);
        m_PinchSpan = qBound(kPinchMinSpan, clamped_span, kPinchMaxSpan);
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
