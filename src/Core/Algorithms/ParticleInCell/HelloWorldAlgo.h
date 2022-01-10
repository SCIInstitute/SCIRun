
#ifndef CORE_ALGORITHMS_ParticleInCell_HelloWorldAlgo_H
#define CORE_ALGORITHMS_ParticleInCell_HelloWorldAlgo_H

#include <sstream>
#include <vector>
#include <algorithm>

#include <thread>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/ParticleInCell/share.h>

namespace SCIRun         {
namespace Core           {
namespace Algorithms     {
namespace ParticleInCell {

    class SCISHARE HelloWorldAlgo : public AlgorithmBase
        {
        public:
            HelloWorldAlgo();
            AlgorithmOutput run(const AlgorithmInput& input) const;
            static const AlgorithmOutputName x_coordinates;
            static const AlgorithmOutputName y_coordinates;
            static const AlgorithmOutputName z_coordinates;
        };

/*
************************ Global variables used by HelloWorld ************************
*/

using namespace std;

const int HW_num_particles = 10;
auto HW_pos_x              = new float[HW_num_particles];

/*
************************ End of global variables used by HelloWorld ************************
*/

}}}}
#endif
