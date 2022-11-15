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

#include <queue>
#include <future>
#include <Modules/Basic/SimulationReaderBaseModule.h>
#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Mutex.h>

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Thread;

namespace Impl2Duplicated
{
  using DataChunk = DenseMatrixHandle;
  //TODO: need thread-safe container to share
  using DataStream = std::queue<DataChunk>;


  class StreamAppender
  {
  public:
    StreamAppender(AsyncStreamingTest* module, DenseMatrixHandle input) : module_(module), input_(input) {}

    bool hasData() const
    {
      return sliceIndex_ < input_->nrows();
    }

    int numDataAppended() const { return sliceIndex_; }

    DataStream& stream() { return stream_; }

    void pushDataToStream()
    {
      logInfo("__SR__ ........starting streaming reader");

      while (hasData())
      {
        auto value = makeShared<DenseMatrix>(input_->row(sliceIndex_));

        logInfo("__SR__ >>> pushing new data object: [{}]", sliceIndex_);
        {
          Guard g(dataMutex.get());
          stream_.push(value);
          sliceIndex_++;
        }
        logInfo("__SR__ : waiting for {} ms", appendWaitTime_);
        std::this_thread::sleep_for(std::chrono::milliseconds(appendWaitTime_));
      }
    }

    void beginPushDataAsync()
    {
      f_ = std::async([this]() { pushDataToStream(); });
    }

    void waitAndOutputEach()
    {
      if (hasData())
      {
        //wait for result.
        while (stream().empty())
        {
          logInfo("__MAIN__ Waiting for data");
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        //once data is available, output to ports
        auto data = stream().front();
        {
          Guard g(dataMutex.get());
          stream().pop();
        }

        logInfo("__MAIN__ Received data: [{}] outputting matrix.", (*data)(0, 0));
        module_->sendOutput(module_->OutputSlice, bundleOutputs({ "Slice" }, { data }));

        logInfo("__MAIN__ Enqueue execute again");
        module_->enqueueExecuteAgain(false);
      }
    }

  private:
    AsyncStreamingTest* module_;
    DenseMatrixHandle input_;
    DataStream stream_;
    const int appendWaitTime_ = 2000;
    int sliceIndex_{ 0 };

    std::future<void> f_;
    Mutex dataMutex{ "test" };
  };
}

namespace algo
{

class IndexedIteration
{
public:
  int iterationIndex;
  void seriesFlush() const {}
  void close() const {}
};

enum class Access
{
  READ_ONLY
};

class Series
{
public:
  Series(const std::string&, Access) {}
  std::vector<IndexedIteration> readIterations() const { return {}; }
};

#define openPMDIsAvailable 0

class PIConGPUReader_Stub
{
  SimulationReaderBase* module_;
public:
  explicit PIConGPUReader_Stub(SimulationReaderBase* module) : module_(module) {}

  FieldHandle particleData(/*int buffer_size, float component_x[], float component_y[], float component_z[]*/)
  {
    FieldInformation pcfi("PointCloudMesh",0,"int");
    FieldHandle ofh = CreateField(pcfi);
    /*
    VMesh* omesh    = ofh->vmesh();
    for(VMesh::Node::index_type p=0; p < buffer_size; p++) omesh->add_point(Point(component_z[p],component_y[p],component_x[p]));
    */
    return ofh;
  }

  FieldHandle scalarField(const int numvals, /*std::shared_ptr<float> scalarFieldData_buffer,*/ std::vector<long unsigned int> extent_sFD)
  {
    FieldInformation lfi("LatVolMesh",1,"float");
    std::vector<float> values(numvals);
    MeshHandle mesh = CreateMesh(lfi,extent_sFD[0], extent_sFD[1], extent_sFD[2], Point(0.0,0.0,0.0), Point(extent_sFD[0],extent_sFD[1],extent_sFD[2]));
    FieldHandle ofh = CreateField(lfi,mesh);
    /*
    for(int i=0; i<extent_sFD[0]; i++) for(int j=0; j<extent_sFD[1]; j++) for(int k=0; k<extent_sFD[2]; k++)
        {
        int flat_index    = i*extent_sFD[1]*extent_sFD[2]+j*extent_sFD[2]+k;
        int c_m_index     = k*extent_sFD[0]*extent_sFD[1]+j*extent_sFD[0]+i;
        values[c_m_index] = scalarFieldData_buffer.get()[flat_index];
        }
    */
    VField* ofield = ofh->vfield();
    ofield->set_values(values);

    return ofh;
  }

  FieldHandle vectorField(std::vector<long unsigned int> extent_vFD
    #if openPMDIsAvailable
    , const int numvals, std::shared_ptr<float> vFD_component_x, std::shared_ptr<float> vFD_component_y, std::shared_ptr<float> vFD_component_z
    #endif
  )
  {
    FieldInformation lfi("LatVolMesh",1,"float");
    lfi.make_vector();
    MeshHandle mesh = CreateMesh(lfi, extent_vFD[0], extent_vFD[1], extent_vFD[2], Point(0.0,0.0,0.0), Point(extent_vFD[0],extent_vFD[1],extent_vFD[2]));
    FieldHandle ofh = CreateField(lfi,mesh);

    #if openPMDIsAvailable
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
    #endif
    return ofh;
  }

  Series getSeries(const std::string& SST_dir)
  {
    //Wait for simulation output data to be generated and posted via SST
    while (!std::filesystem::exists(SST_dir))
      std::this_thread::sleep_for(std::chrono::seconds(1));
    return Series(SST_dir, Access::READ_ONLY);
  }

  void setupStuff(const IndexedIteration& iteration)
  {
    std::cout << "\nFrom PIConGPUReader: Current iteration is: " << iteration.iterationIndex << std::endl;

#if openPMDIsAvailable
    std::string particle_type = "e";                         //set particle related input variables
    int particle_sample_rate  = 10;
                                                             //Read particle data
    Record particlePositions       = iteration.particles[particle_type]["position"];
    Record particlePositionOffsets = iteration.particles[particle_type]["positionOffset"];                     //see 25 August email from Franz

    std::array<std::shared_ptr<position_t>, 3> loadedChunks;
    std::array<std::shared_ptr<int>,        3> loadedChunks1;
    std::array<Extent,                      3> extents;
    std::array<std::string,                 3> const dimensions{{"x", "y", "z"}};

    for (size_t i_dim = 0; i_dim < 3; ++i_dim)
    {
      std::string dim_str  = dimensions[i_dim];
      RecordComponent rc   = particlePositions[dim_str];
      RecordComponent rc1  = particlePositionOffsets[dim_str];

      loadedChunks[i_dim]  = rc.loadChunk<position_t>(Offset(rc.getDimensionality(), 0), rc.getExtent());
      loadedChunks1[i_dim] = rc1.loadChunk<int>(Offset(rc1.getDimensionality(), 0), rc1.getExtent());
      extents[i_dim]       = rc.getExtent();
    }
#endif
  }

  void dataForParticleField()
  {
#if openPMDIsAvailable
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
                                                           //Load (dimensionless) particle xyz position
      if(i_pos==0) for (size_t k = 0; k<num_particles; k+=particle_sample_rate) component_x[k/particle_sample_rate] = chunk1.get()[k] + chunk.get()[k];
      if(i_pos==1) for (size_t i = 0; i<num_particles; i+=particle_sample_rate) component_y[i/particle_sample_rate] = chunk1.get()[i] + chunk.get()[i];
      if(i_pos==2) for (size_t m = 0; m<num_particles; m+=particle_sample_rate) component_z[m/particle_sample_rate] = chunk1.get()[m] + chunk.get()[m];
    }
#endif
  }

  void scalarFieldSetup1(const IndexedIteration& /*iteration*/)
  {
#if openPMDIsAvailable
    std::string scalar_field_component = "e_all_chargeDensity";
    auto scalarFieldData               = iteration.meshes[scalar_field_component][MeshRecordComponent::SCALAR];
    auto scalarFieldData_buffer        = scalarFieldData.loadChunk<float>();
#endif
  }

  void scalarFieldSetup2()
  {
#if openPMDIsAvailable
    auto extent_sFD                    = scalarFieldData.getExtent();
    const int buffer_size_sFD          = extent_sFD[0] * extent_sFD[1] * extent_sFD[2];
#endif
  }

  void vectorFieldSetup1()
  {
#if openPMDIsAvailable
    //Start Vector field data processing Note: See Franz Poschel email, 17 May 2022)
    std::string vector_field_type = "E";                     //set Vector field related input variable
    auto vectorFieldData          = iteration.meshes[vector_field_type];
    auto vFD_component_x          = vectorFieldData["x"].loadChunk<float>();
    auto vFD_component_y          = vectorFieldData["y"].loadChunk<float>();
    auto vFD_component_z          = vectorFieldData["z"].loadChunk<float>();
#endif
  }

  void vectorFieldSetup2()
  {
#if openPMDIsAvailable
    auto extent_vFD               = vectorFieldData["x"].getExtent();
    const int buffer_size_vFD     = extent_vFD[0] * extent_vFD[1] * extent_vFD[2];
#endif
  }

  FieldHandle makeParticleOutput(const IndexedIteration& iteration)
  {
    setupStuff(iteration);
    iteration.seriesFlush();
    dataForParticleField();
    return particleData(/*buffer_size, component_x, component_y, component_z*/);
  }

  FieldHandle makeScalarOutput(const IndexedIteration& iteration)
  {
    scalarFieldSetup1(iteration);
    iteration.seriesFlush();
    scalarFieldSetup2();

    std::vector<long unsigned int> extent_sFD {2,3,4};
    auto buffer_size_sFD = std::accumulate(extent_sFD.begin(), extent_sFD.end(), 1, std::multiplies<long unsigned int>());
    return scalarField(buffer_size_sFD, extent_sFD/*, scalarFieldData_buffer*/);
  }

  FieldHandle makeVectorOutput(const IndexedIteration& iteration)
  {
    vectorFieldSetup1();
    iteration.seriesFlush();
    vectorFieldSetup2();

    std::vector<long unsigned int> extent_vFD {2,3,4};
    return vectorField(extent_vFD /*, vFD_component_x, vFD_component_y, vFD_component_z*/);
  }

  void executeImpl()
  {
    if (module_->needToExecute())
    {
      auto series = getSeries("/home/kj/scratch/runs/SST/simOutput/openPMD/simData.sst");

      for (const auto& iteration : series.readIterations())
      {
        // sendOutput(module_->Particles, );
        // sendOutput(module_->ScalarField, );
        // sendOutput(module_->VectorField, );

        module_->sendOutput(module_->OutputData,
          bundleOutputs({"Particles", "ScalarField", "VectorField"},
            {makeParticleOutput(iteration), makeScalarOutput(iteration), makeVectorOutput(iteration)}
        ));

        iteration.close();
      }
    }
  }
};
}

MODULE_INFO_DEF(SimulationReaderBase, Basic, SCIRun)

SimulationReaderBase::SimulationReaderBase()
  : Module(staticInfo_, false)
{
  INITIALIZE_PORT(OutputData);
}

SimulationReaderBase::~SimulationReaderBase() = default;

void SimulationReaderBase::execute()
{
  // SCIRun::Core::Logging::GeneralLog::Instance().setVerbose(true);
  //
  // logCritical("AsyncStreamingTest Execute called");
  //
  // auto input = getRequiredInput(InputMatrix);
  //
  // if (needToExecute())
  // {
  //   logInfo("__MAIN__ Resetting impl/async thread");
  //   impl_ = std::make_unique<StreamAppender>(this, castMatrix::toDense(input));
  //   impl_->beginPushDataAsync();
  // }
  //
  // impl_->waitAndOutputEach();
}

Core::Datatypes::BundleHandle SCIRun::Modules::Basic::bundleOutputs(std::initializer_list<std::string> names, std::initializer_list<DatatypeHandle> dataList)
{
  auto bundle = makeShared<Bundle>();
  auto nIter = names.begin();
  auto dIter = dataList.begin();
  for (; nIter != names.end(); ++nIter, ++dIter)
  {
    bundle->set(*nIter, *dIter);
  }
  return bundle;
}
