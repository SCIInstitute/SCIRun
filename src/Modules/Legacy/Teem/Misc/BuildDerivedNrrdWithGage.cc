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


/*
 *
 *  Written by:
 *   Anastasia Mironova
 *
 * TODO:
 * - take input from GUI to set gageKind;
 * - figure out how to set gageKind without having to read it from the gui
 *
 */

#include <Modules/Legacy/Teem/Misc/BuildDerivedNrrdWithGage.h>
#include <teem/air.h>
#include <teem/gage.h>
#include <teem/nrrd.h>

#include <sstream>
#include <iostream>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Teem;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Teem;

MODULE_INFO_DEF(BuildDerivedNrrdWithGage, Misc, Teem)

ALGORITHM_PARAMETER_DEF(Teem, FieldKind);
ALGORITHM_PARAMETER_DEF(Teem, OType);
ALGORITHM_PARAMETER_DEF(Teem, Quantity);
ALGORITHM_PARAMETER_DEF(Teem, ValuesType);
ALGORITHM_PARAMETER_DEF(Teem, ValuesNumParm1);
ALGORITHM_PARAMETER_DEF(Teem, ValuesNumParm2);
ALGORITHM_PARAMETER_DEF(Teem, ValuesNumParm3);
ALGORITHM_PARAMETER_DEF(Teem, DType);
ALGORITHM_PARAMETER_DEF(Teem, DNumParm1);
ALGORITHM_PARAMETER_DEF(Teem, DNumParm2);
ALGORITHM_PARAMETER_DEF(Teem, DNumParm3);
ALGORITHM_PARAMETER_DEF(Teem, DDType);
ALGORITHM_PARAMETER_DEF(Teem, DDNumParm1);
ALGORITHM_PARAMETER_DEF(Teem, DDNumParm2);
ALGORITHM_PARAMETER_DEF(Teem, DDNumParm3);

BuildDerivedNrrdWithGage::BuildDerivedNrrdWithGage() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputNrrd);
  INITIALIZE_PORT(OutputNrrd);
}

void BuildDerivedNrrdWithGage::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::FieldKind, std::string("Scalar"));
  state->setValue(Parameters::Quantity, std::string("value"));
  state->setValue(Parameters::OType, std::string("double"));
  state->setValue(Parameters::ValuesType, std::string("zero"));
  state->setValue(Parameters::DType, std::string("zero"));
  state->setValue(Parameters::DDType, std::string("zero"));
  state->setValue(Parameters::ValuesNumParm1, std::string());
  state->setValue(Parameters::ValuesNumParm2, std::string());
  state->setValue(Parameters::ValuesNumParm3, std::string());
  state->setValue(Parameters::DNumParm1, std::string());
  state->setValue(Parameters::DNumParm2, std::string());
  state->setValue(Parameters::DNumParm3, std::string());
  state->setValue(Parameters::DDNumParm1, std::string());
  state->setValue(Parameters::DDNumParm2, std::string());
  state->setValue(Parameters::DDNumParm3, std::string());
}

namespace
{
  void setMiscKind(gageKind *& kind, gageKind *newkind)
  {
    kind = newkind;
  }

  double SPACING(double spc)
  {
    return AIR_EXISTS(spc) ? spc: 1.0;
  }
}

void
BuildDerivedNrrdWithGage::execute()
{
  auto nrrd_handle = getRequiredInput(InputNrrd);

  Nrrd *nin = nrrd_handle->getNrrd();
  Nrrd *nout = nrrdNew();

  gageContext *ctx = nullptr;
  double gmc, ipos[4];
  float scale[3];
  gageKind *kind = nullptr;
  int a, ansLen, E=0, idx, otype, what;
  int six, siy, siz, sox, soy, soz, xi, yi, zi;
  int iBaseDim, oBaseDim;
  gagePerVolume *pvl;
  char *err = nullptr;
  NrrdKernelSpec *k00 = nullptr, *k11 = nullptr, *k22 = nullptr;

  auto state = get_state();
  //attempt to set gageKind
  if (nin->axis[0].size == 1){
    //first axis has only one value, guess it's a scalar field
    setMiscKind(kind, gageKindScl);
  } else if (nin->axis[0].kind == nrrdKindScalar){
    //kind set explicitly in nrrd object, guess it's a scalar field
    setMiscKind(kind, gageKindScl);
  } else if (state->getValue(Parameters::FieldKind).toString() == "Scalar"){
    warning("Field Kind is not set in the input Nrrd, making a guess based "\
    "upon the GUI settings.");
    setMiscKind(kind, gageKindScl);
  } else if (nin->axis[0].size == 3){
    //first axis has three values, guess it's a vector field  (%p)\n",kind
    setMiscKind(kind, gageKindVec);
  } else if (nin->axis[0].kind == nrrdKind3Vector){
    //printf("kind set explicitly in nrrd object, guess it's a vector field\n");
    setMiscKind(kind, gageKindVec);
  } else if (state->getValue(Parameters::FieldKind).toString() == "Vector")
  {
    warning("Field Kind is not set in the input Nrrd, making a guess based "\
    "upon the GUI settings.");
    setMiscKind(kind, gageKindVec);
  } else {
    error("Cannot set gageKind.");
    return;
  }

  //set the type of output nrrd
  if (state->getValue(Parameters::OType).toString() == "double") {
    otype = nrrdTypeDouble;
  } else if (state->getValue(Parameters::OType).toString() == "float") {
    otype = nrrdTypeFloat;
  } else {
    otype = nrrdTypeDefault;
  }

  what = airEnumVal(kind->enm, state->getValue(Parameters::Quantity).toString().c_str());
  if (-1 == what)
  {
    /* -1 indeed always means "unknown" for any gageKind */
    std::string err = "couldn't parse " + state->getValue(Parameters::Quantity).toString() + " as measure of "
     + kind->name + " volume.";
    error(err);
    return;
  }

  //set min grad magnitude, for curvature-based queries, use zero when
  //gradient is below this
  gmc = 0.0;

  //set scaling factor for resampling on each axis
  scale[0] = 1.0;
  scale[1] = 1.0;
  scale[2] = 1.0;

  k00 = nrrdKernelSpecNew();
  k11 = nrrdKernelSpecNew();
  k22 = nrrdKernelSpecNew();

  //set the nrrd kernels' parameters
  std::string k00parms = "";
  k00parms += state->getValue(Parameters::ValuesType).toString();
  k00parms += ":";
  k00parms += state->getValue(Parameters::ValuesNumParm1).toString();
  if (!state->getValue(Parameters::ValuesNumParm2).toString().empty())
  {
    k00parms += ",";
    k00parms += state->getValue(Parameters::ValuesNumParm2).toString();
  }
  if (!state->getValue(Parameters::ValuesNumParm3).toString().empty())
  {
    k00parms += ",";
    k00parms += state->getValue(Parameters::ValuesNumParm3).toString();
  }

  std::string k11parms = "";
  k11parms += state->getValue(Parameters::DType).toString();
  k11parms += ":";
  k11parms += state->getValue(Parameters::DNumParm1).toString();
  if (!state->getValue(Parameters::DNumParm2).toString().empty())
  {
    k11parms += ",";
    k11parms += state->getValue(Parameters::DNumParm2).toString();
  }
  if (!state->getValue(Parameters::DNumParm3).toString().empty())
  {
    k11parms += ",";
    k11parms += state->getValue(Parameters::DNumParm3).toString();
  }

  std::string k22parms = "";
  k22parms += state->getValue(Parameters::DDType).toString();
  k22parms += ":";
  k22parms += state->getValue(Parameters::DDNumParm1).toString();
  if (!state->getValue(Parameters::DDNumParm2).toString().empty()){
    k22parms += ",";
    k22parms += state->getValue(Parameters::DDNumParm2).toString();;
  }
  if (!state->getValue(Parameters::DDNumParm3).toString().empty()){
    k22parms += ",";
    k22parms += state->getValue(Parameters::DDNumParm3).toString();;
  }

  nrrdKernelSpecParse(k00, k00parms.c_str());
  nrrdKernelSpecParse(k11, k11parms.c_str());
  nrrdKernelSpecParse(k22, k22parms.c_str());

  ansLen = kind->table[what].answerLength;
  iBaseDim = kind->baseDim;
  oBaseDim = 1 == ansLen ? 0 : 1;
  six = nin->axis[0+iBaseDim].size;
  siy = nin->axis[1+iBaseDim].size;
  siz = nin->axis[2+iBaseDim].size;
  sox = (int)scale[0]*six;
  soy = (int)scale[1]*siy;
  soz = (int)scale[2]*siz;
  nin->axis[0+iBaseDim].spacing = SPACING(nin->axis[0+iBaseDim].spacing);
  nin->axis[1+iBaseDim].spacing = SPACING(nin->axis[1+iBaseDim].spacing);
  nin->axis[2+iBaseDim].spacing = SPACING(nin->axis[2+iBaseDim].spacing);

  //set up gage
  ctx = gageContextNew();
  gageParmSet(ctx, gageParmGradMagCurvMin, gmc);
  gageParmSet(ctx, gageParmVerbose, 1);
  gageParmSet(ctx, gageParmRenormalize, AIR_FALSE);
  gageParmSet(ctx, gageParmCheckIntegrals, AIR_TRUE);
  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(ctx, nin, kind));
  if (!E) E |= gagePerVolumeAttach(ctx, pvl);
  if (!E) E |= gageKernelSet(ctx, gageKernel00, k00->kernel, k00->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel11, k11->kernel, k11->parm);
  if (!E) E |= gageKernelSet(ctx, gageKernel22, k22->kernel, k22->parm);
  if (!E) E |= gageQueryItemOn(ctx, pvl, what);
  if (!E) E |= gageUpdate(ctx);
  if (E) {
    error(biffGet(GAGE));
    return;
  }
  const double *answer = gageAnswerPointer(ctx, pvl, what);
  gageParmSet(ctx, gageParmVerbose, 0);
  //end gage setup

  if (ansLen > 1) {
    printf("creating %d x %d x %d x %d output\n",
	   ansLen, sox, soy, soz);
    size_t size[NRRD_DIM_MAX];
    size[0] = ansLen; size[1] = sox;
    size[2] = soy;    size[3] = soz;
    if (!E) E |= nrrdMaybeAlloc_nva(nout=nrrdNew(), otype, 4, size);
  } else {
    size_t size[NRRD_DIM_MAX];
    size[0] = sox; size[1] = soy; size[2] = soz;
    printf("creating %d x %d x %d output\n", sox, soy, soz);
    if (!E) E |= nrrdMaybeAlloc_nva(nout=nrrdNew(), otype, 3, size);
  }
  if (E) {
    error(err);
    return;
  }

  //probing the volume
  for (zi=0; zi<=soz-1; zi++)
  {
    AIR_AFFINE(0, zi, soz-1, 0, siz-1);
    for (yi=0; yi<=soy-1; yi++)
    {
      AIR_AFFINE(0, yi, soy-1, 0, siy-1);
      for (xi=0; xi<=sox-1; xi++)
      {
        AIR_AFFINE(0, xi, sox-1, 0, six-1);
        idx = xi + sox*(yi + soy*zi);

        ipos[0] = xi;
        ipos[1] = yi;
        ipos[2] = zi;

        if (gageProbe(ctx, ipos[0], ipos[1], ipos[2]))
        {
          error(ctx->errStr);
        }

        if (1 == ansLen)
        {
          nrrdFInsert[nout->type](nout->data, idx, nrrdFClamp[nout->type](*answer));
        }
        else
        {
          for (a=0; a<=ansLen-1; a++)
          {
            nrrdFInsert[nout->type](nout->data, a + ansLen*idx,
                                    nrrdFClamp[nout->type](answer[a]));
          }
        }
      }
    }
  }

  nrrdContentSet_va(nout, "probe", nin, "%s", airEnumStr(kind->enm, what));
  nout->axis[0+oBaseDim].spacing =
    ((double)six/sox)*SPACING(nin->axis[0+iBaseDim].spacing);
  nout->axis[0+oBaseDim].label = airStrdup(nin->axis[0+iBaseDim].label);
  nout->axis[1+oBaseDim].spacing =
    ((double)six/sox)*SPACING(nin->axis[1+iBaseDim].spacing);
  nout->axis[1+oBaseDim].label = airStrdup(nin->axis[1+iBaseDim].label);
  nout->axis[2+oBaseDim].spacing =
    ((double)six/sox)*SPACING(nin->axis[2+iBaseDim].spacing);
  nout->axis[2+oBaseDim].label = airStrdup(nin->axis[2+iBaseDim].label);


  for (unsigned int i = 0; i < nout->dim; i++)
  {
    if (!(airExists(nout->axis[i].min) &&
	  airExists(nout->axis[i].max)))
      nrrdAxisInfoMinMaxSet(nout, i, nrrdCenterNode);
  }

  NrrdDataHandle ntmp(new NrrdData(nout));
  sendOutput(OutputNrrd, ntmp);
}
