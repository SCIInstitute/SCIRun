
#ifndef CORE_ALGORITHMS_ParticleInCell_PIConGPUAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_PIConGPUAlgo_H

#include <string>
#include <sstream>



#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

namespace SCIRun         {
namespace Core           {
namespace Algorithms     {
namespace ParticleInCell {

ALGORITHM_PARAMETER_DECL(CloneDir);
ALGORITHM_PARAMETER_DECL(OutputDir);
ALGORITHM_PARAMETER_DECL(ConfigFile);
ALGORITHM_PARAMETER_DECL(SimulationFile);

class SCISHARE PIConGPUAlgo : public AlgorithmBase
    {
    public:
        PIConGPUAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;

    private:
        bool StartPIConGPU(const std::string, const std::string, const std::string, const std::string, const int) const;
    };

}}}}
#endif
