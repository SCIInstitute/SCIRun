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


#include <Interface/Modules/Math/BuildNoiseColumnMatrixDialog.h>
#include <Modules/Legacy/Math/BuildNoiseColumnMatrix.h>
#include <Core/Algorithms/Math/BuildNoiseColumnMatrix.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;


BuildNoiseColumnMatrixDialog::BuildNoiseColumnMatrixDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

	addDoubleSpinBoxManager(noiseSpinBox_,SCIRun::Core::Algorithms::Math::BuildNoiseColumnMatrixAlgorithm::SignalToNoiseRatio());

	connect(noiseSlider_, SIGNAL(valueChanged(int)), this, SLOT(setSpinBox()));
	connect(noiseSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setSlider()));
}

void BuildNoiseColumnMatrixDialog::setSlider() {
	noiseSlider_->setValue(noiseSpinBox_->value());
}

void BuildNoiseColumnMatrixDialog::setSpinBox() {
	// Store the decimal portion of the spin box so that it's not lost when the slider value changes.
	double temporary = noiseSpinBox_->value() - (int)noiseSpinBox_->value();
	noiseSpinBox_->setValue(noiseSlider_->value()+temporary);
}
