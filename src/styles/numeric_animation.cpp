#include "numeric_animation.h"

NumericAnimation::NumericAnimation(double oldValue, double newValue, int numFrames)
    : m_oldValue(oldValue),
      m_newValue(newValue),
      m_numFrames(numFrames)
{
    m_frameCount = 1;
    auto totalChange = m_newValue - m_oldValue;
    m_changePerFrame = totalChange / m_numFrames;   
}

std::optional<std::string> NumericAnimation::animate()
{
    m_frameCount += 1;
    if (m_frameCount >= m_numFrames) return std::nullopt;
    auto currentValue = m_oldValue + m_changePerFrame * m_frameCount;
    return std::to_string(currentValue);
}
