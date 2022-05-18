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

#include <Modules/Legacy/Teem/Tend/TendFiber.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>
#include <teem/ten.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Teem;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Teem;
using namespace SCIRun::Core::Geometry;

MODULE_INFO_DEF(TendFiber, Tend, Teem)

ALGORITHM_PARAMETER_DEF(Teem, FiberType);
ALGORITHM_PARAMETER_DEF(Teem, Puncture);
ALGORITHM_PARAMETER_DEF(Teem, Neighborhood);
ALGORITHM_PARAMETER_DEF(Teem, StepSize);
ALGORITHM_PARAMETER_DEF(Teem, Integration);
ALGORITHM_PARAMETER_DEF(Teem, UseAniso);
ALGORITHM_PARAMETER_DEF(Teem, AnisoMetric);
ALGORITHM_PARAMETER_DEF(Teem, AnisoThreshold);
ALGORITHM_PARAMETER_DEF(Teem, UseLength);
ALGORITHM_PARAMETER_DEF(Teem, FiberLength);
ALGORITHM_PARAMETER_DEF(Teem, UseSteps);
ALGORITHM_PARAMETER_DEF(Teem, Steps);
ALGORITHM_PARAMETER_DEF(Teem, UseConf);
ALGORITHM_PARAMETER_DEF(Teem, ConfThreshold);
ALGORITHM_PARAMETER_DEF(Teem, Kernel);

class SCIRun::Modules::Teem::TendFiberImpl
{
  public:
    explicit TendFiberImpl(TendFiber* module) : module_(module) {}
    ~TendFiberImpl();
    unsigned get_aniso(const std::string &s);
    unsigned get_fibertype(const std::string &s);
    unsigned get_integration(const std::string &s);

    TendFiber* module_ {nullptr};
    tenFiberContext *tfx {nullptr};
    Nrrd *tfx_nrrd {nullptr};
};

TendFiber::TendFiber() : Module(staticInfo_), impl_(new TendFiberImpl(this))
{
  INITIALIZE_PORT(InputNrrd);
  INITIALIZE_PORT(SeedPoints);
  INITIALIZE_PORT(Fibers);
}

TendFiberImpl::~TendFiberImpl()
{
  if (tfx) tenFiberContextNix(tfx);
}

TendFiber::~TendFiber() = default;

unsigned
TendFiberImpl::get_aniso(const std::string &s)
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
TendFiberImpl::get_integration(const std::string &s)
{
  if (s == "Euler")
  {
    return tenFiberIntgEuler;
  }
  if (s == "RK4")
  {
    return tenFiberIntgRK4;
  }
  module_->error("Unknown integration method");
  return tenFiberIntgEuler;
}

unsigned
TendFiberImpl::get_fibertype(const std::string &s)
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
  module_->error("Unknown fiber-tracing algorithm");
  return tenFiberTypeEvec1;
}

void TendFiber::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::FiberType, std::string("tensorline"));
  state->setValue(Parameters::Puncture, 0.0);
  state->setValue(Parameters::Neighborhood, 2.0);
  state->setValue(Parameters::StepSize, 0.01);
  state->setValue(Parameters::Integration, std::string("Euler"));
  state->setValue(Parameters::Kernel, std::string("tent"));
  state->setValue(Parameters::UseAniso, true);
  state->setValue(Parameters::AnisoMetric, std::string("tenAniso_Cl2"));
  state->setValue(Parameters::AnisoThreshold, 0.4);
  state->setValue(Parameters::UseLength, true);
  state->setValue(Parameters::FiberLength, 1.0);
  state->setValue(Parameters::UseSteps, false);
  state->setValue(Parameters::Steps, 200);
  state->setValue(Parameters::UseConf, true);
  state->setValue(Parameters::ConfThreshold, 0.5);
}

void
TendFiber::execute()
{
  auto nrrd_handle = getRequiredInput(InputNrrd);
  auto seedPoints = getRequiredInput(SeedPoints);

  if (needToExecute())
  {
    // Force Teem to be locked befoer calling the Teem library
    NrrdGuard nrrd_guard;

    Nrrd*& nin = nrrd_handle->getNrrd();

    VMesh *pcm = seedPoints->vmesh();

    auto state = get_state();
    unsigned fibertype = impl_->get_fibertype(state->getValue(Parameters::FiberType).toString());
    double puncture = state->getValue(Parameters::Puncture).toDouble();
    double stepsize = state->getValue(Parameters::StepSize).toDouble();
    int integration = impl_->get_integration(state->getValue(Parameters::Integration).toString());
    bool use_aniso = state->getValue(Parameters::UseAniso).toBool();
    unsigned aniso_metric = impl_->get_aniso(state->getValue(Parameters::AnisoMetric).toString());
    double aniso_thresh = state->getValue(Parameters::AnisoThreshold).toDouble();
    bool use_length = state->getValue(Parameters::UseLength).toBool();
    double length = state->getValue(Parameters::FiberLength).toDouble();
    bool use_steps = state->getValue(Parameters::UseSteps).toBool();
    int steps = state->getValue(Parameters::Steps).toInt();
    bool use_conf = state->getValue(Parameters::UseConf).toBool();
    double conf_thresh = state->getValue(Parameters::ConfThreshold).toDouble();
    const std::string kernel = state->getValue(Parameters::Kernel).toString();

    NrrdKernel *kern;
    std::vector<double> p(NRRD_KERNEL_PARMS_NUM);
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

    if (!impl_->tfx || nin != impl_->tfx_nrrd)
    {
      if (impl_->tfx)
        tenFiberContextNix(impl_->tfx);
      impl_->tfx = tenFiberContextNew(nin);
    }
    if (!impl_->tfx)
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
      E |= tenFiberStopSet(impl_->tfx, tenFiberStopAniso, aniso_metric, aniso_thresh);
    if (use_length && !E)
      E |= tenFiberStopSet(impl_->tfx, tenFiberStopLength, length);
    if (use_steps && !E)
      E |= tenFiberStopSet(impl_->tfx, tenFiberStopNumSteps, steps);
    if (use_conf && !E)
      E |= tenFiberStopSet(impl_->tfx, tenFiberStopConfidence, conf_thresh);

    if (!E) E |= tenFiberTypeSet(impl_->tfx, fibertype);
    if (!E) E |= tenFiberKernelSet(impl_->tfx, kern, &p[0]);
    if (!E) E |= tenFiberIntgSet(impl_->tfx, integration);
    if (!E) E |= tenFiberUpdate(impl_->tfx);
    if (E)
    {
      char *err = biffGetDone(TEN);
      error(std::string("Error setting fiber stop conditions: ") + err);
      free(err);
      return;
    }

    tenFiberParmSet(impl_->tfx, tenFiberParmStepSize, stepsize);
    tenFiberParmSet(impl_->tfx, tenFiberParmWPunct, puncture);
    tenFiberParmSet(impl_->tfx, tenFiberParmUseIndexSpace, AIR_TRUE);

    Nrrd *nout = nrrdNew();

    VMesh::Node::iterator ib, ie;
    pcm->begin(ib);
    pcm->end(ie);

    std::vector<std::vector<Point>> fibers;

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
      if ((failed = tenFiberTrace(impl_->tfx, nout, start)))
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

    FieldInformation fi("CurveMesh", -1, "double");
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

    sendOutput(Fibers, output);
  }
}
