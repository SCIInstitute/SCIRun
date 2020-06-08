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


#ifndef CORE_DATATYPES_GEOMETRY_H
#define CORE_DATATYPES_GEOMETRY_H

#include <Core/Datatypes/Datatype.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <set>
#include <vector>
#include <Core/Datatypes/share.h>

namespace SCIRun
{
namespace Core
{
namespace Datatypes
{
  enum GeometryType
  {
    NO_TYPE,
    TRI_SURFACE,
    QUAD_SURFACE,
    STRUCTURED_VOLUME,
    UNSTRUCTURED_VOLUME,
    SPHERE,
    CYLINDER,
    EDGE,
    STREAMLINE,
  };

  using GeomList = std::set<GeometryBaseHandle>;

  class SCISHARE GeometryObject : public Datatype
  {
  public:
    GeometryObject(const GeometryIDGenerator& idGenerator, const std::string& tag);

    GeometryObject(const GeometryObject& other) = delete;
    GeometryObject& operator=(const GeometryObject& other) = delete;

    virtual GeometryObject* clone() const override;

    //weird signature--need to avoid raw ptrs and shared_from_this for now
    virtual void addToList(GeometryBaseHandle handle, GeomList& list);

    const std::string uniqueID() const { return objectName_ + std::to_string(idIncrement_); }
    void changeID() { ++idIncrement_; }

    virtual std::string dynamic_type_name() const override { return "GeometryObject"; }

    static const char delimiter = '\t';

  private:
    const std::string objectName_;     ///< Name of this object. Should be unique across all modules in the network.
    boost::atomic<int> idIncrement_ {0}; // Incrementing this lets you update the object without updating every other geometry object
  };

  struct SCISHARE ModuleFeedback
  {
    virtual ~ModuleFeedback() {}
  };

  class SCISHARE OsprayGeometryObject : public Datatype
  {
  public:
    OsprayGeometryObject() {}
    OsprayGeometryObject(const OsprayGeometryObject& other) = delete;
    OsprayGeometryObject& operator=(const OsprayGeometryObject& other) = delete;

    virtual OsprayGeometryObject* clone() const override;

    virtual std::string dynamic_type_name() const override { return "OsprayGeometryObject"; }

    struct FieldData
    {
      std::vector<float> vertex, normal, color, texCoord;
      std::vector<uint32_t> index;
      uint32_t dim[3];
      float origin[3];
      float spacing[3];
    };

    enum class MaterialType
    {
      DIELECTRIC,
      METAL,
      REFRACTIVE,
      CAR_PAINT,
    };

    struct Material
    {
      float albedo[3] {0.8f, 0.1f, 0.8f};
      float roughness {0.0f};
      float opacity {1.0f};
      float ior {1.45f};
      MaterialType type {MaterialType::DIELECTRIC};
    };

    struct TransferFunc
    {
      std::vector<float> colors;
      std::vector<float> opacities;
    };

    GeometryType type {GeometryType::NO_TYPE};

    //TODO:
    Core::Geometry::BBox box;
    FieldData data;
    Material material;
    TransferFunc tfn;
    double radius;

    uint64_t id;
    uint64_t version;
  };

  using OsprayGeometryObjectHandle = SharedPointer<OsprayGeometryObject>;

  class SCISHARE CompositeOsprayGeometryObject : public OsprayGeometryObject
  {
  public:
    explicit CompositeOsprayGeometryObject(const std::vector<OsprayGeometryObjectHandle>& objs);
    const std::vector<OsprayGeometryObjectHandle>& objects() const { return objs_; }
  private:
    std::vector<OsprayGeometryObjectHandle> objs_;
  };

}}}


#endif
