
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

ALGORITHM_PARAMETER_DECL(SimulationFile);
ALGORITHM_PARAMETER_DECL(ConfigFile);
ALGORITHM_PARAMETER_DECL(CloneDir);
ALGORITHM_PARAMETER_DECL(OuputDir);
ALGORITHM_PARAMETER_DECL(IterationIndex);
ALGORITHM_PARAMETER_DECL(MaxIndex);

class SCISHARE PIConGPUAlgo : public AlgorithmBase
    {
    public:
        PIConGPUAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
            static const AlgorithmOutputName x_coordinates;
            static const AlgorithmOutputName y_coordinates;
            static const AlgorithmOutputName z_coordinates;
    private:
        bool PIConGPUAlgo::StartPIConGPU(const std::string, const std::string, const std::string, const std::string) const
        std::string sst_file_ = "/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst";
        bool simulationstarted_;
    };
}}}}
#endif
