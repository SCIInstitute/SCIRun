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


#include <Modules/Visualization/ConvertMatrixToColorMap.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/VariableHelper.h>
#include <Testing/ModuleTestBase/ModuleTestBase.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Dataflow::Networks;

class ConvertMatrixToColorMapModuleTests : public ModuleTest
{
protected:
  // The column mapping lives in module state, so the mock state won't do.
  UseRealModuleStateFactory realState_;

  ColorMapHandle runWith(ModuleHandle mod, DenseMatrixHandle m)
  {
    stubPortNWithThisData(mod, 0, m);
    EXPECT_NO_THROW(mod->execute());
    return std::dynamic_pointer_cast<ColorMap>(getDataOnThisOutputPort(mod, 0));
  }

  void setColumnRoles(ModuleHandle mod, const std::vector<std::string>& roles)
  {
    auto state = mod->get_state();
    state->setValue(Parameters::AutoDetectColumns, false);
    VariableList list;
    for (const auto& role : roles)
      list.push_back(makeVariable("", role));
    state->setValue(Parameters::ColumnRoles, list);
  }
};

TEST_F(ConvertMatrixToColorMapModuleTests, ThrowsForNullMatrixInput)
{
  auto mod = makeModule("ConvertMatrixToColorMap");
  MatrixHandle nullMatrix;
  stubPortNWithThisData(mod, 0, nullMatrix);
  EXPECT_THROW(mod->execute(), NullHandleOnPortException);
}

TEST_F(ConvertMatrixToColorMapModuleTests, ConvertsRgbMatrixInUnitRange)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(3, 3));
  (*m) << 0.0, 0.0, 0.0,
          0.5, 0.25, 0.75,
          1.0, 1.0, 1.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  stubPortNWithThisData(mod, 0, m);
  EXPECT_NO_THROW(mod->execute());

  auto output = getDataOnThisOutputPort(mod, 0);
  ASSERT_TRUE(output != nullptr);
  auto cmap = std::dynamic_pointer_cast<ColorMap>(output);
  ASSERT_TRUE(cmap != nullptr);

  const auto& colors = cmap->getColorData();
  ASSERT_EQ(3u, colors.size());
  EXPECT_DOUBLE_EQ(0.5, colors[1].r());
  EXPECT_DOUBLE_EQ(0.25, colors[1].g());
  EXPECT_DOUBLE_EQ(0.75, colors[1].b());
}

TEST_F(ConvertMatrixToColorMapModuleTests, ScalesMatrixWithByteRange)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 3));
  (*m) << 0.0, 128.0, 255.0,
          255.0, 0.0, 51.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  stubPortNWithThisData(mod, 0, m);
  EXPECT_NO_THROW(mod->execute());

  auto output = getDataOnThisOutputPort(mod, 0);
  ASSERT_TRUE(output != nullptr);
  auto cmap = std::dynamic_pointer_cast<ColorMap>(output);
  ASSERT_TRUE(cmap != nullptr);

  const auto& colors = cmap->getColorData();
  ASSERT_EQ(2u, colors.size());
  EXPECT_DOUBLE_EQ(0.0, colors[0].r());
  EXPECT_DOUBLE_EQ(128.0 / 255.0, colors[0].g());
  EXPECT_DOUBLE_EQ(1.0, colors[0].b());
}

TEST_F(ConvertMatrixToColorMapModuleTests, ConvertsRgbaMatrix)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 4));
  (*m) << 0.0, 0.0, 0.0, 0.0,
          1.0, 1.0, 1.0, 1.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  stubPortNWithThisData(mod, 0, m);
  EXPECT_NO_THROW(mod->execute());

  auto output = getDataOnThisOutputPort(mod, 0);
  ASSERT_TRUE(output != nullptr);
  auto cmap = std::dynamic_pointer_cast<ColorMap>(output);
  ASSERT_TRUE(cmap != nullptr);
  EXPECT_EQ(2u, cmap->getColorData().size());
}

TEST_F(ConvertMatrixToColorMapModuleTests, NoOutputForWrongColumnCount)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 2));
  (*m) << 0.0, 0.0,
          1.0, 1.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  stubPortNWithThisData(mod, 0, m);
  EXPECT_NO_THROW(mod->execute());

  auto output = getDataOnThisOutputPort(mod, 0);
  EXPECT_TRUE(output == nullptr);
}

// Issue #2578: a colormap matrix arrives as position + RGBA, which previously
// had to be trimmed to three columns by hand.
TEST_F(ConvertMatrixToColorMapModuleTests, AutoDetectsPositionRgbaMatrix)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(3, 5));
  (*m) << 0.0,  1.0, 0.0, 0.0, 1.0,
          0.25, 0.0, 1.0, 0.0, 1.0,
          1.0,  0.0, 0.0, 1.0, 1.0;

  auto cmap = runWith(makeModule("ConvertMatrixToColorMap"), m);
  ASSERT_TRUE(cmap != nullptr);

  // The stops are resampled onto an evenly spaced ramp, so the middle stop
  // lands a quarter of the way along instead of halfway.
  const auto& colors = cmap->getColorData();
  ASSERT_EQ(256u, colors.size());
  EXPECT_NEAR(1.0, colors.front().r(), 1e-6);
  EXPECT_NEAR(1.0, colors[colors.size() / 4].g(), 1e-2);
  EXPECT_NEAR(1.0, colors.back().b(), 1e-6);
}

TEST_F(ConvertMatrixToColorMapModuleTests, UsesExplicitColumnRoles)
{
  // Position last, alpha first, and a column the mapping ignores.
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 6));
  (*m) << 1.0, 99.0, 0.25, 0.5, 0.75, 0.0,
          1.0, 99.0, 0.30, 0.6, 0.90, 1.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  setColumnRoles(mod, { "Alpha", "Ignore", "Red", "Green", "Blue", "Position" });

  auto cmap = runWith(mod, m);
  ASSERT_TRUE(cmap != nullptr);
  const auto& colors = cmap->getColorData();
  ASSERT_EQ(256u, colors.size());
  EXPECT_NEAR(0.25, colors.front().r(), 1e-6);
  EXPECT_NEAR(0.90, colors.back().b(), 1e-6);
}

TEST_F(ConvertMatrixToColorMapModuleTests, NoOutputWhenColorRolesUnassigned)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 3));
  (*m) << 0.0, 0.0, 0.0,
          1.0, 1.0, 1.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  setColumnRoles(mod, { "Red", "Green", "Ignore" });

  stubPortNWithThisData(mod, 0, m);
  EXPECT_NO_THROW(mod->execute());
  EXPECT_TRUE(getDataOnThisOutputPort(mod, 0) == nullptr);
}

TEST_F(ConvertMatrixToColorMapModuleTests, NoOutputForDuplicateColumnRoles)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 4));
  (*m) << 0.0, 0.0, 0.0, 0.0,
          1.0, 1.0, 1.0, 1.0;

  auto mod = makeModule("ConvertMatrixToColorMap");
  setColumnRoles(mod, { "Red", "Green", "Blue", "Blue" });

  stubPortNWithThisData(mod, 0, m);
  EXPECT_NO_THROW(mod->execute());
  EXPECT_TRUE(getDataOnThisOutputPort(mod, 0) == nullptr);
}

// Alpha is 0-1 even when the color channels are 0-255, so the byte rescale
// must not be applied to it.
TEST_F(ConvertMatrixToColorMapModuleTests, ByteColorsDoNotRescaleUnitAlpha)
{
  DenseMatrixHandle m(makeShared<DenseMatrix>(2, 4));
  (*m) << 0.0,   0.0,   0.0, 1.0,
          255.0, 255.0, 0.0, 1.0;

  auto cmap = runWith(makeModule("ConvertMatrixToColorMap"), m);
  ASSERT_TRUE(cmap != nullptr);
  EXPECT_DOUBLE_EQ(1.0, cmap->getColorData()[1].r());
  EXPECT_NEAR(1.0, cmap->alpha(1.0), 1e-6);
}
