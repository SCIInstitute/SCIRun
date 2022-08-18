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

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks; 
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;
using namespace SCIRun::Core::Geometry;

using std::cout;
using namespace openPMD;

using position_t = float;                                            //Remove this line when position_t is set in the .h file

MODULE_INFO_DEF(PIConGPUReader,ParticleInCell,SCIRun);

const AlgorithmOutputName PIConGPUReaderAlgo::Particles("Particles");
const AlgorithmOutputName PIConGPUReaderAlgo::ScalarField("ScalarField");
const AlgorithmOutputName PIConGPUReaderAlgo::VectorField("VectorField");

PIConGPUReader::PIConGPUReader() : Module(staticInfo_)
    {
    INITIALIZE_PORT(Particles);
    INITIALIZE_PORT(ScalarField);
    INITIALIZE_PORT(VectorField);
    }

void PIConGPUReader::setStateDefaults()
    {
//    setStateIntFromAlgo(Parameters::particle_sample_rate);
//    setStateStringFromAlgo(Parameters::particle_type);
//    setStateStringFromAlgo(Parameters::vector_field_type);
//    setStateStringFromAlgo(Parameters::scalar_field_component);
    }

FieldHandle PIConGPUReader::particleData(int buffer_size, float component_x[], float component_y[], float component_z[])
    {
    FieldInformation pcfi("PointCloudMesh",0,"int");
    FieldHandle ofh = CreateField(pcfi);
    VMesh* omesh    = ofh->vmesh();
    
    for(VMesh::Node::index_type p=0; p < buffer_size; p++) omesh->add_point(Point(component_x[p],component_y[p],component_z[p]));

    return ofh;
    }

FieldHandle PIConGPUReader::scalarField(const int numvals, std::shared_ptr<float> scalarFieldData_buffer, std::vector<long unsigned int> extent_sFD)
    {
    FieldInformation lfi("LatVolMesh",1,"float");
    std::vector<float> values(numvals);
    MeshHandle mesh = CreateMesh(lfi,extent_sFD[0], extent_sFD[1], extent_sFD[2], Point(0.0,0.0,0.0), Point(1.0,1.0,1.0));
    FieldHandle ofh = CreateField(lfi,mesh);

    for (size_t i = 0; i < extent_sFD[0]; ++i) for (size_t j = 0; j < extent_sFD[1]; ++j) for (size_t k = 0; k < extent_sFD[2]; ++k)
        {
        size_t flat_index = i * extent_sFD[1] * extent_sFD[2] + j * extent_sFD[2] + k;
        values[flat_index] = scalarFieldData_buffer.get()[flat_index];
        }

    VField* ofield = ofh->vfield();
    ofield->set_values(values);

    return ofh;
    }

FieldHandle PIConGPUReader::vectorField(const int numvals, std::vector<long unsigned int> extent_vFD, std::shared_ptr<float> vFD_component_x, std::shared_ptr<float> vFD_component_y, std::shared_ptr<float> vFD_component_z)
    {
    FieldInformation lfi("LatVolMesh",1,"float");
    lfi.make_vector();
    MeshHandle mesh = CreateMesh(lfi, extent_vFD[0], extent_vFD[1], extent_vFD[2], Point(0.0,0.0,0.0), Point(1.0,1.0,1.0));
    FieldHandle ofh = CreateField(lfi,mesh);
    VField* ofield  = ofh->vfield();
    
    for (VMesh::index_type i = 0; i < numvals; i++)
        {
        Vector v;
        v[0] = vFD_component_x.get()[i];
        v[1] = vFD_component_y.get()[i];
        v[2] = vFD_component_z.get()[i];
        ofield->set_value(v, i);
        }
       
    return ofh;
    }

void PIConGPUReader::execute()
    {
    AlgorithmInput input;
    if(needToExecute())
        {
        auto state = get_state();
//        setAlgoIntFromState(Parameters::particle_sample_rate);
//        setAlgoStringFromState(Parameters::particle_type);
//        setAlgoStringFromState(Parameters::vector_field_type);
//        setAlgoStringFromState(Parameters::scalar_field_component);
        auto output=algo().run(input);

//  ************************************************Start the openPMD Reader function and loop

                                                        //Wait for simulation output data to be generated and posted via SST
                                                        // TODO: figure out how to use a general reference for the home directory in these two lines of code

//        std::string SST_dir = "../../../scratch/runs/SST/simOutput/openPMD/simData.sst";
        std::string SST_dir = "/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst";

        while(!std::filesystem::exists(SST_dir)) sleep(1);

        Series series = Series("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst", Access::READ_ONLY);

        for (IndexedIteration iteration : series.readIterations())
            {
            cout << "\nFrom PIConGPUReader: Current iteration is: " << iteration.iterationIndex << std::endl;

                                                                     //Start Particle data processing
            std::string particle_type = "e";                         //set particle related input variables
            int particle_sample_rate  = 100;
                                                                     //Read particle data
            Record particlePositions = iteration.particles[particle_type]["position"];

            std::array<std::shared_ptr<position_t>, 3> loadedChunks;
            std::array<Extent, 3> extents;
            std::array<std::string, 3> const dimensions{{"x", "y", "z"}};

            for (size_t i_dim = 0; i_dim < 3; ++i_dim)
                {
                std::string dim_str = dimensions[i_dim];
                RecordComponent rc  = particlePositions[dim_str];
                loadedChunks[i_dim] = rc.loadChunk<position_t>(Offset(rc.getDimensionality(), 0), rc.getExtent());
                extents[i_dim] = rc.getExtent();
                }

            iteration.seriesFlush();                                 //Data is now available

            Extent const &extent_0 = extents[0];
            int num_particles = extent_0[0];

            const int buffer_size      = 1+(num_particles/particle_sample_rate);
            auto component_x           = new float[buffer_size];
            auto component_y           = new float[buffer_size];
            auto component_z           = new float[buffer_size];

            for (size_t i_pos = 0; i_pos < 3; ++i_pos)
                {
                std::string dim = dimensions[i_pos];
                auto chunk = loadedChunks[i_pos];

                if(i_pos==0) for (size_t k = 0; k<num_particles ; k+=particle_sample_rate) component_x[k/particle_sample_rate]=chunk.get()[k];
                if(i_pos==1) for (size_t i = 0; i<num_particles ; i+=particle_sample_rate) component_y[i/particle_sample_rate]=chunk.get()[i];
                if(i_pos==2) for (size_t m = 0; m<num_particles ; m+=particle_sample_rate) component_z[m/particle_sample_rate]=chunk.get()[m];
                }

                                                                     //Call the output function
            auto Particle_Output = particleData(buffer_size, component_x, component_y, component_z);
            sendOutput(Particles, Particle_Output);
                                                                     //End of Particle data processing

                                                                     //Start Scalar field data processing Note: See Franz Poschel email, 17 May 2022)
                                                                     //set Scalar field related input variable
            std::string scalar_field_component = "e_all_chargeDensity";

                                                                     //Read scalar field data
            auto scalarFieldData               = iteration.meshes[scalar_field_component][MeshRecordComponent::SCALAR];
            auto scalarFieldData_buffer        = scalarFieldData.loadChunk<float>();

            iteration.seriesFlush();                                 //Data is now available

            auto extent_sFD                    = scalarFieldData.getExtent();
            const int buffer_size_sFD          = extent_sFD[0] * extent_sFD[1] * extent_sFD[2];

                                                                     //Call the output function
            auto Scalar_Output = scalarField(buffer_size_sFD, scalarFieldData_buffer, extent_sFD);
            sendOutput(ScalarField, Scalar_Output);
                                                                     //End of Scalar field data processing

                                                                     //Start Vector field data processing Note: See Franz Poschel email, 17 May 2022)
            std::string vector_field_type = "E";                     //set Vector field related input variable

                                                                     //Read Vector field data
            auto vectorFieldData          = iteration.meshes[vector_field_type];
            auto vFD_component_x          = vectorFieldData["x"].loadChunk<float>();
            auto vFD_component_y          = vectorFieldData["y"].loadChunk<float>();
            auto vFD_component_z          = vectorFieldData["z"].loadChunk<float>();

            iteration.seriesFlush();                                 //Data is now available

            auto extent_vFD               = vectorFieldData["x"].getExtent();
            const int buffer_size_vFD     = extent_vFD[0] * extent_vFD[1] * extent_vFD[2];


                                                                     //Call the output function
            auto Vector_Output = vectorField(buffer_size_vFD, extent_vFD, vFD_component_x, vFD_component_y, vFD_component_z);
            sendOutput(VectorField, Vector_Output);
                                                                     //End of Vector field data processing

            iteration.close();

            }  //end of the openPMD reader loop
        }  //end of the "needToExecute" block
    }  //end of the "PIConGPU::execute()" function

