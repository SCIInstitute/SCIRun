
#ifndef CORE_ALGORITHMS_ParticleInCell_PIConGPUReaderAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_PIConGPUReaderAlgo_H

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

//ALGORITHM_PARAMETER_DECL(particle_type);
//ALGORITHM_PARAMETER_DECL(particle_sample_rate);
//ALGORITHM_PARAMETER_DECL(vector_field_type);
//ALGORITHM_PARAMETER_DECL(scalar_field_component);

class SCISHARE PIConGPUReaderAlgo : public AlgorithmBase
    {
    public:
        PIConGPUReaderAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
        static const AlgorithmOutputName Particles;
        static const AlgorithmOutputName ScalarField;
        static const AlgorithmOutputName VectorField;
    };

}}}}
#endif
