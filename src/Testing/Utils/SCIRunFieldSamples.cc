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


#include <Testing/Utils/SCIRunFieldSamples.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <boost/assign.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace boost::assign;

namespace SCIRun
{

namespace TestUtils
{

void tetCubeGeometry(FieldHandle field)
{
  auto vmesh = field->vmesh();
  VMesh::Node::array_type vdata;
  vdata.resize(4);

  vmesh->node_reserve(8);
  vmesh->elem_reserve(1);
  vmesh->add_point( Point(0.0, 0.0, 0.0) );
  vmesh->add_point( Point(1.0, 0.0, 0.0) );
  vmesh->add_point( Point(1.0, 1.0, 0.0) );
  vmesh->add_point( Point(0.0, 1.0, 0.0) );
  vmesh->add_point( Point(0.0, 0.0, 1.0) );
  vmesh->add_point( Point(1.0, 0.0, 1.0) );
  vmesh->add_point( Point(1.0, 1.0, 1.0) );
  vmesh->add_point( Point(0.0, 1.0, 1.0) );

  vdata[0]=5; vdata[1]=6;  vdata[2]=0; vdata[3]=4;
  vmesh->add_elem(vdata);
  vdata[0]=0; vdata[1]=7;  vdata[2]=2; vdata[3]=3;
  vmesh->add_elem(vdata);
  vdata[0]=2; vdata[1]=6;  vdata[2]=0; vdata[3]=1;
  vmesh->add_elem(vdata);
  vdata[0]=0; vdata[1]=6;  vdata[2]=5; vdata[3]=1;
  vmesh->add_elem(vdata);
  vdata[0]=0; vdata[1]=6;  vdata[2]=2; vdata[3]=7;
  vmesh->add_elem(vdata);
  vdata[0]=6; vdata[1]=7;  vdata[2]=0; vdata[3]=4;
  vmesh->add_elem(vdata);
}

void tetTetrahedronGeometry(FieldHandle field)
{
  auto vmesh = field->vmesh();
  VMesh::Node::array_type vdata;
  vdata.resize(4);

  vmesh->node_reserve(4);
  vmesh->elem_reserve(1);
  vmesh->add_point( Point(0, 0, 0) );
  vmesh->add_point( Point(0.5, 1.0, 0) );
  vmesh->add_point( Point(1.0, 0, 0) );
  vmesh->add_point( Point(0.5, 0.5, 1.0) );
  for (size_type i = 0; i < 4; ++i)
  {
    vdata[i] = i;
  }
  vmesh->add_elem(vdata);
}

void triTriangleGeometry(FieldHandle field)
{
  auto vmesh = field->vmesh();
  vmesh->add_point(Point(0.0, 0.0, 0.0));
  vmesh->add_point(Point(1.0, 0.0, 0.0));
  vmesh->add_point(Point(0.5, 1.0, 0.0));

  VMesh::Node::array_type vdata;
  vdata += 0, 1, 2;
  vmesh->add_elem(vdata);
}

void triTetrahedronGeometry(FieldHandle field)
{
  auto vmesh = field->vmesh();
  vmesh->add_point(Point(1.0, 0.0, -0.707));
  vmesh->add_point(Point(-1.0, 0.0, -0.707));
  vmesh->add_point(Point(0.0, 1.0, 0.707));
  vmesh->add_point(Point(0.0, -1.0, 0.707));

  VMesh::Node::array_type vdata1;
  vdata1 += 0, 1, 2;
  vmesh->add_elem(vdata1);
  VMesh::Node::array_type vdata2;
  vdata2 += 0, 1, 3;
  vmesh->add_elem(vdata2);
  VMesh::Node::array_type vdata3;
  vdata3 += 1, 2, 3;
  vmesh->add_elem(vdata2);
  VMesh::Node::array_type vdata4;
  vdata4 += 0, 2, 3;
  vmesh->add_elem(vdata4);
}

void triCubeGeometry(FieldHandle field)
{
  auto vmesh = field->vmesh();
  vmesh->add_point(Point(0.0, 1.0, 0.0));
  vmesh->add_point(Point(0.0, 0.0, 0.0));
  vmesh->add_point(Point(1.0, 1.0, 0.0));
  vmesh->add_point(Point(1.0, 0.0, 0.0));
  vmesh->add_point(Point(1.0, 0.0, -1.0));
  vmesh->add_point(Point(1.0, 1.0, -1.0));
  vmesh->add_point(Point(0.0, 1.0, -1.0));
  vmesh->add_point(Point(0.0, 0.0, -1.0));

  VMesh::Node::array_type vdata1;
  vdata1 += 0, 1, 7;
  vmesh->add_elem(vdata1);
  VMesh::Node::array_type vdata2;
  vdata2 += 0, 7, 6;
  vmesh->add_elem(vdata2);
  VMesh::Node::array_type vdata3;
  vdata3 += 1, 0, 2;
  vmesh->add_elem(vdata3);
  VMesh::Node::array_type vdata4;
  vdata4 += 1, 3, 2;
  vmesh->add_elem(vdata4);
  VMesh::Node::array_type vdata5;
  vdata5 += 2, 3, 4;
  vmesh->add_elem(vdata5);
  VMesh::Node::array_type vdata6;
  vdata6 += 2, 4, 5;
  vmesh->add_elem(vdata6);
  VMesh::Node::array_type vdata7;
  vdata7 += 4, 7, 1;
  vmesh->add_elem(vdata7);
  VMesh::Node::array_type vdata8;
  vdata8 += 4, 3, 1;
  vmesh->add_elem(vdata8);
  VMesh::Node::array_type vdata9;
  vdata9 += 5, 6, 0;
  vmesh->add_elem(vdata9);
  VMesh::Node::array_type vdata10;
  vdata10 += 5, 2, 0;
  vmesh->add_elem(vdata10);
  VMesh::Node::array_type vdata11;
  vdata11 += 7, 6, 5;
  vmesh->add_elem(vdata11);
  VMesh::Node::array_type vdata12;
  vdata12 += 7, 4, 5;
  vmesh->add_elem(vdata12);
}

FieldHandle CubeTetVolConstantBasis(data_info_type type)
{
  FieldInformation fi(TETVOLMESH_E, CONSTANTDATA_E, type);
  FieldHandle field = CreateField(fi);

  tetCubeGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle CubeTetVolLinearBasis(data_info_type type)
{
  FieldInformation fi(TETVOLMESH_E, LINEARDATA_E, type);
  FieldHandle field = CreateField(fi);

  tetCubeGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle TetrahedronTetVolConstantBasis(data_info_type type)
{
  FieldInformation fi(TETVOLMESH_E, CONSTANTDATA_E, type);
  FieldHandle field = CreateField(fi);

  tetTetrahedronGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle TetrahedronTetVolLinearBasis(data_info_type type)
{
  FieldInformation fi(TETVOLMESH_E, LINEARDATA_E, type);
  FieldHandle field = CreateField(fi);

  tetTetrahedronGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle TriangleTriSurfConstantBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, CONSTANTDATA_E, type);
  FieldHandle field = CreateField(fi);

  triTriangleGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle TriangleTriSurfLinearBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, LINEARDATA_E, type);
  FieldHandle field = CreateField(fi);

  triTriangleGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle TetrahedronTriSurfConstantBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, CONSTANTDATA_E, type);
  FieldHandle field = CreateField(fi);

  triTetrahedronGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle TetrahedronTriSurfLinearBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, LINEARDATA_E, type);
  FieldHandle field = CreateField(fi);

  triTetrahedronGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle CubeTriSurfConstantBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, CONSTANTDATA_E, type);
  FieldHandle field = CreateField(fi);

  triCubeGeometry(field);
  field->vfield()->resize_values();

  return field;
}

FieldHandle CubeTriSurfLinearBasis(data_info_type type)
{
  FieldInformation fi(TRISURFMESH_E, LINEARDATA_E, type);
  FieldHandle field = CreateField(fi);

  triCubeGeometry(field);
  field->vfield()->resize_values();

  return field;
}

}}

FieldHandle SCIRun::TestUtils::CreateEmptyLatVol()
{
  size_type sizex = 3, sizey = 4, sizez = 5;
  return CreateEmptyLatVol(sizex, sizey, sizez);
}

FieldHandle SCIRun::TestUtils::CreateEmptyLatVol(size_type sizex, size_type sizey, size_type sizez, data_info_type type,
  const Core::Geometry::Point& minb, const Core::Geometry::Point& maxb)
{
  FieldInformation lfi(LATVOLMESH_E, LINEARDATA_E, type);

  MeshHandle mesh = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
  FieldHandle ofh = CreateField(lfi, mesh);
  ofh->vfield()->clear_all_values();
  return ofh;
}
