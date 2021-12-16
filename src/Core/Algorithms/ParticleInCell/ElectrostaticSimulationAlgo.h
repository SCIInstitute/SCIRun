
#ifndef CORE_ALGORITHMS_ParticleInCell_ElectrostaticSimulationAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_ElectrostaticSimulationAlgo_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace ParticleInCell {

class SCISHARE ElectrostaticSimulationAlgo : public AlgorithmBase
    {
    public:
      ElectrostaticSimulationAlgo();
      AlgorithmOutput run(const AlgorithmInput& input) const override;
    };

}}}}
#endif
