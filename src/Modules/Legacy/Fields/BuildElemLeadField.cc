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


#include <Modules/Legacy/Fields/BuildElemLeadField.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>

// #include <Core/Datatypes/Mesh.h>
// #include <Core/Datatypes/MatrixTypeConverter.h>

#if 0
class BuildElemLeadField : public Module {
    MatrixHandle leadfield_;
    int last_mesh_generation_;
    int last_interp_generation_;
  public:
    BuildElemLeadField(GuiContext *context);
    virtual ~BuildElemLeadField();
    virtual void execute();
};
#endif


using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(BuildElemLeadField, LeadField, SCIRun)

namespace SCIRun::Modules::Fields
{
class BuildElemLeadFieldImpl
{
public:
  DenseMatrixHandle leadfield_;
  int last_mesh_generation_ {-1};
  int last_interp_generation_ {-1};
};
}

BuildElemLeadField::BuildElemLeadField() :
  Module(staticInfo_, false), impl_(new BuildElemLeadFieldImpl)
{
  INITIALIZE_PORT(DomainMesh);
  INITIALIZE_PORT(ElectrodeInterpolant);
  INITIALIZE_PORT(SolutionVectors);
  INITIALIZE_PORT(LeadField);
}

BuildElemLeadField::~BuildElemLeadField() = default;

void BuildElemLeadField::execute()
{
  auto mesh_in = getRequiredInput(DomainMesh);

  auto nnodes = mesh_in->vmesh()->num_nodes();
  auto nelems = mesh_in->vmesh()->num_elems();

  auto interp_in = getRequiredInput(ElectrodeInterpolant);

  // Can't shortcut return, downstream from the send intermediate may be
  // waiting for output, so don't hang.
  //last_mesh_generation_ = mesh_in->generation;
  //last_interp_generation_ = interp_in->generation;

  auto nelecs = interp_in->nrows();
  auto counter = 0;
  DenseMatrixHandle leadfield_mat(new DenseMatrix(nelecs, nelems*3, 0));

  while (counter < (nelecs-1))
  {
    {
      std::ostringstream ostr;
      ostr << "update_progress " << counter << " / " << nelecs - 1;
      remark(ostr.str());
      //TODO
      //update_progress(counter, nelecs - 1);
    }
    DenseColumnMatrixHandle rhs(new DenseColumnMatrix(nnodes));
    rhs->setZero();
    index_type i;

    index_type *idx;
    double *val;
    size_type idxsize;
    //TODO: strided matrices not available--is it required?
    const size_type idxstride = 1;

    interp_in->getRowNonzerosNoCopy(0, idxsize, /*idxstride,*/ idx, val);
    if (!idxsize)
      THROW_ALGORITHM_PROCESSING_ERROR("No mesh node assigned to this element!");

    for (i=0; i<idxsize; i++)
    {
      if (idx[i*idxstride] >= nnodes)
        THROW_ALGORITHM_PROCESSING_ERROR("Mesh node out of range!");
      (*rhs)[idx?idx[i*idxstride]:i] += val[i*idxstride];
    }

    interp_in->getRowNonzerosNoCopy(counter+1, idxsize, /*idxstride,*/ idx, val);
    if (!idxsize)
      THROW_ALGORITHM_PROCESSING_ERROR("No mesh node assigned to this element!");

    for (i=0; i<idxsize; i++)
    {
      if (idx[i*idxstride] >= nnodes)
        THROW_ALGORITHM_PROCESSING_ERROR("Mesh node out of range!");
      (*rhs)[idx?idx[i*idxstride]:i] -= val[i*idxstride];
    }

#if 0
//TODO: looped execution!
    sendOutput(RHSVector, rhs, true, counter < (nelecs-2));
#endif

    auto sol_in = getRequiredInput(SolutionVectors);

    for (i=0; i<nelems; i++)
    {
      for (index_type j=0; j<3; j++)
      {
        (*leadfield_mat)(counter+1, i*3+j) = -(*sol_in)(i, j);
      }
    }
    counter++;
  }

  impl_->leadfield_ = leadfield_mat;

  sendOutput(LeadField, impl_->leadfield_);
}
