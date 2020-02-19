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


///
///@file  ColorMap.h
///@brief ColorMap definitions
///
///@author
///       Steven G. Parker
///       Department of Computer Science
///       University of Utah
///@date  November 1994
///

#ifndef CORE_DATATYPES_COLORMAP_H
#define CORE_DATATYPES_COLORMAP_H 1

#include <vector>

#include <Core/Containers/LockingHandle.h>
#include <Core/Datatypes/PropertyManager.h>
#include <Core/Datatypes/Material.h>

#include <slivr/ColorMap.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE ColorMap : public SLIVR::ColorMap, public PropertyManager
{
public:
  ColorMap(const ColorMap&);
  ColorMap(const SLIVR::ColorMap&);
  ColorMap(const float *);
  ColorMap(const std::vector<Color>& rgb,
           const std::vector<float>& rgbT,
           const std::vector<float>& alphas,
           const std::vector<float>& alphaT,
           unsigned int resolution = 256);
  virtual ~ColorMap();
  virtual ColorMap*             clone();

  // Lookup which color value would be associated with in the colormap.
  const MaterialHandle&         lookup(double value) const;

  // Lookup a color in the colormap by a value that has already been fitted
  // to the min/max of the colormap (value should be between 1 and ncolors).
  const MaterialHandle&         lookup2(double value) const;

  // Persistent representation.
  virtual void                  io(Piostream&);
  static PersistentTypeID       type_id;

  virtual std::string dynamic_type_name() const;

private:
  ColorMap();
  static Persistent *           maker();
  void                          build_materials_from_rgba();

  std::vector<MaterialHandle>	materials_;
};


typedef LockingHandle<ColorMap> ColorMapHandle;


} // End namespace SCIRun


#endif
