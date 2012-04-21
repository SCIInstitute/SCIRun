/********************************************************************************
** Form generated from reading UI file 'ModuleDialogBasic.ui'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODULEDIALOGBASIC_H
#define UI_MODULEDIALOGBASIC_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ModuleDialogBasic
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *executionTimeLabel_;
    QSlider *executionTimeHorizontalSlider_;
    QSpinBox *executionTimeSpinBox_;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ModuleDialogBasic)
    {
        if (ModuleDialogBasic->objectName().isEmpty())
            ModuleDialogBasic->setObjectName(QString::fromUtf8("ModuleDialogBasic"));
        ModuleDialogBasic->resize(435, 75);
        widget = new QWidget(ModuleDialogBasic);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 10, 415, 62));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        executionTimeLabel_ = new QLabel(widget);
        executionTimeLabel_->setObjectName(QString::fromUtf8("executionTimeLabel_"));

        horizontalLayout->addWidget(executionTimeLabel_);

        executionTimeHorizontalSlider_ = new QSlider(widget);
        executionTimeHorizontalSlider_->setObjectName(QString::fromUtf8("executionTimeHorizontalSlider_"));
        executionTimeHorizontalSlider_->setMaximum(5000);
        executionTimeHorizontalSlider_->setSingleStep(10);
        executionTimeHorizontalSlider_->setPageStep(250);
        executionTimeHorizontalSlider_->setOrientation(Qt::Horizontal);
        executionTimeHorizontalSlider_->setTickPosition(QSlider::TicksBelow);

        horizontalLayout->addWidget(executionTimeHorizontalSlider_);

        executionTimeSpinBox_ = new QSpinBox(widget);
        executionTimeSpinBox_->setObjectName(QString::fromUtf8("executionTimeSpinBox_"));
        executionTimeSpinBox_->setReadOnly(false);
        executionTimeSpinBox_->setMaximum(5000);

        horizontalLayout->addWidget(executionTimeSpinBox_);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(widget);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok|QDialogButtonBox::Reset|QDialogButtonBox::RestoreDefaults);
        buttonBox->setCenterButtons(false);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ModuleDialogBasic);
        QObject::connect(buttonBox, SIGNAL(accepted()), ModuleDialogBasic, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ModuleDialogBasic, SLOT(reject()));
        QObject::connect(executionTimeHorizontalSlider_, SIGNAL(valueChanged(int)), executionTimeSpinBox_, SLOT(setValue(int)));
        QObject::connect(executionTimeSpinBox_, SIGNAL(valueChanged(int)), executionTimeHorizontalSlider_, SLOT(setValue(int)));

        QMetaObject::connectSlotsByName(ModuleDialogBasic);
    } // setupUi

    void retranslateUi(QDialog *ModuleDialogBasic)
    {
        ModuleDialogBasic->setWindowTitle(QApplication::translate("ModuleDialogBasic", "Dialog", 0, QApplication::UnicodeUTF8));
        executionTimeLabel_->setText(QApplication::translate("ModuleDialogBasic", "Execution Time (ms)", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ModuleDialogBasic: public Ui_ModuleDialogBasic {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODULEDIALOGBASIC_H
