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

    //std::cout << "method: " << method << std::endl;
    algo.set(Parameters::UseMultithreading, false);
    algo.setOption(Parameters::StreamlineValue, "Distance from seed");
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, singleSeed, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    double min,max;
    output->vfield()->minmax(min, max);
    //std::cout << min << " " << max << std::endl;

    // streamlines contained within 8x8x8 latvol
    EXPECT_EQ(min, 0.0);
    if (method == "AdamsBashforth")
      EXPECT_LT(max, 65.0); // corkscrew streamlines
    else
      EXPECT_LT(max, 12.0);
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
    algo.setOption(Parameters::StreamlineValue, "Distance from seed");
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, singleSeed, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    double min,max;
    output->vfield()->minmax(min, max);
    //std::cout << min << " " << max << std::endl;

    // streamlines contained within 8x8x8 latvol
    EXPECT_EQ(min, 0.0);
    if (method == "AdamsBashforth")
      EXPECT_LT(max, 65.0); // corkscrew streamlines
    else
      EXPECT_LT(max, 12.0);
  }
}

//TODO: use > 16 seeds to force multithreading
TEST(GenerateStreamLinesTests, MultipleSeedsProducesMultipleStreamlinesSingleThreaded)
{
  auto multiSeeds = LoadMultiSeeds();
  auto vectorField = LoadVectorField();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, false);
    algo.setOption(Parameters::StreamlineValue, "Distance from seed");
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, multiSeeds, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    double min,max;
    output->vfield()->minmax(min, max);
    //std::cout << min << " " << max << std::endl;

    // streamlines contained within 8x8x8 latvol
    EXPECT_EQ(min, 0.0);
    if (method == "AdamsBashforth")
      EXPECT_LT(max, 93.0); // corkscrew streamlines
    else
      EXPECT_LT(max, 17.0);
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
    algo.setOption(Parameters::StreamlineValue, "Distance from seed");
    algo.setOption(Parameters::StreamlineMethod, method);

    algo.runImpl(vectorField, multiSeeds, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    double min,max;
    output->vfield()->minmax(min, max);
    //std::cout << min << " " << max << std::endl;

    // streamlines contained within 8x8x8 latvol
    EXPECT_EQ(min, 0.0);
    if (method == "AdamsBashforth")
      EXPECT_LT(max, 93.0); // corkscrew streamlines
    else
      EXPECT_LT(max, 17.0);
  }
}

static std::map<std::string, std::vector<int>> meshOutputByMethod
  { { "AdamsBashforth", {400617, 399617, 400617} },
    { "Heun", {400617, 399617, 400617} },
    { "RungeKutta", {400617, 399617, 400617} },
    { "RungeKuttaFehlberg", {214099, 213099, 214099} },
    { "CellWalk", {98120, 97120, 98120} }
  };

TEST(GenerateStreamLinesTests, ManySeedsMultithreaded)
{
  auto torsoSeeds = LoadTorsoSeeds();
  auto torso = LoadTorso();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, true);
    algo.set(Parameters::StreamlineMaxSteps, 200);
    algo.setOption(Parameters::StreamlineValue, "Distance from seed");
    algo.setOption(Parameters::StreamlineMethod, method);

    //std::cout << "processing real streamlines using " << method << " method." << std::endl;
    algo.runImpl(torso, torsoSeeds, output);

    EXPECT_GT(output->vmesh()->num_nodes(), 0);
    EXPECT_GT(output->vmesh()->num_elems(), 0);
    EXPECT_GT(output->vfield()->num_values(), 0);

    EXPECT_EQ(output->vmesh()->num_nodes(), meshOutputByMethod[method][0]);
    EXPECT_EQ(output->vmesh()->num_elems(), meshOutputByMethod[method][1]);
    EXPECT_EQ(output->vfield()->num_values(), meshOutputByMethod[method][2]);

    double min,max;
    output->vfield()->minmax(min, max);
    //std::cout << min << " " << max << std::endl;

    // streamlines contained within mesh with greatest dimension <= 600
    EXPECT_EQ(min, 0.0);
    if (method == "AdamsBashforth")
      EXPECT_LT(max, 6300.0); // corkscrew streamlines
    else if (method == "CellWalk")
      EXPECT_LT(max, 2900.0);
    else
      EXPECT_LT(max, 600.0);
  }
}

static std::map<std::string, std::pair<double,double>> meshOutputByMethodTotalLength
  { { "AdamsBashforth", {0.0269025, 6260.01} },
    { "Heun", {0.0269025, 517.404} },
    { "RungeKutta", {0.0269025, 476.269} },
    { "RungeKuttaFehlberg", {0.767026, 127.16} },
    { "CellWalk", {0.0, 2861.98} }
  };

TEST(GenerateStreamLinesTests, ManySeedsMultithreadedStreamlineLength)
{
  auto torsoSeeds = LoadTorsoSeeds();
  auto torso = LoadTorso();

  for (const auto& method : methods)
  {
    GenerateStreamLinesAlgo algo;
    FieldHandle output;

    algo.set(Parameters::UseMultithreading, true);
    algo.set(Parameters::StreamlineMaxSteps, 200);
    algo.setOption(Parameters::StreamlineValue, "Streamline length");
    algo.setOption(Parameters::StreamlineMethod, method);

    //std::cout << "processing real streamlines using " << method << " method." << std::endl;
    algo.runImpl(torso, torsoSeeds, output);

    double min,max;
    output->vfield()->minmax(min, max);
    //std::cout << min << " " << max << std::endl;
    EXPECT_NEAR(min, meshOutputByMethodTotalLength[method].first, 1e-2);
    EXPECT_NEAR(max, meshOutputByMethodTotalLength[method].second, 1e-1);
  }
}
