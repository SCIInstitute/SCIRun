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


Coding Sources:
   https://openpmd-api.readthedocs.io/en/latest/usage/streaming.html#c
   https://openpmd-api.readthedocs.io/en/latest/usage/serial.html#c
   email from Franz Poschel, dated 17 May 2022
   email from Franz Poschel, dated 16 June 2022: PIConGPU Issue 4162; https://github.com/ComputationalRadiationPhysics/picongpu/issues/4162


To Do:
    Implement a process for particle sample rate, mesh (node) sample rate, and mesh slice through a specified x, y, or z axis point
    Implement all of the above as config settings

The current known to be functional implementation is the LaserWakefield simulation using any of the following config files 1.cfg, 2.cfg and sst.cfg
I have successfully tested the following PIConGPU module UI entries for Simulation and Config file:
    $PIC_EXAMPLES/LaserWakefield
    $PIC_CFG/sst.cfg
    $PIC_CFG/1.cfg
*/

#include <openPMD/openPMD.hpp>
#include <filesystem>

#include<Core/Algorithms/ParticleInCell/PIConGPUAlgo.h>
#include<Core/Datatypes/MatrixTypeConversions.h>
#include <chrono>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::ParticleInCell;

using std::cout;
using namespace openPMD;
using position_t = float; // TODO: move to header file

ALGORITHM_PARAMETER_DEF(ParticleInCell, SimulationFile);
ALGORITHM_PARAMETER_DEF(ParticleInCell, ConfigFile);
ALGORITHM_PARAMETER_DEF(ParticleInCell, CloneDir);
ALGORITHM_PARAMETER_DEF(ParticleInCell, OutputDir);
ALGORITHM_PARAMETER_DEF(ParticleInCell, IterationIndex);
ALGORITHM_PARAMETER_DEF(ParticleInCell, MaxIndex);

const AlgorithmOutputName PIConGPUAlgo::x_coordinates("x_coordinates");
const AlgorithmOutputName PIConGPUAlgo::y_coordinates("y_coordinates");
const AlgorithmOutputName PIConGPUAlgo::z_coordinates("z_coordinates");

PIConGPUAlgo::PIConGPUAlgo()
    {
    addParameter(Paramaters::SimulationFile, std::string("[Enter the path to your PIConGPU Simulation here]"));
    addParameter(Paramaters::ConfigFile, std::string("[Enter the path to your .config file here]"));
    addParameter(Paramaters::CloneString, std::string("[Enter the path to the simulation clone directory here]"));
    addParameter(Paramaters::OutputString, std::string("[Enter the path to the output directory here]"));
    addParameter(Parameters::IterationIndex, 0);
    simulationstarted_ = false;
    }

bool PIConGPUAlgo::StartPIConGPU(const std::string sim_input, const std::string cfg_input, const std::string sim_clone, const std::string sim_output) const
    {
        #include <stdlib.h>
        using namespace std;
        string text_file;

        text_file = "printf '#!/usr/bin bash\n\ncd /home/kj && source picongpu.profile && pic-create "
                            +sim_input+" "+sim_clone+"\ncd "+sim_clone+" && pic-build && tbg -s bash -c "
                            +cfg_input+" -t etc/picongpu/bash/mpiexec.tpl "+sim_output+" &' > /home/kj/Test_compile_run";

        if(cfg_input.compare("$PIC_CFG/sst.cfg")==0)
            {
            text_file = "printf '#!/usr/bin bash\n\ncd /home/kj && source picongpu.profile && pic-create "
                      +sim_input+" "+sim_clone+"\ncd "+sim_clone+" && pic-build && tbg -s bash -c "
                      +cfg_input+" -t etc/picongpu/bash/mpiexec.tpl /home/kj/scratch/runs/SST &' > /home/kj/Test_compile_run";
            }

            const char *command=text_file.c_str();
            system(command);

            string str="cd $HOME && python3 Test1.py";
            const char *command_1=str.c_str();
            system(command_1);
        
            //Wait for simulation output data to be generated and posted via SST
            while(!std::filesystem::exists(sst_file_) sleep(1);
                  
      return true;
        
    }
                                        
                                        

AlgorithmOutput PIConGPUAlgo::run(const AlgorithmInput&) const
    {
    AlgorithmOutput output;
    auto sim_input  = get(Paramaters::SimulationFile).toString();
    auto cfg_input  = get(Paramaters::ConfigFile).toString();
    auto sim_clone  = get(Paramaters::CloneDir).toString();
    auto sim_output = get(Paramaters::OutputDir).toString();
    auto iter_ind = get(Paramaters::IterationIndex).toInt();
                
    if (!simulationstarted_)
    {
        StartPIConGPU(sim_input, cfg_input, sim_clone, sim_output);
        simulation_started_ = true;
    }
                
    

    


/*
************************************************End of the simulation code call (for SST output)
************************************************Start the openPMD Reader
*/

        
                                                        

    //#if openPMD_HAVE_ADIOS2
        

    //    auto backends = openPMD::getFileExtensions();
    //    if (std::find(backends.begin(), backends.end(), "sst") == backends.end())
    //        {
    //        std::cout << "SST engine not available in ADIOS2." << std::endl;
    //        return 0;
    //        }

        Series series = Series(sst_file_, Access::READ_ONLY);

                
//        for (IndexedIteration iteration : series.readIterations())
//            {
                
                
                //check if  iteration is available (new function)
                
                
                // something like this
            IndexedIteration iteration = series.readIterations(iter_ind);
            cout << "\nCurrent iteration: " << iter_ind << std::endl;

                                                        //From https://openpmd-api.readthedocs.io/en/latest/usage/serial.html#c
            Iteration iter = series.iterations[iter_ind];
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

            MeshRecordComponent B_x = iter.meshes["B"]["x"];
            Extent extent_B = B_x.getExtent();
            cout << "\nField B_x has shape (";
            for (auto const &dim : extent_B) cout << dim << ',';
            cout << ") and has datatype " << B_x.getDatatype() << '\n';

            MeshRecordComponent E_charge_density = iter.meshes["e_all_chargeDensity"][MeshRecordComponent::SCALAR];
            Extent extent_cd = E_charge_density.getExtent();
            cout << "\nField E_charge_density has shape (";
            for (auto const &dim : extent_cd) cout << dim << ',';
            cout  << ") and has datatype " << E_charge_density.getDatatype() << '\n';


                                                        //Load particles xyz position (back to https://openpmd-api.readthedocs.io/en/latest/usage/streaming.html#c)
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

                                                        //Load mesh data; ijk values at xyz node points (from Franz Poschel email, 17 May 2022)
            auto mesh = iteration.meshes["E"];
            auto E_x = mesh["x"].loadChunk<float>();
            auto E_y = mesh["y"].loadChunk<float>();
            auto E_z = mesh["z"].loadChunk<float>();
            iteration.seriesFlush();

            iteration.close();                          //Data is now available

                                                        //Output some useful information to the terminal

            cout << "\nAfter loading particle position data\n";
            Extent const &extent_0 = extents[0];
            int num_particles = extent_0[0];
            cout << "\nNumber of particles is " << num_particles;

                                                        //Set the particle sample rate (this should be set by user interface)
            int particle_sample_rate = 1000000;
    //        int particle_sample_rate = 100;
            cout << "\nParticle sample_rate is " << particle_sample_rate << "\n";
            cout << "The number of particles sampled is " << 1+(num_particles/particle_sample_rate) << "\n";

    //If output data is to be buffered, extend the loadedChunks array defined above as a 2D array to allow including all series iterations and do the following:
    //    collect data needed to allocate arrays that hold the buffered data
    //    end the series iteration loop here:
    //        } //end of the openPMD Reader series iteration loop
    //    include code to load data from all series iterations as noted below

    /*
    ***************************************************** Set up and load the buffers
    */

            const int buffer_size   = 1+(num_particles/particle_sample_rate);
            auto buffer_pos_x       = new double[buffer_size];
            auto buffer_pos_y       = new double[buffer_size];
            auto buffer_pos_z       = new double[buffer_size];

    // implement a loop to load all series iterations:
    //    for (TBD)
    //        {

            for (size_t i_pos = 0; i_pos < 3; ++i_pos)
                {
                std::string dim = dimensions[i_pos];
                auto chunk = loadedChunks[i_pos];

    /*Debug option: The next 3 lines of code output particle position data to the terminal window
    */
                cout <<"\nThe sampled values for particle position in dimension " << dim << " are\n";
                for (size_t j = 0; j<num_particles ; j+=particle_sample_rate) cout << "\t" << chunk.get()[j] << ", ";
                cout << "\n----------" << std::endl;

                if(i_pos==0) for (size_t k = 0; k<num_particles ; k+=particle_sample_rate) buffer_pos_x[k/particle_sample_rate]=chunk.get()[k];
                if(i_pos==1) for (size_t i = 0; i<num_particles ; i+=particle_sample_rate) buffer_pos_y[i/particle_sample_rate]=chunk.get()[i];
                if(i_pos==3) for (size_t m = 0; m<num_particles ; m+=particle_sample_rate) buffer_pos_z[m/particle_sample_rate]=chunk.get()[m];
                }

    /*
    *****************************************************  Set up the output data
    */

            DenseMatrixHandle output_mat_0(new DenseMatrix(buffer_size, 1));
            DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size, 1));
            DenseMatrixHandle output_mat_2(new DenseMatrix(buffer_size, 1));

            double *data0=output_mat_0->data();
            double *data1=output_mat_1->data();
            double *data2=output_mat_2->data();

            std::copy(buffer_pos_x, buffer_pos_x+buffer_size, data0);
            std::copy(buffer_pos_y, buffer_pos_y+buffer_size, data1);
            std::copy(buffer_pos_z, buffer_pos_z+buffer_size, data2);

            output[x_coordinates] = output_mat_0;
            output[y_coordinates] = output_mat_1;
            output[z_coordinates] = output_mat_2;

//            } //end of the openPMD Reader series iteration loop

        return output;
        } //end of the SST output
/*
************************************************End of the simulation code call (for SST output)
************************************************Save openPMD data to storage


    else
        {
	    const char *command=text_file.c_str();
	    system(command);

        string str="cd $HOME && python3 Test1.py";
	    const char *command_1=str.c_str();
	    system(command_1);
        } //end of saving openPMD output (to storage)
*/

//#else
//    std::cout << "The streaming example requires that openPMD has been built with ADIOS2." << std::endl;
//    return 0;
//#endif

    } //end of the PIConGPUAlgo algorithm run
