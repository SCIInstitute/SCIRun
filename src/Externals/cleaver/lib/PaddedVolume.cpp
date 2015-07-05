//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
//
// -- Padded Volume Class
//
// Author: Jonathan Bronson (bronson@sci.utah.ed)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, Jonathan Bronson
//  Scientific Computing & Imaging Institute
//  University of Utah
//
//  Permission is  hereby  granted, free  of charge, to any person
//  obtaining a copy of this software and associated documentation
//  files  ( the "Software" ),  to  deal in  the  Software without
//  restriction, including  without limitation the rights to  use,
//  copy, modify,  merge, publish, distribute, sublicense,  and/or
//  sell copies of the Software, and to permit persons to whom the
//  Software is  furnished  to do  so,  subject  to  the following
//  conditions:
//
//  The above  copyright notice  and  this permission notice shall
//  be included  in  all copies  or  substantial  portions  of the
//  Software.
//
//  THE SOFTWARE IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF ANY
//  KIND,  EXPRESS OR IMPLIED, INCLUDING  BUT NOT  LIMITED  TO THE
//  WARRANTIES   OF  MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT  SHALL THE AUTHORS OR
//  COPYRIGHT HOLDERS  BE  LIABLE FOR  ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
//  USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#include "PaddedVolume.h"

namespace Cleaver
{


const int   PaddedVolume::DefaultThickness =     2;
const float PaddedVolume::DefaultHighValue = +1000;
const float PaddedVolume::DefaultLowValue =  -1000;

PaddedVolume::PaddedVolume(const AbstractVolume *volume, int thickness, float highValue, float lowValue)
    : m_volume(volume), m_thickness(thickness), m_highValue(highValue), m_lowValue(lowValue)
{
    computeSize();
}

float PaddedVolume::valueAt(const vec3 &x, int m) const
{
    vec3 p = x - vec3(m_thickness, m_thickness, m_thickness);
    bool inside = m_volume->bounds().contains(p);

    if(inside)
    {
        // if normal material, return real value
        if(m < numberOfMaterials()-1)
            return m_volume->valueAt(p,m);
        // if padding material, return low value
        else
            return m_lowValue;
    }
    else
    {
        // if normal material, return low value
        if(m < numberOfMaterials()-1)
            return m_lowValue;
        // if padding material, return high value;
        else
            return m_highValue;
    }


    return 0;
}

float PaddedVolume::valueAt(float x, float y, float z, int material) const
{    
    return this->valueAt(vec3(x,y,z),material);
}

int PaddedVolume::numberOfMaterials() const
{
    return m_volume->numberOfMaterials() + 1;
}

const BoundingBox& PaddedVolume::bounds() const
{
    return m_bounds;
}


void PaddedVolume::computeSize()
{
    m_bounds.origin = vec3::zero;
    m_bounds.size = m_volume->bounds().size + vec3(2*m_thickness,2*m_thickness,2*m_thickness);
}

void PaddedVolume::setThickness(float thickness)
{
    m_thickness = thickness;
    computeSize();
}

void PaddedVolume::setHighValue(float value)
{
    m_highValue = value;
}

void PaddedVolume::setLowValue(float value)
{
    m_lowValue = value;
}

}
