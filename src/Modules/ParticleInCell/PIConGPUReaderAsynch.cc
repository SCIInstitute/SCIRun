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

#include <Modules/ParticleInCell/PIConGPUReaderAsynch.h>
#include <Core/Algorithms/ParticleInCell/PIConGPUReaderAsynchAlgo.h>

#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
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
using namespace SCIRun::Core::Algorithms::ParticleInCell;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;

using std::cout;
using namespace openPMD;

MODULE_INFO_DEF(PIConGPUReaderAsynch,ParticleInCell,SCIRun);

const AlgorithmOutputName PIConGPUReaderAsynchAlgo::Particles("Particles");
const AlgorithmOutputName PIConGPUReaderAsynchAlgo::ScalarField("ScalarField");
const AlgorithmOutputName PIConGPUReaderAsynchAlgo::VectorField("VectorField");

PIConGPUReaderAsynch::PIConGPUReaderAsynch() : Module(staticInfo_)
    {
    INITIALIZE_PORT(Particles);
    INITIALIZE_PORT(ScalarField);
    INITIALIZE_PORT(VectorField);
    INITIALIZE_PORT(OutputData);
    }

void PIConGPUReaderAsynch::setStateDefaults()
    {
    setStateIntFromAlgo(Parameters::SampleRate);
    setStateStringFromAlgo(Parameters::ParticleType);
    setStateStringFromAlgo(Parameters::VectorFieldType);
    setStateStringFromAlgo(Parameters::ScalarFieldComp);
    }

/*
namespace SCIRun::Modules::ParticleInCell
    {
    using DataChunk = BundleHandle;
    using DataStream = std::queue<DataChunk>;

    class StreamAppenderImpl
        {
        public:
            //explicit StreamAppenderImpl(SimulationStreamingReaderBase* module) : module_(module) {} 
            DataStream& stream() { return stream_; } 

            void pushDataToStream()
                {
                //while (module_->hasData())
                    //{
                //auto value = module_->nextData();
                //auto value = module_->theData();
                    //{
                    //Guard g(dataMutex_.get());
                    //stream_.push(value);
                    //}

                    //std::this_thread::sleep_for(std::chrono::milliseconds(appendWaitTime_));
                    //}
                }

            void beginPushDataAsync()
                {
                f_ = std::async([this]() { pushDataToStream(); });
                }

            void waitAndOutputEach()
                {
                //if (module_->hasData())
                    //{
                    //wait for result.
                    //while (stream().empty()) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        
                    //once data is available, output to ports
                    auto data = stream().front();
                        {
                        Guard g(dataMutex_.get());
                        stream().pop();
                        }

                    //module_->sendOutput(module_->OutputData, data);
                    //module_->enqueueExecuteAgain(false);
                    //}
                //else
                //module_->shutdownStream();
                }

        private:
            //SimulationStreamingReaderBase* module_;
            DataStream stream_;
            //const int appendWaitTime_ = 2000;

            std::future<void> f_;
            Mutex dataMutex_{ "streamingData" };
        };
    } 
*/
/*
namespace openPMDStub
{
  class IndexedIteration
  {
  public:
    int iterationIndex{ 0 };
    IndexedIteration(int i) : iterationIndex(i) {}
    void seriesFlush() const {}
    void close() const {}
  };

  enum class Access
  {
    READ_ONLY
  };

  using IndexedIterationContainer = std::vector<IndexedIteration>;
  using IndexedIterationIterator = IndexedIterationContainer::const_iterator;

  class Series
  {
  public:
    Series() {}
    Series(const std::string&, Access) {}
    const IndexedIterationContainer& readIterations() const { return dummySeriesData_; }
  private:
    IndexedIterationContainer dummySeriesData_ {1,2,3,4,5,6};
  };
}
*/

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
        //std::string particle_type = "e";

                                                                 //Read particle data
        Record particlePositions       = iteration.particles[particle_type]["position"];
        Record particlePositionOffsets = iteration.particles[particle_type]["positionOffset"];

        std::array<std::shared_ptr<float>, 3> loadedChunks;
        std::array<std::shared_ptr<int>,        3> loadedChunks1;
        std::array<Extent,                      3> extents;
        std::array<std::string,                 3> const dimensions{{"x", "y", "z"}};

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

        //const int buffer_size  = 1+(num_particles/particle_sample_rate);
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
        //std::string scalar_field_component = "e_all_chargeDensity";
                                                                 //Read scalar field data
        auto scalarFieldData               = iteration.meshes[scalar_field_component][MeshRecordComponent::SCALAR];
        auto scalarFieldData_buffer        = scalarFieldData.loadChunk<float>();

        iteration.seriesFlush();                                 //Data is now available

        auto extent_sFD                    = scalarFieldData.getExtent();
        const int buffer_size_sFD          = extent_sFD[0] * extent_sFD[1] * extent_sFD[2];

                                                                 //Send data to output
        return scalarField(buffer_size_sFD, scalarFieldData_buffer, extent_sFD);
        }

    FieldHandle makeVectorOutput(openPMD::IndexedIteration iteration, std::string vector_field_type)
        {
        //std::string vector_field_type = "E";
                                                                 //Read Vector field data
        auto vectorFieldData          = iteration.meshes[vector_field_type];
        auto vFD_component_x          = vectorFieldData["x"].loadChunk<float>();
        auto vFD_component_y          = vectorFieldData["y"].loadChunk<float>();
        auto vFD_component_z          = vectorFieldData["z"].loadChunk<float>();

        iteration.seriesFlush();                                 //Data is now available

        auto extent_vFD               = vectorFieldData["x"].getExtent();
        const int buffer_size_vFD     = extent_vFD[0] * extent_vFD[1] * extent_vFD[2];

                                                                 //Send data to output
        return vectorField(buffer_size_vFD, extent_vFD, vFD_component_x, vFD_component_y, vFD_component_z);
        }
/*
    Series getSeries(const std::string& SST_dir)
        {
        //Wait for simulation output data to be generated and posted via SST
        while (!std::filesystem::exists(SST_dir)) std::this_thread::sleep_for(std::chrono::seconds(1));
        return Series(SST_dir, Access::READ_ONLY);
        } //end of the getSeries fn
*/

    }; //end of class SimulationStreamingReaderBaseImpl
} //end of namespace SCIRun::Modules::ParticleInCell


void PIConGPUReaderAsynch::execute()
    {
    AlgorithmInput input;
    auto state = get_state();




    //int particle_sample_rate = get(Parameters::SampleRate);   //Need to get the actual value of SampleRate, and the rest of them

    int particle_sample_rate = SampleRate;
    std::string particle_type = ParticleType;
    std::string scalar_field_component = ScalarFieldComp;
    std::string vector_field_type = VectorFieldType;




    auto output=algo().run(input);
    SimulationStreamingReaderBaseImpl P;
    if (!setup_)
        {
        while (!std::filesystem::exists(SST_dir)) std::this_thread::sleep_for(std::chrono::seconds(1));
        series = Series(SST_dir, Access::READ_ONLY);
        end    = series.readIterations().end();
        it     = series.readIterations().begin();
        setup_ = true;
        }

    IndexedIteration iteration = *it;
    //Need to handle having or not having particle and mesh data better for both individual ports and bundled output
    //Possibly use the UI variables: particle_type, vector_field_type and vector_field_component when those are implemented
    if(iteration.particles.size()) sendOutput(Particles, P.makeParticleOutput(iteration, particle_sample_rate, particle_type));
    if(iteration.meshes.size())    sendOutput(ScalarField, P.makeScalarOutput(iteration, scalar_field_component));
    if(iteration.meshes.size())    sendOutput(VectorField, P.makeVectorOutput(iteration, vector_field_type));
    BundleHandle TheData = bundleOutputs({"ScalarField", "VectorField"}, {P.makeScalarOutput(iteration, scalar_field_component), P.makeVectorOutput(iteration, vector_field_type)});
    sendOutput(OutputData, TheData);
    iteration.close();

    cout << "From the Reader: iteration counter is " << iteration_counter <<"\n";
    ++it;
    ++iteration_counter;
    if(it != end) enqueueExecuteAgain(false);
    }

/*
void PIConGPUReaderAsynch::setupStream()
    {
    if (!impl_->setup_)
        {
        impl_->series = impl_->getSeries(SST_dir);
        impl_->it = impl_->series.readIterations().begin();
        impl_->end = impl_->series.readIterations().end();
        impl_->setup_ = true;
        }
    }

void PIConGPUReaderAsynch::shutdownStream()
    {
    //impl_->series = {};
    //impl_->iterationIterator = {};
    //impl_->iterationIteratorEnd = {};
    impl_->setup_ = false;
    }

bool PIConGPUReaderAsynch::hasData() const
    {
    //return impl_->iterationIterator != impl_->iterationIteratorEnd;
    return true;
    }

//BundleHandle PIConGPUReaderAsynch::nextData() const
    //{
    //const auto& ii = *(impl_->iterationIterator++);
    //return bundleOutputs({"Particles", "ScalarField", "VectorField"}, {impl_->makeParticleOutput(ii), impl_->makeScalarOutput(ii), impl_->makeVectorOutput(ii)});
    //return bundleOutputs({"Particles", "ScalarField", "VectorField"}, {SimulationStreamingReaderBaseImpl::makeParticleOutput,SimulationStreamingReaderBaseImpl::makeScalarOutput,SimulationStreamingReaderBaseImpl::makeVectorOutput});
    //}
*/

Core::Datatypes::BundleHandle SCIRun::Modules::ParticleInCell::bundleOutputs(std::initializer_list<std::string> names, std::initializer_list<DatatypeHandle> dataList)
    {
    auto bundle = makeShared<Bundle>();
    auto nIter = names.begin();
    auto dIter = dataList.begin();
    for (; nIter != names.end(); ++nIter, ++dIter) bundle->set(*nIter, *dIter);
    return bundle;
    }

