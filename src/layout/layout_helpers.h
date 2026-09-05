#pragma once

#include "layout/layout.h"

#include <QRect>

 /// Converts from a CSS pixel to a device pixel.
double dpx(double cssPx, double zoom);

/// Finds the absolute bounds for a layout object in
/// coordinates of what the user sees including the translation
/// offset by walking up the node tree and mapping through 
/// each translation found.
QRect absoluteBoundsForObj(Layout* layout);
