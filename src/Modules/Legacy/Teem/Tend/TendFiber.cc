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

//    File   : TendFiber.cc
//    Author : Martin Cole
//    Date   : Mon Sep  8 09:46:49 2003

#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <teem/ten.h>

#include <sstream>
#include <iostream>
#include <stdio.h>

namespace SCITeem {

using namespace SCIRun;

class TendFiber : public Module {
  public:
    TendFiber(SCIRun::GuiContext *ctx);
    virtual ~TendFiber();
    virtual void execute();

  private:
    unsigned get_aniso(const std::string &s);
    unsigned get_fibertype(const std::string &s);
    unsigned get_integration(const std::string &s);

    GuiString    fibertype_;
    GuiDouble    puncture_;
    GuiDouble    neighborhood_;
    GuiDouble    stepsize_;
    GuiString    integration_;
    GuiInt       use_aniso_;
    GuiString    aniso_metric_;
    GuiDouble    aniso_thresh_;
    GuiInt       use_length_;
    GuiDouble    length_;
    GuiInt       use_steps_;
    GuiInt       steps_;
    GuiInt       use_conf_;
    GuiDouble    conf_thresh_;
    GuiString    kernel_;

    tenFiberContext *tfx;
    Nrrd *tfx_nrrd;
};

DECLARE_MAKER(TendFiber)

TendFiber::TendFiber(SCIRun::GuiContext *ctx) : 
  Module("TendFiber", ctx, Filter, "Tend", "Teem"), 
  fibertype_(get_ctx()->subVar("fibertype"), "tensorline"),
  puncture_(get_ctx()->subVar("puncture"), 0.0),
  neighborhood_(get_ctx()->subVar("neighborhood"), 2.0),
  stepsize_(get_ctx()->subVar("stepsize"), 0.01),
  integration_(get_ctx()->subVar("integration"), "Euler"),
  use_aniso_(get_ctx()->subVar("use-aniso"), 1),
  aniso_metric_(get_ctx()->subVar("aniso-metric"), "tenAniso_Cl2"),
  aniso_thresh_(get_ctx()->subVar("aniso-thresh"), 0.4),
  use_length_(get_ctx()->subVar("use-length"), 1),
  length_(get_ctx()->subVar("length"), 1),
  use_steps_(get_ctx()->subVar("use-steps"), 0),
  steps_(get_ctx()->subVar("steps"), 200),
  use_conf_(get_ctx()->subVar("use-conf"), 1),
  conf_thresh_(get_ctx()->subVar("conf-thresh"), 0.5),
  kernel_(get_ctx()->subVar("kernel"), "tent"),
  tfx(0),
  tfx_nrrd(0)
{
}


TendFiber::~TendFiber()
{
  if (tfx) tenFiberContextNix(tfx);
}


unsigned 
TendFiber::get_aniso(const std::string &s)
{
  if (s == "tenAniso_Cl1") 
  { /* Westin's linear (first version) */
    return tenAniso_Cl1;
  }
  if (s == "tenAniso_Cp1") 
  { /* Westin's planar (first version) */
    return tenAniso_Cp1;
  }
  if (s == "tenAniso_Ca1") 
  { /* Westin's linear + planar (first version) */
    return tenAniso_Ca1;
  }
  if (s == "tenAniso_Clpmin1")
  { /* minimum of Cl and Cp (first version) */
    return tenAniso_Clpmin1;
  }
  if (s == "tenAniso_Cs1") 
  { /* Westin's spherical (first version) */
    return tenAniso_Cs1;
  }
  if (s == "tenAniso_Ct1") 
  { /* gk's anisotropy type (first version) */
    return tenAniso_Ct1;
  }
  if (s == "tenAniso_Cl2") 
  { /* Westin's linear (second version) */
    return tenAniso_Cl2;
  }
  if (s == "tenAniso_Cp2") 
  { /* Westin's planar (second version) */
    return tenAniso_Cp2;
  }
  if (s == "tenAniso_Ca2") 
  { /* Westin's linear + planar (second version) */
    return tenAniso_Ca2;
  }
  if (s == "tenAniso_Clpmin2")
  { /* minimum of Cl and Cp (second version) */
    return tenAniso_Clpmin2;
  }
  if (s == "tenAniso_Cs2") 
  { /* Westin's spherical (second version) */
    return tenAniso_Cs2;
  }
  if (s == "tenAniso_Ct2") 
  { /* gk's anisotropy type (second version) */
    return tenAniso_Ct2;
  }
  if (s == "tenAniso_RA") 
  {  /* Bass+Pier's relative anisotropy */
    return   tenAniso_RA;
  }
  if (s == "tenAniso_FA") 
  {  /* (Bass+Pier's fractional anisotropy)/sqrt(2) */
    return   tenAniso_FA;
  }
  if (s == "tenAniso_VF") 
  {  /* volume fraction= 1-(Bass+Pier's volume ratio)*/
    return tenAniso_VF;
  }
  if (s == "tenAniso_B") 
  {   /* linear term in cubic characteristic polynomial */
    return tenAniso_B;
  }
  if (s == "tenAniso_Q") 
  {   /* radius of root circle is 2*sqrt(Q/9) 
				his is 9 times proper Q in cubic solution) */
    return tenAniso_Q;
  }
  if (s == "tenAniso_R") 
  {   /* phase of root circle is acos(R/Q^3) */
    return tenAniso_R;
  }
  if (s == "tenAniso_S") 
  {   /* sqrt(Q^3 - R^2) */
    return   tenAniso_S;
  }
  if (s == "tenAniso_Skew") 
  {   /* R/sqrt(2*Q^3) */
    return tenAniso_Skew;
  }
  if (s == "tenAniso_Mode") 
  {   /* 3*sqrt(6)*det(dev)/norm(dev) = sqrt(2)*skew */
    return tenAniso_Mode;
  }
  if (s == "tenAniso_Th") 
  {  /* acos(sqrt(2)*skew)/3 */
    return tenAniso_Th;
  }
  if (s == "tenAniso_Omega") 
  {  /* FA*(1+mode)/2 */
    return tenAniso_Omega;
  }
  if (s == "tenAniso_Tr") 
  {  /* plain old trace */
    return tenAniso_Tr;
  }
  return 0;
}

unsigned
TendFiber::get_integration(const std::string &s)
{
  if (s == "Euler") 
  {
    return tenFiberIntgEuler;
  }
  if (s == "RK4") 
  {
    return tenFiberIntgRK4;
  }
  error("Unknown integration method");
  return tenFiberIntgEuler;
}

unsigned 
TendFiber::get_fibertype(const std::string &s)
{
  if (s == "evec1") 
  {
    return tenFiberTypeEvec1;
  }
  if (s == "tensorline") 
  {
    return tenFiberTypeTensorLine;
  }
  if (s == "zhukov") 
  {
    return tenFiberTypeZhukov;
  }
  error("Unknown fiber-tracing algorithm");
  return tenFiberTypeEvec1;
}

void 
TendFiber::execute()
{
  NrrdDataHandle nrrd_handle;
  FieldHandle fldH;
  
  get_input_handle("nin", nrrd_handle, true);
  get_input_handle("SeedPoints",fldH,true);

  if (inputs_changed_ || fibertype_.changed() ||
      puncture_.changed() || stepsize_.changed() ||
      neighborhood_.changed() ||  integration_.changed() ||
      use_aniso_.changed() || aniso_metric_.changed() ||
      aniso_thresh_.changed() || use_length_.changed() ||
      length_.changed() || use_steps_.changed() ||
      steps_.changed() || use_conf_.changed() ||
      conf_thresh_.changed() || kernel_.changed() ||
      !oport_cached("Fibers"))
  {
    // Force Teem to be locked befoer calling the Teem library
    NrrdGuard nrrd_guard;

    // Inform module that execution started
    update_state(Executing);
    
    Nrrd *nin = nrrd_handle->nrrd_;

    VMesh *pcm = fldH->vmesh();
    
    unsigned fibertype = get_fibertype(fibertype_.get());
    double puncture = puncture_.get();
    double stepsize = stepsize_.get();
    int integration = get_integration(integration_.get());
    bool use_aniso = use_aniso_.get();
    unsigned aniso_metric = get_aniso(aniso_metric_.get());
    double aniso_thresh = aniso_thresh_.get();
    bool use_length = use_length_.get();
    double length = length_.get();
    bool use_steps = use_steps_.get();
    int steps = steps_.get();
    bool use_conf = use_conf_.get();
    double conf_thresh = conf_thresh_.get();
    std::string kernel = kernel_.get();

    NrrdKernel *kern;
    double p[NRRD_KERNEL_PARMS_NUM];
    memset(p, 0, NRRD_KERNEL_PARMS_NUM * sizeof(double));
    p[0] = 1.0;
    
    if (kernel == "box") 
    {
      kern = nrrdKernelBox;
    } 
    else if (kernel == "tent") 
    {
      kern = nrrdKernelTent;
    } 
    else if (kernel == "gaussian") 
    { 
      kern = nrrdKernelGaussian; 
      p[1] = 1.0;
      p[2] = 3.0;
    } 
    else if (kernel == "cubicCR") 
    { 
      kern = nrrdKernelBCCubic; 
      p[1] = 0; 
      p[2] = 0.5; 
    } 
    else if (kernel == "cubicBS") 
    { 
      kern = nrrdKernelBCCubic; 
      p[1] = 1; 
      p[2] = 0; 
    } 
    else  
    { // default is quartic
      kern = nrrdKernelAQuartic; 
      p[1] = 0.0834; 
    }

    if (!tfx || nin != tfx_nrrd) 
    {
      if (tfx)
        tenFiberContextNix(tfx);
      tfx = tenFiberContextNew(nin);
    }
    if (!tfx) 
    {
      char *err = biffGetDone(TEN);
      error(std::string("Failed to create the fiber context: ") + err);
      free(err);
      return;
    }

    double start[3];
    int E;

    E = 0;
    if (use_aniso && !E) 
      E |= tenFiberStopSet(tfx, tenFiberStopAniso, aniso_metric, aniso_thresh);
    if (use_length && !E)
      E |= tenFiberStopSet(tfx, tenFiberStopLength, length);
    if (use_steps && !E)
      E |= tenFiberStopSet(tfx, tenFiberStopNumSteps, steps);
    if (use_conf && !E)
      E |= tenFiberStopSet(tfx, tenFiberStopConfidence, conf_thresh);
    
    if (!E) E |= tenFiberTypeSet(tfx, fibertype);
    if (!E) E |= tenFiberKernelSet(tfx, kern, p);
    if (!E) E |= tenFiberIntgSet(tfx, integration);
    if (!E) E |= tenFiberUpdate(tfx);
    if (E) 
    {
      char *err = biffGetDone(TEN);
      error(std::string("Error setting fiber stop conditions: ") + err);
      free(err);
      return;
    }

    tenFiberParmSet(tfx, tenFiberParmStepSize, stepsize);
    tenFiberParmSet(tfx, tenFiberParmWPunct, puncture);
    tenFiberParmSet(tfx, tenFiberParmUseIndexSpace, AIR_TRUE);

    Nrrd *nout = nrrdNew();

    VMesh::Node::iterator ib, ie;
    pcm->begin(ib);
    pcm->end(ie);

    Array1<Array1<Point> > fibers;

    Vector min(nin->axis[1].min, nin->axis[2].min, nin->axis[3].min);
    Vector spacing(nin->axis[1].spacing, nin->axis[2].spacing, nin->axis[3].spacing);
    int fiberIdx=0;
    while (ib != ie) 
    {
      Point p;
      pcm->get_center(p, *ib);
      p-=min;
      start[0]=p.x()/spacing.x();
      start[1]=p.y()/spacing.y();
      start[2]=p.z()/spacing.z();

      bool failed;
      if (failed = tenFiberTrace(tfx, nout, start)) 
      {
        char *err = biffGetDone(TEN);
        free(err);
      }

      fibers.resize(fiberIdx+1);
      if (!failed) 
      {
        fibers[fiberIdx].resize(nout->axis[1].size);
        double *data = (double *)(nout->data);
        for (unsigned int i=0; i<nout->axis[1].size * 3; i+=3)
          fibers[fiberIdx][i/3] = Point(data[i]*spacing.x(),
                data[i+1]*spacing.y(),
                data[i+2]*spacing.z())+min;
      }
      ++fiberIdx;
      ++ib;
    }
    
    nrrdNuke(nout);

    FieldInformation fi("CurveMesh",-1,"double");
    FieldHandle output = CreateField(fi);
    VMesh* cm = output->vmesh();
    VMesh::Node::array_type a(2);

    for (int i=0; i<fibers.size(); i++) 
    {
      if (fibers[i].size()) 
      {
        a[1] = cm->add_point(fibers[i][0]);
        for (int j=1; j<fibers[i].size(); j++) 
        {
          a[0] = a[1];
          a[1] = cm->add_point(fibers[i][j]);
          cm->add_elem(a);
        }
      }
    }

    send_output_handle("Fibers", output);
  }
}


} // End namespace SCITeem
