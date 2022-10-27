/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <openPMD/openPMD.hpp>
#include <filesystem>

#include <Modules/ParticleInCell/PIConGPU.h>
#include <Core/Algorithms/ParticleInCell/PIConGPUAlgo.h>


using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

using std::cout;
using namespace openPMD;

MODULE_INFO_DEF(PIConGPU,ParticleInCell,SCIRun);

PIConGPU::PIConGPU() : Module(staticInfo_) {}

void PIConGPU::setStateDefaults()
    {
    setStateIntFromAlgo(Variables::Method);
    setStateStringFromAlgo(Parameters::CloneDir);
    setStateStringFromAlgo(Parameters::OutputDir);
    setStateStringFromAlgo(Parameters::ConfigFile);
    setStateStringFromAlgo(Parameters::SimulationFile);
    }

void PIConGPU::execute()
    {
    AlgorithmInput input;
//    if(needToExecute())
        {
        auto state = get_state();
        setAlgoIntFromState(Variables::Method);
        setAlgoStringFromState(Parameters::CloneDir);
        setAlgoStringFromState(Parameters::OutputDir);
        setAlgoStringFromState(Parameters::ConfigFile);
        setAlgoStringFromState(Parameters::SimulationFile);
        auto output=algo().run(input);
/**/
                                                        //Wait for simulation output data to be generated and posted via SST
                                                        // TODO: figure out how to use a general reference for the home directory in these two lines of code

        while(!std::filesystem::exists("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst")) sleep(1);
//        while(!std::filesystem::exists("scratch/runs/SST/simOutput/openPMD/simData.sst")) sleep(1);;

        Series series = Series("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);
//        Series series = Series("scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);

        for (IndexedIteration iteration : series.readIterations())
            {
            cout << "\nFrom PIConGPU: Current iteration is: " << iteration.iterationIndex << std::endl;

                                                        //From https://openpmd-api.readthedocs.io/en/latest/usage/serial.html#c
                                                        //Output data about the Series

            Iteration iter = series.iterations[iteration.iterationIndex];
            cout << "Iteration " << iteration.iterationIndex << " contains "
                 << iter.meshes.size()    << " meshes " << "and "
                 << iter.particles.size() << " particle species\n";
            cout << "The Series contains " << series.iterations.size() << " iterations\n";

                                                        //Output data about particles
            cout << "\nParticle data \n";
            for (auto const &ps : iter.particles)
                {
                cout << "\n\t" << ps.first;
                cout << "\n";
                for (auto const &r : ps.second) cout << "\n\t" << r.first;
                }
            cout << '\n';
                                                        //Output data about meshes
            cout << "\nMesh data \n";

            for (auto const &pm : iter.meshes) cout << "\n\t" << pm.first;
            cout << "\n";
/*
            MeshRecordComponent B_x = iter.meshes["B"]["x"];
            Extent extent_B = B_x.getExtent();
            cout << "\nField B has shape (";
            for (auto const &dim : extent_B) cout << dim << ',';
            cout << ") and has datatype " << B_x.getDatatype() << '\n';
*/
            MeshRecordComponent E_x = iter.meshes["E"]["x"];
            Extent extent_E = E_x.getExtent();
            cout << "\nField E is vector valued, has shape (";
            for (auto const &dim : extent_E) cout << dim << ',';
            cout << ") and has datatype " << E_x.getDatatype() << '\n';

            MeshRecordComponent E_charge_density = iter.meshes["e_all_chargeDensity"][MeshRecordComponent::SCALAR];
            Extent extent_cd = E_charge_density.getExtent();
            cout << "\nField e_all_chargeDensity is scalar valued, has shape (";
            for (auto const &dim : extent_cd) cout << dim << ',';
            cout  << ") and has datatype " << E_charge_density.getDatatype() << '\n';
//            cout << "\n----------" << std::endl;
            }
/**/
        }
    }

