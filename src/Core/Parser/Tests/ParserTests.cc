/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
#include <gmock/gmock.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Parser/ArrayMathEngine.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using ::testing::NotNull;

namespace
{
  FieldHandle CreateEmptyLatVol(size_type sizex = 3, size_type sizey = 4, size_type sizez = 5)
  {
    FieldInformation lfi("LatVolMesh", 1, "double");
    Point minb(-1.0, -1.0, -1.0);
    Point maxb(1.0, 1.0, 1.0);
    MeshHandle mesh = CreateMesh(lfi,sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    ofh->vfield()->clear_all_values();
    return ofh;
  }
}

TEST(BasicParserTests, CanCreateEngine)
{
  NewArrayMathEngine engine;
  EXPECT_FALSE(engine.run());
}

TEST(BasicParserTests, CreateFieldDataDuplicate)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);
  
  NewArrayMathEngine engine;
  
  std::string format = "double";
  std::string basis = "Linear";
  int basis_order = 1;
  
  ASSERT_TRUE(engine.add_input_fielddata_location("POS",field,basis_order));
  
  ASSERT_TRUE(engine.add_input_fielddata_coordinates("X","Y","Z",field,basis_order));
  
  ASSERT_TRUE(engine.add_input_fielddata_element("ELEMENT",field,basis_order));
  
  ASSERT_TRUE(engine.add_output_fielddata("RESULT",field,basis_order,format));
  
  ASSERT_TRUE(engine.add_index("INDEX"));
  ASSERT_TRUE(engine.add_size("SIZE"));
  
  std::string function = "RESULT = X + Y + Z;";
  ASSERT_TRUE(engine.add_expressions(function));
  
  ASSERT_TRUE(engine.run());
 
  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);
  EXPECT_EQ(-3, min);
  EXPECT_EQ(3, max);
}