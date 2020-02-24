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


#include <Core/Algorithms/Legacy/FiniteElements/Mapping/BuildFEGridMapping.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(FiniteElements, build_potential_gridtogeom);
ALGORITHM_PARAMETER_DEF(FiniteElements, build_potential_geomtogrid);
ALGORITHM_PARAMETER_DEF(FiniteElements, build_current_gridtogeom);
ALGORITHM_PARAMETER_DEF(FiniteElements, build_current_geomtogrid);

// Set default values
BuildFEGridMappingAlgo::BuildFEGridMappingAlgo()
{
  addParameter(Parameters::build_potential_gridtogeom, true);
  addParameter(Parameters::build_potential_geomtogrid, true);
  addParameter(Parameters::build_current_gridtogeom, true);
  addParameter(Parameters::build_current_geomtogrid, true);
}

bool
BuildFEGridMappingAlgo::run(
  MatrixHandle nodeLink,
  SparseRowMatrixHandle& PotentialGeomToGrid,
  SparseRowMatrixHandle& PotentialGridToGeom,
  SparseRowMatrixHandle& CurrentGeomToGrid,
  SparseRowMatrixHandle& CurrentGridToGeom) const
{
  ScopedAlgorithmStatusReporter asr(this, "BuildFEGridMapping");

  if (!nodeLink)
  {
    error("No matrix on input");
    return (false);
  }

  if (!matrixIs::sparse(nodeLink))
  {
    error("NodeLink Matrix is not sparse");
    return (false);
  }

  if (nodeLink->nrows() != nodeLink->ncols())
  {
    error("NodeLink Matrix needs to be square");
    return (false);
  }

  SparseRowMatrixHandle spr = castMatrix::toSparse(nodeLink);
  size_type m = spr->ncols();
  const index_type *rows = spr->get_rows();
  const index_type *cols = spr->get_cols();

  LegacySparseDataContainer<double> matData(m+1, m);
  auto rr = matData.rows();
  auto cc = matData.columns();
  auto vv = matData.data();

  for (index_type r=0; r<m; r++) rr[r] = r;

  for (index_type r=0; r<m; r++)
  {
    for (index_type c=rows[r]; c<rows[r+1]; c++)
    {
      if (cols[c] > r)
      {
        rr[cols[c]] = r;
      }
    }
  }

  for (index_type r=0; r<m; r++)
  {
    if (rr[r] != r)
    {
      index_type q = r;
      while (rr[q] != q) q = rr[q];
      rr[r] = q;
    }
  }

  index_type k=0;
  for (index_type r=0; r<m; r++)
  {
    if (rr[r] == r)
    {
      cc[r] = k; k++;
    }
  }

  for (index_type r=0; r<m; r++)
  {
    rr[r] = cc[rr[r]];
  }


  for (index_type r = 0; r < m; r++)
  {
    cc[r] = rr[r];
    rr[r] = r;
    vv[r] = 1.0;
  }
  rr[m] = m; // An extra entry goes on the end of rr.

  SparseRowMatrixHandle mat(new SparseRowMatrix(m, k, rr.get(), cc.get(), vv.get(), m));

  if (!mat)
  {
    error("Could not build geometry to gridutational mesh mapping matrix");
    return (false);
  }

  const bool pot_geomtogrid = get(Parameters::build_potential_geomtogrid).toBool();
  const bool pot_gridtogeom = get(Parameters::build_potential_gridtogeom).toBool();
  const bool cur_geomtogrid = get(Parameters::build_current_geomtogrid).toBool();
  const bool cur_gridtogeom = get(Parameters::build_current_gridtogeom).toBool();

  if (pot_gridtogeom)
  {
    PotentialGridToGeom = mat;
    if (!PotentialGridToGeom)
    {
      error("Could not build PotentialGridToGeom mapping matrix");
      return (false);
    }
  }

  if (cur_geomtogrid)
  {
    CurrentGeomToGrid.reset(new SparseRowMatrix(mat->transpose()));
    if (!CurrentGeomToGrid)
    {
      error("Could not build CurrentGeomToGrid mapping matrix");
      return (false);
    }
  }

  if (cur_gridtogeom || pot_geomtogrid)
  {
    SparseRowMatrixHandle mat2(new SparseRowMatrix(mat->transpose()));

    const index_type* rr = mat2->get_rows();
    double* vv = mat2->valuePtr();

    for (index_type r=0; r<k; r++)
    {
      size_type s = rr[r+1]-rr[r];
      double scalar = 1.0/static_cast<double>(s);
      for (index_type p=rr[r]; p<rr[r+1]; p++)
        vv[p] = scalar;
    }

    if (cur_gridtogeom)
    {
      CurrentGridToGeom.reset(new SparseRowMatrix(mat2->transpose()));
      if (!CurrentGridToGeom)
      {
        error("Could not build CurrentGridToGeom mapping matrix");
        return (false);
      }
    }

    if (pot_geomtogrid)
    {
      PotentialGeomToGrid = mat2;
      if (!PotentialGeomToGrid)
      {
        error("Could not build PotentialGeomToGrid mapping matrix");
        return (false);
      }
    }
  }
  return (true);
}

bool
BuildFEGridMappingAlgo::
run(FieldHandle domainField,
    MatrixHandle nodeLink,
    SparseRowMatrixHandle& PotentialGeomToGrid,
    SparseRowMatrixHandle& PotentialGridToGeom,
    SparseRowMatrixHandle& CurrentGeomToGrid,
    SparseRowMatrixHandle& CurrentGridToGeom) const
{
  ScopedAlgorithmStatusReporter asr(this, "BuildFEGridMapping");

  if (!domainField)
  {
    error("CreateLinkBetweenMeshAndGridGridByDomain: No input field");
    return (false);
  }

  FieldInformation fi(domainField);

  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("The field needs to have data assigned to the elements");
    return (false);
  }

  if (!(fi.is_volume()||fi.is_surface()||fi.is_curve()))
  {
    error("This function is only defined for curve, surface and volume data");
    return (false);
  }

  if (!nodeLink)
  {
    error("No matrix on input");
    return (false);
  }

  if (!matrixIs::sparse(nodeLink))
  {
    error("NodeLink Matrix is not sparse");
    return (false);
  }

  if (nodeLink->nrows() != nodeLink->ncols())
  {
    error("NodeLink Matrix needs to be square");
    return (false);
  }

  VField *ifield = domainField->vfield();
  if (!ifield)
  {
    error("FieldHandle is empty");
    return (false);
  }

  VMesh *imesh = ifield->vmesh();
  if (!imesh)
  {
    error("Field does not have a mesh");
    return (false);
  }

  VMesh::Node::size_type numnodes;
  imesh->size(numnodes);

  if (nodeLink->nrows() != numnodes)
  {
    error("NodeLink Matrix has improper dimensions");
    return (false);
  }

  SparseRowMatrixHandle spr = castMatrix::toSparse(nodeLink);
  size_type m = spr->ncols();
  const index_type *rows = spr->get_rows();
  const index_type *cols = spr->get_cols();

  LegacySparseDataContainer<double> sparseData(m+1, m);

  auto rr = sparseData.rows();
  auto cc = sparseData.columns();
  auto vv = sparseData.data();

  imesh->synchronize(Mesh::NODE_NEIGHBORS_E);

  for (index_type r=0; r<m; r++) rr[r] = r;

  for (index_type r=0; r<m; r++)
  {
    for (index_type c=rows[r]; c<rows[r+1]; c++)
    {
      if (cols[c] > r)
      {
        VMesh::Elem::array_type elems1;
        VMesh::Elem::array_type elems2;
        imesh->get_elems(elems1,VMesh::Node::index_type(r));
        imesh->get_elems(elems2,VMesh::Node::index_type(cols[c]));
        int val1, val2;
        ifield->get_value(val1,elems1[0]);
        ifield->get_value(val2,elems2[0]);
        if (val1 == val2) rr[cols[c]] = r;
      }
    }
  }

  for (index_type r=0; r<m; r++)
  {
    if (rr[r] != r)
    {
      index_type q = r;
      while (rr[q] != q) q = rr[q];
      rr[r] = q;
    }
  }

  index_type k=0;
  for (index_type r=0; r<m; r++)
  {
    if (rr[r] == r)
    {
      cc[r] = k; k++;
    }
  }

  for (index_type r=0; r<m; r++)
  {
    rr[r] = cc[rr[r]];
  }

  for (index_type r = 0; r < m; r++)
  {
    cc[r] = rr[r];
    rr[r] = r;
    vv[r] = 1.0;
  }
  rr[m] = m; // An extra entry goes on the end of rr.

  SparseRowMatrixHandle mat(new SparseRowMatrix(m, k, rr.get(), cc.get(), vv.get(), m));

  if (!mat)
  {
    error("Could not build geometry to gridutational mesh mapping matrix");
    return (false);
  }

  const bool pot_geomtogrid = get(Parameters::build_potential_geomtogrid).toBool();
  const bool pot_gridtogeom = get(Parameters::build_potential_gridtogeom).toBool();
  const bool cur_geomtogrid = get(Parameters::build_current_geomtogrid).toBool();
  const bool cur_gridtogeom = get(Parameters::build_current_gridtogeom).toBool();

  if (pot_gridtogeom)
  {
    PotentialGridToGeom = mat;
    if (!PotentialGridToGeom)
    {
      error("Could not build mapping matrix");
      return (false);
    }
  }

  if (cur_geomtogrid)
  {
    CurrentGeomToGrid.reset(new SparseRowMatrix(mat->transpose()));
    if (!CurrentGeomToGrid)
    {
      error("Could not build mapping matrix");
      return (false);
    }
  }

  if (cur_gridtogeom || pot_geomtogrid)
  {
    SparseRowMatrixHandle mat2(new SparseRowMatrix(mat->transpose()));

    index_type* rr = mat2->get_rows();
    double* const vv = mat2->valuePtr();

    for (index_type r=0; r<k; r++)
    {
      size_type s = rr[k+1]-rr[k];
      double scalar = 1.0/s;
      for (index_type p=rr[k]; p<rr[k+1]; p++)
        vv[k] = scalar;
    }

    if (cur_gridtogeom)
    {
      CurrentGridToGeom.reset(new SparseRowMatrix(mat2->transpose()));
      if (!CurrentGridToGeom)
      {
        error("Could not build mapping matrix");
        return (false);
      }
    }

    if (pot_geomtogrid)
    {
      PotentialGeomToGrid = mat2;
      if (!PotentialGeomToGrid)
      {
        error("Could not build mapping matrix");
        return (false);
      }
    }
  }

  return (true);
}

AlgorithmOutput BuildFEGridMappingAlgo::run(const AlgorithmInput &) const
{
  throw "not implemented";
}
