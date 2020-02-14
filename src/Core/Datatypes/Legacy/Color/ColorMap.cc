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


/// @todo Documentation Core/Datatypes/Legacy/Color/ColorMap.cc

#include <Core/Datatypes/ColorMap.h>
#include <Core/Persistent/PersistentSTL.h>
#include <Core/Math/MiscMath.h>
#include <Core/Util/Debug.h>

namespace SCIRun {


Persistent* ColorMap::maker()
{
  return new ColorMap;
}

std::string ColorMap::dynamic_type_name() const { return type_id.type; }

PersistentTypeID ColorMap::type_id("ColorMap", "Datatype", maker);

// Used by maker function above
ColorMap::ColorMap() :
  SLIVR::ColorMap(),
  materials_()
{
  DEBUG_CONSTRUCTOR("ColorMap")
}

ColorMap::ColorMap(const ColorMap& copy) :
  SLIVR::ColorMap(copy),
  PropertyManager(copy),
  materials_(copy.materials_)
{
  DEBUG_CONSTRUCTOR("ColorMap")
}

ColorMap::ColorMap(const SLIVR::ColorMap& copy) :
  SLIVR::ColorMap(copy),
  materials_()
{
  DEBUG_CONSTRUCTOR("ColorMap")
  build_materials_from_rgba();
}


ColorMap::ColorMap(const std::vector<Color>& rgb,
                   const std::vector<float>& rgbT,
                   const std::vector<float>& ialpha,
                   const std::vector<float>& alphaT,
                   unsigned int res) :
  SLIVR::ColorMap(rgb, rgbT, ialpha, alphaT, res),
  materials_()
{
  DEBUG_CONSTRUCTOR("ColorMap")
  build_materials_from_rgba();
}


ColorMap::ColorMap(const float *rgba) :
  SLIVR::ColorMap(rgba),
  materials_()
{
  DEBUG_CONSTRUCTOR("ColorMap")
  build_materials_from_rgba();
}


ColorMap::~ColorMap()
{
  DEBUG_DESTRUCTOR("ColorMap")
}

ColorMap* ColorMap::clone()
{
  return new ColorMap(*this);
}


void
ColorMap::build_materials_from_rgba()
{
  const int size = 256;
  materials_.resize(size);
  const Color ambient(0.0, 0.0, 0.0);
  const Color specular(0.7, 0.7, 0.7);
  for (int i = 0; i < size; i++)
  {
    Color diffuse(rawRGBA_[i*4], rawRGBA_[i*4+1], rawRGBA_[i*4+2]);
    materials_[i] = new Material(ambient, diffuse, specular, 10);
    materials_[i]->transparency = rawRGBA_[i*4+3];
  }
}


#define COLORMAP_VERSION 6

void
ColorMap::io(Piostream& stream)
{

  int version= stream.begin_class("ColorMap", COLORMAP_VERSION);

  if ( version > 5)
  {
    PropertyManager::io(stream);
  }

  if ( version > 4)
  {
    Pio(stream, resolution_);
    Pio(stream, min_);
    Pio(stream, max_);
  }
  else if (version == 4 && stream.reading())
  {
    // Bad guess, better than nothing.
    resolution_ = 256;
  }
  Pio(stream, materials_);
  if ( version > 2 )
    Pio(stream, units_);
  if ( version > 1 )
  {
    Pio(stream,rawRampAlpha_);
    Pio(stream,rawRampAlphaT_);
    Pio(stream,rawRampColor_);
    Pio(stream,rawRampColorT_);

    if ( stream.reading() )
    {
      build_rgba_from_ramp();
      build_materials_from_rgba();
    }
  }
  stream.end_class();
}

const MaterialHandle&
ColorMap::lookup(double value) const
{
  int idx = static_cast<int>( (materials_.size()-1)*(value-min_)/(max_-min_) );
  idx = Clamp(idx, 0, static_cast<int>( materials_.size()-1) );
  return materials_[idx];
}

const MaterialHandle&
ColorMap::lookup2(double nvalue) const
{
  int idx = static_cast<int>( (materials_.size()-1)*nvalue );
  idx = Clamp(idx, 0, static_cast<int>( materials_.size()-1 ));
  return materials_[idx];
}

} // End namespace SCIRun
