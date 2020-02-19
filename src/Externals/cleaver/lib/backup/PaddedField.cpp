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


#include "PaddedField.h"

namespace Cleaver
{


const float PaddedField::DefaultThickness = 2;
const float PaddedField::DefaultHighValue = +10000;
const float PaddedField::DefaultLowValue  = -10000;

PaddedField::PaddedField(const Cleaver::ScalarField *field, float thickness, float high, float low)
    : m_origi_field(field), m_thickness(thickness), m_highValue(high), m_lowValue(low)
{
    computeBounds();
}


void PaddedField::setField(const Cleaver::ScalarField *field)
{
    m_origi_field = field;
    computeBounds();
}

void PaddedField::setThickness(float thickness)
{
    m_thickness = thickness;
    computeBounds();
}

void PaddedField::setHighValue(float value)
{
    m_highValue = value;
}

void PaddedField::setLowValue(float value)
{
    m_lowValue = value;
}

void PaddedField::computeBounds()
{
    vec3 origin = m_origi_field->bounds().origin - vec3(m_thickness,m_thickness,m_thickness);
    vec3 size = m_origi_field->bounds().size + vec3(2*m_thickness,2*m_thickness,2*m_thickness);
    m_bounds = BoundingBox(origin, size);
}

}
