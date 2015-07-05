//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- 3D Float Point Data Field
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
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

#ifndef FLOATFIELD_H
#define FLOATFIELD_H

#include "ScalarField.h"
#include "BoundingBox.h"

namespace Cleaver
{


class FloatField : public ScalarField
{
public:
    enum CenteringType { NodeCentered, CellCentered };

public:
    FloatField(int w, int h, int d, float *data);
    ~FloatField();

    virtual float valueAt(float x, float y, float z) const;

    void setData(float *data);
    float* data() const;

    CenteringType getCenterType() const;
    void setCenter(CenteringType center);

    void setBounds(const BoundingBox &bounds);
    virtual BoundingBox bounds() const;

    BoundingBox dataBounds() const;

    void setScale(const vec3 &scale);
    const vec3& scale() const;

private:
    int m_w, m_h, m_d;
    vec3 m_scale;
    vec3 m_scaleInv;
    BoundingBox m_bounds;    
    CenteringType m_centeringType;
    float *m_data;

    static CenteringType DefaultCenteringType;
};


}

#endif // FLOATFIELD_H
