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

#pragma once

#define NOMINMAX
#include <ospray/ospray.h>
#include <QtGui>
#include <stdexcept>
#include <iostream>

class LightEditor : public QObject
{

Q_OBJECT

public:

  LightEditor(OSPLight ambientLight, OSPLight directionalLight,
    QDoubleSpinBox* ambientLightIntensitySpinBox,
    QDoubleSpinBox* directionalLightIntensitySpinBox,
    QSlider* directionalLightAzimuthSlider,
    QSlider* directionalLightElevationSlider
  );

  friend std::ostream& operator<< (std::ostream& out, LightEditor& v)
  {
    return out << "--ambientLight " << v.ambientLightIntensitySpinBox_->value() << " --directionalLight "
        << v.directionalLightIntensitySpinBox_->value() << " " << v.directionalLightAzimuthSlider_->value()
        << " " << v.directionalLightElevationSlider_->value();
  }
  void setAmbientLightIntensity(float v) { ambientLightIntensitySpinBox_->setValue(v); }
  void setDirectionalLightIntensity(float v) { directionalLightIntensitySpinBox_->setValue(v); }
  void setDirectionalLightAzimuth(float v) { directionalLightAzimuthSlider_->setValue(v); }
  void setDirectionalLightElevation(float v) { directionalLightElevationSlider_->setValue(v); }

Q_SIGNALS:

  void lightsChanged();

protected Q_SLOTS:

  void ambientLightChanged();
  void directionalLightChanged();

public:

  //! OSPRay ambient light.
  OSPLight ambientLight;

  //! OSPRay directional light.
  OSPLight directionalLight;

  // Ambient light UI elements.
  QDoubleSpinBox* ambientLightIntensitySpinBox_ {nullptr};

  // Directional light UI elements.
  QDoubleSpinBox* directionalLightIntensitySpinBox_{nullptr};
  QSlider* directionalLightAzimuthSlider_{nullptr};
  QSlider* directionalLightElevationSlider_{nullptr};

};
