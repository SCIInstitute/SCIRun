/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
 *  BuildElemLeadField.cc: Build the lead field matrix through reciprocity
 *
 *  Written by:
 *   David Weinstein
 *   University of Utah
 *   September 1999
 *
 */

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <iostream>
#include <stdio.h>
#include <math.h>

namespace BioPSE {

using std::pair;

using namespace SCIRun;


class BuildElemLeadField : public Module {    
    MatrixHandle leadfield_;
    int last_mesh_generation_;
    int last_interp_generation_;
  public:
    BuildElemLeadField(GuiContext *context);
    virtual ~BuildElemLeadField();
    virtual void execute();
};


DECLARE_MAKER(BuildElemLeadField)


//---------------------------------------------------------------
BuildElemLeadField::BuildElemLeadField(GuiContext *context)
  : Module("BuildElemLeadField", context, Filter, "LeadField", "BioPSE"),
    leadfield_(0),
    last_mesh_generation_(-1),
    last_interp_generation_(-1)
{
}


BuildElemLeadField::~BuildElemLeadField()
{
}


void
BuildElemLeadField::execute()
{
  FieldHandle mesh_in;
  if (!get_input_handle("Domain Mesh", mesh_in)) return;

  VMesh::size_type nnodes = mesh_in->vmesh()->num_nodes();
  VMesh::size_type nelems = mesh_in->vmesh()->num_elems();

  MatrixHandle interp_in;
  if (!get_input_handle("Electrode Interpolant", interp_in)) return;

  // Can't shortcut return, downstream from the send intermediate may be 
  // waiting for output, so don't hang.
  last_mesh_generation_ = mesh_in->generation;
  last_interp_generation_ = interp_in->generation;

  VField::size_type nelecs=interp_in->nrows();
  VField::index_type counter=0;
  DenseMatrixHandle leadfield_mat = new DenseMatrix(nelecs, nelems*3, 0);

  while (counter<(nelecs-1)) 
  {
    update_progress(counter*1./(nelecs-1));
    ColumnMatrixHandle rhs = new ColumnMatrix(nnodes);
    rhs->zero();
    index_type i;

    index_type *idx;
    double *val;
    size_type idxsize;
    size_type idxstride;

    interp_in->getRowNonzerosNoCopy(0, idxsize, idxstride, idx, val);
    if (!idxsize) ASSERTFAIL("No mesh node assigned to this element!");
    for (i=0; i<idxsize; i++) {
      if (idx[i*idxstride] >= nnodes) ASSERTFAIL("Mesh node out of range!");
      (*rhs)[idx?idx[i*idxstride]:i]+=val[i*idxstride];
    }

    interp_in->getRowNonzerosNoCopy(counter+1, idxsize, idxstride, idx, val);
    if (!idxsize) ASSERTFAIL("No mesh node assigned to this element!");
    for (i=0; i<idxsize; i++) {
      if (idx[i*idxstride] >= nnodes) ASSERTFAIL("Mesh node out of range!");
      (*rhs)[idx?idx[i*idxstride]:i]-=val[i*idxstride];
    }

    send_output_handle("RHS Vector", rhs, true, counter < (nelecs-2));

    // read sol'n
    MatrixHandle sol_in;
    if (!get_input_handle("Solution Vectors", sol_in)) return;

    for (i=0; i<nelems; i++)
      for (index_type j=0; j<3; j++) 
      {
        (*leadfield_mat)[counter+1][i*3+j] =- sol_in->get(i, j);
      }
    counter++;
  }

  leadfield_ = leadfield_mat;

  send_output_handle("Leadfield (nelecs x nelemsx3)", leadfield_, true);
} 


} // End namespace BioPSE
