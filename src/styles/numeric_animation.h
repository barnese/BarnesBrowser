#pragma once

#include "css_animation.h"

class NumericAnimation : public CSSAnimation
{
public:
    NumericAnimation(double oldValue, double newValue, int numFrames);
    std::optional<std::string> animate() override;

private:
    double m_oldValue;
    double m_newValue;
    int m_numFrames;
    int m_frameCount;
    double m_changePerFrame;
};
