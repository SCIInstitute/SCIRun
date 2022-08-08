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

#include <Modules/ParticleInCell/PIConGPUReader.h>
#include <Core/Algorithms/ParticleInCell/PIConGPUReaderAlgo.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
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
void PIConGPUReader::particleData(num_particles, component_x, component_y, component_z)
    {
    const int one_Dim = 3*num_particles;                                         //set up the output vector dimension
    auto flat_particle_feature = new double[one_Dim];                            //set up then load the working vector that holds data to be output
    for (size_t k = 0; k<num_particles; k++)
        {
        flat_particle_feature[k]                =component_x.get()[k];
        flat_particle_feature[k+num_particles]  =component_y.get()[k];
        flat_particle_feature[k+2*num_particles]=component_z.get()[k];
        }

//    *****************************************************  Set up the data to be output
    DenseMatrixHandle output_mat_0(new DenseMatrix(one_Dim, 1));
    double *data0=output_mat_0->data();
    std::copy(flat_particle_feature, flat_particle_feature+one_Dim, data0);

//    *****************************************************  Send data to the output port
    sendOutput(x_coordinates, output_mat_0);
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


//************************************************Start the openPMD Reader function and loop

                                                        //Wait for simulation output data to be generated and posted via SST
                                                        // TODO: figure out how to use a general reference for the home directory in these two lines of code
        cout << "\nDebug: Prior while(!std::filesystem::exists( ...\n";

        while(!std::filesystem::exists("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst")) sleep(1);
//        while(!std::filesystem::exists("scratch/runs/SST/simOutput/openPMD/simData.sst")) sleep(1);

        Series series = Series("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);
//        Series series = Series("scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);

        for (IndexedIteration iteration : series.readIterations())
            {
            cout << "\nFrom PIConGPUParticleReader: Current iteration is: " << iteration.iterationIndex << std::endl;

/*
// New code.  Use the input variables to determine what data to load and which output function to call
            auto spec = "e";                          //setting a development input variable
//            auto particle_feature = "position";                      //setting a development input variable
            std::string output_option = "particles";
            Extent extents_s[3];                                                                               //trying something new
            Extent const &extent_0_s = extents_s[0];
            int num_particles_s = extent_0_s[0];

            if(output_option.compare("particles")==0) //output_option is an input data.
                {
                Record particlePositions = iteration.particles[spec]["position"];
                auto component_x = particlePositions["x"].loadChunk<float>();
                auto component_y = particlePositions["y"].loadChunk<float>();
                auto component_z = particlePositions["z"].loadChunk<float>();
                iteration.seriesFlush();                    //Data is now available
                iteration.close();
                                                            //Call the output function
//                particleData(num_particles_s, component_x, component_y, component_z);
                }
*/
                                                 // old code Load particles xyz position (back to https://openpmd-api.readthedocs.io/en/latest/usage/streaming.html#c)
            Record electronPositions = iteration.particles["e"]["position"];
            std::array<std::shared_ptr<position_t>, 3> loadedChunks;
            std::array<Extent, 3> extents;
            std::array<std::string, 3> const dimensions{{"x", "y", "z"}};

            for (size_t i_dim = 0; i_dim < 3; ++i_dim)
                {
                std::string dim_str = dimensions[i_dim];
                RecordComponent rc = electronPositions[dim_str];
                loadedChunks[i_dim] = rc.loadChunk<position_t>(Offset(rc.getDimensionality(), 0), rc.getExtent());
                extents[i_dim] = rc.getExtent();
                }
/*

                                                        //Prototype scalar field output function call (ijk values at xyz node points is from Franz Poschel email, 17 May 2022)
            if(output_option=="scalar_field")
                {
                auto mesh = iteration.meshes["E"];
                auto component_x = mesh["x"].loadChunk<float>();
                iteration.seriesFlush();                    //Data is now available
                iteration.close();
                                                            //Call the output function
                vectorField(mesh, component_x);
                }
*/


/*
                                                        //Prototype vector field output function call (ijk values at xyz node points is from Franz Poschel email, 17 May 2022)
            if(output_option=="vector_field")
                {
                auto mesh = iteration.meshes["E"];
                auto component_x = mesh["x"].loadChunk<float>();
                auto component_y = mesh["y"].loadChunk<float>();
                auto component_z = mesh["z"].loadChunk<float>();
                iteration.seriesFlush();                    //Data is now available
                iteration.close();
                                                            //Call the output function
                vectorField(mesh, component_x, component_y, component_z);
                }
*/

                                                        //Extract the number of particles and set a particle sampling rate

            Extent const &extent_0 = extents[0];
            int num_particles = extent_0[0];
//            int particle_sample_rate = 1000000;         //Number of samples in the final frame is 4
//            int particle_sample_rate = 100000;          //Number of samples in the final frame is 38
            int particle_sample_rate = 100;             //Number of samples in the final frame is 37154 on WS1, 37149 on the laptop

//*********************Debug: Number of particles and sampling rate are output to the terminal
/*
            cout << "\nAfter loading particle position data\n";
            cout << "\nNumber of particles is " << num_particles;
            cout << "\nParticle sample_rate is " << particle_sample_rate << "\n";
*/
//*********************End of Debug

//            cout << "The number of particles sampled is " << 1+(num_particles/particle_sample_rate) << "\n";
            iteration.close();

//    ***************************************************** Set up and load the module output buffers


            const int buffer_size   = 1+(num_particles/particle_sample_rate);
            auto buffer_pos_x       = new double[buffer_size];
            auto buffer_pos_y       = new double[buffer_size];
            auto buffer_pos_z       = new double[buffer_size];

            for (size_t i_pos = 0; i_pos < 3; ++i_pos)
                {
                std::string dim = dimensions[i_pos];
                auto chunk = loadedChunks[i_pos];

//*********************Debug: Sampled particle position data sent to the terminal
/*
                cout <<"\nThe sampled values for particle position in dimension " << dim << " are\n";
                cout <<"not printed\n";
//                for (size_t j = 0; j<num_particles ; j+=particle_sample_rate) cout << "\t" << chunk.get()[j] << ", ";
//                cout << "\n----------" << std::endl;
*/
//*********************End of Debug

                if(i_pos==0) for (size_t k = 0; k<num_particles ; k+=particle_sample_rate) buffer_pos_x[k/particle_sample_rate]=chunk.get()[k];
                if(i_pos==1) for (size_t i = 0; i<num_particles ; i+=particle_sample_rate) buffer_pos_y[i/particle_sample_rate]=chunk.get()[i];
                if(i_pos==2) for (size_t m = 0; m<num_particles ; m+=particle_sample_rate) buffer_pos_z[m/particle_sample_rate]=chunk.get()[m];
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


            sendOutput(x_coordinates, output_mat_0);
            sendOutput(y_coordinates, output_mat_1);
            sendOutput(z_coordinates, output_mat_2);

            }  //end of the openPMD reader loop
        }  //end of the "needToExecute" block
    }  //end of the "PIConGPU::execute()" function

