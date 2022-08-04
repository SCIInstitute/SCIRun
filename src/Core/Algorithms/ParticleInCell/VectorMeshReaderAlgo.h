
#ifndef CORE_ALGORITHMS_ParticleInCell_VectorMeshReaderAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_VectorMeshReaderAlgo_H

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

class SCISHARE VectorMeshReaderAlgo : public AlgorithmBase
    {
    public:
        AlgorithmOutput run(const AlgorithmInput& input) const;
        static const AlgorithmOutputName VectorMesh_X;
        static const AlgorithmOutputName VectorMesh_Y;
        static const AlgorithmOutputName VectorMesh_Z;
    };

}}}}
#endif
