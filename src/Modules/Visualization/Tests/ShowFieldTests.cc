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


#include <Testing/ModuleTestBase/ModuleTestBase.h>
#include <Testing/Utils/SCIRunFieldSamples.h>
#include <Modules/Visualization/ShowField.h>
#include <Core/Logging/Log.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <cmath>
#include <cstring>

using namespace SCIRun::Testing;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace Visualization;
using namespace Parameters;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core;
using namespace SCIRun;
using namespace Logging;
using ::testing::Values;
using ::testing::Combine;
using ::testing::Range;

class ShowFieldScalingTest : public ParameterizedModuleTest<int>
{
protected:
  void SetUp() override
  {
    LogSettings::Instance().setVerbose(false);
    showField_ = makeModule("ShowField");
    showField_->setStateDefaults();
    showField_->get_state()->setValue(ShowEdges, false);
    auto size = GetParam();
    latVol_ = CreateEmptyLatVol(size, size, size);
    stubPortNWithThisData(showField_, 0, latVol_);
    LOG_DEBUG("Setting up ShowField with size {}^3 latvol", size);
  }

  UseRealModuleStateFactory f_;
  ModuleHandle showField_;
  FieldHandle latVol_;
};

TEST_P(ShowFieldScalingTest, ConstructLatVolGeometry)
{
  LOG_DEBUG("Start ShowField::execute");
  showField_->execute();
  LOG_DEBUG("End ShowField::execute");
}

INSTANTIATE_TEST_CASE_P(
  ConstructLatVolGeometry,
  ShowFieldScalingTest,
  Values(20, 40, 60, 80, 100, 120 //,150 //,200 //,256
  )
  );

class ShowFieldStateGeometryNameSynchronizationTest : public ModuleTest
{
protected:
  void SetUp() override
  {
    LogSettings::Instance().setVerbose(false);
    showField_ = makeModule("ShowField");
    showField_->setStateDefaults();
    const auto size = 2;
    latVol_ = CreateEmptyLatVol(size, size, size);
    stubPortNWithThisData(showField_, 0, latVol_);
  }

  UseRealModuleStateFactory f_;
  ModuleHandle showField_;
  FieldHandle latVol_;
};

TEST_F(ShowFieldStateGeometryNameSynchronizationTest, GeometryNameSynchronizesWithShowFieldState)
{
  const ModuleLevelUniqueIDGenerator generator(*showField_, "EntireField");
  auto hash1 = generator();
  const auto hash2NoChange = generator();
  EXPECT_EQ(hash1, hash2NoChange);

  showField_->get_state()->setValue(Parameters::CylinderRadius, 2);

  auto stateChangeShouldBeDifferent = generator();
  EXPECT_NE(hash2NoChange, stateChangeShouldBeDifferent);
  EXPECT_EQ(stateChangeShouldBeDifferent, generator());

  const auto size = 3;
  latVol_ = CreateEmptyLatVol(size, size, size);
  stubPortNWithThisData(showField_, 0, latVol_);

  const auto inputChangeShouldBeDifferent = generator();
  EXPECT_NE(stateChangeShouldBeDifferent, inputChangeShouldBeDifferent);

  stubPortNWithThisData(showField_, 1, ColorMapHandle());
  const auto addInputShouldBeDifferent = generator();
  EXPECT_NE(inputChangeShouldBeDifferent, addInputShouldBeDifferent);

  EXPECT_NE(hash1, addInputShouldBeDifferent);
  EXPECT_NE(inputChangeShouldBeDifferent, hash1);
}

class ShowFieldPerformanceTest : public ModuleTest {};

TEST_F(ShowFieldPerformanceTest, TestFacePerformance)
{
  LogSettings::Instance().setVerbose(false);
  UseRealModuleStateFactory f;
  auto showField = makeModule("ShowField");
  showField->setStateDefaults();
  auto state = showField->get_state();
  state->setValue(ShowFaces, true);
  state->setValue(ShowEdges, false);
  state->setValue(ShowNodes, false);
  state->setValue(FacesColoring, 1);

  const auto colorMap = StandardColorMapFactory::create();
  stubPortNWithThisData(showField, 1, colorMap);

  std::vector<std::pair<FieldHandle, std::string>> vectorOfInputData =
  {
    { CreateEmptyLatVol(64, 64, 64), "64x64x64 LatVol" }
  };

  const static int NUM_RUNS = 2;

  for (const auto& fs : vectorOfInputData)
  {
    auto start = std::chrono::steady_clock::now();
    stubPortNWithThisData(showField, 0, fs.first);
    for(int j = 0; j < NUM_RUNS; ++j)
    {
      std::cout << "Starting execute #" << j << std::endl;
      showField->execute();
    } 
    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    std::cout << fs.second << " : " << std::chrono::duration<double, std::milli>(diff).count()/NUM_RUNS << " ms\n";
  }
  std::cout << "\n";
}


// Faces of a cell-data field are two sided: the texture coordinate's x holds the
// first adjacent cell's colormap index and y holds the second's, and the shaders
// pick between them with gl_FrontFacing. Regression test for #2439, where the
// vector and tensor paths left y holding a stale value on boundary faces.
class ShowFieldTwoSidedFaceTest : public ModuleTest
{
protected:
  // Distinct, in-range values so a stale y is visible as a different coordinate.
  static std::vector<double> cellValues() { return {-0.8, -0.5, -0.2, 0.1, 0.4, 0.7}; }

  static FieldHandle cubeWithCellData(data_info_type type)
  {
    auto field = CubeTetVolConstantBasis(type);
    auto vfield = field->vfield();
    const auto values = cellValues();
    for (size_t i = 0; i < values.size(); ++i)
    {
      const VMesh::Elem::index_type idx = static_cast<VMesh::Elem::index_type>(i);
      switch (type)
      {
        case data_info_type::DOUBLE_E: vfield->set_value(values[i], idx); break;
        case data_info_type::VECTOR_E: vfield->set_value(Geometry::Vector(values[i], 0.0, 0.0), idx); break;
        case data_info_type::TENSOR_E: vfield->set_value(Geometry::Tensor(values[i] / std::sqrt(3.0)), idx); break;
        default: break;
      }
    }
    return field;
  }

  // Per-vertex texture coordinates from the face VBO, in mesh face order.
  std::vector<std::pair<float, float>> faceTexCoords(FieldHandle field)
  {
    std::vector<std::pair<float, float>> coords;

    auto showField = makeModule("ShowField");
    showField->setStateDefaults();
    auto state = showField->get_state();
    state->setValue(ShowFaces, true);
    state->setValue(ShowEdges, false);
    state->setValue(ShowNodes, false);
    state->setValue(FacesColoring, 1);

    stubPortNWithThisData(showField, 0, field);
    stubPortNWithThisData(showField, 1, StandardColorMapFactory::create());
    showField->execute();

    auto geom = std::dynamic_pointer_cast<Graphics::Datatypes::GeometryObjectSpire>(
      getDataOnThisOutputPort(showField, 0));
    EXPECT_THAT(geom, ::testing::NotNull());
    if (!geom || geom->vbos().empty()) return coords;

    const auto& vbo = geom->vbos().front();
    size_t stride = 0, texOffset = 0;
    bool foundTexCoords = false;
    for (const auto& attrib : vbo.attributes)
    {
      if (attrib.name == "aTexCoords")
      {
        texOffset = stride;
        foundTexCoords = true;
      }
      stride += attrib.sizeInBytes;
    }
    EXPECT_TRUE(foundTexCoords);
    if (!foundTexCoords) return coords;

    const char* buffer = vbo.data->getBuffer();
    const size_t bufferSize = vbo.data->getBufferSize();
    for (size_t offset = texOffset; offset + 2 * sizeof(float) <= bufferSize; offset += stride)
    {
      float xy[2];
      std::memcpy(xy, buffer + offset, sizeof(xy));
      coords.emplace_back(xy[0], xy[1]);
    }
    return coords;
  }

  UseRealModuleStateFactory f_;
};

TEST_F(ShowFieldTwoSidedFaceTest, CellDataFacesAreColoredFromBothAdjacentCells)
{
  LogSettings::Instance().setVerbose(false);

  for (auto type : {data_info_type::DOUBLE_E, data_info_type::VECTOR_E, data_info_type::TENSOR_E})
  {
    auto field = cubeWithCellData(type);
    const auto coords = faceTexCoords(field);

    auto mesh = field->vmesh();
    mesh->synchronize(Mesh::FACES_E);
    VMesh::Face::size_type numFaces;
    mesh->size(numFaces);
    ASSERT_EQ(static_cast<size_t>(3 * numFaces), coords.size());

    size_t boundaryFaces = 0, interiorFaces = 0;
    VMesh::Face::iterator fiter, fend;
    mesh->begin(fiter);
    mesh->end(fend);
    for (size_t vertex = 0; fiter != fend; ++fiter, vertex += 3)
    {
      VMesh::Elem::array_type cells;
      mesh->get_elems(cells, *fiter);

      for (size_t i = 0; i < 3; ++i)
      {
        if (cells.size() == 1)
          EXPECT_EQ(coords[vertex + i].first, coords[vertex + i].second)
            << "boundary face " << *fiter << ", data type " << static_cast<int>(type);
        else
          EXPECT_NE(coords[vertex + i].first, coords[vertex + i].second)
            << "interior face " << *fiter << ", data type " << static_cast<int>(type);
      }

      (cells.size() == 1 ? boundaryFaces : interiorFaces)++;
    }

    // The cube is six tets: twelve surface triangles and six shared faces.
    EXPECT_EQ(size_t(12), boundaryFaces);
    EXPECT_EQ(size_t(6), interiorFaces);
  }
}
