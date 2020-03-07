#include <Modules/Visualization/ShowMeshBoundingBox.h>
#include <Core/Algorithms/Visualization/ShowMeshBoundingBoxAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ShowMeshBoundingBox, Visualization, SCIRun);

ShowMeshBoundingBox::ShowMeshBoundingBox() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputGeom);
}

void ShowMeshBoundingBox::setStateDefaults()
{
  // setStateIntFromAlgo(Variables::Method);
}

void
ShowMeshBoundingBox::execute()
{
  auto input = getRequiredInput(InputField);
  if (needToExecute())
  {
    // setAlgoIntFromState(Variables::Method);
    auto output = algo().run(withInputData((InputField, input)));
    sendOutputFromAlgorithm(OutputGeom, output);
  }
}
