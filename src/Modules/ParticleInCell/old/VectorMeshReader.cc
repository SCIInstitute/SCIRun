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
#include <Modules/ParticleInCell/VectorMeshReader.h>
#include <Core/Algorithms/ParticleInCell/VectorMeshReaderAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;                          //possible duplicate for MatrixTypeConversion above
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

using namespace openPMD;
using std::cout;

MODULE_INFO_DEF(VectorMeshReader,ParticleInCell,SCIRun);

const AlgorithmOutputName VectorMeshReaderAlgo::VectorMesh_X("VectorMesh_X");
const AlgorithmOutputName VectorMeshReaderAlgo::VectorMesh_Y("VectorMesh_Y");
const AlgorithmOutputName VectorMeshReaderAlgo::VectorMesh_Z("VectorMesh_Z");

VectorMeshReader::VectorMeshReader() : Module(staticInfo_)
    {
    INITIALIZE_PORT(VectorMesh_X);
    INITIALIZE_PORT(VectorMesh_Y);
    INITIALIZE_PORT(VectorMesh_Z);
    }

void VectorMeshReader::setStateDefaults() {}

void VectorMeshReader::execute()
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
            cout << "\nFrom PIConGPUVectorMeshReader: Current iteration is: " << iteration.iterationIndex << std::endl;

                                                      //Load mesh data; ijk values at xyz node points (from Franz Poschel email, 17 May 2022)
            auto mesh = iteration.meshes["E"];
            auto E_x = mesh["x"].loadChunk<float>();
            auto E_y = mesh["y"].loadChunk<float>();
            auto E_z = mesh["z"].loadChunk<float>();
            iteration.seriesFlush();
            iteration.close();                        //Data is now available

//    ***************************************************** Set up and load the output buffers

            auto extent_x = mesh["x"].getExtent();
            const int buffer_size   = extent_x[0] * extent_x[1] * extent_x[2];
            auto buffer_pos_x       = new double[buffer_size];
            auto buffer_pos_y       = new double[buffer_size];
            auto buffer_pos_z       = new double[buffer_size];

            for (size_t i = 0; i < extent_x[0]; ++i)
                {
                for (size_t j = 0; j < extent_x[1]; ++j)
                    {
                    for (size_t k = 0; k < extent_x[2]; ++k)
                        {
                        size_t flat_index = i * extent_x[1] * extent_x[2] + j * extent_x[2] + k;
                        buffer_pos_x[flat_index]=E_x.get()[flat_index];
                        buffer_pos_y[flat_index]=E_y.get()[flat_index];
                        buffer_pos_z[flat_index]=E_z.get()[flat_index];
/*
                        if (i == 1 && j == 1 && k ==1)  //implement (ijk % something) == 0 here to get a sample set
                            {
                                                      //testing and debug: Output 3 dim vector from E field
                            cout << "\nxyz values at mesh E node point(s) are\n";
                            cout << "\t x: " << E_x.get()[flat_index] << "\t y: " << E_y.get()[flat_index] << "\t z: " << E_z.get()[flat_index] << "\n----------\n";
                            }*/
                        }
                    }
                }

//    *****************************************************  Set up the output data

            DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size, 1));
            DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size, 1));
            DenseMatrixHandle output_mat_2(new DenseMatrix(buffer_size, 1));

            double *data0=output_mat_0->data();
            double *data1=output_mat_1->data();
            double *data2=output_mat_2->data();

            std::copy(buffer_pos_x, buffer_pos_x+buffer_size, data0);
            std::copy(buffer_pos_y, buffer_pos_y+buffer_size, data1);
            std::copy(buffer_pos_z, buffer_pos_z+buffer_size, data2);

//    *****************************************************  Send data to the output ports

            sendOutput(VectorMesh_X, output_mat_0);
            sendOutput(VectorMesh_Y, output_mat_1);
            sendOutput(VectorMesh_Z, output_mat_2);
            }  //end of the openPMD reader loop
        }  //end of the "needToExecute" block
    }  //end of the "VectorMeshReader::execute()" function

