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
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkColorPickerButton.h>
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkPopupWidget.h>

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

ButtonStylesheetTogglerVtk::ButtonStylesheetTogglerVtk(QPushButton* toolbarButton, std::function<void()> whatToToggle) : toolbarButton_(toolbarButton), whatToToggle_(whatToToggle)
{
  QObject::connect(toolbarButton, &QPushButton::clicked, whatToToggle_);
}

void ButtonStylesheetTogglerVtk::updateToolbarButton(const QColor& color)
{
  if (!linkedCheckable_) return;
  if (linkedCheckable_())
  {
    const QColor complimentary(255 - color.red(), 255 - color.green(), 255 - color.blue());
    toolbarButton_->setStyleSheet("QPushButton { background-color: " + color.name() + "; color: " + complimentary.name() + " }");
  }
  else
  {
    toolbarButton_->setStyleSheet("");
  }
}

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

namespace {
struct PopupProperties
{
  Qt::Alignment alignment;
  Qt::Orientation orientation;
  ctkBasePopupWidget::VerticalDirection verticalDirection;
  Qt::LayoutDirection horizontalDirection;
};

constexpr PopupProperties bottomOutHorizontal{Qt::AlignBottom | Qt::AlignHCenter, Qt::Vertical, ctkBasePopupWidget::VerticalDirection::TopToBottom, Qt::LayoutDirectionAuto};
constexpr PopupProperties topOutHorizontal{Qt::AlignTop | Qt::AlignHCenter, Qt::Vertical, ctkBasePopupWidget::VerticalDirection::BottomToTop, Qt::LayoutDirectionAuto};
constexpr PopupProperties leftOutVertical{Qt::AlignLeft | Qt::AlignVCenter, Qt::Horizontal, ctkBasePopupWidget::VerticalDirection::TopToBottom, Qt::LayoutDirectionAuto};
constexpr PopupProperties rightOutVertical{Qt::AlignRight | Qt::AlignVCenter, Qt::Horizontal, ctkBasePopupWidget::VerticalDirection::TopToBottom, Qt::LeftToRight};

PopupProperties popupPropertiesFor(Qt::Orientation toolbarOrientation, Qt::ToolBarArea area, bool flipped)
{
  switch (toolbarOrientation)
  {
  case Qt::Horizontal:
  {
    switch (area)
    {
    case Qt::BottomToolBarArea: return flipped ? topOutHorizontal : bottomOutHorizontal;
    default: return flipped ? bottomOutHorizontal : topOutHorizontal;
    }
  }
  case Qt::Vertical:
  {
    switch (area)
    {
    case Qt::LeftToolBarArea: return flipped ? rightOutVertical : leftOutVertical;
    default: return flipped ? leftOutVertical : rightOutVertical;
    }
  }
  }
  return {};
}

QStyle::StandardPixmap oppositeArrow(const QPushButton* button)
{
  switch (static_cast<QStyle::StandardPixmap>(button->property(ViewSceneVtkToolBarController::DirectionProperty).toInt()))
  {
  case QStyle::SP_ArrowRight: return QStyle::SP_ArrowLeft;
  case QStyle::SP_ArrowLeft: return QStyle::SP_ArrowRight;
  case QStyle::SP_ArrowUp: return QStyle::SP_ArrowDown;
  case QStyle::SP_ArrowDown: return QStyle::SP_ArrowUp;
  default: return QStyle::SP_BrowserStop;
  }
}

QStyle::StandardPixmap outArrowForBarAt(Qt::ToolBarArea area)
{
  switch (area)
  {
  case Qt::LeftToolBarArea: return QStyle::SP_ArrowLeft;
  case Qt::RightToolBarArea: return QStyle::SP_ArrowRight;
  case Qt::BottomToolBarArea: return QStyle::SP_ArrowDown;
  case Qt::TopToolBarArea: return QStyle::SP_ArrowUp;
  default: return QStyle::SP_BrowserStop;
  }
}

}

ViewSceneVtkToolBarController::ViewSceneVtkToolBarController(ViewSceneVtkDialog* dialog) : QObject(dialog), dialog_(dialog)
{
  SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay.connectValueChanged([this](int) { updateDelays(); });
  SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay.connectValueChanged([this](int) { updateDelays(); });
}

void ViewSceneVtkToolBarController::setDefaultProperties(QToolBar* toolbar, ctkPopupWidget* popup)
{
  updatePopupProperties(toolbar, popup, false);

  popup->setShowDelay(SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay);
  popup->setHideDelay(SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay);
}

void ViewSceneVtkToolBarController::updateDelays()
{
  for (const auto& [toolbar, popups] : toolBarPopups_)
  {
    for (auto& popup : popups)
    {
      popup->setShowDelay(SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay);
      popup->setHideDelay(SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay);
    }
  }
}

void ViewSceneVtkToolBarController::registerPopup(QToolBar* toolbar, ctkPopupWidget* popup)
{
  connect(toolbar, &QToolBar::orientationChanged, [this, popup, toolbar](Qt::Orientation /*orientation*/) { updatePopupProperties(toolbar, popup, false); });
  connect(toolbar, &QToolBar::topLevelChanged, [this, popup, toolbar](bool /*topLevel*/) { updatePopupProperties(toolbar, popup, false); });
  connect(dialog_, &ViewSceneVtkDialog::fullScreenChanged, [this, popup, toolbar]() { updatePopupProperties(toolbar, popup, false); });
  toolBarPopups_[toolbar].push_back(popup);
}

void ViewSceneVtkToolBarController::updatePopupProperties(QToolBar* toolbar, ctkPopupWidget* popup, bool flipped)
{
  const auto props = popupPropertiesFor(toolbar->orientation(), dialog_->whereIs(toolbar), dialog_->isFullScreen() || flipped);
  popup->setAlignment(props.alignment);
  popup->setOrientation(props.orientation);
  popup->setVerticalDirection(props.verticalDirection);
  popup->setHorizontalDirection(props.horizontalDirection);
}

void ViewSceneVtkToolBarController::registerDirectionButton(QToolBar* toolbar, QPushButton* button)
{
  button->setProperty(FlipProperty, true);

  auto arrow = outArrowForBarAt(dialog_->whereIs(toolbar));
  button->setIcon(QApplication::style()->standardIcon(arrow));
  button->setProperty(DirectionProperty, static_cast<int>(arrow));

  connect(button, &QPushButton::clicked, [button, toolbar, this]() {
    const auto opp = oppositeArrow(button);
    button->setIcon(QApplication::style()->standardIcon(opp));
    button->setProperty(DirectionProperty, static_cast<int>(opp));
    bool flip = button->property(FlipProperty).toBool();
    for (auto& pop : toolBarPopups_[toolbar])
      updatePopupProperties(toolbar, pop, flip);
    button->setProperty(FlipProperty, !flip);
  });

  connect(toolbar, &QToolBar::topLevelChanged, [button, toolbar, this](bool /*topLevel*/) {
    button->setProperty(FlipProperty, true);
    auto outArrow = outArrowForBarAt(dialog_->whereIs(toolbar));
    button->setIcon(QApplication::style()->standardIcon(outArrow));
    button->setProperty(DirectionProperty, static_cast<int>(outArrow));
  });
}
