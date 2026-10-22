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

#ifndef CORE_DATATYPES_VTKGEOMETRY_H
#define CORE_DATATYPES_VTKGEOMETRY_H

#include <Core/Algorithms/Visualization/VtkIncludes.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Datatypes/VTK/share.h>

namespace SCIRun {
namespace Graphics {
  namespace Datatypes {

    class WidgetBase;
    using WidgetHandle = SharedPointer<WidgetBase>;
    using WidgetList = std::vector<WidgetHandle>;
    using WidgetListIterator = WidgetList::const_iterator;

  }
}
}

namespace SCIRun
{
namespace Core
{
namespace Datatypes
{

class SCISHARE VtkGeometryObject : public Datatype
{
public:
    struct Material
    {
    float color[3]{1.f, 1.f, 1.f};
    float opacity{1.f};
    };

    struct TransferFunc
    {
      bool fromColorMap{false};
      std::vector<float> colors;
      std::vector<float> opacities;
      std::vector<float> range;
    };

    VtkGeometryObject() = default;
    VtkGeometryObject(const GeometryIDGenerator& idGenerator, const std::string& tag);
    VtkGeometryObject(const VtkGeometryObject& other) = delete;
    VtkGeometryObject& operator=(const VtkGeometryObject& other) = delete;

    VtkGeometryObject* clone() const override;

    void setObjectName(const std::string& name) { objectName_ = name; }
    const std::string& objectName() const { return objectName_; }
    const std::string uniqueID() const { return objectName_ + std::to_string(idIncrement_); }
    void changeID() { ++idIncrement_; }

    std::string dynamic_type_name() const override { return "VtkGeometryObject"; }
    
#ifdef WITH_VTK
    vtkSmartPointer<vtkDataObject> dataObject;
#endif

    GeometryType type{GeometryType::NO_TYPE};

    Material material;
    TransferFunc tfn;
    double radius{1.0};

    Core::Geometry::BBox box;

    uint64_t id{0};
    uint64_t version{0};

    Graphics::Datatypes::WidgetHandle widget;

private:
    std::string objectName_;       ///< Name of this object. Should be unique across all modules in the network.
    uint64_t idIncrement_{0};  // Incrementing this lets you update the object without updating every other geometry object
};

using VtkGeometryObjectHandle = SharedPointer<VtkGeometryObject>;

class SCISHARE CompositeVtkGeometryObject : public VtkGeometryObject
{
public:
    explicit CompositeVtkGeometryObject(const std::vector<VtkGeometryObjectHandle>& objs);
    const std::vector<VtkGeometryObjectHandle>& objects() const { return objs_; }

private:
    std::vector<VtkGeometryObjectHandle> objs_;
};

using CompositeVtkGeometryObjectHandle = SharedPointer<CompositeVtkGeometryObject>;
}
}
}


#endif
