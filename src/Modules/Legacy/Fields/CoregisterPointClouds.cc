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


///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  March 2001

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Geometry/CoregPts.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <iostream>
#include <boost/scoped_ptr.hpp>

namespace SCIRun {

/// @class CoregisterPointClouds
/// @brief CoregisterPointClouds the first three points of two PointCloudFields

class CoregisterPointClouds : public Module
{
  public:
    CoregisterPointClouds(GuiContext* ctx);
    virtual ~CoregisterPointClouds();
    virtual void execute();
    void tcl_command( GuiArgs&, void * );
  private:
    GuiInt allowScale_;
    GuiInt allowRotate_;
    GuiInt allowTranslate_;
    GuiInt seed_;
    GuiInt iters_;
    GuiDouble misfitTol_;
    GuiString method_;
    int abort_;
    boost::shared_ptr<MusilRNG> mr_;
};


DECLARE_MAKER(CoregisterPointClouds)

CoregisterPointClouds::CoregisterPointClouds(GuiContext* ctx)
  : Module("CoregisterPointClouds", ctx, Filter, "MiscField", "SCIRun"),
    allowScale_(get_ctx()->subVar("allowScale"), 1),
    allowRotate_(get_ctx()->subVar("allowRotate"), 1),
    allowTranslate_(get_ctx()->subVar("allowTranslate"), 1),
    seed_(get_ctx()->subVar("seed"), 1),
    iters_(get_ctx()->subVar("iters"), 1000),
    misfitTol_(get_ctx()->subVar("misfitTol"), 0.001),
    method_(get_ctx()->subVar("method"))
{
}


CoregisterPointClouds::~CoregisterPointClouds()
{
}


void
CoregisterPointClouds::execute()
{
  FieldHandle fixedH, mobileH;
  VMesh::Node::size_type nnodes;

  get_input_handle("Fixed PointCloudField", fixedH, true);

  FieldInformation ffi(fixedH);
  if (!(ffi.is_pointcloudmesh()))
  {
    error("input is not a pointcloud");
    return;
  }

  VField* fixedPC = fixedH->vfield();
  VMesh* fixedM = fixedH->vmesh();

  get_input_handle("Mobile PointCloudField", mobileH, true);


  FieldInformation mfi(mobileH);
  if (!(mfi.is_pointcloudmesh()))
  {
    error("input is not a pointcloud");
    return;
  }

  VField* mobilePC = mobileH->vfield();
  VMesh* mobileM = mobileH->vmesh();

  update_state(Executing);

  fixedM->size(nnodes);
  if (nnodes < 3)
  {
    error("Fixed PointCloudField needs at least 3 input points.");
    return;
  }

  mobileM->size(nnodes);
  if (nnodes < 3)
  {
    error("Mobile PointCloudField needs at least 3 input points.");
    return;
  }

  std::vector<Point> fixedPts, mobilePts;
  Transform trans;

  VMesh::Node::iterator fni, fne, mni, mne;
  fixedM->begin(fni); fixedM->end(fne);
  mobileM->begin(mni); mobileM->end(mne);
  Point p;
  while (fni != fne)
  {
    fixedM->get_center(p, *fni);
    fixedPts.push_back(p);
    ++fni;
  }
  while (mni != mne)
  {
    mobileM->get_center(p, *mni);
    mobilePts.push_back(p);
    ++mni;
  }

  int allowScale = allowScale_.get();
  int allowRotate = allowRotate_.get();
  int allowTranslate = allowTranslate_.get();
  std::string method = method_.get();

  boost::scoped_ptr<CoregPts> coreg;
  if (method == "Analytic")
  {
    coreg.reset(new CoregPtsAnalytic(allowScale, allowRotate, allowTranslate));
  }
  else if (method == "Procrustes")
  {
    coreg.reset(new CoregPtsProcrustes(allowScale, allowRotate, allowTranslate));
  }
  else
  { // method == "Simplex"
    FieldHandle dfieldH;
    if (!get_input_handle("DistanceField From Fixed", dfieldH))
    {
      error("Simplex needs a distance field.");
      return;
    }

    VField* dfieldP = dfieldH->vfield();

    if (!dfieldP->is_scalar())
    {
      error("Simplex needs a distance field.");
      return;
    }

    int seed = seed_.get();
    seed_.set(seed+1);
    mr_.reset(new MusilRNG(seed));
    (*mr_)();
    abort_=0;
    coreg.reset(new CoregPtsSimplexSearch(iters_.get(), misfitTol_.get(), abort_, dfieldP,
                                      *mr_, allowScale, allowRotate, allowTranslate));
  }
  coreg->setOrigPtsA(mobilePts);
  coreg->setOrigPtsP(fixedPts);
  coreg->getTrans(trans);
  double misfit;
  coreg->getMisfit(misfit);

  MatrixHandle dm(new DenseMatrix(trans));
  send_output_handle("Transform", dm);
}


/// Commands invoked from the Gui.  Pause/unpause/stop the search.

void
CoregisterPointClouds::tcl_command(GuiArgs& args, void* userdata)
{
  if (args[1] == "stop")
  {
    abort_ = 1;
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}
} // End namespace SCIRun
