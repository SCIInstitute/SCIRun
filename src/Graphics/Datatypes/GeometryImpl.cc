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


#include <Graphics/Datatypes/GeometryImpl.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;

GeometryObjectSpire::GeometryObjectSpire(const GeometryIDGenerator& idGenerator, const std::string& tag, bool isClippable) :
GeometryObject(idGenerator, tag),
isClippable_(isClippable)
{

}

CompositeGeometryObject::~CompositeGeometryObject()
{
}

void CompositeGeometryObject::addToList(GeometryBaseHandle handle, GeomList& list)
{
  if (handle.get() == this)
  {
    list.insert(geoms_.begin(), geoms_.end());
  }
}

void SpireSubPass::addUniform(const std::string& name, const glm::vec4& vector)
{
  for (auto& i : mUniforms)
  {
    if (i.name == name && i.type == Uniform::UniformType::UNIFORM_VEC4)
    {
      i.data = vector;
      return;
    }
  }
  mUniforms.push_back(Uniform(name, vector));
}

void SpireSubPass::addOrModifyUniform(const Uniform& uniform)
{
  for (auto& i : mUniforms)
  {
    if (i.name == uniform.name && i.type == uniform.type)
    {
      i.data = uniform.data;
      return;
    }
  }
  mUniforms.push_back(uniform);
}

void SpireSubPass::addUniform(const Uniform& uniform)
{
  mUniforms.push_back(uniform);
}
