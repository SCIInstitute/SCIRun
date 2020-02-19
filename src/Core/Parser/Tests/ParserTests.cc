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
#include <gmock/gmock.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Parser/ArrayMathEngine.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using ::testing::NotNull;

class BasicParserTests : public ::testing::Test
{
public:

protected:
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
  void setupEngine(NewArrayMathEngine& engine, FieldHandle field)
  {
    std::string format = "double";
    std::string basis = "Linear";
    int basis_order = 1;

    ASSERT_TRUE(engine.add_input_fielddata_location("POS",field,basis_order));

    ASSERT_TRUE(engine.add_input_fielddata_coordinates("X","Y","Z",field,basis_order));

    ASSERT_TRUE(engine.add_input_fielddata_element("ELEMENT",field,basis_order));

    ASSERT_TRUE(engine.add_output_fielddata("RESULT",field,basis_order,format));

    ASSERT_TRUE(engine.add_index("INDEX"));
    ASSERT_TRUE(engine.add_size("SIZE"));
  }
  void testMinMaxOfFunction(const std::string& function, double expectedMin, double expectedMax)
  {
    FieldHandle field(CreateEmptyLatVol(3,3,3));

    NewArrayMathEngine engine;
    setupEngine(engine, field);

    const std::string resultStr = "RESULT = ";
    ASSERT_TRUE(engine.add_expressions(resultStr + function));
    ASSERT_TRUE(engine.run());

    FieldHandle ofield;
    engine.get_field("RESULT",ofield);

    ASSERT_THAT(ofield, NotNull());
    auto ovfield = ofield->vfield();
    double min, max;
    ovfield->minmax(min,max);
    EXPECT_EQ(expectedMin, min);
    EXPECT_EQ(expectedMax, max);
  }
  void testBadParseFunction(const std::string& function)
  {
    FieldHandle field(CreateEmptyLatVol(3,3,3));

    NewArrayMathEngine engine;
    setupEngine(engine, field);

    const std::string resultStr = "RESULT = ";
    ASSERT_TRUE(engine.add_expressions(resultStr + function));
    ASSERT_FALSE(engine.run());
  }
};

TEST_F(BasicParserTests, CanCreateEngine)
{
  NewArrayMathEngine engine;
  EXPECT_FALSE(engine.run());
}

TEST_F(BasicParserTests, CreateFieldDataDuplicate)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  const std::string resultStr = "RESULT = ";
  std::string function = resultStr + "X + Y + Z;";
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

TEST_F(BasicParserTests, CreateField_Parenthesis_Unbalanced)
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
  std::string function = "RESULT = ());";
  ASSERT_TRUE(engine.add_expressions(function));
  ASSERT_FALSE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
}

TEST_F(BasicParserTests, CreateField_Brackets_Unbalanced)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = [;";
  ASSERT_TRUE(engine.add_expressions(function));
  ASSERT_FALSE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
}

TEST_F(BasicParserTests, CreateFieldData_Arithmetic)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = 1/Y + 2*X - Z;";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(-6, min,1e-1);
  EXPECT_NEAR(6, max,1e-1);
}

TEST_F(BasicParserTests, CreateFieldData_add)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = add(X,100);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(99, min);
  EXPECT_EQ(101, max);
}

TEST_F(BasicParserTests, CreateFieldData_sub)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =sub(X,Y);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(-2, min);
  EXPECT_EQ( 2, max);
}

TEST_F(BasicParserTests, CreateFieldData_mult)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =mult(X,Y);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(-1, min);
  EXPECT_EQ( 1, max);
}

TEST_F(BasicParserTests, CreateFieldData_div)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =div(X,100);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(-0.01, min);
  EXPECT_EQ( 0.01, max);
}

TEST_F(BasicParserTests, CreateFieldData_exp)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =exp(3);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(20.08, min, 1e-1);
  EXPECT_NEAR(20.08, max, 1e-1);
}

TEST_F(BasicParserTests, CreateFieldData_log)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =log(10);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(2.30259, min, 1e-3);
  EXPECT_NEAR(2.30259, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_ln)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =ln(100);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(4.60517, min, 1e-3);
  EXPECT_NEAR(4.60517, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_log2)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =log2(16);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(4, min);
  EXPECT_EQ(4, max);
}

TEST_F(BasicParserTests, CreateFieldData_log10)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = log10(50*2);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(2, min, 1e-13);
  EXPECT_NEAR(2, max, 1e-13);

  //EXPECT_EQ(2.0, min);
  //EXPECT_EQ(2.0, max);
}

TEST_F(BasicParserTests, CreateFieldData_sin)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =sin(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(0, min, 1e-8);
  EXPECT_NEAR(0, max, 1e-8);
}

TEST_F(BasicParserTests, CreateFieldData_asin)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =asin(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  //check for NaN, if should be true
  //this TEST should checks NaN or specifically: -1.#IND
  EXPECT_TRUE(min != min && max != max);
}

TEST_F(BasicParserTests, CreateFieldData_sinh)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = sinh(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(267.745, min, 1e-3);
  EXPECT_NEAR(267.745, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_asinh)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = asinh(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(2.5373, min, 1e-4);
  EXPECT_NEAR(2.5373, max, 1e-4);
}

TEST_F(BasicParserTests, CreateFieldData_cos)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =cos(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, min);
  EXPECT_EQ(1, max);

  //EXPECT_NEAR(0, min, 1e-8);
  //EXPECT_NEAR(0, max, 1e-8);
}

TEST_F(BasicParserTests, CreateFieldData_acos)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = acos(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  //another check for nan/imaginary
  EXPECT_TRUE(min != min && max != max);
}

TEST_F(BasicParserTests, CreateFieldData_acosh)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = acosh(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  //EXPECT_EQ(2.52463, min);
  //EXPECT_EQ(2.52463, max);

  EXPECT_NEAR(2.52463, min, 1e-5);
  EXPECT_NEAR(2.52463, max, 1e-5);
}

TEST_F(BasicParserTests, CreateFieldData_tan)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =tan(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  //EXPECT_EQ(0, min);
  //EXPECT_EQ(0, max);

  EXPECT_NEAR(0, min, 1e-8);
  EXPECT_NEAR(0, max, 1e-8);
}

TEST_F(BasicParserTests, CreateFieldData_atan)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =atan(2*pi);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(1.41297, min, 1e-5);
  EXPECT_NEAR(1.41297, max, 1e-5);
}

TEST_F(BasicParserTests, CreateFieldData_pow)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = pow(2,2.5);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(5.65685, min, 1e-5);
  EXPECT_NEAR(5.65685, max, 1e-5);
}

TEST_F(BasicParserTests, CreateFieldData_ceil)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = ceil(2.3) + ceil(2.9);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(6, min);
  EXPECT_EQ(6, max);
}

TEST_F(BasicParserTests, CreateFieldData_floor)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = floor(2.3) + floor(2.9);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(4, min, 1e-3);
  EXPECT_NEAR(4, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_round)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = round(2.3) + round(2.9);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(5, min, 1e-3);
  EXPECT_NEAR(5, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_boolean)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =boolean(0) + boolean(1);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(1, min, 1e-3);
  EXPECT_NEAR(1, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_norm)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = norm(2.9*X);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(0, min, 1e-3);
  EXPECT_NEAR(2.9, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_isnan)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = isnan(div(1,0));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);
}

TEST_F(BasicParserTests, CreateFieldData_isfinite)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = isfinite(div(1,0));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);
}

TEST_F(BasicParserTests, CreateFieldData_isinfinite)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = isinfinite(div(1,0));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, min);
  EXPECT_EQ(1, max);
}

TEST_F(BasicParserTests, CreateFieldData_select)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = select(X>2,1,10);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(10, min);
  EXPECT_EQ(10, max);
}

TEST_F(BasicParserTests, CreateFieldData_sign)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = sign(X);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(-1, min);
  EXPECT_EQ(1, max);
}

TEST_F(BasicParserTests, CreateFieldData_sqrt)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = sqrt(2.9-X);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(1.3784, min, 1e-3);
  EXPECT_NEAR(1.9748, max, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_not)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = not(1);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(0, min, 1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_inv)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = inv(1/10);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(10, min);
  EXPECT_EQ(10, max);
}

TEST_F(BasicParserTests, CreateFieldData_abs)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = abs(-9*X+1);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, min);
  EXPECT_EQ(10, max);
}

TEST_F(BasicParserTests, CreateFieldData_and)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT =  and(1,0);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(0, min);
}

TEST_F(BasicParserTests, CreateFieldData_or)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = or(1,0);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, min);
}

TEST_F(BasicParserTests, CreateFieldData_eq)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = eq(1, X);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, max);
}

TEST_F(BasicParserTests, CreateFieldData_neq)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = neq(5,Y);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, min);
  EXPECT_EQ(1, max);
}

TEST_F(BasicParserTests, CreateFieldData_dot)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = dot(vector(1,1,1),vector(2,2,2));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(6, min);
  EXPECT_EQ(6, max);
}

TEST_F(BasicParserTests, CreateFieldData_cross)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = cross(vector(1,2,3),vector(4,5,6));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(7.34847, min,1e-3);
  EXPECT_NEAR(7.34847, max,1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_normalize)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = normalize(vector(1,2,3));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(1, min,1e-3);
  EXPECT_NEAR(1, max,1e-3);
}

TEST_F(BasicParserTests, CreateFieldData_find_normal1)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = find_normal1(vector(1,2,3));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_EQ(1, min);
  EXPECT_EQ(1, max);
}

TEST_F(BasicParserTests, CreateFieldData_find_normal2)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = find_normal2(vector(1,2,3));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(19.4422, min,1e-4);
  EXPECT_NEAR(19.4422, max,1e-4);
}


//Run these tests when the functions below are implemented
/*
TEST_F(BasicParserTests, CreateFieldData_quality)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "A = tensor(1,2,3,4,5,6); RESULT = quality(A);";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(14.7778, min,1e-4);
  EXPECT_NEAR(14.7778, max,1e-4);
}

TEST_F(BasicParserTests, CreateFieldData_trace)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = trace(tensor(1,2,3,4,5,6));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(11, min,1e-4);
  EXPECT_NEAR(11, max,1e-4);
}

TEST_F(BasicParserTests, CreateFieldData_det)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = det(tensor(1,2,3,4,5,6));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(-1, min,1e-4);
  EXPECT_NEAR(-1, max,1e-4);
}

TEST_F(BasicParserTests, CreateFieldData_frobenius)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = frobenius(tensor(1,2,3,4,5,6));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(11.3578, min,1e-4);
  EXPECT_NEAR(11.3578, max,1e-4);
}

TEST_F(BasicParserTests, CreateFieldData_frobenius2)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = frobenius2(tensor(1,2,3,4,5,6));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(129, min,1e-4);
  EXPECT_NEAR(129, max,1e-4);
}

TEST_F(BasicParserTests, CreateFieldData_fracanisotropy)
{
  FieldHandle field(CreateEmptyLatVol());
  auto ivfield = field->vfield();
  double min, max;
  ivfield->minmax(min,max);
  EXPECT_EQ(0, min);
  EXPECT_EQ(0, max);

  NewArrayMathEngine engine;
  setupEngine(engine, field);

  std::string function = "RESULT = fracanisotropy(tensor(1,2,3,4,5,6));";
  ASSERT_TRUE(engine.add_expressions(function));

  ASSERT_TRUE(engine.run());

  FieldHandle ofield;
  engine.get_field("RESULT",ofield);

  ASSERT_THAT(ofield, NotNull());
  auto ovfield = ofield->vfield();
  ovfield->minmax(min,max);

  EXPECT_NEAR(1.0154, min,1e-4);
  EXPECT_NEAR(1.0154, max,1e-4);
}

*/

TEST(FieldHashTests, TestShiftingZero)
{
  // copied from TetVolMesh.h, failing compilation on GCC 6.2.
  static const int sz_int = sizeof(int) * 8; // in bits

  /// These are for our own use (making the hash function).
  static const int sz_third_int = (int)(sz_int / 3);
  static const int up_mask = -1048576; // (~((int)0) << sz_third_int << sz_third_int);
  static const int mid_mask = 1047552; // up_mask ^ (~((int)0) << sz_third_int);
  static const int low_mask = ~(up_mask | mid_mask);

  std::cout << "sz_int: " << sz_int << std::endl;
  std::cout << "sz_third_int: " << sz_third_int << std::endl;
  std::cout << "up_mask: " << up_mask << std::endl;
  std::cout << "mid_mask: " << mid_mask << std::endl;
  std::cout << "low_mask: " << low_mask << std::endl;

  auto badGcc = ~((int)0);
  std::cout << "badGcc: " << badGcc << std::endl;

  auto redo_up_mask = -(1 << 10 << 10);
  std::cout << "redo_up_mask: " << redo_up_mask << std::endl;

  auto redo_mid_mask = redo_up_mask ^ (-(1 << sz_third_int));
  auto redo_low_mask = ~(redo_up_mask | redo_mid_mask);

  ASSERT_EQ(redo_up_mask, up_mask);
  ASSERT_EQ(redo_mid_mask, mid_mask);
  ASSERT_EQ(redo_low_mask, low_mask);
}
