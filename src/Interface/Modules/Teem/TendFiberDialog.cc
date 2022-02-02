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


#include <Interface/Modules/Teem/TendFiberDialog.h>
#include <Modules/Legacy/Teem/Tend/TendFiber.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Teem;

TendFiberDialog::TendFiberDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addDoubleSpinBoxManager(punctureDoubleSpinBox_, Parameters::Puncture);
  addDoubleSpinBoxManager(neighborhoodDoubleSpinBox_, Parameters::Neighborhood);
  addDoubleSpinBoxManager(stepSizeDoubleSpinBox_, Parameters::StepSize);
  addDoubleSpinBoxManager(fiberLengthDoubleSpinBox_, Parameters::FiberLength);
  addDoubleSpinBoxManager(confidenceThresholdDoubleSpinBox_, Parameters::ConfThreshold);
  addDoubleSpinBoxManager(anisotropyThresholdDoubleSpinBox_, Parameters::AnisoThreshold);
  addSpinBoxManager(numStepsSpinBox_, Parameters::Steps);
  addCheckBoxManager(fiberLengthCheckBox_, Parameters::UseLength);
  addCheckBoxManager(numStepsCheckBox_, Parameters::UseSteps);
  addCheckBoxManager(confidenceThresholdCheckBox_, Parameters::UseConf);
  addCheckBoxManager(anisotropyThresholdCheckBox_, Parameters::UseAniso);
  addComboBoxManager(integrationComboBox_, Parameters::Integration);
  addComboBoxManager(fiberAlgoComboBox_, Parameters::FiberType,
    {{"Tensorline (TL)", "tensorline"},
    {"Major eigenvector", "evec1"},
    {"Oriented tensors (OT)", "zhukov"}
  });
  addComboBoxManager(kernelComboBox_, Parameters::Kernel,
    {{"Box", "box"},
    {"Tent", "tent"},
    {"Cubic (Catmull-Rom)", "cubicCR"},
    {"Cubic (B-spline)", "cubicBS"},
    {"Gaussian", "gaussian"},
    {"Quartic", "quartic"}
  });
  addComboBoxManager(anisotropyThresholdComboBox_, Parameters::AnisoMetric,
    {{"Westin's linear (first version)", "tenAniso_Cl1"},
    {"Westin's planar (first version)", "tenAniso_Cp1"},
    {"Westin's linear + planar (first version)", "tenAniso_Ca1"},
    {"minimum of Cl and Cp (first version)", "tenAniso_Clpmin1"},
    {"Westin's spherical (first version)", "tenAniso_Cs1"},
    {"gk's anisotropy type (first version)", "tenAniso_Ct1"},
    {"Westin's linear (second version)", "tenAniso_Cl2"},
    {"Westin's planar (second version)", "tenAniso_Cp2"},
    {"Westin's linear + planar (second version)", "tenAniso_Ca2"},
    {"minimum of Cl and Cp (second version)", "tenAniso_Clpmin2"},
    {"Westin's spherical (second version)", "tenAniso_Cs2"},
    {"gk's anisotropy type (second version)", "tenAniso_Ct2"},
    {"Bass+Pier's relative anisotropy", "tenAniso_RA"},
    {"(Bass+Pier's fractional anisotropy)/sqrt(2)", "tenAniso_FA"},
    {"volume fraction = 1-(Bass+Pier's volume ratio)", "tenAniso_VF"},
    {"linear term in cubic characteristic polynomial", "tenAniso_B"},
    {"radius of root circle is 2*sqrt(Q/9)", "tenAniso_Q"},
    {"phase of root circle is acos(R/Q^3)", "tenAniso_R"},
    {"sqrt(Q^3 - R^2)", "tenAniso_S"},
    {"R/sqrt(2*Q^3)", "tenAniso_Skew"},
    {"3*sqrt(6)*det(dev)/norm(dev) = sqrt(2)*skew", "tenAniso_Mode"},
    {"acos(sqrt(2)*skew)/3", "tenAniso_Th"},
    {"FA*(1+mode)/2", "tenAniso_Omega"},
    {"plain old trace", "tenAniso_Tr"}
  });
}
