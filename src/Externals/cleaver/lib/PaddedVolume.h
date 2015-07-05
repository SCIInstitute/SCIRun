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

#ifndef PADDEDVOLUME_H
#define PADDEDVOLUME_H

#include "AbstractVolume.h"

namespace Cleaver
{

class PaddedVolume : public AbstractVolume
{
public:

    PaddedVolume(const AbstractVolume *volume, int thickness = PaddedVolume::DefaultThickness, float highValue=PaddedVolume::DefaultHighValue, float lowValue=PaddedVolume::DefaultLowValue);

    virtual float valueAt(const vec3 &x, int material) const;
    virtual float valueAt(float x, float y, float z, int material) const;
    virtual int   numberOfMaterials() const;
    virtual const BoundingBox& bounds() const;

    void setThickness(float thickness);
    void setHighValue(float value);
    void setLowValue(float value);

    static const int DefaultThickness;
    static const float DefaultHighValue;
    static const float DefaultLowValue;

private:

    const Cleaver::AbstractVolume *m_volume;
    Cleaver::BoundingBox m_bounds;
    int m_thickness;
    float m_highValue;
    float m_lowValue;
    void computeSize();
};

}

#endif // PADDEDVOLUME_H
