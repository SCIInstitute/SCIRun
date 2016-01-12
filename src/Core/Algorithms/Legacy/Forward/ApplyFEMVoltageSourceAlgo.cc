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

/*
*  ApplyFEMVoltageSource.cc:  Builds the RHS of the FE matrix for voltage sources
*
*  Written by:
*   David Weinstein
*   University of Utah
*   May 1999
*  Modified by:
*   Alexei Samsonov, March 2001
*   Frank B. Sachse, February 2006
*
*/

#include <Core/Algorithms/Legacy/Forward/ApplyFEMVoltageSourceAlgo.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>

#include <Dataflow/Network/Module.h>
#include <vector>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Forward;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

ApplyFEMVoltageSourceAlgo::ApplyFEMVoltageSourceAlgo()
{

}


void ApplyFEMVoltageSourceAlgo::ExecuteAlgorithm(FieldHandle& hField, MatrixHandle& hMatIn)
{/*
  //! Obtaining handles to computation objects
  //FieldHandle hField;
  //get_input_handle("Mesh", hField, true);
  
  std::vector<std::pair<int, double> > dirBC;

  if (bcFlag_.get() == "DirSub") 
  {
    if (!hField->get_property("dirichlet", dirBC))
      //warning("The input field doesn't contain Dirichlet boundary conditions.");
  }
  
  //MatrixHandle hMatIn;
  //get_input_handle("Stiffness Matrix", hMatIn, true); 
  
  SparseRowMatrix *matIn;
  if (!(matIn = dynamic_cast<SparseRowMatrix*>(hMatIn.get()))) 
  {
    //error("Input stiffness matrix wasn't sparse.");
    return;
  }
  
  if (matIn->nrows() != matIn->ncols()) 
  {
    //error("Input stiffness matrix wasn't square.");
    return;
  }
  
  SparseRowMatrix *mat = matIn->clone();
  
  unsigned int nsize=matIn->ncols();
  DenseColumnMatrix* rhs = new DenseColumnMatrix(nsize);
  
  MatrixHandle  hRhsIn;
  DenseColumnMatrix* rhsIn = 0;
  
  // -- if the user passed in a vector the right size, copy it into ours 
  if (get_input_handle("RHS", hRhsIn, false) && 
      hRhsIn.get())
  {
    rhsIn = hRhsIn->column();
    if (rhsIn && (rhsIn->nrows() == nsize))
    {
      for (unsigned int i=0; i < nsize; i++) 
        (*rhs)[i]=(*rhsIn)[i];
    }
    else
    {
      rhs->Zero();    
    }
  }
  else
  {
    rhs->Zero();
  }
  
  std::string bcFlag = bcFlag_.get();
    
  if (bcFlag=="GroundZero") dirBC.push_back(std::pair<int, double>(0,0.0));
  else if (bcFlag == "DirSub") hField->vfield()->get_property("dirichlet", dirBC);

  //! adjusting matrix for Dirichlet BC
  index_type *idcNz; 
  double *valNz;
  index_type idcNzsize;
  index_type idcNzstride;
    
  std::vector<double> dbc;
  index_type idx;
  size_type size = dirBC.size();
  for(idx = 0; idx<size; ++idx)
  {
    index_type ni = dirBC[idx].first;
    double val = dirBC[idx].second;
  
    // -- getting column indices of non-zero elements for the current row
    mat->getRowNonzerosNoCopy(ni, idcNzsize, idcNzstride, idcNz, valNz);
  
    // -- updating rhs
    for (index_type i=0; i<idcNzsize; ++i)
    {
      index_type j = idcNz?idcNz[i*idcNzstride]:i;
      (*rhs)[j] += - val * valNz[i*idcNzstride]; 
    }
  }
 
  //! zeroing matrix row and column corresponding to the dirichlet nodes
  size = dirBC.size();
  for(idx = 0; idx<size; ++idx)
  {
    index_type ni = dirBC[idx].first;
    double val = dirBC[idx].second;
  
    mat->getRowNonzerosNoCopy(ni, idcNzsize, idcNzstride, idcNz, valNz);
    
    for (index_type i=0; i<idcNzsize; ++i)
    {
      index_type j = idcNz?idcNz[i*idcNzstride]:i;
      mat->put(ni, j, 0.0);
      mat->put(j, ni, 0.0); 
    }
    
    //! updating dirichlet node and corresponding entry in rhs
    mat->put(ni, ni, 1);
    (*rhs)[ni] = val;
  }

  //! Sending result
  MatrixHandle mat_tmp(mat);
  //send_output_handle("Forward Matrix", mat_tmp);

  MatrixHandle rhs_tmp(rhs);
  //send_output_handle("RHS", rhs_tmp);
  */
}

ALGORITHM_PARAMETER_DEF(Forward, ApplyDirichlet);