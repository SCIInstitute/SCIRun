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

#ifndef Graphics_Widgets_WidgetTestingUtility_H
#define Graphics_Widgets_WidgetTestingUtility_H

#include <Core/Datatypes/Feedback.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/GlyphFactory.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      class SCISHARE StubGeometryIDGenerator : public SCIRun::Core::GeometryIDGenerator
      {
      public:
        std::string generateGeometryID(const std::string& tag) const override
        {
          return "<dummyGeomId>" + tag;
        }
      };

      class SCISHARE StubWidgetEvent : public WidgetEvent
      {
      public:
        StubWidgetEvent(Core::Datatypes::WidgetMovement movement, const std::string& label)
        {
          movement_ = movement; label_ = label;
        }
        Core::Datatypes::WidgetMovement baseMovement() const override
        {
          return movement_;
        }
        void move(WidgetBase* widget, Core::Datatypes::WidgetMovement moveType) const override;
        int numMoves() const { return numMoves_; }
      private:
        Core::Datatypes::WidgetMovement movement_;
        std::string label_;
        mutable int numMoves_{ 0 };
      };

      class SCISHARE StubGlyphFactory : public AbstractGlyphFactory
      {
      public:
        std::string sphere(SphereParameters params, WidgetBase& widget) const override { return "__sphere__" + std::to_string(instanceCount_++); }
        std::string disk(DiskParameters params, WidgetBase& widget) const override { return "__disk__" + std::to_string(instanceCount_++); }
        std::string cone(ConeParameters params, WidgetBase& widget) const override { return "__cone__" + std::to_string(instanceCount_++); }
        std::string cylinder(CylinderParameters params, WidgetBase& widget) const override { return "__cylinder__" + std::to_string(instanceCount_++); }
        std::string basicBox(BasicBoundingBoxParameters params, WidgetBase& widget) const override { return "__basicBox__" + std::to_string(instanceCount_++); }
        std::string box(BoundingBoxParameters params, WidgetBase& widget) const override { return "__box__" + std::to_string(instanceCount_++); }
        std::string superquadric(SuperquadricParameters params, WidgetBase& widget) const override { return "__superquadric__" + std::to_string(instanceCount_++); }
      private:
        mutable int instanceCount_{0};
      };
    }
  }
}
#endif
