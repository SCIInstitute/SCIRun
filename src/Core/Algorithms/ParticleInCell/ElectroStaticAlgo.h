
#ifndef CORE_ALGORITHMS_ParticleInCell_ElectroStaticAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_ElectroStaticAlgo_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

//      Originally from electrostatic simulation Main.cpp
#include <math.h>
#include <iostream>
#include <iomanip>
#include "World.h"
#include "PotentialSolver.h"
#include "Species.h"
#include "Output.h"
//

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>

namespace SCIRun         {
namespace Core           {
namespace Algorithms     {
namespace ParticleInCell {

ALGORITHM_PARAMETER_DECL(NumTimeSteps);

class SCISHARE ElectroStaticAlgo : public AlgorithmBase
    {
    public:
        ElectroStaticAlgo();
        AlgorithmOutput run(const AlgorithmInput& input) const;
            static const AlgorithmOutputName x_coordinates;
            static const AlgorithmOutputName y_coordinates;
            static const AlgorithmOutputName z_coordinates;
    };
}}}}
#endif
