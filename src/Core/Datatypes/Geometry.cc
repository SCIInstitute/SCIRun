/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

/// @todo Documentation Core/Datatypes/Geometry.cc
#include <sstream>
#include <Core/Datatypes/Geometry.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;

GeometryObject::GeometryObject(DatatypeConstHandle dh, const GeometryIDGenerator& idGenerator, const std::string& tag) :
    mLowestValue(0.0),
    mHighestValue(0.0),
    isVisible(true),
    data_(dh),
    objectName_(idGenerator.generateGeometryID(tag))
{
}

GeometryObject::GeometryObject(const GeometryObject& other) :
    mLowestValue(0.0),
    mHighestValue(0.0),
    isVisible(true),
    data_(other.data_->clone()),
    objectName_(other.objectName_) // TODO?
{}

//TODO: rewrite
GeometryObject& GeometryObject::operator=(const GeometryObject& other)
{
  if (this != &other)
  {
    data_.reset(other.data_->clone());
  }
  return *this;
}
