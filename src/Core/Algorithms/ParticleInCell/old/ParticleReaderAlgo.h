
#ifndef CORE_ALGORITHMS_ParticleInCell_ParticleReaderAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_ParticleReaderAlgo_H

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

//ALGORITHM_PARAMETER_DECL(ParticleType);
//ALGORITHM_PARAMETER_DECL(ParticleAttribute);
//ALGORITHM_PARAMETER_DECL(SampleRate);

class SCISHARE ParticleReaderAlgo : public AlgorithmBase
    {
    public:
        ParticleReaderAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
        static const AlgorithmOutputName x_coordinates;
        static const AlgorithmOutputName y_coordinates;
        static const AlgorithmOutputName z_coordinates;
    };

}}}}
#endif
