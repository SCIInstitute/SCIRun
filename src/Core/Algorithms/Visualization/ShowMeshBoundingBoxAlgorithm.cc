#include <Core/Algorithms/Visualization/ShowMeshBoundingBoxAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;
using namespace Graphics::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, XSize);
ALGORITHM_PARAMETER_DEF(Visualization, YSize);
ALGORITHM_PARAMETER_DEF(Visualization, ZSize);

const AlgorithmOutputName ShowMeshBoundingBoxAlgorithm::OutputGeom("OutputGeom");

class ShowMeshBoundingBoxImpl
{
public:
  ShowMeshBoundingBoxImpl(int x, int y, int z);
  GeometryHandle run(const FieldHandle& field);
private:
  int x_ = 0;
  int y_ = 0;
  int z_ = 0;
};

ShowMeshBoundingBoxImpl::ShowMeshBoundingBoxImpl(int x, int y, int z)
{
  x_ = x;
  y_ = y;
  z_ = z;
}

GeometryHandle ShowMeshBoundingBoxImpl::run(const FieldHandle& field)
{
  return nullptr;
}

ShowMeshBoundingBoxAlgorithm::ShowMeshBoundingBoxAlgorithm()
{
  using namespace Parameters;
  addParameter(XSize, 0);
  addParameter(YSize, 0);
  addParameter(ZSize, 0);
}


AlgorithmOutput ShowMeshBoundingBoxAlgorithm::run(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Variables::InputField);
  AlgorithmOutput output;

  auto impl = ShowMeshBoundingBoxImpl(get(Parameters::XSize).toInt(), get(Parameters::YSize).toInt(),
                                      get(Parameters::ZSize).toInt());
  output[OutputGeom] = impl.run(input_field);
  return output;
}
