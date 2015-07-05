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
//
// Copyright (c) 2011, Scientific Computing & Imaging Institute and
// the University of Utah. All Rights Reserved.
//
// Redistribution and use in source and binary forms is strictly
// prohibited without the direct consent of the author.
//
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
