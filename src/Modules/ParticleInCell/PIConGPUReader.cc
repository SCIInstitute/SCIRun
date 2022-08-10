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

#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Modules/ParticleInCell/PIConGPUReader.h>
#include <Core/Algorithms/ParticleInCell/PIConGPUReaderAlgo.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
//using namespace SCIRun::Dataflow::Networks; 
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

using std::cout;
using namespace openPMD;

MODULE_INFO_DEF(PIConGPUReader,ParticleInCell,SCIRun);

const AlgorithmOutputName PIConGPUReaderAlgo::x_coordinates("x_coordinates");
const AlgorithmOutputName PIConGPUReaderAlgo::y_coordinates("y_coordinates");
const AlgorithmOutputName PIConGPUReaderAlgo::z_coordinates("z_coordinates");

PIConGPUReader::PIConGPUReader() : Module(staticInfo_)
    {
    INITIALIZE_PORT(x_coordinates);
    INITIALIZE_PORT(y_coordinates);
    INITIALIZE_PORT(z_coordinates);
    }

void PIConGPUReader::setStateDefaults()
    {
//    setStateStringFromAlgo(Parameters::ParticleType);
//    setStateStringFromAlgo(Parameters::ParticleAttribute);
//    setStateIntFromAlgo(Parameters::SampleRate);
    }

/*
void particleData(int buffer_size, float* component_x, float* component_y, float* component_z)
    {


//    *****************************************************  Set up and load the working vector that holds data to be output

    auto flat_particle_feature = new float[buffer_size*3];                            
    for (int k = 0; k<buffer_size; k++)
        {
        flat_particle_feature[k]              =component_x[k];
        flat_particle_feature[k+buffer_size]  =component_y[k];
        flat_particle_feature[k+2*buffer_size]=component_z[k];
        }

//    *****************************************************  Set up the data to be output
    DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size*3, 1));
    double *data0=output_mat_0->data();
    std::copy(flat_particle_feature, flat_particle_feature+buffer_size*3, data0);

//    *****************************************************  Send data to the output port
    sendOutput(flat_particle_feature, output_mat_0);

    }
*/


/*
void PIConGPUReader::scalarField(mesh, node_values)
    {
//    *****************************************************  Get mesh dimensions, calculate the size of the one dimensional output vector, set up and load the one dimensional vector
    auto mesh_extent = mesh["x"].getExtent();
    const int one_Dim = (mesh_extent[0]*mesh_extent[1]*mesh_extent[2]);
    auto flat_node_values = new double[one_Dim];
    for(size_t i = 0; i < mesh_extent[0]; ++i) for(size_t j = 0; j < mesh_extent[1]; ++j) for(size_t k = 0; k < mesh_extent[2]; ++k)
        {
        size_t flat_index = i * mesh_extent[1] * mesh_extent[2] + j * mesh_extent[2] + k;

        flat_node_values[flat_index] = node_values.get()[flat_index];
        }

//    *****************************************************  Set up the data to be output
    DenseMatrixHandle output_mat_0(new DenseMatrix(one_Dim, 1));
    double *data0=output_mat_0->data();
    std::copy(flat_node_values, flat_node_values+one_Dim, data0);

//    *****************************************************  Send data to the output port
    sendOutput(x_coordinates, output_mat_0);
    }
*/

/*
void PIConGPUReader::vectorField(mesh, component_x, component_y, component_z)
    {
//    *****************************************************  Get mesh dimensions, calculate the size of the one dimensional output vector, set up and load the one dimensional vector
    auto mesh_extent = mesh["x"].getExtent();
    const int one_Dim = 3*(mesh_extent[0]*mesh_extent[1]*mesh_extent[2]);
    auto XYZ_vec = new double[one_Dim];
    for(size_t i = 0; i < mesh_extent[0]; ++i) for(size_t j = 0; j < mesh_extent[1]; ++j) for(size_t k = 0; k < mesh_extent[2]; ++k)
        {
        size_t flat_index = i * mesh_extent[1] * mesh_extent[2] + j * mesh_extent[2] + k;

        XYZ_vec[flat_index]                                                      = component_x.get()[flat_index];
        XYZ_vec[flat_index + (mesh_extent[0]*mesh_extent[1]*mesh_extent[2])]     = component_y.get()[flat_index];
        XYZ_vec[flat_index + (mesh_extent[0]*mesh_extent[1]*mesh_extent[2]) * 2] = component_z.get()[flat_index];
        }

//    *****************************************************  Set up the data to be output
    DenseMatrixHandle output_mat_0(new DenseMatrix(one_Dim, 1));
    double *data0=output_mat_0->data();
    std::copy(XYZ_vec, XYZ_vec+one_Dim, data0);

//    *****************************************************  Send data to the output port
    sendOutput(x_coordinates, output_mat_0);
    }
*/

void PIConGPUReader::execute()
    {
    AlgorithmInput input;
    if(needToExecute())
        {
        auto state = get_state();
        auto output=algo().run(input);


//  ************************************************Start the openPMD Reader function and loop

                                                        //Wait for simulation output data to be generated and posted via SST
                                                        // TODO: figure out how to use a general reference for the home directory in these two lines of code

        std::string SST_dir = "../../../scratch/runs/SST/simOutput/openPMD/simData.sst";

        while(!std::filesystem::exists(SST_dir)) sleep(1);

        Series series = Series("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);

        for (IndexedIteration iteration : series.readIterations())
            {
            cout << "\nFrom PIConGPUReader: Current iteration is: " << iteration.iterationIndex << std::endl;

/**/
            std::string spec          = "e";                         //set development input variables
            std::string output_option = "particles";
            int particle_sample_rate  = 100;

                                                                     //Read particle data
            Record particlePositions = iteration.particles[spec]["position"];

            std::array<std::shared_ptr<position_t>, 3> loadedChunks;
            std::array<Extent, 3> extents;
            std::array<std::string, 3> const dimensions{{"x", "y", "z"}};

            for (size_t i_dim = 0; i_dim < 3; ++i_dim)
                {
                std::string dim_str = dimensions[i_dim];
                RecordComponent rc = particlePositions[dim_str];
                loadedChunks[i_dim] = rc.loadChunk<position_t>(Offset(rc.getDimensionality(), 0), rc.getExtent());
                extents[i_dim] = rc.getExtent();
                }

            iteration.seriesFlush();                                 //Data is now available

            Extent const &extent_0 = extents[0];
            int num_particles = extent_0[0];
//                const int one_Dim = 3*num_particles;

            const int buffer_size  = 1+(num_particles/particle_sample_rate);
            auto component_x       = new float[buffer_size];
            auto component_y       = new float[buffer_size];
            auto component_z       = new float[buffer_size];

                                                        //Call the output function
//                particleData(buffer_size, component_x, component_y, component_z);

//    *****************************************************  The output function code is moved here for now
//    *****************************************************  Load the data to be output

            auto flat_particle_feature = new float[buffer_size*3];                            
            for (int k = 0; k<buffer_size; k++)
                {
                flat_particle_feature[k]              =component_x[k];
                flat_particle_feature[k+buffer_size]  =component_y[k];
                flat_particle_feature[k+2*buffer_size]=component_z[k];
                }

        //    *****************************************************  Set up the output data structure
            DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size*3, 1));
            double *data0=output_mat_0->data();
            std::copy(flat_particle_feature, flat_particle_feature+buffer_size*3, data0);

        //    *****************************************************  Send data to the output port
            sendOutput(x_coordinates, output_mat_0);
                                                                     //End of Particle data processing

/**/
                                                                     //Read Scalar field data (ijk values at xyz node points is from Franz Poschel email, 17 May 2022)

            std::string scalar_field_component = "e_all_chargeDensity";

            auto scalarFieldData               = iteration.meshes[scalar_field_component][MeshRecordComponent::SCALAR];
            auto scalarFieldData_buffer        = scalarFieldData.loadChunk<float>();
            iteration.seriesFlush();                    //Data is now available

                                                        //Call the output function
    //            scalarField(mesh, component_x);

//    *****************************************************  The output function code is moved here for now
//    *****************************************************  Load the data to be output

            auto extent_sFD           = scalarFieldData.getExtent();
            const int buffer_size_sFD = extent_sFD[0] * extent_sFD[1] * extent_sFD[2];
            auto buffer_sFD           = new double[buffer_size_sFD];

            for (size_t i = 0; i < extent_sFD[0]; ++i) for (size_t j = 0; j < extent_sFD[1]; ++j) for (size_t k = 0; k < extent_sFD[2]; ++k)
                {
                size_t flat_index = i * extent_sFD[1] * extent_sFD[2] + j * extent_sFD[2] + k;
                buffer_sFD[flat_index] = scalarFieldData_buffer.get()[flat_index];
                }

//    *****************************************************  Set up the output data structure

            DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size_sFD, 1));
            double *data1=output_mat_1->data();
            std::copy(buffer_sFD, buffer_sFD+buffer_size_sFD, data1);

//    *****************************************************  Send data to the output ports
            sendOutput(y_coordinates, output_mat_1);
                                                                     //End of Scalar Field data processing


                                                                     //Read vector field output (ijk values at xyz node points is from Franz Poschel email, 17 May 2022)
            std::string vector_field_type = "E";
            auto vectorFieldData          = iteration.meshes[vector_field_type];
            auto vFD_component_x          = vectorFieldData["x"].loadChunk<float>();
            auto vFD_component_y          = vectorFieldData["y"].loadChunk<float>();
            auto vFD_component_z          = vectorFieldData["z"].loadChunk<float>();
            iteration.seriesFlush();                    //Data is now available

                                                        //Call the output function
    //            vectorField(mesh, component_x, component_y, component_z);

//    *****************************************************  The output function code is moved here for now
//    *****************************************************  Load the data to be output

            auto extent_vFD   = vectorFieldData["x"].getExtent();
            const int one_Dim = 3*(extent_vFD[0] * extent_vFD[1] * extent_vFD[2]);
            auto XYZ_vec = new double[one_Dim];

            for(size_t i = 0; i < extent_vFD[0]; ++i) for(size_t j = 0; j < extent_vFD[1]; ++j) for(size_t k = 0; k < extent_vFD[2]; ++k)
                {
                size_t flat_index = i * extent_vFD[1] * extent_vFD[2] + j * extent_vFD[2] + k;

                XYZ_vec[flat_index]                                                   = vFD_component_x.get()[flat_index];
                XYZ_vec[flat_index + (extent_vFD[0]*extent_vFD[1]*extent_vFD[2])]     = vFD_component_y.get()[flat_index];
                XYZ_vec[flat_index + (extent_vFD[0]*extent_vFD[1]*extent_vFD[2]) * 2] = vFD_component_z.get()[flat_index];
                }

//    *****************************************************  Set up the output data structure

            DenseMatrixHandle output_mat_2(new DenseMatrix(one_Dim, 1));
            double *data2=output_mat_2->data();
            std::copy(XYZ_vec, XYZ_vec+one_Dim, data2);

//    *****************************************************  Send data to the output ports
            sendOutput(z_coordinates, output_mat_2);

            iteration.close();

            }  //end of the openPMD reader loop
        }  //end of the "needToExecute" block
    }  //end of the "PIConGPU::execute()" function

