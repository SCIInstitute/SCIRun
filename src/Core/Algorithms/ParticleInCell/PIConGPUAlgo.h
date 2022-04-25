
#ifndef CORE_ALGORITHMS_ParticleInCell_PIConGPU_H
#define CORE_ALGORITHMS_ParticleInCell_PIConGPU_H

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

//ALGORITHM_PARAMETER_DECL(NumTimeSteps);

class SCISHARE PIConGPUAlgo : public AlgorithmBase
    {
    public:
        PIConGPUAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
            static const AlgorithmOutputName x_coordinates;
            static const AlgorithmOutputName y_coordinates;
            static const AlgorithmOutputName z_coordinates;
    };

}}}}
#endif
