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


#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ViewSceneVtkControlsDock.h>
#include <Interface/Modules/Render/ViewSceneVtkDialog.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Modules/Visualization/ShowFieldVtk.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;
//using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Gui;
using namespace SCIRun::Render;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Render;
using namespace SCIRun::Modules::Visualization;

ViewSceneVtkControlPopupWidget::ViewSceneVtkControlPopupWidget(ViewSceneVtkDialog* parent) : QWidget(parent)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &ViewSceneVtkControlPopupWidget::showContextMenu);
	pinToggleAction_ = new QAction("Pin popup", this);
	pinToggleAction_->setCheckable(true);
	closeAction_ = new QAction("Close popup", this);
}

void ViewSceneVtkControlPopupWidget::showContextMenu(const QPoint& pos)
{
	QMenu contextMenu(tr("Context menu"), this);
	contextMenu.addAction(pinToggleAction_);
	contextMenu.addAction(closeAction_);
	contextMenu.exec(mapToGlobal(pos));
}

void ClippingPlaneControlsVtk::updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal)
{
	planeVisibleCheckBox_->setChecked(visible);
	showPlaneFrameCheckBox_->setChecked(showPlane);
	reversePlaneNormalCheckBox_->setChecked(reverseNormal);
}

void ClippingPlaneControlsVtk::updatePlaneControlDisplay(double x, double y, double z, double d)
{
	QString xtext, ytext, ztext, dtext;
	if (x >= 0)
	{
		xtext = "  " + QString::number(x, 'f', 2);
	}
	else
	{
		xtext = QString::number(x, 'f', 2);
	}
	if (y >= 0)
	{
		ytext = "  " + QString::number(y, 'f', 2);
	}
	else
	{
		ytext = QString::number(y, 'f', 2);
	}
	if (z >= 0)
	{
		ztext = "  " + QString::number(z, 'f', 2);
	}
	else
	{
		ztext = QString::number(z, 'f', 2);
	}
	if (d >= 0)
	{
		dtext = "  " + QString::number(d, 'f', 2);
	}
	else
	{
		dtext = QString::number(d, 'f', 2);
	}

	xSliderValueLabel_->setText(xtext);
	ySliderValueLabel_->setText(ytext);
	zSliderValueLabel_->setText(ztext);
	dSliderValueLabel_->setText(dtext);

	ScopedWidgetSignalBlocker xBlocker(xValueHorizontalSlider_);
	ScopedWidgetSignalBlocker yBlocker(yValueHorizontalSlider_);
	ScopedWidgetSignalBlocker zBlocker(zValueHorizontalSlider_);
	ScopedWidgetSignalBlocker dBlocker(dValueHorizontalSlider_);
	xValueHorizontalSlider_->setSliderPosition(x * 100);
	yValueHorizontalSlider_->setSliderPosition(y * 100);
	zValueHorizontalSlider_->setSliderPosition(z * 100);
	dValueHorizontalSlider_->setSliderPosition(d * 100);
}

ClippingPlaneControlsVtk::ClippingPlaneControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton)
	: ViewSceneVtkControlPopupWidget(parent), ButtonStylesheetTogglerVtk(toolbarButton, [this]() { toggleVisible(); })
{
	setupUi(this);

	planeButtonGroup_->setId(plane1RadioButton_, 0);
	planeButtonGroup_->setId(plane2RadioButton_, 1);
	planeButtonGroup_->setId(plane3RadioButton_, 2);
	planeButtonGroup_->setId(plane4RadioButton_, 3);
	planeButtonGroup_->setId(plane5RadioButton_, 4);
	planeButtonGroup_->setId(plane6RadioButton_, 5);

	plane1RadioButton_->setStyleSheet("QRadioButton { color: rgb(219, 56, 22) }");
	plane2RadioButton_->setStyleSheet("QRadioButton { color: rgb(242, 102, 19) }");
	plane3RadioButton_->setStyleSheet("QRadioButton { color: rgb(205, 212, 74) }");
	plane4RadioButton_->setStyleSheet("QRadioButton { color: rgb(87, 184, 53) }");
	plane5RadioButton_->setStyleSheet("QRadioButton { color: rgb(126, 195, 237) }");
	plane6RadioButton_->setStyleSheet("QRadioButton { color: rgb(189, 54, 191) }");

	connect(planeButtonGroup_, BUTTON_GROUP_SIGNAL, parent, &ViewSceneVtkDialog::setClippingPlaneIndex);
	connect(planeVisibleCheckBox_, &QCheckBox::toggled,
		[parent, this](bool b) {
			parent->setClippingPlaneVisible(b); updateToolbarButton("lightGray"); }
	);
	linkedCheckable_ = [this]() { return planeVisibleCheckBox_->isChecked(); };
	connect(showPlaneFrameCheckBox_, &QCheckBox::clicked, parent, &ViewSceneVtkDialog::setClippingPlaneFrameOn);
	connect(reversePlaneNormalCheckBox_, &QCheckBox::clicked, parent, &ViewSceneVtkDialog::reverseClippingPlaneNormal);
	connect(xValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneX);
	connect(yValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneY);
	connect(zValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneZ);
	connect(dValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneD);
}

namespace {
template <typename Checkable>
void toggleCheckable(Checkable* box)
{
  box->setChecked(!box->isChecked());
}
}

void ClippingPlaneControlsVtk::toggleVisible()
{
	toggleCheckable(planeVisibleCheckBox_);
}

