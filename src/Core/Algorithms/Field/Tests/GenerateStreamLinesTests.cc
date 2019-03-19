/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2019 Scientific Computing and Imaging Institute,
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

#include <gtest/gtest.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Legacy/Fields/StreamLines/GenerateStreamLines.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

FieldHandle LoadMultiSeeds()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/streamlines/multiSeeds.fld");
}

FieldHandle LoadSingleSeed()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/streamlines/singleSeed.fld");
}

FieldHandle LoadTorsoSeeds()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/seedPointsFromTorso.fld");
}

FieldHandle LoadTorso()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/utahtorso-lowres/gradient.fld");
}

FieldHandle LoadVectorField()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/streamlines/vectorField.fld");
}

FieldHandle LoadExpected()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_latvol.fld");
}

/*
ALGORITHM_PARAMETER_DECL(StreamlineStepSize);
ALGORITHM_PARAMETER_DECL(StreamlineTolerance);
ALGORITHM_PARAMETER_DECL(StreamlineMaxSteps);
ALGORITHM_PARAMETER_DECL(StreamlineDirection);
ALGORITHM_PARAMETER_DECL(StreamlineValue);
ALGORITHM_PARAMETER_DECL(RemoveColinearPoints);
ALGORITHM_PARAMETER_DECL(StreamlineMethod);
//
ALGORITHM_PARAMETER_DECL(AutoParameters);
ALGORITHM_PARAMETER_DECL(NumStreamlines);
ALGORITHM_PARAMETER_DECL(UseMultithreading);
*/

static std::vector<std::string> methods { "AdamsBashforth", "Heun", "RungeKutta",
  "RungeKuttaFehlberg", "CellWalk" };

TEST(GenerateStreamLinesTests, SingleSeedProducesSinglePositiveStreamlineSingleThreaded)
{
  auto singleSeed = LoadSingleSeed();
  auto vectorField = LoadVectorField();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, false);
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, singleSeed, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    // EXPECT_EQ(output->vmesh()->num_nodes(), 65);
    // EXPECT_EQ(output->vmesh()->num_elems(), 116);
    // EXPECT_EQ(output->vfield()->num_values(), 65);
  }
}

TEST(GenerateStreamLinesTests, SingleSeedProducesSinglePositiveStreamlineMultiThreaded)
{
  auto singleSeed = LoadSingleSeed();
  auto vectorField = LoadVectorField();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, true);
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, singleSeed, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    // EXPECT_EQ(output->vmesh()->num_nodes(), 65);
    // EXPECT_EQ(output->vmesh()->num_elems(), 116);
    // EXPECT_EQ(output->vfield()->num_values(), 65);
  }
}

TEST(GenerateStreamLinesTests, MultipleSeedsProducesMultipleStreamlinesSingleThreaded)
{
  auto multiSeeds = LoadMultiSeeds();
  auto vectorField = LoadVectorField();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, false);
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, multiSeeds, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    // EXPECT_EQ(output->vmesh()->num_nodes(), 8568);
    // EXPECT_EQ(output->vmesh()->num_elems(), 8565);
    // EXPECT_EQ(output->vfield()->num_values(), 8568);
  }
}

TEST(GenerateStreamLinesTests, MultipleSeedsProducesMultipleStreamlinesMultiThreaded)
{
  auto multiSeeds = LoadMultiSeeds();
  auto vectorField = LoadVectorField();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, true);
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, multiSeeds, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    // EXPECT_EQ(output->vmesh()->num_nodes(), 8568);
    // EXPECT_EQ(output->vmesh()->num_elems(), 8565);
    // EXPECT_EQ(output->vfield()->num_values(), 8568);
  }
}


TEST(GenerateStreamLinesTests, ManySeedsMultithreaded)
{
  auto torsoSeeds = LoadTorsoSeeds();
  auto torso = LoadTorso();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, true);
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(torso, torsoSeeds, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    // EXPECT_EQ(output->vmesh()->num_nodes(), 65);
    // EXPECT_EQ(output->vmesh()->num_elems(), 116);
    // EXPECT_EQ(output->vfield()->num_values(), 65);
  }
}
