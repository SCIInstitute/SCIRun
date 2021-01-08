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


#include <Interface/Modules/Render/ES/ObjectTransformCalculators.h>
#include <Interface/Modules/Render/ES/SRCamera.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vec_swizzle.hpp>

using namespace SCIRun::Render;

ObjectTranslationCalculator::ObjectTranslationCalculator(const BasicRendererObjectProvider* s, const Params& t) :
  ObjectTransformCalculatorBase(s),
  initialPosition_(t.initialPosition_),
  w_(t.w_),
  invViewProj_(glm::inverse(t.viewProj))
{}

gen::Transform ObjectTranslationCalculator::computeTransform(int x, int y) const
{
  auto screenPos = service_->screen().positionFromClick(x, y);
  glm::vec2 transVec = (screenPos - initialPosition_) * glm::vec2(w_, w_);
  auto trans = gen::Transform();
  trans.setPosition(xyz(invViewProj_ * glm::vec4(transVec, 0.0, 0.0)));
  return trans;
}

ObjectScaleCalculator::ObjectScaleCalculator(const BasicRendererObjectProvider* s, const Params& p) : ObjectTransformCalculatorBase(s),
  flipAxisWorld_(p.flipAxisWorld_), originWorld_(p.originWorld_)
{
  originView_ = glm::vec3(service_->camera().getWorldToView() * glm::vec4(originWorld_, 1.0));
  glm::vec4 projectedOrigin = service_->camera().getViewToProjection() * glm::vec4(originView_, 1.0);
  projectedW_ = projectedOrigin.w;
  auto sposView = glm::vec3(glm::inverse(service_->camera().getViewToProjection()) * glm::vec4(p.initialPosition_ * projectedW_, 0.0, 1.0));
  sposView.z = -projectedW_;
  originToSpos_ = sposView - originView_;
}

gen::Transform ObjectScaleCalculator::computeTransform(int x, int y) const
{
  auto spos = service_->screen().positionFromClick(x, y);

  glm::vec3 currentSposView = glm::vec3(glm::inverse(service_->camera().getViewToProjection()) * glm::vec4(spos * projectedW_, 0.0, 1.0));
  currentSposView.z = -projectedW_;
  glm::vec3 originToCurrentSpos = currentSposView - glm::vec3(xy(originView_), originView_.z);

  float scaling_factor = glm::dot(glm::normalize(originToCurrentSpos), glm::normalize(originToSpos_))
    * (glm::length(originToCurrentSpos) / glm::length(originToSpos_));

  // Flip if negative to avoid inverted normals
  glm::mat4 flip;
  bool negativeScale = scaling_factor < 0.0;
  if (negativeScale)
  {
    //TODO: use more precise pi? or actual constant value?
    flip = glm::rotate(glm::mat4(1.0f), 3.1415926f, flipAxisWorld_);
    scaling_factor = -scaling_factor;
  }

  auto trans = gen::Transform();
  glm::mat4 translation = glm::translate(-originWorld_);
  glm::mat4 scale = glm::scale(trans.transform, glm::vec3(scaling_factor));
  glm::mat4 reverse_translation = glm::translate(originWorld_);

  trans.transform = scale * translation;

  if (negativeScale)
    trans.transform = flip * trans.transform;

  trans.transform = reverse_translation * trans.transform;
  return trans;
}

ObjectRotationCalculator::ObjectRotationCalculator(const BasicRendererObjectProvider* s, const Params& p) : ObjectTransformCalculatorBase(s),
  originWorld_(p.originWorld_), initialW_(p.w_)
{
  auto sposView = glm::vec3(glm::inverse(service_->camera().getViewToProjection()) * glm::vec4(p.initialPosition_ * p.w_, 0.0, 1.0));
  sposView.z = -p.w_;
  auto originView = glm::vec3(service_->camera().getWorldToView() * glm::vec4(p.originWorld_, 1.0));
  auto originToSpos = sposView - originView;
  auto radius = glm::length(originToSpos);
  bool negativeZ = (originToSpos.z < 0.0);
  widgetBall_.reset(new spire::ArcBall(originView, radius, negativeZ));
  widgetBall_->beginDrag(glm::vec2(sposView));
}

gen::Transform ObjectRotationCalculator::computeTransform(int x, int y) const
{
  if (!widgetBall_)
    return {};

  auto spos = service_->screen().positionFromClick(x, y);

  glm::vec2 sposView = glm::vec2(glm::inverse(service_->camera().getViewToProjection()) * glm::vec4(spos * initialW_, 0.0, 1.0));
  widgetBall_->drag(sposView);

  glm::quat rotationView = widgetBall_->getQuat();
  glm::vec3 axis = glm::vec3(rotationView.x, rotationView.y, rotationView.z);
  axis = glm::vec3(glm::inverse(service_->camera().getWorldToView()) * glm::vec4(axis, 0.0));
  glm::quat rotationWorld = glm::quat(rotationView.w, axis);

  glm::mat4 translation = glm::translate(-originWorld_);
  glm::mat4 reverse_translation = glm::translate(originWorld_);
  glm::mat4 rotation = glm::mat4_cast(rotationWorld);

  auto trans = gen::Transform();
  trans.transform = reverse_translation * rotation * translation;
  return trans;
}
