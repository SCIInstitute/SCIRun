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


/// @todo Documentation Core/Datatypes/Legacy/Color/Material.cc

#include <Core/Util/Debug.h>
#include <Core/Datatypes/Material.h>

namespace SCIRun {

static Persistent* make_Material()
{
  return new Material;
}

PersistentTypeID Material::type_id("Material", "Persistent", make_Material);

static const char* mutex_name = "Material mutex";

Material::Material()
  : UsedWithLockingHandle<Mutex>(mutex_name), ambient(0,0,0), diffuse(0,0,0),
    specular(0,0,0), shininess(0), emission(0,0,0), reflectivity(0.5),
    transparency(0), refraction_index(1)
{
  DEBUG_CONSTRUCTOR("Material")
}

Material::Material(const Color& ambient, const Color& diffuse,
		   const Color& specular, double shininess)
  : UsedWithLockingHandle<Mutex>(mutex_name), ambient(ambient), diffuse(diffuse),
    specular(specular), shininess(shininess),
    emission(0,0,0), reflectivity(0.5),
    transparency(0), refraction_index(1)
{
  DEBUG_CONSTRUCTOR("Material")
}

Material::Material(const Color& diffuse)
  : UsedWithLockingHandle<Mutex>(mutex_name), diffuse(diffuse), emission(0,0,0),
    reflectivity(0.5), transparency(0), refraction_index(1)
{
  DEBUG_CONSTRUCTOR("Material")
  ambient=diffuse;
  specular=Color(.8,.8,.8);
  shininess=20;
}

Material::Material(const Material& copy)
: Persistent(copy),
  UsedWithLockingHandle<Mutex>(mutex_name), ambient(copy.ambient),
  diffuse(copy.diffuse),
  specular(copy.specular), shininess(copy.shininess),
  emission(copy.emission), reflectivity(copy.reflectivity),
  transparency(copy.transparency),
  refraction_index(copy.refraction_index)
{
  DEBUG_CONSTRUCTOR("Material")
}

Material::~Material()
{
  DEBUG_DESTRUCTOR("Material")
}

Material& Material::operator=(const Material& copy)
{
  ambient=copy.ambient;
  diffuse=copy.diffuse;
  specular=copy.specular;
  shininess=copy.shininess;
  emission=copy.emission;
  reflectivity=copy.reflectivity;
  transparency=copy.transparency;
  refraction_index=copy.refraction_index;
  return *this;
}

Material* Material::clone()
{
  return new Material(*this);
}

#define MATERIAL_VERSION 1

void Material::io(Piostream& stream)
{
  stream.begin_class("Material", MATERIAL_VERSION);
  Pio(stream, ambient);
  Pio(stream, diffuse);
  Pio(stream, specular);
  Pio(stream, shininess);
  Pio(stream, emission);
  Pio(stream, reflectivity);
  Pio(stream, transparency);
  Pio(stream, refraction_index);
}



} // End namespace SCIRun
