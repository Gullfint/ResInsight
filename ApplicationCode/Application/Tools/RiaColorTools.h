/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-    Equinor ASA
//  Copyright (C) 2017     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cvfBase.h"
#include "cvfArray.h"

#include <QColor>

//==================================================================================================
///  
///  
//==================================================================================================
class RiaColorTools
{
public:
    static bool         isBrightnessAboveThreshold(cvf::Color3f backgroundColor);
    
    static cvf::Color3f computeOffsetColor(cvf::Color3f color, float offsetFactor);
    static cvf::Color3f darkContrastColor();
    static cvf::Color3f brightContrastColor();
    static cvf::Color3f darkContrastColorSofter();
    static cvf::Color3f brightContrastColorSofter();
    static cvf::Color3f contrastColor(cvf::Color3f backgroundColor, bool softerContrast = false);
    static QColor       toQColor(cvf::Color3f color, float alpha = 1.0f);
    static QColor       toQColor(cvf::Color4f color);
    static float        contrastRatio(cvf::Color3f color1, cvf::Color3f color2);
private:
    static float relativeLuminance(cvf::Color3f backgroundColor);
    static float calculateNonLinearColorValue(float colorFraction);
};
