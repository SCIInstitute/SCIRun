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

#include <Modules/Fields/CalculateNodeLocationFrequency.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks; 
using namespace SCIRun::Modules::ParticleInCell;
using namespace SCIRun::Core::Algorithms::ParticleInCell;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Modules::Fields;

using std::cout;
using namespace openPMD;

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
//    setStateStringFromAlgo(Parameters::ParticleType);
//    setStateIntFromAlgo(Parameters::SampleRate);
    }

FieldHandle PIConGPUReader::particleData(int buffer_size, float component_x[], float component_y[], float component_z[])
    {
    FieldInformation pcfi("PointCloudMesh",0,"int");
    MeshHandle mesh = CreateMesh(pcfi);

//    mesh->reserve_nodes(buffer_size);                              //this variable produces a compile error, I have temporarily made it a comment
                                                                     //the error is: error: ‘using element_type = class SCIRun::Mesh’ {aka ‘class SCIRun::Mesh’} has no member named ‘reserve_nodes’
    
    FieldHandle field = CreateField(pcfi,mesh);

    VField* ofield = field->vfield();
    VMesh*  omesh =  field->vmesh();
    
    for(VMesh::Node::index_type p=0; p < buffer_size; p++)
        {
        omesh->add_point(Point(component_x[p],component_y[p],component_z[p]));
        ofield->set_value(p, p);
        }

    return field;
    }

FieldHandle PIConGPUReader::scalarField(const int buffer_size_sFD, std::shared_ptr<float> scalarFieldData_buffer, std::vector<long unsigned int> extent_sFD)
    {
    FieldInformation lfi("LatVolMesh",1,"double");
    MeshHandle mesh = CreateMesh(lfi,extent_sFD[0], extent_sFD[1], extent_sFD[2], Point(0.0,0.0,0.0), Point(1.0,1.0,1.0));
    FieldHandle ofh = CreateField(lfi,mesh);
    
    VMesh* omesh = ofh->vmesh();                                     //This variable is noted as not being used - possibly not needed??
    VField* ofield = ofh->vfield();

//    ofield->set_values(scalarFieldData_buffer);                    //This step causes a compile error, appears to require a data type of const T* and may need a secong argument, I have temporarily made it a comment
                                                                     //the error is: error: no matching function for call to ‘SCIRun::VField::set_values(std::shared_ptr<float>&)’
    
    return ofh;
/*
                                                                     //The original code for saving a vector field follows
            std::string vector_field_type = "E";
            auto vectorFieldData          = iteration.meshes[vector_field_type];
            auto vFD_component_x          = vectorFieldData["x"].loadChunk<float>();
            auto vFD_component_y          = vectorFieldData["y"].loadChunk<float>();
            auto vFD_component_z          = vectorFieldData["z"].loadChunk<float>();
            iteration.seriesFlush();                    //Data is now available

            auto extent_vFD   = vectorFieldData["x"].getExtent();
            const int one_Dim = 3*(extent_vFD[0] * extent_vFD[1] * extent_vFD[2]);
            auto XYZ_vec      = new double[one_Dim];

//    *****************************************************  The output function
//    *****************************************************  Load the data to be output

            for(size_t i = 0; i < extent_vFD[0]; ++i) for(size_t j = 0; j < extent_vFD[1]; ++j) for(size_t k = 0; k < extent_vFD[2]; ++k)
                {
                size_t flat_index                                                     = i * extent_vFD[1] * extent_vFD[2] + j * extent_vFD[2] + k;
                XYZ_vec[flat_index]                                                   = vFD_component_x.get()[flat_index];
                XYZ_vec[flat_index + (extent_vFD[0]*extent_vFD[1]*extent_vFD[2])]     = vFD_component_y.get()[flat_index];
                XYZ_vec[flat_index + (extent_vFD[0]*extent_vFD[1]*extent_vFD[2]) * 2] = vFD_component_z.get()[flat_index];
                }

//    *****************************************************  Set up the output data structure
            DenseMatrixHandle output_mat_2(new DenseMatrix(one_Dim, 1));
            double *data2=output_mat_2->data();
            std::copy(XYZ_vec, XYZ_vec+one_Dim, data2);

//    *****************************************************  Send data to the output port
            sendOutput(z_coordinates, output_mat_2);
            sendOutput(XYZ_vec, output_mat_2);
*/
    }

FieldHandle PIConGPUReader::vectorField(const int numvals, std::vector<long unsigned int> extent_vFD, std::shared_ptr<float> vFD_component_x, std::shared_ptr<float> vFD_component_y, std::shared_ptr<float> vFD_component_z)
    {
    FieldInformation lfi("LatVolMesh",1,"double");
    lfi.make_vector();
    MeshHandle mesh = CreateMesh(lfi, extent_vFD[0], extent_vFD[1], extent_vFD[2], Point(0.0,0.0,0.0), Point(1.0,1.0,1.0));
    FieldHandle ofh = CreateField(lfi,mesh);
    
    VMesh* omesh = ofh->vmesh();                                     //This variable is noted as not being used - possibly not needed??
    VField* ofield = ofh->vfield();
    
    for (VMesh::index_type i = 0; i < numvals; i++)
        {
        Vector v;
        v[0] = vFD_component_x.get()[i];
        v[1] = vFD_component_y.get()[i];
        v[2] = vFD_component_z.get()[i];
        ofield->set_value(v, i);
        }

    for (VMesh::index_type i = numvals; i< numvals+numvals; i++)
        {
        Vector v(vFD_component_x.get()[i], vFD_component_y.get()[i], vFD_component_z.get()[i]);
        ofield->set_evalue(v,i);
        }
        
    return ofh;
    }

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

                                                                     //Start Particle data processing
            std::string spec          = "e";                         //set particle related input variables
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

            const int buffer_size      = 1+(num_particles/particle_sample_rate);
            auto component_x           = new float[buffer_size];
            auto component_y           = new float[buffer_size];
            auto component_z           = new float[buffer_size];
//            auto flat_particle_feature = new float[buffer_size*3];

                                                                     //Call the output function
            auto Particle_Output = particleData(buffer_size, component_x, component_y, component_z);
/*
//    *****************************************************  Set up the output data structure
            DenseMatrixHandle output_mat_0(new DenseMatrix(3,buffer_size));
            double *data0=output_mat_0->data();
            std::copy(Particle_Output, Particle_Output+buffer_size, data0);    //Need to figure out how to add an integer to a std::shared_ptr<SCIRun::Field> (all 3 functions)
*/
//    *****************************************************  Send data to the output port
            sendOutput(Particles, Particle_Output);
                                                                     //End of Particle data processing


                                                                     //Start Scalar field data processing
                                                                     //Note: Reading Scalar field data (ijk values at xyz node points is from Franz Poschel email, 17 May 2022)

                                                                     //set Scalar field related input variables
            std::string scalar_field_component = "e_all_chargeDensity";

                                                                     //Read scalar field data
            auto scalarFieldData               = iteration.meshes[scalar_field_component][MeshRecordComponent::SCALAR];
            auto scalarFieldData_buffer        = scalarFieldData.loadChunk<float>();

            iteration.seriesFlush();                                 //Data is now available

            auto extent_sFD                    = scalarFieldData.getExtent();
            const int buffer_size_sFD          = extent_sFD[0] * extent_sFD[1] * extent_sFD[2];
//            auto buffer_sFD                    = new double[buffer_size_sFD];

                                                                     //Call the output function
            auto Scalar_Output = scalarField(buffer_size_sFD, scalarFieldData_buffer, extent_sFD);
/*
//    *****************************************************  Set up the output data structure
            DenseMatrixHandle output_mat_1(new DenseMatrix(buffer_size_sFD, 1));
            double *data1=output_mat_1->data();
            std::copy(Scalar_Output, Scalar_Output+buffer_size_sFD, data1);
*/
//    *****************************************************  Send data to the output port
            sendOutput(ScalarField, Scalar_Output);
                                                                     //End of Scalar field data processing


                                                                     //Start Vector field data processing
                                                                     //Note: Reading Vector field data (ijk values at xyz node points is from Franz Poschel email, 17 May 2022)

            std::string vector_field_type = "E";                     //set Vector field related input variables

                                                                     //Read Vector field data
            auto vectorFieldData          = iteration.meshes[vector_field_type];
            auto vFD_component_x          = vectorFieldData["x"].loadChunk<float>();
            auto vFD_component_y          = vectorFieldData["y"].loadChunk<float>();
            auto vFD_component_z          = vectorFieldData["z"].loadChunk<float>();

            iteration.seriesFlush();                                 //Data is now available

            auto extent_vFD               = vectorFieldData["x"].getExtent();
            const int buffer_size_vFD     = extent_vFD[0] * extent_vFD[1] * extent_vFD[2];
 //           auto XYZ_vec                  = new double[buffer_size_vFD*3];

                                                                     //Call the output function
            auto Vector_Output = vectorField(buffer_size_vFD, extent_vFD, vFD_component_x, vFD_component_y, vFD_component_z);
/*
//    *****************************************************  Set up the output data structure
            DenseMatrixHandle output_mat_2(new DenseMatrix(3,buffer_size_vFD);
            double *data2=output_mat_2->data();
            std::copy(Vector_Output, Vector_Output+buffer_size_vFD, data2);
*/
//    *****************************************************  Send data to the output port
            sendOutput(VectorField, Vector_Output);
                                                                     //End of Vector field data processing

            iteration.close();

            }  //end of the openPMD reader loop
        }  //end of the "needToExecute" block
    }  //end of the "PIConGPU::execute()" function

