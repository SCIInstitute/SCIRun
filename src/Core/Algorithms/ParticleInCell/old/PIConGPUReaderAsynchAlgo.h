
#ifndef CORE_ALGORITHMS_ParticleInCell_PIConGPUReaderAsynchAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_PIConGPUReaderAsynchAlgo_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

namespace SCIRun         {
namespace Core           {
namespace Algorithms     {
namespace ParticleInCell {

ALGORITHM_PARAMETER_DECL(ParticleType);
ALGORITHM_PARAMETER_DECL(SampleRate);
ALGORITHM_PARAMETER_DECL(VectorFieldType);
ALGORITHM_PARAMETER_DECL(ScalarFieldComp);

class SCISHARE PIConGPUReaderAsynchAlgo : public AlgorithmBase
    {
    public:
        PIConGPUReaderAsynchAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
        static const AlgorithmOutputName Particles;
        static const AlgorithmOutputName ScalarField;
        static const AlgorithmOutputName VectorField;
    };

}}}}
#endif
