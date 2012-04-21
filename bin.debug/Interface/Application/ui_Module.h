/********************************************************************************
** Form generated from reading UI file 'Module.ui'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODULE_H
#define UI_MODULE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Module
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLabel *titleLabel_;
    QHBoxLayout *horizontalLayout;
    QPushButton *optionsButton_;
    QProgressBar *progressBar_;
    QToolButton *helpButton_;
    QToolButton *logButton_;

    void setupUi(QFrame *Module)
    {
        if (Module->objectName().isEmpty())
            Module->setObjectName(QString::fromUtf8("Module"));
        Module->resize(184, 92);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Module->sizePolicy().hasHeightForWidth());
        Module->setSizePolicy(sizePolicy);
        Module->setAutoFillBackground(false);
        Module->setFrameShape(QFrame::WinPanel);
        Module->setFrameShadow(QFrame::Raised);
        Module->setLineWidth(5);
        Module->setMidLineWidth(0);
        layoutWidget = new QWidget(Module);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 10, 167, 71));
        verticalLayout_2 = new QVBoxLayout(layoutWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(-1, 0, -1, 0);
        titleLabel_ = new QLabel(layoutWidget);
        titleLabel_->setObjectName(QString::fromUtf8("titleLabel_"));
        titleLabel_->setLineWidth(-4);

        verticalLayout->addWidget(titleLabel_);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        optionsButton_ = new QPushButton(layoutWidget);
        optionsButton_->setObjectName(QString::fromUtf8("optionsButton_"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(optionsButton_->sizePolicy().hasHeightForWidth());
        optionsButton_->setSizePolicy(sizePolicy1);
        optionsButton_->setMaximumSize(QSize(23, 23));

        horizontalLayout->addWidget(optionsButton_);

        progressBar_ = new QProgressBar(layoutWidget);
        progressBar_->setObjectName(QString::fromUtf8("progressBar_"));
        progressBar_->setValue(24);
        progressBar_->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        progressBar_->setOrientation(Qt::Horizontal);
        progressBar_->setTextDirection(QProgressBar::TopToBottom);

        horizontalLayout->addWidget(progressBar_);

        helpButton_ = new QToolButton(layoutWidget);
        helpButton_->setObjectName(QString::fromUtf8("helpButton_"));

        horizontalLayout->addWidget(helpButton_);

        logButton_ = new QToolButton(layoutWidget);
        logButton_->setObjectName(QString::fromUtf8("logButton_"));
        QFont font;
        font.setFamily(QString::fromUtf8("MS Shell Dlg 2"));
        font.setPointSize(8);
        logButton_->setFont(font);

        horizontalLayout->addWidget(logButton_);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(Module);

        QMetaObject::connectSlotsByName(Module);
    } // setupUi

    void retranslateUi(QFrame *Module)
    {
        Module->setWindowTitle(QApplication::translate("Module", "Frame", 0, QApplication::UnicodeUTF8));
        titleLabel_->setText(QApplication::translate("Module", "<b><h2>ModuleTitle</h2></b>", 0, QApplication::UnicodeUTF8));
        optionsButton_->setText(QApplication::translate("Module", "UI", 0, QApplication::UnicodeUTF8));
        helpButton_->setText(QApplication::translate("Module", "?", 0, QApplication::UnicodeUTF8));
        logButton_->setText(QApplication::translate("Module", "i", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Module: public Ui_Module {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODULE_H
