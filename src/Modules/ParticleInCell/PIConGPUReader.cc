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
#include <stdlib.h>

#include <Modules/ParticleInCell/PIConGPUReader.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

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
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;

using std::cout;
using namespace std;
using namespace openPMD;

MODULE_INFO_DEF(PIConGPUReader,ParticleInCell,SCIRun);

PIConGPUReader::PIConGPUReader() : Module(staticInfo_)
    {
    INITIALIZE_PORT(Particles);
    INITIALIZE_PORT(ScalarField);
    INITIALIZE_PORT(VectorField);
    }

void PIConGPUReader::setStateDefaults()
    {
    auto state = get_state();
    state->setValue(Variables::SampleRate, 100);
    state->setValue(Variables::ParticleType, std::string("e"));
    state->setValue(Variables::ScalarFieldComp, std::string("e_all_chargeDensity"));
    state->setValue(Variables::VectorFieldType, std::string("E"));
    state->setValue(Variables::Method, 1);
    }

namespace SCIRun::Modules::ParticleInCell
{

class SimulationStreamingReaderBaseImpl
    {
    public:

    FieldHandle particleData(int buffer_size, float component_x[], float component_y[], float component_z[])
        {
        FieldInformation pcfi("PointCloudMesh",0,"int");
        FieldHandle ofh = CreateField(pcfi);
        VMesh* omesh    = ofh->vmesh();
        
        for(VMesh::Node::index_type p=0; p < buffer_size; p++) omesh->add_point(Point(component_z[p],component_y[p],component_x[p]));

        return ofh;
        }

    FieldHandle scalarField(const int numvals, std::shared_ptr<float> scalarFieldData_buffer, std::vector<long unsigned int> extent_sFD)
        {
        FieldInformation lfi("LatVolMesh",1,"float");
        std::vector<float> values(numvals);
        MeshHandle mesh = CreateMesh(lfi,extent_sFD[0], extent_sFD[1], extent_sFD[2], Point(0.0,0.0,0.0), Point(extent_sFD[0],extent_sFD[1],extent_sFD[2]));
        FieldHandle ofh = CreateField(lfi,mesh);

        for(int i=0; i<extent_sFD[0]; i++) for(int j=0; j<extent_sFD[1]; j++) for(int k=0; k<extent_sFD[2]; k++)
            {
            int flat_index    = i*extent_sFD[1]*extent_sFD[2]+j*extent_sFD[2]+k;
            int c_m_index     = k*extent_sFD[0]*extent_sFD[1]+j*extent_sFD[0]+i;
            values[c_m_index] = scalarFieldData_buffer.get()[flat_index];
            }

        VField* ofield = ofh->vfield();
        ofield->set_values(values);

        return ofh;
        }

    FieldHandle vectorField(const int numvals, std::vector<long unsigned int> extent_vFD, std::shared_ptr<float> vFD_component_x, std::shared_ptr<float> vFD_component_y, std::shared_ptr<float> vFD_component_z)
        {
        FieldInformation lfi("LatVolMesh",1,"float");
        lfi.make_vector();
        MeshHandle mesh = CreateMesh(lfi, extent_vFD[0], extent_vFD[1], extent_vFD[2], Point(0.0,0.0,0.0), Point(extent_vFD[0],extent_vFD[1],extent_vFD[2]));
        FieldHandle ofh = CreateField(lfi,mesh);
        VField* ofield  = ofh->vfield();

        for(int i=0; i<extent_vFD[0]; i++) for(int j=0; j<extent_vFD[1]; j++) for(int k=0; k<extent_vFD[2]; k++)
            {
            int flat_index = i*extent_vFD[1]*extent_vFD[2]+j*extent_vFD[2]+k;
            int c_m_index  = k*extent_vFD[0]*extent_vFD[1]+j*extent_vFD[0]+i;

            Vector v;
            v[0] = vFD_component_x.get()[flat_index];
            v[1] = vFD_component_y.get()[flat_index];
            v[2] = vFD_component_z.get()[flat_index];
            ofield->set_value(v, c_m_index);
            }

        return ofh;
        }

    FieldHandle makeParticleOutput(openPMD::IndexedIteration iteration, int particle_sample_rate, std::string particle_type)
        {
        //cout << "Debug 01: particle data is being processed\n";
                                                                 //Read particle data
        Record particlePositions       = iteration.particles[particle_type]["position"];
        Record particlePositionOffsets = iteration.particles[particle_type]["positionOffset"];

        std::array<std::shared_ptr<float>, 3> loadedChunks;
        std::array<std::shared_ptr<int>,   3> loadedChunks1;
        std::array<Extent,                 3> extents;
        std::array<std::string,            3> const dimensions{{"x", "y", "z"}};

        for (size_t i_dim = 0; i_dim < 3; ++i_dim)
            {
            std::string dim_str  = dimensions[i_dim];
            RecordComponent rc   = particlePositions[dim_str];
            RecordComponent rc1  = particlePositionOffsets[dim_str];

            loadedChunks[i_dim]  = rc.loadChunk<float>(Offset(rc.getDimensionality(), 0), rc.getExtent());
            loadedChunks1[i_dim] = rc1.loadChunk<int>(Offset(rc1.getDimensionality(), 0), rc1.getExtent());
            extents[i_dim]       = rc.getExtent();
            }

        iteration.seriesFlush();                                 //Data is now available

        Extent const &extent_0 = extents[0];
        int num_particles      = extent_0[0];

        const int buffer_size  = 1+(num_particles/particle_sample_rate);
        auto component_x       = new float[buffer_size];
        auto component_y       = new float[buffer_size];
        auto component_z       = new float[buffer_size];

        for (size_t i_pos = 0; i_pos < 3; ++i_pos)
            {
            std::string dim_str = dimensions[i_pos];
            auto chunk          = loadedChunks[i_pos];
            auto chunk1         = loadedChunks1[i_pos];
                                                                 //Calculate (dimensionless) particle xyz position
            if(i_pos==0) for (size_t k = 0; k<num_particles; k+=particle_sample_rate) component_x[k/particle_sample_rate] = chunk1.get()[k] + chunk.get()[k];
            if(i_pos==1) for (size_t i = 0; i<num_particles; i+=particle_sample_rate) component_y[i/particle_sample_rate] = chunk1.get()[i] + chunk.get()[i];
            if(i_pos==2) for (size_t m = 0; m<num_particles; m+=particle_sample_rate) component_z[m/particle_sample_rate] = chunk1.get()[m] + chunk.get()[m];
            }
                                                                 //Send data to output
        return particleData(buffer_size, component_x, component_y, component_z);
        }

    FieldHandle makeScalarOutput(openPMD::IndexedIteration iteration, std::string scalar_field_component)
        {
        //cout << "Debug 02: scalar field data is being processed\n";
                                                                 //Read scalar field data
        auto scalarFieldData        = iteration.meshes[scalar_field_component][MeshRecordComponent::SCALAR];
        auto scalarFieldData_buffer = scalarFieldData.loadChunk<float>();

        iteration.seriesFlush();                                 //Data is now available

        auto extent_sFD             = scalarFieldData.getExtent();
        const int buffer_size_sFD   = extent_sFD[0] * extent_sFD[1] * extent_sFD[2];

                                                                 //Send data to output
        return scalarField(buffer_size_sFD, scalarFieldData_buffer, extent_sFD);
        }

    FieldHandle makeVectorOutput(openPMD::IndexedIteration iteration, std::string vector_field_type)
        {
        //cout << "Debug 03: vector field data is being processed\n";
                                                                 //Read Vector field data
        auto vectorFieldData      = iteration.meshes[vector_field_type];
        auto vFD_component_x      = vectorFieldData["x"].loadChunk<float>();
        auto vFD_component_y      = vectorFieldData["y"].loadChunk<float>();
        auto vFD_component_z      = vectorFieldData["z"].loadChunk<float>();

        iteration.seriesFlush();                                 //Data is now available

        auto extent_vFD           = vectorFieldData["x"].getExtent();
        const int buffer_size_vFD = extent_vFD[0] * extent_vFD[1] * extent_vFD[2];

                                                                 //Send data to output
        return vectorField(buffer_size_vFD, extent_vFD, vFD_component_x, vFD_component_y, vFD_component_z);
        }
    }; //end of class SimulationStreamingReaderBaseImpl
} //end of namespace SCIRun::Modules::ParticleInCell

void PIConGPUReader::execute()
    {
    AlgorithmInput input;
    SimulationStreamingReaderBaseImpl P;
    if (!setup_) setupStream();
    IndexedIteration iteration = *it;

    if(iteration.particles.size() && (ParticleType    != "None")) sendOutput(Particles,   P.makeParticleOutput(iteration, SampleRate, ParticleType));
    if(iteration.meshes.size()    && (ScalarFieldComp != "None")) sendOutput(ScalarField, P.makeScalarOutput(iteration,   ScalarFieldComp));
    if(iteration.meshes.size()    && (VectorFieldType != "None")) sendOutput(VectorField, P.makeVectorOutput(iteration,   VectorFieldType));
    iteration.close();

    ++it;
    ++iteration_counter;
    if(it != end) enqueueExecuteAgain(false);
    else shutdownStream();
    }

void PIConGPUReader::setupStream()
    {
    auto state      = get_state();
    SampleRate      = state->getValue(Variables::SampleRate).toInt();
    ParticleType    = state->getValue(Variables::ParticleType).toString();
    ScalarFieldComp = state->getValue(Variables::ScalarFieldComp).toString();
    VectorFieldType = state->getValue(Variables::VectorFieldType).toString();
    DataSet         = state->getValue(Variables::Method).toInt();

    //cout << "Debug 10: SST_dir is " << SST_dir << "\n";
    while (!std::filesystem::exists(SST_dir)) std::this_thread::sleep_for(std::chrono::seconds(1));
    series = Series(SST_dir, Access::READ_ONLY);
    end    = series.readIterations().end();
    it     = series.readIterations().begin();
    setup_ = true;
    if(!DataSet) showDataSet();
    }

void PIConGPUReader::shutdownStream()
    {
    string text_file;
    text_file = "rm ~/picongpu.profile ~/picongpu_reRun.profile ~/Sim.py ~/Sim_run";
    const char *command_shutDown = text_file.c_str();
    system(command_shutDown);

    setup_ = false;
    iteration_counter = 0;
    }

void PIConGPUReader::showDataSet()
    {
    IndexedIteration iteration_00 = *it;
    cout << "\nShowing the data set content for iteration " << iteration_00.iterationIndex << "\n";

                                                //Output data about the Series

    Iteration iter = series.iterations[iteration_00.iterationIndex];
    cout << "Iteration " << iteration_00.iterationIndex << " contains "
         << iter.meshes.size()    << " meshes " << "and "
         << iter.particles.size() << " particle species\n";
    //cout << "The Series contains " << series.iterations.size() << " iterations\n";

                                                //Output data about particles
    if(iter.particles.size())
        {
        cout << "\nParticle data \n";
        for (auto const &ps : iter.particles)
            {
            cout << "\nSpecies\t" << ps.first;
            //cout << "\n";
            for (auto const &r : ps.second) cout << "\n\t" << r.first;
            }
        cout << '\n';
        }
    else cout << "\nThere is no particle data in this data set\n";
                                                //Output data about meshes
    if(iter.meshes.size())
        {
        cout << "\nMesh data \n";
        for (auto const &pm : iter.meshes) cout << "\n\t" << pm.first;
        cout << "\n";
        }

    MeshRecordComponent B_x = iter.meshes["B"]["x"];
    Extent extent_B = B_x.getExtent();
    //if(extent_B)
        {
        cout << "\nField B is vector valued, has shape (";
        for (auto const &dim : extent_B) cout << dim << ',';
        cout << ") and datatype " << B_x.getDatatype() << '\n';
        }


    MeshRecordComponent E_x = iter.meshes["E"]["x"];
    Extent extent_E = E_x.getExtent();
    //if(extent_E)
        {
        cout << "\nField E is vector valued, has shape (";
        for (auto const &dim : extent_E) cout << dim << ',';
        cout << ") and datatype " << E_x.getDatatype() << '\n';
        }

    MeshRecordComponent E_charge_density = iter.meshes["e_all_chargeDensity"][MeshRecordComponent::SCALAR];
    Extent extent_cd = E_charge_density.getExtent();
    //if(extent_cd)
        {
        cout << "\nField e_all_chargeDensity is scalar valued, has shape (";
        for (auto const &dim : extent_cd) cout << dim << ',';
        cout  << ") and datatype " << E_charge_density.getDatatype() << '\n';
        }

    }

