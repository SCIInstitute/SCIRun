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


#include <Modules/Math/CreateGeometricTransform.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Module.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Plane.h>
#include <Core/Datatypes/MatrixAlgorithms.h>

#include <Core/Math/MiscMath.h>

using namespace SCIRun::Core;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;
using namespace Geometry;

///@file  CreateGeometricTransform.cc
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  March 1999

MODULE_INFO_DEF(CreateGeometricTransform, Math, SCIRun)

ALGORITHM_PARAMETER_DEF(Math, TransformType);
ALGORITHM_PARAMETER_DEF(Math, MultiplyRadioButton);
ALGORITHM_PARAMETER_DEF(Math, TranslateVectorX);
ALGORITHM_PARAMETER_DEF(Math, TranslateVectorY);
ALGORITHM_PARAMETER_DEF(Math, TranslateVectorZ);
ALGORITHM_PARAMETER_DEF(Math, ScalePointX);
ALGORITHM_PARAMETER_DEF(Math, ScalePointY);
ALGORITHM_PARAMETER_DEF(Math, ScalePointZ);
ALGORITHM_PARAMETER_DEF(Math, LogScale);
ALGORITHM_PARAMETER_DEF(Math, LogScaleX);
ALGORITHM_PARAMETER_DEF(Math, LogScaleY);
ALGORITHM_PARAMETER_DEF(Math, LogScaleZ);
ALGORITHM_PARAMETER_DEF(Math, RotatePointX);
ALGORITHM_PARAMETER_DEF(Math, RotatePointY);
ALGORITHM_PARAMETER_DEF(Math, RotatePointZ);
ALGORITHM_PARAMETER_DEF(Math, RotateAxisX);
ALGORITHM_PARAMETER_DEF(Math, RotateAxisY);
ALGORITHM_PARAMETER_DEF(Math, RotateAxisZ);
ALGORITHM_PARAMETER_DEF(Math, RotateTheta);
ALGORITHM_PARAMETER_DEF(Math, ShearVectorX);
ALGORITHM_PARAMETER_DEF(Math, ShearVectorY);
ALGORITHM_PARAMETER_DEF(Math, ShearVectorZ);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneA);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneB);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneC);
ALGORITHM_PARAMETER_DEF(Math, ShearPlaneD);
ALGORITHM_PARAMETER_DEF(Math, FieldMapX);
ALGORITHM_PARAMETER_DEF(Math, FieldMapY);
ALGORITHM_PARAMETER_DEF(Math, FieldMapZ);
ALGORITHM_PARAMETER_DEF(Math, PermuteX);
ALGORITHM_PARAMETER_DEF(Math, PermuteY);
ALGORITHM_PARAMETER_DEF(Math, PermuteZ);
ALGORITHM_PARAMETER_DEF(Math, UniformScale);
ALGORITHM_PARAMETER_DEF(Math, ResizeSeparably);
ALGORITHM_PARAMETER_DEF(Math, IgnoreChanges);

CreateGeometricTransform::CreateGeometricTransform() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputMatrix);
  INITIALIZE_PORT(Widget);
}

void CreateGeometricTransform::setStateDefaults()
{
  auto state = get_state();
  using namespace Parameters;
  state->setValue(TransformType, 0);
  state->setValue(MultiplyRadioButton, 1);
  state->setValue(TranslateVectorX, 0.0);
  state->setValue(TranslateVectorY, 0.0);
  state->setValue(TranslateVectorZ, 0.0);
  state->setValue(ScalePointX, 0.0);
  state->setValue(ScalePointY, 0.0);
  state->setValue(ScalePointZ, 0.0);
  state->setValue(LogScale, 0.0);
  state->setValue(LogScaleX, 0.0);
  state->setValue(LogScaleY, 0.0);
  state->setValue(LogScaleZ, 0.0);
  state->setValue(RotatePointX, 0.0);
  state->setValue(RotatePointY, 0.0);
  state->setValue(RotatePointZ, 0.0);
  state->setValue(RotateAxisX, 0.0);
  state->setValue(RotateAxisY, 0.0);
  state->setValue(RotateAxisZ, 1.0);
  state->setValue(RotateTheta, 0.0);
  state->setValue(ShearVectorX, 0.0);
  state->setValue(ShearVectorY, 0.0);
  state->setValue(ShearVectorZ, 0.0);
  state->setValue(ShearPlaneA, 0.0);
  state->setValue(ShearPlaneB, 0.0);
  state->setValue(ShearPlaneC, 0.0);
  state->setValue(ShearPlaneD, 1.0);
  std::string x = "x+";
  std::string y = "y+";
  std::string z = "z+";
  state->setValue(FieldMapX, x);
  state->setValue(FieldMapY, y);
  state->setValue(FieldMapZ, z);
  state->setValue(PermuteX, 1);
  state->setValue(PermuteY, 2);
  state->setValue(PermuteZ, 3);
  state->setValue(UniformScale, 1.0);
  state->setValue(ResizeSeparably, true);
  state->setValue(IgnoreChanges, true);
}

void CreateGeometricTransform::execute()
{
  auto input_matrix_H = getOptionalInput(InputMatrix);


  if (needToExecute())
  {
    auto state = get_state();
    /*
    GeometryOPortHandle ogeom;
    get_oport_handle("Geometry", ogeom);
    const std::string which_transform = which_transform_gui_.get();

    // Create the widget.
    if (!have_been_initialized_)
    {
      Point C, R, D, I;
      box_widget_->GetPosition(C, R, D, I);
      C = Point(0, 0, 0); R = Point(1, 0, 0); D = Point(0, 1, 0), I = Point(0, 0, 1);
      widget_pose_center_ = C;
      box_widget_->SetPosition(C, R, D, I);
      box_widget_->SetCurrentMode(2);
      if (which_transform != "widget")
      {
        ((GeomSwitch *)(widget_switch_.get_rep()))->set_state(0);
      }
      widgetid_ = ogeom->addObj(widget_switch_, widget_name, &widget_lock_);
      ogeom->flushViews();
      have_been_initialized_ = 1;
    }
    */
    // get the input matrix if there is one
    //MatrixHandle input_matrix_H;
    Transform input_transform;
    if (input_matrix_H)
    {
      auto input = *input_matrix_H;
      input_transform = MatrixAlgorithms::matrix_to_transform(*input);
    }

    Transform local_transform;

    // get the "fixed point"  This only gets the translation vector.  From the translation tab.
    Vector t(state->getValue(Parameters::TranslateVectorX).toDouble(),
      state->getValue(Parameters::TranslateVectorY).toDouble(),
      state->getValue(Parameters::TranslateVectorZ).toDouble());

    // build the local transform
    auto which_transform = state->getValue(Parameters::TransformType).toInt();
    if (which_transform == 0) //translate
    {
      local_transform.post_translate(t);
    }
    else if (which_transform == 1) //scale
    {
      Vector scale_point(state->getValue(Parameters::ScalePointX).toDouble(),
               state->getValue(Parameters::ScalePointY).toDouble(),
               state->getValue(Parameters::ScalePointZ).toDouble());

      double new_scale = state->getValue(Parameters::UniformScale).toDouble();
      double s = pow(10., new_scale);
      double new_scalex = state->getValue(Parameters::ScalePointX).toDouble();
      double sx = pow(10., new_scalex)*s;
      double new_scaley = state->getValue(Parameters::ScalePointY).toDouble();
      double sy = pow(10., new_scaley)*s;
      double new_scalez = state->getValue(Parameters::ScalePointZ).toDouble();
      double sz = pow(10., new_scalez)*s;
      Vector sc(sx, sy, sz);
      local_transform.post_translate(scale_point);
      local_transform.post_scale(sc);
      local_transform.post_translate(-scale_point);
    }
    else if (which_transform == 2) //rotate
    {
      Vector rotate_point(state->getValue(Parameters::RotatePointX).toDouble(),
                         state->getValue(Parameters::RotatePointY).toDouble(),
                         state->getValue(Parameters::RotatePointZ).toDouble());
      //std::cout<<"orig transform = "<<local_transform<<std::endl;
      //std::cout<<"orig transform = "<<std::endl;

      Vector axis(state->getValue(Parameters::RotateAxisX).toDouble(),
        state->getValue(Parameters::RotateAxisY).toDouble(),
        state->getValue(Parameters::RotateAxisZ).toDouble());
      if (!axis.length2()) axis.x(1);
      axis.normalize();
      local_transform.post_translate(rotate_point);
      //std::cout<<"1st transform = "<<local_transform<<std::endl;
      local_transform.post_rotate(state->getValue(Parameters::RotateTheta).toDouble()*M_PI / 180., axis);
      //std::cout<<"2nd transform = "<<local_transform<<std::endl;
      local_transform.post_translate(-rotate_point);
      //std::cout<<"3rd transform = "<<local_transform<<std::endl;
    }
    else if (which_transform == 3) //shear
    {
      local_transform.post_shear(t, Plane(state->getValue(Parameters::ShearPlaneA).toDouble(),
        state->getValue(Parameters::ShearPlaneB).toDouble(),
        state->getValue(Parameters::ShearPlaneC).toDouble(),
        state->getValue(Parameters::ShearPlaneD).toDouble()));
    }
    else if (which_transform == 4) //permute
    {
      local_transform.post_permute(state->getValue(Parameters::PermuteX).toInt(),
        state->getValue(Parameters::PermuteY).toInt(),
        state->getValue(Parameters::PermuteZ).toInt());
    }
    else //widget
    {
      /*
      Point R, D, I, C;
      box_widget_->GetPosition(C, R, D, I);

      double ratio = widget_scale_gui_.get();
      widget_scale_gui_.set(1);
      R = C + (R - C)*ratio;
      D = C + (D - C)*ratio;
      I = C + (I - C)*ratio;
      box_widget_->SetPosition(C, R, D, I);

      // find the difference between widget_pose(_inv) and the current pose
      if (!ignoring_widget_changes_gui_.get()) {
        local_transform.load_basis(C, R - C, D - C, I - C);
        local_transform.post_trans(widget_pose_inv_trans_);
        local_transform.post_translate(-widget_pose_center_.vector());
        local_transform.pre_translate(C.vector());
      }
      local_transform.post_trans(latest_widget_trans_);
      latest_widget_trans_ = local_transform;
      widget_pose_center_ = C;
      widget_pose_inv_trans_.load_basis(C, R - C, D - C, I - C);
      widget_pose_inv_trans_.invert();
      */
    }

    omatrixH_.reset(new DenseMatrix(4, 4));

    // now either pre- or post-multiply the transforms and store in matrix
    if (state->getValue(Parameters::MultiplyRadioButton).toInt() == 0) {
      local_transform.post_trans(composite_trans_);
      latest_trans_ = local_transform;
      local_transform.post_trans(input_transform);
    }
    else {
      local_transform.pre_trans(composite_trans_);
      latest_trans_ = local_transform;
      local_transform.pre_trans(input_transform);
    }

    MatrixHandle mtmp(new DenseMatrix(local_transform));
    sendOutput(OutputMatrix, mtmp);
  }
}
