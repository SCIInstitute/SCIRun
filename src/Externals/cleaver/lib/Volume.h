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

#ifndef VOLUME_H
#define VOLUME_H

#include <vector>
#include "vec3.h"
#include "ScalarField.h"
#include "BoundingBox.h"
#include "AbstractVolume.h"

namespace Cleaver
{

class Volume : public AbstractVolume
{
public:
    Volume(){}
    Volume(const Volume &volume);
    Volume(const std::vector<ScalarField*> &fields, int width=0, int height=0, int depth=0);
    Volume(const std::vector<ScalarField*> &fields, vec3 size);
    Volume& operator= (const Volume &volume);

    virtual float valueAt(const vec3 &x, int material) const;
    virtual float valueAt(float x, float y, float z, int material) const;
    virtual int numberOfMaterials() const;
    virtual const BoundingBox& bounds() const;

    void setSize(const vec3 &size);
    void setSize(int width, int height, int depth);

    ScalarField* getMaterial(int i) const { return m_fields[i]; }

protected:
    std::vector<ScalarField*> m_fields;
    Cleaver::BoundingBox m_bounds;
};

//static Volume createPaddedVolume(const Volume &volume, float thickness, float high, float low);

}

#endif // VOLUME_H
