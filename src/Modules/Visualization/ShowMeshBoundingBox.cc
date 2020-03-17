#include <Modules/Visualization/ShowMeshBoundingBox.h>
#include <Core/Algorithms/Visualization/ShowMeshBoundingBoxAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ShowMeshBoundingBox, Visualization, SCIRun);

ShowMeshBoundingBox::ShowMeshBoundingBox() : GeometryGeneratingModule(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputGeom);
}

void ShowMeshBoundingBox::setStateDefaults()
{
  setStateIntFromAlgo(Parameters::XSize);
  setStateIntFromAlgo(Parameters::YSize);
  setStateIntFromAlgo(Parameters::ZSize);
}

void
ShowMeshBoundingBox::execute()
{
  auto input = getRequiredInput(InputField);
  if (needToExecute())
  {
    setAlgoIntFromState(Parameters::XSize);
    setAlgoIntFromState(Parameters::YSize);
    setAlgoIntFromState(Parameters::ZSize);

    auto output = algo().run(withInputData((InputField, input)));
    sendOutputFromAlgorithm(OutputGeom, output);
  }
}
