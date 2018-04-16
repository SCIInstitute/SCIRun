// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "LightEditor.h"

LightEditor::LightEditor(OSPLight ambientLight, OSPLight directionalLight,
  QDoubleSpinBox* ambientLightIntensitySpinBox,
  QDoubleSpinBox* directionalLightIntensitySpinBox,
  QSlider* directionalLightAzimuthSlider,
  QSlider* directionalLightElevationSlider) :
  ambientLight(ambientLight),
  directionalLight(directionalLight),
  ambientLightIntensitySpinBox_(ambientLightIntensitySpinBox),
  directionalLightIntensitySpinBox_(directionalLightIntensitySpinBox),
  directionalLightAzimuthSlider_(directionalLightAzimuthSlider),
  directionalLightElevationSlider_(directionalLightElevationSlider)
{
  // Make sure we have existing lights.
  if (!ambientLight || !directionalLight)
    throw std::runtime_error("LightEditor: must be constructed with an existing lights");

  // Ambient light intensity.
  ambientLightIntensitySpinBox_->setRange(0.0, 3.0);
  ambientLightIntensitySpinBox_->setSingleStep(0.01);
  connect(ambientLightIntensitySpinBox_, SIGNAL(valueChanged(double)), this, SLOT(ambientLightChanged()));

  // Directional light intensity.
  directionalLightIntensitySpinBox_->setRange(0.0, 3.0);
  directionalLightIntensitySpinBox_->setSingleStep(0.01);
  connect(directionalLightIntensitySpinBox_, SIGNAL(valueChanged(double)), this, SLOT(directionalLightChanged()));

  // Directional light: azimuth and elevation angles for direction.
  connect(directionalLightAzimuthSlider_, SIGNAL(valueChanged(int)), this, SLOT(directionalLightChanged()));
  connect(directionalLightElevationSlider_, SIGNAL(valueChanged(int)), this, SLOT(directionalLightChanged()));

  // Set default light parameters.
  ambientLightIntensitySpinBox_->setValue(0.1);  //doesn't seem to fire if it's 0 first
  ambientLightIntensitySpinBox_->setValue(0.2);

  directionalLightIntensitySpinBox_->setValue(1.7);
  directionalLightAzimuthSlider_->setValue(0.8 * (directionalLightAzimuthSlider_->minimum() + directionalLightAzimuthSlider_->maximum())); // 45 degrees azimuth
  directionalLightElevationSlider_->setValue(0.65 * (directionalLightElevationSlider_->minimum() + directionalLightElevationSlider_->maximum())); // 45 degrees elevation
}

void LightEditor::ambientLightChanged()
{
  ospSet1f(ambientLight, "intensity", float(ambientLightIntensitySpinBox_->value()*3.14));
  ospCommit(ambientLight);
  Q_EMIT lightsChanged();
}

void LightEditor::directionalLightChanged()
{
  ospSet1f(directionalLight, "intensity", float(directionalLightIntensitySpinBox_->value()*3.14));

  // Get alpha value in [-180, 180] degrees.
  float alpha = -180.0f + float(directionalLightAzimuthSlider_->value() - directionalLightAzimuthSlider_->minimum()) / float(directionalLightAzimuthSlider_->maximum() - directionalLightAzimuthSlider_->minimum()) * 360.0f;

  // Get beta value in [-90, 90] degrees.
  float beta = -90.0f + float(directionalLightElevationSlider_->value() - directionalLightElevationSlider_->minimum()) / float(directionalLightElevationSlider_->maximum() - directionalLightElevationSlider_->minimum()) * 180.0f;

  // Compute unit vector.
  float lightX = cos(alpha * M_PI/180.0f) * cos(beta * M_PI/180.0f);
  float lightY = sin(alpha * M_PI/180.0f) * cos(beta * M_PI/180.0f);
  float lightZ = sin(beta * M_PI/180.0f);

  // Update OSPRay light direction.
  ospSet3f(directionalLight, "direction", lightX, lightY, lightZ);

  // Commit and Q_EMIT signal.
  ospCommit(directionalLight);
  Q_EMIT lightsChanged();
}
