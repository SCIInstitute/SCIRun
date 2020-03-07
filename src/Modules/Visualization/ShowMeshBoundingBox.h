#ifndef MODULES_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_H
#define MODULES_VISUALIZATION_SHOW_MESH_BOUNDING_BX_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
namespace Modules {
namespace Visualization {
  class SCISHARE ShowMeshBoundingBox : public Dataflow::Networks::GeometryGeneratingModule,
    public Has1InputPort<FieldPortTag>, public Has1OutputPort<GeometryPortTag>
  {
  public:
    ShowMeshBoundingBox();
    virtual void execute();
    virtual void setStateDefaults();

    INPUT_PORT(0, InputField, Field);
    OUTPUT_PORT(0, OutputGeom, GeometryObject);

    MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
  };
}}}

#endif

