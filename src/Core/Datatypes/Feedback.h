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


#ifndef CORE_DATATYPES_FEEDBACK_H
#define CORE_DATATYPES_FEEDBACK_H

#include <iosfwd>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
  //TODO: relocate
  enum class MouseButton { NONE = 0, LEFT, MIDDLE, RIGHT };

namespace Core {
namespace Datatypes {
// These will give different types of widget movement through ViewScene.
// To use rotation and scaling, an origin point must be given.
enum WidgetMovement
{
  NONE,
  TRANSLATE,
  ROTATE,
  SCALE,
  TRANSLATE_AXIS,
  TRANSLATE_AXIS_HALF,
  TRANSLATE_AXIS_REVERSE,
  SCALE_UNIDIRECTIONAL,
  SCALE_AXIS,
  SCALE_AXIS_HALF,
  SCALE_AXIS_UNIDIRECTIONAL,
};

struct SCISHARE ClippingPlane
{
  static constexpr int MaxCount = 6;
  bool visible{ false }, showFrame{ false }, reverseNormal{ false };
  double x{ 0 }, y{ 0 }, z{ 0 }, d{ 0 };
};

struct EnumClassHash
{
  template <typename T> std::size_t operator()(T t) const
  {
    return static_cast<std::size_t>(t);
  }
};

struct SCISHARE ViewSceneFeedback : ModuleFeedback
{
  Geometry::Transform transform;
  MouseButton buttonClicked;
  WidgetMovement movementType;
  std::string selectionName;
  std::tuple<int,int> windowSize;

  bool matchesWithModuleId(const std::string& modId) const;
};

struct SCISHARE MeshComponentSelectionFeedback : ModuleFeedback
{
  MeshComponentSelectionFeedback() {}
  MeshComponentSelectionFeedback(const std::string& mod, const std::string& comp, bool sel) :
    moduleId(mod), component(comp), selected(sel) {}
  std::string moduleId;
  std::string component;
  bool selected {false};
};
}}}

#endif
