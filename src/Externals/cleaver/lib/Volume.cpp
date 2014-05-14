//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
//
// -- Volume Class
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

#include "Volume.h"
#include "BoundingBox.h"

namespace Cleaver
{

Volume::Volume(const Cleaver::Volume &volume)
{
    this->m_fields = volume.m_fields;
    this->m_bounds = volume.m_bounds;
}

Volume::Volume(const std::vector<ScalarField*> &fields, int width, int height, int depth) :
    m_fields(fields), m_bounds(BoundingBox(vec3::zero, vec3(width, height, depth)))
{
    if(m_fields.size() > 0)
    {
        if(width == 0)
            width = m_fields[0]->bounds().size.x;
        if(height == 0)
            height = m_fields[0]->bounds().size.y;
        if(depth == 0)
            depth = m_fields[0]->bounds().size.z;

        m_bounds = BoundingBox(vec3::zero, vec3(width, height, depth));
    }
}


Volume::Volume(const std::vector<ScalarField*> &fields, vec3 size) :
    m_fields(fields), m_bounds(BoundingBox(vec3::zero, size))
{
    if(m_fields.size() > 0)
    {
        if(size.x == 0)
            size.x = m_fields[0]->bounds().size.x;
        if(size.y == 0)
            size.y = m_fields[0]->bounds().size.y;
        if(size.z == 0)
            size.z = m_fields[0]->bounds().size.z;

        m_bounds = BoundingBox(vec3::zero, size);
    }
}

Volume& Volume::operator= (const Volume &volume)
{
    this->m_bounds = volume.m_bounds;
    this->m_fields = volume.m_fields;
    return *this;
}

void Volume::setSize(const vec3 &size)
{
    m_bounds.size = size;
}

void Volume::setSize(int width, int height, int depth)
{
    m_bounds.size = vec3(width, height, depth);
}

float Volume::valueAt(const vec3 &x, int material) const
{    
    vec3 tx = vec3((x.x / m_bounds.size.x)*m_fields[material]->bounds().size.x,
                   (x.y / m_bounds.size.y)*m_fields[material]->bounds().size.y,
                   (x.z / m_bounds.size.z)*m_fields[material]->bounds().size.z);
    return m_fields[material]->valueAt(tx);
}

float Volume::valueAt(float x, float y, float z, int material) const
{
    vec3 tx = vec3((x / m_bounds.size.x)*m_fields[material]->bounds().size.x,
                   (y / m_bounds.size.y)*m_fields[material]->bounds().size.y,
                   (z / m_bounds.size.z)*m_fields[material]->bounds().size.z);
    return m_fields[material]->valueAt(tx);
}

int Volume::numberOfMaterials() const
{
    return m_fields.size();
}

const BoundingBox& Volume::bounds() const
{
    return m_bounds;
}

//----------------------------------------------------
// This function creates a new volume that contains
// the original volume with padding on the edges to
// avoid having transitions on the boundary
// User is responsible for freeing the scalar fields
// web done.
//----------------------------------------------------
/*
static Volume createPaddedVolume(const Volume &volume, float thickness, float high, float low)
{
    std::vector<ScalarField*> paddedFields;

    for(int i=0; i < volume.materials(); i++)
    {
        paddedFields.push_back(new PaddedField(volume.getMaterial(i)));
    }

    return Volume(paddedFields);
}
*/

}
