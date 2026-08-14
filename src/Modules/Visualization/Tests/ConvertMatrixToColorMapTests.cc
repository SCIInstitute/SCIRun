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
#include <Testing/ModuleTestBase/ModuleTestBase.h>

using namespace SCIRun;
using namespace SCIRun::Testing;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Dataflow::Networks;

class ConvertMatrixToColorMapModuleTests : public ModuleTest
{
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
