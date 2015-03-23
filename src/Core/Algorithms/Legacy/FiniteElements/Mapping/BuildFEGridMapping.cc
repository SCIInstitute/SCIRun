/*
  For more information, please see: http://software.sci.utah.edu
  
  The MIT License

  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/FiniteElements/Mapping/BuildFEGridMapping.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
BuildFEGridMappingAlgo::
run(MatrixHandle& NodeLink, 
    MatrixHandle& PotentialGeomToGrid, 
    MatrixHandle& PotentialGridToGeom,
    MatrixHandle& CurrentGeomToGrid, 
    MatrixHandle& CurrentGridToGeom)
{
  algo_start("BuildFEGridMapping");
  bool pot_geomtogrid = get_bool("build_potential_geomtogrid");
  bool pot_gridtogeom = get_bool("build_potential_gridtogeom");
  bool cur_geomtogrid = get_bool("build_current_geomtogrid");
  bool cur_gridtogeom = get_bool("build_current_gridtogeom");
  
  if (NodeLink.get_rep() == 0)
  {
    error("No matrix on input");
    algo_end(); return (false);
  }

  if (!(matrix_is::sparse(NodeLink)))
  {
    error("NodeLink Matrix is not sparse");
    algo_end(); return (false);  
  }

  if (NodeLink->nrows() != NodeLink->ncols())
  {
    error("NodeLink Matrix needs to be square");
    algo_end(); return (false);      
  }
  
  SparseRowMatrix* spr = matrix_cast::as_sparse(NodeLink);
  size_type m = spr->ncols();
  index_type *rows = spr->get_rows();
  index_type *cols = spr->get_cols();
  
  SparseRowMatrix::Data matData(m+1, m);

  if (!matData.allocated())
  {
    error("Could not allocate memory for sparse matrix");
    algo_end(); return (false);     
  }
  const SparseRowMatrix::Rows& rr = matData.rows();
  const SparseRowMatrix::Columns& cc = matData.columns();
  const SparseRowMatrix::Storage& vv = matData.data();

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

  MatrixHandle mat = new SparseRowMatrix(m, k, matData, m);
  
  if (mat.get_rep() == 0)
  {
    error("Could not build geometry to gridutational mesh mapping matrix");
    algo_end(); return (false);
  }

  if (pot_gridtogeom)
  {
    PotentialGridToGeom = mat;
    if (PotentialGridToGeom.get_rep() == 0)
    { 
      error("Could not build PotentialGridToGeom mapping matrix");
      algo_end(); return (false);  
    }
  }
  
  if (cur_geomtogrid)
  {
    CurrentGeomToGrid = mat->sparse()->make_transpose();
    if (CurrentGeomToGrid.get_rep() == 0)
    { 
      error("Could not build CurrentGeomToGrid mapping matrix");
      algo_end(); return (false);  
    }
  }
  
  if (cur_gridtogeom || pot_geomtogrid)
  {
    MatrixHandle mat2 = mat->sparse()->make_transpose();
    mat2.detach();
    
    index_type* cc = mat2->sparse()->get_cols();
    index_type* rr = mat2->sparse()->get_rows();
    double* vv = mat2->sparse()->get_vals();
    
    for (index_type r=0; r<k; r++)
    {    
      size_type s = rr[r+1]-rr[r];
      double scalar = 1.0/static_cast<double>(s);
      for (index_type p=rr[r]; p<rr[r+1]; p++)
        vv[p] = scalar;
    }
    
    if (cur_gridtogeom)
    {
      CurrentGridToGeom = mat2->sparse()->make_transpose();
      if (CurrentGridToGeom.get_rep() == 0)
      { 
        error("Could not build CurrentGridToGeom mapping matrix");
        algo_end(); return (false);  
      }
    }
    
    if (pot_geomtogrid)
    {
      PotentialGeomToGrid = mat2;
      if (PotentialGeomToGrid.get_rep() == 0)
      { 
        error("Could not build PotentialGeomToGrid mapping matrix");
        algo_end(); return (false);  
      }
    }
  }
  
  algo_end(); return (true);
}

bool 
BuildFEGridMappingAlgo::
run(FieldHandle& DomainField,
    MatrixHandle& NodeLink, 
    MatrixHandle& PotentialGeomToGrid, 
    MatrixHandle& PotentialGridToGeom,
    MatrixHandle& CurrentGeomToGrid, 
    MatrixHandle& CurrentGridToGeom)
{
  algo_start("BuildFEGridMapping");
  
  bool pot_geomtogrid = get_bool("build_potential_geomtogrid");
  bool pot_gridtogeom = get_bool("build_potential_gridtogeom");
  bool cur_geomtogrid = get_bool("build_current_geomtogrid");
  bool cur_gridtogeom = get_bool("build_current_gridtogeom");

  if (DomainField.get_rep() == 0)
  {
    error("CreateLinkBetweenMeshAndGridGridByDomain: No input field");
    algo_end(); return (false);
  }

  // no pregridiled version available, so gridile one

  FieldInformation fi(DomainField);
  
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("The field needs to have data assigned to the elements");
    algo_end(); return (false);   
  }
   
  if (!(fi.is_volume()||fi.is_surface()||fi.is_curve()))
  {
    error("This function is only defined for curve, surface and volume data");
    algo_end(); return (false);
  }

  if (NodeLink.get_rep() == 0)
  {
    error("No matrix on input");
    algo_end(); return (false);
  }

  if (!(matrix_is::sparse(NodeLink)))
  {
    error("NodeLink Matrix is not sparse");
    algo_end(); return (false);  
  }

  if (NodeLink->nrows() != NodeLink->ncols())
  {
    error("NodeLink Matrix needs to be square");
    algo_end(); return (false);      
  }
  
  VField *ifield = DomainField->vfield();
  if (ifield == 0)
  {
    error("FieldHandle is empty");
    algo_end(); return (false);     
  }
  
  VMesh *imesh = ifield->vmesh();
  if (imesh == 0)
  {
    error("Field does not have a mesh");
    algo_end(); return (false);       
  }
        
  VMesh::Node::size_type numnodes;
  imesh->size(numnodes);
  
  if (NodeLink->nrows() != numnodes)
  {
    error("NodeLink Matrix has improper dimensions");
    algo_end(); return (false);        
  }  

  SparseRowMatrix* spr = matrix_cast::as_sparse(NodeLink);
  size_type m = spr->ncols();
  index_type *rows = spr->get_rows();
  index_type *cols = spr->get_cols();
  
  SparseRowMatrix::Data matData(m+1, m);

  if (!matData.allocated())
  {
    error("Could not allocate memory for sparse matrix");
    algo_end(); return (false);     
  }
  const SparseRowMatrix::Rows& rr = matData.rows();
  const SparseRowMatrix::Columns& cc = matData.columns();
  const SparseRowMatrix::Storage& vv = matData.data();
  
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

  MatrixHandle mat = new SparseRowMatrix(m, k, matData, m);
  
  if (mat.get_rep() == 0)
  {
    error("Could not build geometry to gridutational mesh mapping matrix");
    algo_end(); return (false);
  }

  if (pot_gridtogeom)
  {
    PotentialGridToGeom = mat;
    if (PotentialGridToGeom.get_rep() == 0)
    { 
      error("Could not build mapping matrix");
      algo_end(); return (false);  
    }
  }
  
  if (cur_geomtogrid)
  {
    CurrentGeomToGrid = mat->sparse()->make_transpose();
    if (CurrentGeomToGrid.get_rep() == 0)
    { 
      error("Could not build mapping matrix");
      algo_end(); return (false);  
    }
  }

  if (cur_gridtogeom || pot_geomtogrid)
  {
    SparseRowMatrixHandle mat2 = mat->sparse()->make_transpose();
    
    index_type* cc = mat2->get_cols();
    index_type* rr = mat2->get_rows();
    double* const vv = mat2->get_vals();
    
    for (index_type r=0; r<k; r++)
    {    
      size_type s = rr[k+1]-rr[k];
      double scalar = 1.0/s;
      for (index_type p=rr[k]; p<rr[k+1]; p++)
        vv[k] = scalar;
    }
    
    if (cur_gridtogeom)
    {
      CurrentGridToGeom = mat2->make_transpose();
      if (CurrentGridToGeom.get_rep() == 0)
      { 
        error("Could not build mapping matrix");
        algo_end(); return (false);  
      }
    }
    
    if (pot_geomtogrid)
    {
      PotentialGeomToGrid = mat2;
      std::cerr << "ncols="<<mat2->ncols()<<" nrows="<<mat->nrows()<<"\n";
      if (PotentialGeomToGrid.get_rep() == 0)
      { 
        error("Could not build mapping matrix");
        algo_end(); return (false);  
      }
    }
  }
  
  algo_end(); return (true);
}

} // end namespace
