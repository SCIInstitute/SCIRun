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

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Testing/Utils/SCIRunFieldSamples.h>

#include <vector>

using namespace SCIRun;
using namespace SCIRun::TestUtils;

TEST(VFieldTest, EmptyFieldConstantBasis)
{
  FieldHandle field = EmptyTriSurfFieldConstantBasis(DOUBLE_E);
  ASSERT_TRUE(field.get() != nullptr);

  VField *vfield = field->vfield();

  ASSERT_EQ(vfield->num_values(), 0);

//  VMesh::dimension_type dim;
//  vfield->get_values_dimension(dim);
//
//std::cerr << dim[0] << ", " << dim[1] << ", " << dim[2] << std::endl;
//
//  ASSERT_EQ(dim[0], 0);
//  ASSERT_EQ(dim[1], 0);
//  ASSERT_EQ(dim[2], 0);
}

TEST(VFieldTest, EmptyFieldAddValuesConstantBasis)
{
  FieldHandle field = EmptyTriSurfFieldConstantBasis(DOUBLE_E);
  ASSERT_TRUE(field.get() != nullptr);

  VField *vfield = field->vfield();
  vfield->resize_values();

  std::vector<double> values(1);
  values[0] = 10.0;
  vfield->set_values(values);

  ASSERT_EQ(vfield->num_values(), 0);
}

TEST(VFieldTest, EmptyFieldLinearBasis)
{
  FieldHandle field = EmptyTriSurfFieldLinearBasis(DOUBLE_E);
  ASSERT_TRUE(field.get() != nullptr);

  VField *vfield = field->vfield();

  ASSERT_EQ(vfield->num_values(), 0);
}

TEST(VFieldTest, EmptyFieldAddValuesLinearBasis)
{
  FieldHandle field = EmptyTriSurfFieldLinearBasis(DOUBLE_E);
  ASSERT_TRUE(field.get() != nullptr);

  VField *vfield = field->vfield();
  vfield->resize_values();

  std::vector<double> values(1);
  values[0] = 10.0;
  vfield->set_values(values);

  ASSERT_EQ(vfield->num_values(), 0);
}

TEST(VFieldTest, TetVolMeshAddValuesConstantBasis)
{
  FieldHandle field = TetrahedronTetVolConstantBasis(DOUBLE_E);
  ASSERT_TRUE(field.get() != nullptr);

  VField *vfield = field->vfield();
  vfield->resize_values();

  std::vector<double> values(1);
  values[0] = 1.0;
  vfield->set_values(values);

  ASSERT_EQ(vfield->num_values(), 1);
}

TEST(VFieldTest, TetVolMeshAddValuesLinearBasis)
{
  FieldHandle field = TetrahedronTetVolLinearBasis(DOUBLE_E);
  ASSERT_TRUE(field.get() != nullptr);

  VField *vfield = field->vfield();
  vfield->resize_values();

  std::vector<double> values(4);
  values[0] = 1.0;
  values[1] = 2.0;
  values[2] = 3.0;
  values[3] = 4.0;
  vfield->set_values(values);

  ASSERT_EQ(vfield->num_values(), 4);
}

TEST(VFieldTest, TetVolMeshSetFieldValueTest1)
{
  FieldInformation fieldinfo("TetVolMesh", 0, "double");
  FieldHandle field = CreateField(fieldinfo);

  ASSERT_TRUE(field.get() != nullptr);

  VMesh *vmesh = field->vmesh();
  VField *vfield = field->vfield();
  VMesh::Node::array_type onodes(4);
  onodes[0]=0;
  onodes[1]=1;
  onodes[2]=2;
  onodes[3]=3;
  vmesh->add_elem(onodes);

  for(int i=0; i<4; i++)
  {
   vmesh->add_point(SCIRun::Core::Geometry::Point(1,2,3));
  }

  vfield->resize_values();
  vfield->set_value(1.234, 0);
  ASSERT_EQ(vfield->num_values(), 1);

}

TEST(VFieldTest, TetVolMeshSetFieldValueTest2)
{
  FieldInformation fieldinfo("TetVolMesh", 1, "double");
  FieldHandle field = CreateField(fieldinfo);

  ASSERT_TRUE(field.get() != nullptr);

  VMesh *vmesh = field->vmesh();
  VField *vfield = field->vfield();
  VMesh::Node::array_type onodes(4);
  onodes[0]=0;
  onodes[1]=1;
  onodes[2]=2;
  onodes[3]=3;
  vmesh->add_elem(onodes);

  for(int i=0; i<4; i++)
  {
   vmesh->add_point(SCIRun::Core::Geometry::Point(1,2,3));
   vfield->resize_values();
   vfield->set_value(1.234+i, i);
  }

  ASSERT_EQ(vfield->num_values(), 4);

  for(int i=0;i<4;i++)
  {
   double tmp;
   vfield->get_value(tmp,i);
   ASSERT_EQ(tmp, 1.234+i);
  }

}
