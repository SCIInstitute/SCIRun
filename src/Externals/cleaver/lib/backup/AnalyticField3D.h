/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- 3D Analytic Field
//
// Primary Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#ifndef _ANALYTIC_FIELD_3D_
#define _ANALYTIC_FIELD_3D_

#include "Field3D.h"
#include "vec3.h"
#include <vector>

namespace Cleaver
{

class AnalyticField3D: public Field3D
{
public:
    AnalyticField3D();
    ~AnalyticField3D(){}

    float value(float x, float y, float z, int mat) const;
    float operator()(float x, float y, float z, int mat) const;

    int width() const { return w; }
    int height() const { return h; }
    int depth() const { return d; }
    int numMaterials() const { return m; }

private:
    int w,h,d,m;


    std::vector<vec3> x1;
    std::vector<vec3> x2;
    std::vector<float> r;
};

}

#endif // _ANALYTIC_FIELD_3D_
