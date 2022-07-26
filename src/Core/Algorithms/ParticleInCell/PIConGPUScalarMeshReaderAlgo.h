
#ifndef CORE_ALGORITHMS_ParticleInCell_PIConGPUScalarMeshReaderAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_PIConGPUScalarMeshReaderAlgo_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>

namespace SCIRun         {
namespace Core           {
namespace Algorithms     {
namespace ParticleInCell {

class SCISHARE PIConGPUScalarMeshReaderAlgo : public AlgorithmBase
    {
    public:
//        PIConGPUScalarMeshReaderAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
        static const AlgorithmOutputName ScalarMesh_value;
        static const AlgorithmOutputName ScalarMesh_unused1;
        static const AlgorithmOutputName ScalarMesh_unused2;
    };

}}}}
#endif
