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
#include <vector>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace
{
  // Node data set to the node index (0, 1, 2, ...) so mapped output values
  // can be checked against known source indices.
  FieldHandle makeIndexedScalarLatVol(int nx, int ny, int nz)
  {
    FieldInformation lfi("LatVolMesh", 1, "double");
    Point minb(0.0, 0.0, 0.0), maxb(1.0, 1.0, 1.0);
    MeshHandle mesh = CreateMesh(lfi, nx, ny, nz, minb, maxb);
    FieldHandle field = CreateField(lfi, mesh);

    VField* vfield = field->vfield();
    vfield->resize_values();
    for (VMesh::Node::index_type idx = 0; idx < field->vmesh()->num_nodes(); idx++)
      vfield->set_value(static_cast<double>(static_cast<index_type>(idx)), idx);
    return field;
  }

  SparseRowMatrixHandle makeIdentityMapping(int n)
  {
    std::vector<index_type> rows(n + 1);
    std::vector<index_type> cols(n);
    std::vector<double> vals(n, 1.0);
    for (int i = 0; i < n; i++)
    {
      rows[i] = i;
      cols[i] = i;
    }
    rows[n] = n;
    return SparseRowMatrixHandle(new SparseRowMatrix(n, n, rows.data(), cols.data(), vals.data(), n));
  }

  // Row i averages source columns i and (i+1)%n with equal weight, so the
  // mapping is not simply a permutation or the identity.
  SparseRowMatrixHandle makeCircularAverageMapping(int n)
  {
    std::vector<index_type> rows(n + 1);
    std::vector<index_type> cols(2 * n);
    std::vector<double> vals(2 * n, 0.5);
    for (int i = 0; i < n; i++)
    {
      rows[i] = 2 * i;
      cols[2 * i] = i;
      cols[2 * i + 1] = (i + 1) % n;
    }
    rows[n] = 2 * n;
    return SparseRowMatrixHandle(new SparseRowMatrix(n, n, rows.data(), cols.data(), vals.data(), 2 * n));
  }
}

TEST(ApplyMappingMatrixAlgoTest, IdentityMappingReturnsSourceValuesUnchanged)
{
  ApplyMappingMatrixAlgo algo;
  FieldHandle source = makeIndexedScalarLatVol(2, 2, 2); // 8 nodes, 1 elem
  FieldHandle destination = source;
  MatrixHandle mapping = makeIdentityMapping(8);

  FieldHandle output = algo.run(source, destination, mapping);
  ASSERT_TRUE(output != nullptr);

  VField* ofield = output->vfield();
  ASSERT_EQ(8, ofield->num_values());
  for (VMesh::Node::index_type i = 0; i < 8; i++)
  {
    double val = 0.0;
    ofield->get_value(val, i);
    EXPECT_NEAR(static_cast<double>(static_cast<index_type>(i)), val, 1e-12);
  }
}

TEST(ApplyMappingMatrixAlgoTest, WeightedMappingAveragesNeighboringSourceValues)
{
  ApplyMappingMatrixAlgo algo;
  FieldHandle source = makeIndexedScalarLatVol(2, 2, 2); // values 0..7
  FieldHandle destination = source;
  MatrixHandle mapping = makeCircularAverageMapping(8);

  FieldHandle output = algo.run(source, destination, mapping);
  ASSERT_TRUE(output != nullptr);

  VField* ofield = output->vfield();
  for (VMesh::Node::index_type i = 0; i < 8; i++)
  {
    double val = 0.0;
    ofield->get_value(val, i);
    index_type ii = static_cast<index_type>(i);
    double expected = 0.5 * (static_cast<double>(ii) + static_cast<double>((ii + 1) % 8));
    EXPECT_NEAR(expected, val, 1e-12);
  }
}

TEST(ApplyMappingMatrixAlgoTest, MapsOntoElementDataWhenRowsMatchElementCount)
{
  ApplyMappingMatrixAlgo algo;
  FieldHandle source = makeIndexedScalarLatVol(3, 3, 3); // 27 nodes

  FieldInformation efi("LatVolMesh", 0, "double");
  Point minb(0.0, 0.0, 0.0), maxb(1.0, 1.0, 1.0);
  MeshHandle emesh = CreateMesh(efi, 3, 3, 3, minb, maxb); // same geometry, 8 elems
  FieldHandle destination = CreateField(efi, emesh);

  // Row i (one of 8 elems) picks out source node 2*i.
  std::vector<index_type> rows(9);
  std::vector<index_type> cols(8);
  std::vector<double> vals(8, 1.0);
  for (int i = 0; i < 8; i++)
  {
    rows[i] = i;
    cols[i] = 2 * i;
  }
  rows[8] = 8;
  MatrixHandle mapping(new SparseRowMatrix(8, 27, rows.data(), cols.data(), vals.data(), 8));

  FieldHandle output = algo.run(source, destination, mapping);
  ASSERT_TRUE(output != nullptr);

  VField* ofield = output->vfield();
  ASSERT_EQ(8, ofield->num_values());
  for (VMesh::Elem::index_type i = 0; i < 8; i++)
  {
    double val = 0.0;
    ofield->get_value(val, i);
    EXPECT_NEAR(static_cast<double>(2 * static_cast<index_type>(i)), val, 1e-12);
  }
}

TEST(ApplyMappingMatrixAlgoTest, DenseMappingMatrixThrows)
{
  ApplyMappingMatrixAlgo algo;
  FieldHandle source = makeIndexedScalarLatVol(2, 2, 2);
  FieldHandle destination = source;
  MatrixHandle mapping(new DenseMatrix(8, 8));

  EXPECT_THROW(algo.run(source, destination, mapping), SCIRun::Core::Algorithms::AlgorithmInputException);
}

TEST(ApplyMappingMatrixAlgoTest, RowCountMismatchThrows)
{
  ApplyMappingMatrixAlgo algo;
  FieldHandle source = makeIndexedScalarLatVol(2, 2, 2); // 8 nodes, 1 elem
  FieldHandle destination = source;
  // 4 rows matches neither the 8 nodes nor the 1 elem of the destination.
  MatrixHandle mapping = makeIdentityMapping(4);

  EXPECT_THROW(algo.run(source, destination, mapping), SCIRun::Core::Algorithms::AlgorithmInputException);
}

TEST(ApplyMappingMatrixAlgoTest, ColumnCountMismatchThrows)
{
  ApplyMappingMatrixAlgo algo;
  FieldHandle source = makeIndexedScalarLatVol(2, 2, 2); // 8 node values
  FieldHandle destination = source; // 8 nodes

  std::vector<index_type> rows(9);
  std::vector<index_type> cols(8, 0);
  std::vector<double> vals(8, 1.0);
  for (int i = 0; i < 8; i++) rows[i] = i;
  rows[8] = 8;
  // 8 rows matches the destination's 8 nodes, but only 5 columns, which
  // does not match the source field's 8 values.
  MatrixHandle mapping(new SparseRowMatrix(8, 5, rows.data(), cols.data(), vals.data(), 8));

  EXPECT_THROW(algo.run(source, destination, mapping), SCIRun::Core::Algorithms::AlgorithmInputException);
}
