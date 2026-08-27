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
#include <Core/Math/MiscMath.h>
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
  originWorld_(p.originWorld_)
{
  // Get transforms
  auto view = service_->camera().getWorldToView();
  auto projection = service_->camera().getViewToProjection();
  auto inverseProjection = glm::inverse(projection);

  // Apply transformations to vectors
  originView_ = glm::vec3(view * glm::vec4(originWorld_, 1.0));
  glm::vec4 projectedOrigin = projection * glm::vec4(originView_, 1.0);
  projectedW_ = projectedOrigin.w;
  glm::vec3 sposView = xyz(inverseProjection * glm::vec4(p.initialPosition_ * projectedW_, 0.0, 1.0));

  // Store initial data
  sposView.z = -projectedW_;
  originToInitialSpos_ = sposView - originView_;
  originToInitialSposLength_ = glm::length(originToInitialSpos_);
}

gen::Transform ObjectScaleCalculator::computeTransform(int x, int y) const
{
  auto spos = service_->screen().positionFromClick(x, y);

  // Get transforms
  auto projection = service_->camera().getViewToProjection();
  auto inverseProjection = glm::inverse(projection);

  // Apply transformations to vectors
  glm::vec3 sposView = glm::vec3(inverseProjection * glm::vec4(spos * projectedW_, 0.0, 1.0));
  sposView.z = -projectedW_;
  glm::vec3 originToSposView = sposView - originView_;
  float originToSposViewLength = glm::length(originToSposView);
  glm::vec3 originToSposViewNormal = originToSposView / originToSposViewLength;
  float scaling_factor = glm::dot(originToSposViewNormal, glm::normalize(originToInitialSpos_))
    * (originToSposViewLength / originToInitialSposLength_);

  // Flip if negative to avoid inverted normals
  scaling_factor = std::abs(scaling_factor);

  // Generate new transforms
  glm::mat4 translation = glm::translate(-originWorld_);
  glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(scaling_factor));
  glm::mat4 reverse_translation = glm::translate(originWorld_);

  auto trans = gen::Transform();
  trans.transform = scale * translation;

  trans.transform = reverse_translation * trans.transform;
  return trans;
}

ObjectRotationCalculator::ObjectRotationCalculator(const BasicRendererObjectProvider* s, const Params& p) : ObjectTransformCalculatorBase(s),
  originWorld_(p.originWorld_), initialW_(p.w_)
{
  // Get transforms
  auto view = service_->camera().getWorldToView();
  auto projection = service_->camera().getViewToProjection();
  auto inverseProjection = glm::inverse(projection);

  // Apply transformations to vectors
  auto sposView = glm::vec3(inverseProjection * glm::vec4(p.initialPosition_ * p.w_, 0.0, 1.0));
  sposView.z = -p.w_;
  auto originView = glm::vec3(view * glm::vec4(p.originWorld_, 1.0));
  auto originToSposView = sposView - originView;

  // Initialive arcball
  auto radius = glm::length(originToSposView);
  bool negativeZ = (originToSposView.z < 0.0);
  widgetBall_.reset(new spire::ArcBall(originView, radius, negativeZ));
  widgetBall_->beginDrag(glm::vec2(sposView));
}

gen::Transform ObjectRotationCalculator::computeTransform(int x, int y) const
{
  if (!widgetBall_)
    return {};

  auto spos = service_->screen().positionFromClick(x, y);

  // Get transforms
  auto view = service_->camera().getWorldToView();
  auto inverseView = glm::inverse(view);
  auto projection = service_->camera().getViewToProjection();
  auto inverseProjection = glm::inverse(projection);

  // Calculate rotation
  glm::vec2 sposView = glm::vec2(inverseProjection * glm::vec4(spos * initialW_, 0.0, 1.0));
  widgetBall_->drag(sposView);
  glm::quat rotationView = widgetBall_->getQuat();
  glm::vec3 axis = glm::vec3(rotationView.x, rotationView.y, rotationView.z);
  axis = glm::vec3(inverseView * glm::vec4(axis, 0.0));
  glm::quat rotationWorld = glm::quat(rotationView.w, axis);

  // Generate new transforms
  glm::mat4 translation = glm::translate(-originWorld_);
  glm::mat4 reverse_translation = glm::translate(originWorld_);
  glm::mat4 rotation = glm::mat4_cast(rotationWorld);

  auto trans = gen::Transform();
  trans.transform = reverse_translation * rotation * translation;
  return trans;
}

ObjectScaleAxisCalculator::ObjectScaleAxisCalculator(const BasicRendererObjectProvider* s, const Params& p)
  : ObjectTransformCalculatorBase(s),
    axis_(p.axis_),
    originWorld_(p.originWorld_),
    flipAxisWorld_(p.flipAxisWorld_),
    scaleAxisIndex_(p.scaleAxisIndex_)
{
  // Get transforms
  auto view = service_->camera().getWorldToView();
  auto projection = service_->camera().getViewToProjection();
  auto inverseProjection = glm::inverse(projection);

  // Apply transformations to vectors
  originView_ = glm::vec3(view * glm::vec4(originWorld_, 1.0));
  glm::vec4 projectedOrigin = projection * glm::vec4(originView_, 1.0);
  projectedW_ = projectedOrigin.w;
  glm::vec3 sposView = xyz(inverseProjection * glm::vec4(p.initialPosition_ * projectedW_, 0.0, 1.0));

  // Store initial data
  sposView.z = -projectedW_;
  originToInitialSpos_ = sposView - originView_;
  originToInitialSposLength_ = glm::length(originToInitialSpos_);
}

gen::Transform ObjectScaleAxisCalculator::computeTransform(int x, int y) const
{
  auto spos = service_->screen().positionFromClick(x, y);

  // Get transforms
  auto view = service_->camera().getWorldToView();
  auto inverseProjection = glm::inverse(service_->camera().getViewToProjection());

  // Apply transformations to vectors
  glm::vec3 sposView = glm::vec3(inverseProjection * glm::vec4(spos * projectedW_, 0.0, 1.0));
  sposView.z = -projectedW_;
  glm::vec3 scaleAxisView = xyz(view * glm::vec4(axis_, 0.0));
  glm::vec3 originToSposView = sposView - originView_;
  glm::vec3 shiftedOriginToCurrentSpos = originToSposView - (originToInitialSpos_ - scaleAxisView);

  float initLen = glm::length(scaleAxisView);
  float scaling_factor = glm::dot(shiftedOriginToCurrentSpos/initLen, scaleAxisView/initLen);
  //scaling_factor = (multiplier_ * scaling_factor) + (1.0-multiplier_;

  // Flip if negative to avoid inverted normals
  glm::mat4 flip;
  bool negativeScale = scaling_factor < 0.0;
  if (negativeScale)
  {
    flip = glm::rotate(glm::mat4(1.0f), static_cast<float>(M_PI), flipAxisWorld_);
    scaling_factor = -scaling_factor;
  }

  // Generate new transforms
  auto scaleVec = glm::vec3(1.0);
  scaleVec[scaleAxisIndex_] = scaling_factor;

  glm::mat4 translation = glm::translate(-originWorld_);
  glm::mat4 scale = glm::scale(glm::mat4(1.0), scaleVec);
  glm::mat4 reverse_translation = glm::translate(originWorld_);

  auto trans = gen::Transform();
  trans.transform = scale * translation;

  if (negativeScale)
    trans.transform = flip * trans.transform;

  trans.transform = reverse_translation * trans.transform;
  return trans;
}

ObjectAxisTranslationCalculator::ObjectAxisTranslationCalculator(const BasicRendererObjectProvider* s, const Params& p)
  : ObjectTransformCalculatorBase(s), initialPosition_(p.initialPosition_),
  axis_(p.axis_), w_(p.w_)
{}

gen::Transform ObjectAxisTranslationCalculator::computeTransform(int x, int y) const
{
  auto inverseProjection = glm::inverse(service_->camera().getViewToProjection());

  auto spos = service_->screen().positionFromClick(x, y);
  glm::vec2 transVec = (spos - initialPosition_) * glm::vec2(w_, w_);
  auto trans = gen::Transform();

  glm::vec3 worldPos = xyz(inverseProjection * glm::vec4(transVec, 0.0, 0.0));
  glm::vec3 newPos = glm::dot(worldPos, axis_) * axis_;
  trans.transform[3] = glm::vec4(newPos, 1.0);
  return trans;
}

void ObjectScaleAxisCalculator::setMultiplier(double multiplier)
{
  multiplier_ = multiplier;
}
