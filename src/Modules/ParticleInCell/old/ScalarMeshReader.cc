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

#include <Core/Datatypes/MatrixTypeConversions.h>                     //possible duplicate for Dataflow::Networks below
#include <Modules/ParticleInCell/ScalarMeshReader.h>
#include <Core/Algorithms/ParticleInCell/ScalarMeshReaderAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;                          //possible duplicate for MatrixTypeConversion above
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

using std::cout;
using namespace openPMD;

MODULE_INFO_DEF(ScalarMeshReader,ParticleInCell,SCIRun);

const AlgorithmOutputName ScalarMeshReaderAlgo::ScalarMesh_value("ScalarMesh_value");

ScalarMeshReader::ScalarMeshReader() : Module(staticInfo_)
    {
    INITIALIZE_PORT(ScalarMesh_value);
    }

void ScalarMeshReader::setStateDefaults() {}

void ScalarMeshReader::execute()
    {
    AlgorithmInput input;
    if(needToExecute())
        {
        auto state = get_state();
        auto output=algo().run(input);

//************************************************Start the openPMD Reader function and loop

                                                        //Wait for simulation output data to be generated and posted via SST
                                                        // TODO: figure out how to use a general reference for the home directory in these two lines of code

        while(!std::filesystem::exists("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst")) sleep(1);
//        while(!std::filesystem::exists("scratch/runs/SST/simOutput/openPMD/simData.sst")) sleep(1);

        Series series = Series("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);
//        Series series = Series("scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);

        for (IndexedIteration iteration : series.readIterations())
            {
            cout << "\nFrom PIConGPUScalarMeshReader: Current iteration is: " << iteration.iterationIndex << std::endl;

                                                      //Load mesh data; ijk values at xyz node points (modified from code provided by Franz Poschel email, 17 May 2022)
            auto E_charge_density = iteration.meshes["e_all_chargeDensity"][MeshRecordComponent::SCALAR];
            auto E_cd_buffer = E_charge_density.loadChunk<float>();


            iteration.seriesFlush();
            iteration.close();                        //Data is now available

//    ***************************************************** Set up and load the output buffers

            auto extent_cd = E_charge_density.getExtent();
            const int buffer_size = extent_cd[0] * extent_cd[1] * extent_cd[2];
            cout << "\nField e_all_chargeDensity buffer_size is " << buffer_size <<"\n";
            auto buffer_cd        = new double[buffer_size];


            for (size_t i = 0; i < extent_cd[0]; ++i)
                {
                for (size_t j = 0; j < extent_cd[1]; ++j)
                    {
                    for (size_t k = 0; k < extent_cd[2]; ++k)
                        {
                        size_t flat_index = i * extent_cd[1] * extent_cd[2] + j * extent_cd[2] + k;
                        buffer_cd[flat_index]=E_cd_buffer.get()[flat_index];


/*
                        if (i == 1 && j == 1 && k ==1)  //implement (ijk % something) == 0 here to get a sample set
                            {
                                                      //testing and debug: Output 3 dim vector from E field
                            cout << "\nxyz values at Field e_all_chargeDensity node point(s) are\n";
                            cout << "\t x: " << E_cd_buffer.get()[flat_index] << "\t y: " << E_cd_buffer.get()[flat_index] << "\t z: " << E_cd_buffer.get()[flat_index] << "\n----------\n";
                            }*/
                        }
                    }
                }

//    *****************************************************  Set up the output data

            DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size, 1));



            double *data0=output_mat_0->data();



            std::copy(buffer_cd, buffer_cd+buffer_size, data0);



//    *****************************************************  Send data to the output ports
    
            sendOutput(ScalarMesh_value, output_mat_0);


            }  //end of the openPMD reader loop
        }  //end of the "needToExecute" block
    }  //end of the "ScalarMeshReader::execute()" function

