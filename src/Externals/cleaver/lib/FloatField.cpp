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

#include "FloatField.h"
#include <cmath>

namespace Cleaver
{

FloatField::CenteringType FloatField::DefaultCenteringType = FloatField::CellCentered;

FloatField::FloatField(int width=0, int height=0, int depth = 0, float *data = 0)
    : m_w(width), m_h(height), m_d(depth), m_data(data)
{    
    // default to data bounds
    m_scale = vec3(vec3::unitX.x, vec3::unitY.y, vec3::unitZ.z);
    m_scaleInv = m_scale;

    m_centeringType = FloatField::DefaultCenteringType;
    m_bounds = dataBounds();
}

FloatField::~FloatField()
{
    // no memory cleanup
}

float FloatField::valueAt(float x, float y, float z) const
{
    x *= m_scaleInv.x;
    y *= m_scaleInv.y;
    z *= m_scaleInv.z;

    if(m_centeringType == CellCentered){
        x -= 0.5f;
        y -= 0.5f;
        z -= 0.5f;
    }

    float t = fmod(x,1.0f);
    float u = fmod(y,1.0f);
    float v = fmod(z,1.0f);

    int i0 = floor(x);   int i1 = i0+1;
    int j0 = floor(y);   int j1 = j0+1;
    int k0 = floor(z);   int k1 = k0+1;


    if(m_centeringType == CellCentered)
    {
        i0 = clamp(i0, 0, m_w-1);
        j0 = clamp(j0, 0, m_h-1);
        k0 = clamp(k0, 0, m_d-1);

        i1 = clamp(i1, 0, m_w-1);
        j1 = clamp(j1, 0, m_h-1);
        k1 = clamp(k1, 0, m_d-1);
    }
    else if(m_centeringType == NodeCentered)
    {
        i0 = clamp(i0, 0, m_w-2);
        j0 = clamp(j0, 0, m_h-2);
        k0 = clamp(k0, 0, m_d-2);

        i1 = clamp(i1, 0, m_w-2);
        j1 = clamp(j1, 0, m_h-2);
        k1 = clamp(k1, 0, m_d-2);
    }


    double C000 = m_data[i0 + j0*m_w + k0*m_w*m_h];
    double C001 = m_data[i0 + j0*m_w + k1*m_w*m_h];
    double C010 = m_data[i0 + j1*m_w + k0*m_w*m_h];
    double C011 = m_data[i0 + j1*m_w + k1*m_w*m_h];
    double C100 = m_data[i1 + j0*m_w + k0*m_w*m_h];
    double C101 = m_data[i1 + j0*m_w + k1*m_w*m_h];
    double C110 = m_data[i1 + j1*m_w + k0*m_w*m_h];
    double C111 = m_data[i1 + j1*m_w + k1*m_w*m_h];

    return float((1-t)*(1-u)*(1-v)*C000 + (1-t)*(1-u)*(v)*C001 +
                 (1-t)*  (u)*(1-v)*C010 + (1-t)*  (u)*(v)*C011 +
                 (t)*(1-u)*(1-v)*C100 +   (t)*(1-u)*(v)*C101 +
                 (t)*  (u)*(1-v)*C110 +   (t)*  (u)*(v)*C111);
}


float* FloatField::data() const
{
    return m_data;
}

FloatField::CenteringType FloatField::getCenterType() const
{
    return m_centeringType;
}

void FloatField::setCenter(FloatField::CenteringType center)
{
    m_centeringType = center;
}

void FloatField::setBounds(const BoundingBox &bounds)
{
    m_bounds = bounds;
}

BoundingBox FloatField::bounds() const
{
    return m_bounds;
}

BoundingBox FloatField::dataBounds() const
{
    switch(m_centeringType){
        case CellCentered:
            return BoundingBox(vec3::zero, vec3(m_w*m_scale.x, m_h*m_scale.y, m_d*m_scale.z));
            break;

        case NodeCentered:
            return BoundingBox(vec3::zero, vec3((m_w-1)*m_scale.x, (m_h-1)*m_scale.y, (m_d-1)*m_scale.z));
            break;

        default:
            std::cerr << "Bad CenterType Defined, Returning empty BoundingBox" << std::endl;
            return BoundingBox(vec3::zero, vec3::zero);
            break;
    }
}

void FloatField::setScale(const vec3 &scale)
{
    m_scale = scale;
    m_scaleInv = vec3(1.0/scale.x, 1.0/scale.y, 1.0/scale.z);
    m_bounds.origin = vec3(m_bounds.origin.x*m_scale.x,
                           m_bounds.origin.y*m_scale.y,
                           m_bounds.origin.z*m_scale.z);
    m_bounds.size =    vec3(m_bounds.size.x*m_scale.x,
                            m_bounds.size.y*m_scale.y,
                            m_bounds.size.z*m_scale.z);
}

const vec3& FloatField::scale() const
{
    return m_scale;
}


}
