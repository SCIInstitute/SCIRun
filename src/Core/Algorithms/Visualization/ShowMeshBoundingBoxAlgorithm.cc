#include <Core/Algorithms/Visualization/ShowMeshBoundingBoxAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;

const AlgorithmOutputName ShowMeshBoundingBoxAlgorithm::OutputGeom("OutputGeom");

ShowMeshBoundingBoxAlgorithm::ShowMeshBoundingBoxAlgorithm()
{
  //set parameter defaults for UI
  // addParameter(Variables::Method, 0);
}


AlgorithmOutput ShowMeshBoundingBoxAlgorithm::run(const AlgorithmInput& input) const
{
  auto input_matrix = input.get<Field>(Variables::InputField);
  AlgorithmOutput output;

  output[OutputGeom] = nullptr;
  return output;
}
