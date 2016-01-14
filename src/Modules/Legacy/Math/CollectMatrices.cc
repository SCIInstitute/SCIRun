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

///@brief Matrix operations -- concatenate, replace
///
///@author
///   David Weinstein &
///   Chris Butson
///   Department of Computer Science
///   University of Utah
///@date  July 2002

#include <Modules/Legacy/Math/CollectMatrices.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Math/CollectMatrices/CollectMatricesAlgorithm.h>


using namespace SCIRun;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Dataflow::Networks;

/// @class CollectMatrices
/// @detail This module appends/replaces rows or columns of a matrix while
/// looping through a network.

const ModuleLookupInfo CollectMatrices::staticInfo_("CollectMatrices", "Math", "SCIRun");

namespace SCIRun {
	namespace Modules {
		namespace Math {
class CollectMatricesImpl
{
public:
  MatrixHandle matrixH_;
  boost::shared_ptr<CollectMatricesAlgorithmBase> create_algo(MatrixHandle aH, MatrixHandle bH) const;
  void resetMatrix() { matrixH_.reset(); }
};
}}}

CollectMatrices::CollectMatrices() : Module(staticInfo_), impl_(new CollectMatricesImpl)
{
  INITIALIZE_PORT(Optional_BaseMatrix);
  INITIALIZE_PORT(SubMatrix);
  INITIALIZE_PORT(CompositeMatrix);
}

void CollectMatrices::setStateDefaults()
{
  auto state = get_state();
  //TODO: these might need to be all 1s
  state->setValue(Parameters::CollectAppendIndicator, 0);
  state->setValue(Parameters::CollectRowIndicator, 0);
  state->setValue(Parameters::CollectPrependIndicator, 0);
  postStateChangeInternalSignalHookup();
}

void CollectMatrices::postStateChangeInternalSignalHookup()
{
  get_state()->connect_state_changed([this]() { checkForClearOutput(); });
}

/// @todo: match output matrix type with input type.
void
CollectMatrices::execute()
{
  update_state(NeedData);

  auto aHOpt = getOptionalInput(Optional_BaseMatrix);
  auto bH = getRequiredInput(SubMatrix);

  auto state = get_state();
  bool append = state->getValue(Parameters::CollectAppendIndicator).toInt() == 0;
  bool row = state->getValue(Parameters::CollectRowIndicator).toInt() == 0;
  bool front = state->getValue(Parameters::CollectPrependIndicator).toInt() == 0;
  
  MatrixHandle omatrix;

  if (!append)               // Replace -- just send B matrix
  {
    impl_->matrixH_ = bH;
    sendOutput(CompositeMatrix, impl_->matrixH_);
    return;
  }
  else if (!aHOpt || !*aHOpt)    // No A matrix
  {
    if (!impl_->matrixH_)
    {
      // No previous CompositeMatrix, so send B
      impl_->matrixH_ = bH;
      sendOutput(CompositeMatrix, impl_->matrixH_);
      return;
    }
    else
    {
      auto algo = impl_->create_algo(impl_->matrixH_, bH);
      // Previous CompositeMatrix exists, concatenate with B
      if (row)
      {
        if (impl_->matrixH_->ncols() != bH->ncols())
        {
          warning("SubMatrix and CompositeMatrix must have same number of columns");
          return;
        }
        else
        {
          if (front)
            omatrix = algo->concat_rows(bH, impl_->matrixH_);
          else
            omatrix = algo->concat_rows(impl_->matrixH_, bH);
        }
      }
      else
      {
        if (impl_->matrixH_->nrows() != bH->nrows())
        {
          warning("SubMatrix and CompositeMatrix must have same number of rows");
          return;
        }
        else
        {
          if (front)
            omatrix = algo->concat_cols(bH, impl_->matrixH_);
          else
            omatrix = algo->concat_cols(impl_->matrixH_, bH);
        } // columns
      } // rows - columns
    } // previous matrix exists
  }
  else
  { // A exists
    auto aH = *aHOpt;
    auto algo = impl_->create_algo(aH, bH);
    if (row)
    {
      if (aH->ncols() != bH->ncols())
      {
        warning("BaseMatrix and CompositeMatrix must have same number of columns");
        return;
      }
      else
      {
        if (front)
          omatrix = algo->concat_rows(bH,aH);
        else
          omatrix = algo->concat_rows(aH,bH);
      }
    }
    else
    {
      if (aH->nrows() != bH->nrows())
      {
        warning("BaseMatrix and CompositeMatrix must have same number of rows");
        return;
      }
      else
      {
        if (front)
          omatrix = algo->concat_cols(bH,aH);
        else
          omatrix = algo->concat_cols(aH,bH);
      } // columns
    } // rows - columns
  } // A exists

  if (omatrix)
  {
    impl_->matrixH_ = omatrix;
    sendOutput(CompositeMatrix, impl_->matrixH_);
  }
}

boost::shared_ptr<CollectMatricesAlgorithmBase>
CollectMatricesImpl::create_algo(MatrixHandle aH, MatrixHandle bH) const
{
  if (matrix_is::sparse(aH) && matrix_is::sparse(bH))
    return boost::make_shared<CollectSparseRowMatricesAlgorithm>();
  else
    return boost::make_shared<CollectDenseMatricesAlgorithm>();
}

void CollectMatrices::checkForClearOutput()
{
  bool clear = transient_value_cast<bool>(get_state()->getTransientValue(Parameters::ClearCollectMatricesOutput));
  if (clear)
  {
    impl_->resetMatrix();
    get_state()->setTransientValue(Parameters::ClearCollectMatricesOutput, false);
    remark("Collected matrix buffer has been cleared.");
  }
}