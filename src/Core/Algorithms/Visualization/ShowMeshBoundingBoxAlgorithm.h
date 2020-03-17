#ifndef CORE_ALGORITHMS_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_ALGORITHM_H
#define CORE_ALGORITHMS_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_ALGORITHM_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Visualization/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Visualization {
  ALGORITHM_PARAMETER_DECL(XSize);
  ALGORITHM_PARAMETER_DECL(YSize);
  ALGORITHM_PARAMETER_DECL(ZSize);
  class SCISHARE ShowMeshBoundingBoxAlgorithm : public AlgorithmBase
  {
  public:
    ShowMeshBoundingBoxAlgorithm();
    AlgorithmOutput run(const AlgorithmInput& input) const override;

    static const AlgorithmOutputName OutputGeom;
  };
}}}}
#endif
