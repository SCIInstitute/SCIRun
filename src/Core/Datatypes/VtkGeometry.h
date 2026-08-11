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

#ifdef WITH_VTK
#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#endif

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/Datatypes/share.h>

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
    std::vector<float> colors;
    std::vector<float> opacities;
    std::vector<float> range;
    };

    VtkGeometryObject() {}
    VtkGeometryObject(const VtkGeometryObject& other) = delete;
    VtkGeometryObject& operator=(const VtkGeometryObject& other) = delete;

    VtkGeometryObject* clone() const override;

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
    uint64_t version{ 0 };
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

}
}
}


#endif
