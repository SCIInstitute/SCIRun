/********************************************************************************
** Form generated from reading UI file 'SCIRunMainWindow.ui'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCIRUNMAINWINDOW_H
#define UI_SCIRUNMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QScrollArea>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SCIRunMainWindow
{
public:
    QAction *actionLoad_;
    QAction *actionInsert_;
    QAction *actionSave_;
    QAction *actionSave_As_;
    QAction *actionClear_Network_;
    QAction *actionSelect_All_;
    QAction *actionExecute_All_;
    QAction *actionCreate_Module_Skeleton_;
    QAction *actionQuit_;
    QAction *actionShow_Tooltips_;
    QAction *actionAbout_;
    QAction *actionCredits_;
    QAction *actionLicense_;
    QAction *actionConfiguration_;
    QAction *actionModule_Selector;
    QWidget *centralWidget_;
    QGridLayout *gridLayout_2;
    QScrollArea *scrollArea_;
    QWidget *scrollAreaWidgetContents_;
    QGridLayout *gridLayout_5;
    QMenuBar *menubar_;
    QMenu *menuFile_;
    QMenu *menuModules_;
    QMenu *menuSubnets_;
    QMenu *menuToolkits_;
    QMenu *menuHelp_;
    QMenu *menuWindow;
    QStatusBar *statusbar_;
    QDockWidget *moduleSelectorDockWidget_;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *moduleFilterLabel_;
    QLineEdit *moduleFilterLineEdit_;
    QTreeWidget *moduleSelectorTreeWidget_;
    QDockWidget *configurationDockWidget_;
    QWidget *dockWidgetContents_4;
    QGridLayout *gridLayout;
    QTabWidget *optionsTabWidget_;
    QWidget *networkEditorTab_;
    QGroupBox *layoutOptionsGroupBox_;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *moduleSizeHorizontalLayout_;
    QLabel *moduleSizeLabel_;
    QRadioButton *largeModuleSizeRadioButton_;
    QRadioButton *smallModuleSizeRadioButton_;
    QHBoxLayout *dataflowPipelinesHorizontalLayout_;
    QLabel *pipelinesLabel_;
    QRadioButton *straightPipelinesRadioButton_;
    QRadioButton *jaggedPipelinesRadioButton_;
    QWidget *optionsTab_;
    QGridLayout *gridLayout_4;
    QHBoxLayout *optionsTabHorizontalLayout_;
    QGroupBox *pathSettingsGroupBox_;
    QWidget *layoutWidget1;
    QGridLayout *pathSettingsGridLayout_;
    QLabel *scirunDataLabel_;
    QLineEdit *scirunDataLineEdit_;
    QPushButton *scirunDataPushButton_;
    QLabel *scirunNetsLabel_;
    QLineEdit *scirunNetsLineEdit_;
    QPushButton *scirunNetsPushButton_;
    QLabel *userDataLabel_;
    QLineEdit *userDataLineEdit_;
    QPushButton *userDataPushButton_;
    QGroupBox *dataSetGroupBox_;
    QWidget *layoutWidget2;
    QHBoxLayout *dataSetHorizontalLayout_;
    QLabel *dataSetLabel_;
    QLineEdit *dataSetLineEdit_;
    QPushButton *dataSetPushButton_;
    QGroupBox *optionsGroupBox_;
    QWidget *logTab;
    QGridLayout *gridLayout_3;
    QTextBrowser *logTextBrowser_;
    QButtonGroup *moduleSizeButtonGroup_;
    QButtonGroup *pipelinesButtonGroup_;

    void setupUi(QMainWindow *SCIRunMainWindow)
    {
        if (SCIRunMainWindow->objectName().isEmpty())
            SCIRunMainWindow->setObjectName(QString::fromUtf8("SCIRunMainWindow"));
        SCIRunMainWindow->resize(1119, 998);
        QIcon icon;
        icon.addFile(QString::fromUtf8("scirun-icon-small.ico"), QSize(), QIcon::Normal, QIcon::Off);
        SCIRunMainWindow->setWindowIcon(icon);
        actionLoad_ = new QAction(SCIRunMainWindow);
        actionLoad_->setObjectName(QString::fromUtf8("actionLoad_"));
        actionInsert_ = new QAction(SCIRunMainWindow);
        actionInsert_->setObjectName(QString::fromUtf8("actionInsert_"));
        actionSave_ = new QAction(SCIRunMainWindow);
        actionSave_->setObjectName(QString::fromUtf8("actionSave_"));
        actionSave_As_ = new QAction(SCIRunMainWindow);
        actionSave_As_->setObjectName(QString::fromUtf8("actionSave_As_"));
        actionClear_Network_ = new QAction(SCIRunMainWindow);
        actionClear_Network_->setObjectName(QString::fromUtf8("actionClear_Network_"));
        actionSelect_All_ = new QAction(SCIRunMainWindow);
        actionSelect_All_->setObjectName(QString::fromUtf8("actionSelect_All_"));
        actionExecute_All_ = new QAction(SCIRunMainWindow);
        actionExecute_All_->setObjectName(QString::fromUtf8("actionExecute_All_"));
        actionCreate_Module_Skeleton_ = new QAction(SCIRunMainWindow);
        actionCreate_Module_Skeleton_->setObjectName(QString::fromUtf8("actionCreate_Module_Skeleton_"));
        actionQuit_ = new QAction(SCIRunMainWindow);
        actionQuit_->setObjectName(QString::fromUtf8("actionQuit_"));
        actionShow_Tooltips_ = new QAction(SCIRunMainWindow);
        actionShow_Tooltips_->setObjectName(QString::fromUtf8("actionShow_Tooltips_"));
        actionShow_Tooltips_->setCheckable(true);
        actionShow_Tooltips_->setChecked(true);
        actionAbout_ = new QAction(SCIRunMainWindow);
        actionAbout_->setObjectName(QString::fromUtf8("actionAbout_"));
        actionCredits_ = new QAction(SCIRunMainWindow);
        actionCredits_->setObjectName(QString::fromUtf8("actionCredits_"));
        actionLicense_ = new QAction(SCIRunMainWindow);
        actionLicense_->setObjectName(QString::fromUtf8("actionLicense_"));
        actionConfiguration_ = new QAction(SCIRunMainWindow);
        actionConfiguration_->setObjectName(QString::fromUtf8("actionConfiguration_"));
        actionConfiguration_->setCheckable(true);
        actionConfiguration_->setChecked(true);
        actionModule_Selector = new QAction(SCIRunMainWindow);
        actionModule_Selector->setObjectName(QString::fromUtf8("actionModule_Selector"));
        actionModule_Selector->setCheckable(true);
        actionModule_Selector->setChecked(true);
        centralWidget_ = new QWidget(SCIRunMainWindow);
        centralWidget_->setObjectName(QString::fromUtf8("centralWidget_"));
        centralWidget_->setAcceptDrops(true);
        gridLayout_2 = new QGridLayout(centralWidget_);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        scrollArea_ = new QScrollArea(centralWidget_);
        scrollArea_->setObjectName(QString::fromUtf8("scrollArea_"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(scrollArea_->sizePolicy().hasHeightForWidth());
        scrollArea_->setSizePolicy(sizePolicy);
        scrollArea_->setAcceptDrops(true);
        scrollArea_->setWidgetResizable(true);
        scrollAreaWidgetContents_ = new QWidget();
        scrollAreaWidgetContents_->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_"));
        scrollAreaWidgetContents_->setGeometry(QRect(0, 0, 821, 662));
        scrollAreaWidgetContents_->setAcceptDrops(true);
        gridLayout_5 = new QGridLayout(scrollAreaWidgetContents_);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        scrollArea_->setWidget(scrollAreaWidgetContents_);

        gridLayout_2->addWidget(scrollArea_, 0, 0, 1, 1);

        SCIRunMainWindow->setCentralWidget(centralWidget_);
        menubar_ = new QMenuBar(SCIRunMainWindow);
        menubar_->setObjectName(QString::fromUtf8("menubar_"));
        menubar_->setGeometry(QRect(0, 0, 1119, 21));
        menuFile_ = new QMenu(menubar_);
        menuFile_->setObjectName(QString::fromUtf8("menuFile_"));
        menuModules_ = new QMenu(menubar_);
        menuModules_->setObjectName(QString::fromUtf8("menuModules_"));
        menuSubnets_ = new QMenu(menubar_);
        menuSubnets_->setObjectName(QString::fromUtf8("menuSubnets_"));
        menuToolkits_ = new QMenu(menubar_);
        menuToolkits_->setObjectName(QString::fromUtf8("menuToolkits_"));
        menuHelp_ = new QMenu(menubar_);
        menuHelp_->setObjectName(QString::fromUtf8("menuHelp_"));
        menuWindow = new QMenu(menubar_);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        SCIRunMainWindow->setMenuBar(menubar_);
        statusbar_ = new QStatusBar(SCIRunMainWindow);
        statusbar_->setObjectName(QString::fromUtf8("statusbar_"));
        SCIRunMainWindow->setStatusBar(statusbar_);
        moduleSelectorDockWidget_ = new QDockWidget(SCIRunMainWindow);
        moduleSelectorDockWidget_->setObjectName(QString::fromUtf8("moduleSelectorDockWidget_"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(moduleSelectorDockWidget_->sizePolicy().hasHeightForWidth());
        moduleSelectorDockWidget_->setSizePolicy(sizePolicy1);
        moduleSelectorDockWidget_->setMinimumSize(QSize(220, 200));
        moduleSelectorDockWidget_->setFeatures(QDockWidget::AllDockWidgetFeatures);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        moduleFilterLabel_ = new QLabel(dockWidgetContents);
        moduleFilterLabel_->setObjectName(QString::fromUtf8("moduleFilterLabel_"));

        horizontalLayout->addWidget(moduleFilterLabel_);

        moduleFilterLineEdit_ = new QLineEdit(dockWidgetContents);
        moduleFilterLineEdit_->setObjectName(QString::fromUtf8("moduleFilterLineEdit_"));

        horizontalLayout->addWidget(moduleFilterLineEdit_);


        verticalLayout_2->addLayout(horizontalLayout);

        moduleSelectorTreeWidget_ = new QTreeWidget(dockWidgetContents);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem(moduleSelectorTreeWidget_);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem(__qtreewidgetitem);
        new QTreeWidgetItem(__qtreewidgetitem1);
        QTreeWidgetItem *__qtreewidgetitem2 = new QTreeWidgetItem(__qtreewidgetitem);
        new QTreeWidgetItem(__qtreewidgetitem2);
        new QTreeWidgetItem(__qtreewidgetitem2);
        moduleSelectorTreeWidget_->setObjectName(QString::fromUtf8("moduleSelectorTreeWidget_"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(1);
        sizePolicy2.setVerticalStretch(1);
        sizePolicy2.setHeightForWidth(moduleSelectorTreeWidget_->sizePolicy().hasHeightForWidth());
        moduleSelectorTreeWidget_->setSizePolicy(sizePolicy2);
        moduleSelectorTreeWidget_->setMinimumSize(QSize(200, 100));
        moduleSelectorTreeWidget_->setAutoFillBackground(false);
        moduleSelectorTreeWidget_->setDragDropMode(QAbstractItemView::DragOnly);
        moduleSelectorTreeWidget_->setDefaultDropAction(Qt::IgnoreAction);
        moduleSelectorTreeWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
        moduleSelectorTreeWidget_->setAnimated(true);

        verticalLayout_2->addWidget(moduleSelectorTreeWidget_);

        moduleSelectorDockWidget_->setWidget(dockWidgetContents);
        SCIRunMainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), moduleSelectorDockWidget_);
        configurationDockWidget_ = new QDockWidget(SCIRunMainWindow);
        configurationDockWidget_->setObjectName(QString::fromUtf8("configurationDockWidget_"));
        dockWidgetContents_4 = new QWidget();
        dockWidgetContents_4->setObjectName(QString::fromUtf8("dockWidgetContents_4"));
        gridLayout = new QGridLayout(dockWidgetContents_4);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        optionsTabWidget_ = new QTabWidget(dockWidgetContents_4);
        optionsTabWidget_->setObjectName(QString::fromUtf8("optionsTabWidget_"));
        optionsTabWidget_->setMinimumSize(QSize(0, 200));
        optionsTabWidget_->setTabPosition(QTabWidget::North);
        optionsTabWidget_->setTabShape(QTabWidget::Triangular);
        networkEditorTab_ = new QWidget();
        networkEditorTab_->setObjectName(QString::fromUtf8("networkEditorTab_"));
        layoutOptionsGroupBox_ = new QGroupBox(networkEditorTab_);
        layoutOptionsGroupBox_->setObjectName(QString::fromUtf8("layoutOptionsGroupBox_"));
        layoutOptionsGroupBox_->setGeometry(QRect(10, 10, 261, 81));
        layoutOptionsGroupBox_->setAlignment(Qt::AlignCenter);
        layoutWidget = new QWidget(layoutOptionsGroupBox_);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 20, 226, 46));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        moduleSizeHorizontalLayout_ = new QHBoxLayout();
        moduleSizeHorizontalLayout_->setObjectName(QString::fromUtf8("moduleSizeHorizontalLayout_"));
        moduleSizeLabel_ = new QLabel(layoutWidget);
        moduleSizeLabel_->setObjectName(QString::fromUtf8("moduleSizeLabel_"));

        moduleSizeHorizontalLayout_->addWidget(moduleSizeLabel_);

        largeModuleSizeRadioButton_ = new QRadioButton(layoutWidget);
        moduleSizeButtonGroup_ = new QButtonGroup(SCIRunMainWindow);
        moduleSizeButtonGroup_->setObjectName(QString::fromUtf8("moduleSizeButtonGroup_"));
        moduleSizeButtonGroup_->addButton(largeModuleSizeRadioButton_);
        largeModuleSizeRadioButton_->setObjectName(QString::fromUtf8("largeModuleSizeRadioButton_"));

        moduleSizeHorizontalLayout_->addWidget(largeModuleSizeRadioButton_);

        smallModuleSizeRadioButton_ = new QRadioButton(layoutWidget);
        moduleSizeButtonGroup_->addButton(smallModuleSizeRadioButton_);
        smallModuleSizeRadioButton_->setObjectName(QString::fromUtf8("smallModuleSizeRadioButton_"));

        moduleSizeHorizontalLayout_->addWidget(smallModuleSizeRadioButton_);


        verticalLayout->addLayout(moduleSizeHorizontalLayout_);

        dataflowPipelinesHorizontalLayout_ = new QHBoxLayout();
        dataflowPipelinesHorizontalLayout_->setObjectName(QString::fromUtf8("dataflowPipelinesHorizontalLayout_"));
        pipelinesLabel_ = new QLabel(layoutWidget);
        pipelinesLabel_->setObjectName(QString::fromUtf8("pipelinesLabel_"));

        dataflowPipelinesHorizontalLayout_->addWidget(pipelinesLabel_);

        straightPipelinesRadioButton_ = new QRadioButton(layoutWidget);
        pipelinesButtonGroup_ = new QButtonGroup(SCIRunMainWindow);
        pipelinesButtonGroup_->setObjectName(QString::fromUtf8("pipelinesButtonGroup_"));
        pipelinesButtonGroup_->addButton(straightPipelinesRadioButton_);
        straightPipelinesRadioButton_->setObjectName(QString::fromUtf8("straightPipelinesRadioButton_"));

        dataflowPipelinesHorizontalLayout_->addWidget(straightPipelinesRadioButton_);

        jaggedPipelinesRadioButton_ = new QRadioButton(layoutWidget);
        pipelinesButtonGroup_->addButton(jaggedPipelinesRadioButton_);
        jaggedPipelinesRadioButton_->setObjectName(QString::fromUtf8("jaggedPipelinesRadioButton_"));

        dataflowPipelinesHorizontalLayout_->addWidget(jaggedPipelinesRadioButton_);


        verticalLayout->addLayout(dataflowPipelinesHorizontalLayout_);

        optionsTabWidget_->addTab(networkEditorTab_, QString());
        optionsTab_ = new QWidget();
        optionsTab_->setObjectName(QString::fromUtf8("optionsTab_"));
        gridLayout_4 = new QGridLayout(optionsTab_);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        optionsTabHorizontalLayout_ = new QHBoxLayout();
        optionsTabHorizontalLayout_->setObjectName(QString::fromUtf8("optionsTabHorizontalLayout_"));
        pathSettingsGroupBox_ = new QGroupBox(optionsTab_);
        pathSettingsGroupBox_->setObjectName(QString::fromUtf8("pathSettingsGroupBox_"));
        pathSettingsGroupBox_->setAlignment(Qt::AlignCenter);
        layoutWidget1 = new QWidget(pathSettingsGroupBox_);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 30, 261, 111));
        pathSettingsGridLayout_ = new QGridLayout(layoutWidget1);
        pathSettingsGridLayout_->setObjectName(QString::fromUtf8("pathSettingsGridLayout_"));
        pathSettingsGridLayout_->setContentsMargins(0, 0, 0, 0);
        scirunDataLabel_ = new QLabel(layoutWidget1);
        scirunDataLabel_->setObjectName(QString::fromUtf8("scirunDataLabel_"));

        pathSettingsGridLayout_->addWidget(scirunDataLabel_, 0, 0, 1, 1);

        scirunDataLineEdit_ = new QLineEdit(layoutWidget1);
        scirunDataLineEdit_->setObjectName(QString::fromUtf8("scirunDataLineEdit_"));

        pathSettingsGridLayout_->addWidget(scirunDataLineEdit_, 0, 1, 1, 1);

        scirunDataPushButton_ = new QPushButton(layoutWidget1);
        scirunDataPushButton_->setObjectName(QString::fromUtf8("scirunDataPushButton_"));

        pathSettingsGridLayout_->addWidget(scirunDataPushButton_, 0, 2, 1, 1);

        scirunNetsLabel_ = new QLabel(layoutWidget1);
        scirunNetsLabel_->setObjectName(QString::fromUtf8("scirunNetsLabel_"));

        pathSettingsGridLayout_->addWidget(scirunNetsLabel_, 1, 0, 1, 1);

        scirunNetsLineEdit_ = new QLineEdit(layoutWidget1);
        scirunNetsLineEdit_->setObjectName(QString::fromUtf8("scirunNetsLineEdit_"));

        pathSettingsGridLayout_->addWidget(scirunNetsLineEdit_, 1, 1, 1, 1);

        scirunNetsPushButton_ = new QPushButton(layoutWidget1);
        scirunNetsPushButton_->setObjectName(QString::fromUtf8("scirunNetsPushButton_"));

        pathSettingsGridLayout_->addWidget(scirunNetsPushButton_, 1, 2, 1, 1);

        userDataLabel_ = new QLabel(layoutWidget1);
        userDataLabel_->setObjectName(QString::fromUtf8("userDataLabel_"));

        pathSettingsGridLayout_->addWidget(userDataLabel_, 2, 0, 1, 1);

        userDataLineEdit_ = new QLineEdit(layoutWidget1);
        userDataLineEdit_->setObjectName(QString::fromUtf8("userDataLineEdit_"));

        pathSettingsGridLayout_->addWidget(userDataLineEdit_, 2, 1, 1, 1);

        userDataPushButton_ = new QPushButton(layoutWidget1);
        userDataPushButton_->setObjectName(QString::fromUtf8("userDataPushButton_"));

        pathSettingsGridLayout_->addWidget(userDataPushButton_, 2, 2, 1, 1);


        optionsTabHorizontalLayout_->addWidget(pathSettingsGroupBox_);

        dataSetGroupBox_ = new QGroupBox(optionsTab_);
        dataSetGroupBox_->setObjectName(QString::fromUtf8("dataSetGroupBox_"));
        dataSetGroupBox_->setAlignment(Qt::AlignCenter);
        layoutWidget2 = new QWidget(dataSetGroupBox_);
        layoutWidget2->setObjectName(QString::fromUtf8("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(20, 30, 241, 25));
        dataSetHorizontalLayout_ = new QHBoxLayout(layoutWidget2);
        dataSetHorizontalLayout_->setObjectName(QString::fromUtf8("dataSetHorizontalLayout_"));
        dataSetHorizontalLayout_->setContentsMargins(0, 0, 0, 0);
        dataSetLabel_ = new QLabel(layoutWidget2);
        dataSetLabel_->setObjectName(QString::fromUtf8("dataSetLabel_"));

        dataSetHorizontalLayout_->addWidget(dataSetLabel_);

        dataSetLineEdit_ = new QLineEdit(layoutWidget2);
        dataSetLineEdit_->setObjectName(QString::fromUtf8("dataSetLineEdit_"));

        dataSetHorizontalLayout_->addWidget(dataSetLineEdit_);

        dataSetPushButton_ = new QPushButton(layoutWidget2);
        dataSetPushButton_->setObjectName(QString::fromUtf8("dataSetPushButton_"));

        dataSetHorizontalLayout_->addWidget(dataSetPushButton_);


        optionsTabHorizontalLayout_->addWidget(dataSetGroupBox_);

        optionsGroupBox_ = new QGroupBox(optionsTab_);
        optionsGroupBox_->setObjectName(QString::fromUtf8("optionsGroupBox_"));
        optionsGroupBox_->setAlignment(Qt::AlignCenter);

        optionsTabHorizontalLayout_->addWidget(optionsGroupBox_);


        gridLayout_4->addLayout(optionsTabHorizontalLayout_, 0, 0, 1, 1);

        optionsTabWidget_->addTab(optionsTab_, QString());
        logTab = new QWidget();
        logTab->setObjectName(QString::fromUtf8("logTab"));
        gridLayout_3 = new QGridLayout(logTab);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        logTextBrowser_ = new QTextBrowser(logTab);
        logTextBrowser_->setObjectName(QString::fromUtf8("logTextBrowser_"));
        logTextBrowser_->setMinimumSize(QSize(0, 192));

        gridLayout_3->addWidget(logTextBrowser_, 0, 0, 1, 1);

        optionsTabWidget_->addTab(logTab, QString());

        gridLayout->addWidget(optionsTabWidget_, 0, 0, 1, 1);

        configurationDockWidget_->setWidget(dockWidgetContents_4);
        SCIRunMainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(8), configurationDockWidget_);

        menubar_->addAction(menuFile_->menuAction());
        menubar_->addAction(menuModules_->menuAction());
        menubar_->addAction(menuSubnets_->menuAction());
        menubar_->addAction(menuToolkits_->menuAction());
        menubar_->addAction(menuWindow->menuAction());
        menubar_->addAction(menuHelp_->menuAction());
        menuFile_->addAction(actionLoad_);
        menuFile_->addAction(actionInsert_);
        menuFile_->addAction(actionSave_);
        menuFile_->addAction(actionSave_As_);
        menuFile_->addSeparator();
        menuFile_->addAction(actionClear_Network_);
        menuFile_->addAction(actionSelect_All_);
        menuFile_->addAction(actionExecute_All_);
        menuFile_->addSeparator();
        menuFile_->addAction(actionCreate_Module_Skeleton_);
        menuFile_->addSeparator();
        menuFile_->addAction(actionQuit_);
        menuHelp_->addAction(actionShow_Tooltips_);
        menuHelp_->addAction(actionAbout_);
        menuHelp_->addAction(actionCredits_);
        menuHelp_->addAction(actionLicense_);
        menuWindow->addAction(actionConfiguration_);
        menuWindow->addAction(actionModule_Selector);

        retranslateUi(SCIRunMainWindow);
        QObject::connect(actionModule_Selector, SIGNAL(toggled(bool)), moduleSelectorDockWidget_, SLOT(setVisible(bool)));
        QObject::connect(actionConfiguration_, SIGNAL(toggled(bool)), configurationDockWidget_, SLOT(setVisible(bool)));

        optionsTabWidget_->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(SCIRunMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *SCIRunMainWindow)
    {
        SCIRunMainWindow->setWindowTitle(QApplication::translate("SCIRunMainWindow", "SCIRun", 0, QApplication::UnicodeUTF8));
        actionLoad_->setText(QApplication::translate("SCIRunMainWindow", "Load...", 0, QApplication::UnicodeUTF8));
        actionInsert_->setText(QApplication::translate("SCIRunMainWindow", "Insert...", 0, QApplication::UnicodeUTF8));
        actionSave_->setText(QApplication::translate("SCIRunMainWindow", "Save", 0, QApplication::UnicodeUTF8));
        actionSave_As_->setText(QApplication::translate("SCIRunMainWindow", "Save As...", 0, QApplication::UnicodeUTF8));
        actionClear_Network_->setText(QApplication::translate("SCIRunMainWindow", "Clear Network", 0, QApplication::UnicodeUTF8));
        actionSelect_All_->setText(QApplication::translate("SCIRunMainWindow", "Select All", 0, QApplication::UnicodeUTF8));
        actionExecute_All_->setText(QApplication::translate("SCIRunMainWindow", "Execute All", 0, QApplication::UnicodeUTF8));
        actionCreate_Module_Skeleton_->setText(QApplication::translate("SCIRunMainWindow", "Create Module Skeleton...", 0, QApplication::UnicodeUTF8));
        actionQuit_->setText(QApplication::translate("SCIRunMainWindow", "Quit", 0, QApplication::UnicodeUTF8));
        actionShow_Tooltips_->setText(QApplication::translate("SCIRunMainWindow", "Show Tooltips", 0, QApplication::UnicodeUTF8));
        actionAbout_->setText(QApplication::translate("SCIRunMainWindow", "About...", 0, QApplication::UnicodeUTF8));
        actionCredits_->setText(QApplication::translate("SCIRunMainWindow", "Credits...", 0, QApplication::UnicodeUTF8));
        actionLicense_->setText(QApplication::translate("SCIRunMainWindow", "License...", 0, QApplication::UnicodeUTF8));
        actionConfiguration_->setText(QApplication::translate("SCIRunMainWindow", "Configuration", 0, QApplication::UnicodeUTF8));
        actionModule_Selector->setText(QApplication::translate("SCIRunMainWindow", "Module Selector", 0, QApplication::UnicodeUTF8));
        menuFile_->setTitle(QApplication::translate("SCIRunMainWindow", "&File", 0, QApplication::UnicodeUTF8));
        menuModules_->setTitle(QApplication::translate("SCIRunMainWindow", "&Modules", 0, QApplication::UnicodeUTF8));
        menuSubnets_->setTitle(QApplication::translate("SCIRunMainWindow", "&Subnets", 0, QApplication::UnicodeUTF8));
        menuToolkits_->setTitle(QApplication::translate("SCIRunMainWindow", "&Toolkits", 0, QApplication::UnicodeUTF8));
        menuHelp_->setTitle(QApplication::translate("SCIRunMainWindow", "&Help", 0, QApplication::UnicodeUTF8));
        menuWindow->setTitle(QApplication::translate("SCIRunMainWindow", "Window", 0, QApplication::UnicodeUTF8));
        moduleSelectorDockWidget_->setWindowTitle(QApplication::translate("SCIRunMainWindow", "Module Selector", 0, QApplication::UnicodeUTF8));
        moduleFilterLabel_->setText(QApplication::translate("SCIRunMainWindow", "Filter:", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem = moduleSelectorTreeWidget_->headerItem();
        ___qtreewidgetitem->setText(1, QApplication::translate("SCIRunMainWindow", "Description", 0, QApplication::UnicodeUTF8));
        ___qtreewidgetitem->setText(0, QApplication::translate("SCIRunMainWindow", "Name", 0, QApplication::UnicodeUTF8));

        const bool __sortingEnabled = moduleSelectorTreeWidget_->isSortingEnabled();
        moduleSelectorTreeWidget_->setSortingEnabled(false);
        QTreeWidgetItem *___qtreewidgetitem1 = moduleSelectorTreeWidget_->topLevelItem(0);
        ___qtreewidgetitem1->setText(0, QApplication::translate("SCIRunMainWindow", "SCIRun", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem2 = ___qtreewidgetitem1->child(0);
        ___qtreewidgetitem2->setText(0, QApplication::translate("SCIRunMainWindow", "Math", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem3 = ___qtreewidgetitem2->child(0);
        ___qtreewidgetitem3->setText(0, QApplication::translate("SCIRunMainWindow", "ComputeSVD", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem4 = ___qtreewidgetitem1->child(1);
        ___qtreewidgetitem4->setText(0, QApplication::translate("SCIRunMainWindow", "DataIO", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem5 = ___qtreewidgetitem4->child(0);
        ___qtreewidgetitem5->setText(0, QApplication::translate("SCIRunMainWindow", "ReadMatrix", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem6 = ___qtreewidgetitem4->child(1);
        ___qtreewidgetitem6->setText(0, QApplication::translate("SCIRunMainWindow", "WriteMatrix", 0, QApplication::UnicodeUTF8));
        moduleSelectorTreeWidget_->setSortingEnabled(__sortingEnabled);

        configurationDockWidget_->setWindowTitle(QApplication::translate("SCIRunMainWindow", "Configuration", 0, QApplication::UnicodeUTF8));
        layoutOptionsGroupBox_->setTitle(QApplication::translate("SCIRunMainWindow", "Layout Options", 0, QApplication::UnicodeUTF8));
        moduleSizeLabel_->setText(QApplication::translate("SCIRunMainWindow", "Module Size:", 0, QApplication::UnicodeUTF8));
        largeModuleSizeRadioButton_->setText(QApplication::translate("SCIRunMainWindow", "Large", 0, QApplication::UnicodeUTF8));
        smallModuleSizeRadioButton_->setText(QApplication::translate("SCIRunMainWindow", "Small", 0, QApplication::UnicodeUTF8));
        pipelinesLabel_->setText(QApplication::translate("SCIRunMainWindow", "Dataflow Pipelines:", 0, QApplication::UnicodeUTF8));
        straightPipelinesRadioButton_->setText(QApplication::translate("SCIRunMainWindow", "Straight", 0, QApplication::UnicodeUTF8));
        jaggedPipelinesRadioButton_->setText(QApplication::translate("SCIRunMainWindow", "Jagged", 0, QApplication::UnicodeUTF8));
        optionsTabWidget_->setTabText(optionsTabWidget_->indexOf(networkEditorTab_), QApplication::translate("SCIRunMainWindow", "Network Editor", 0, QApplication::UnicodeUTF8));
        pathSettingsGroupBox_->setTitle(QApplication::translate("SCIRunMainWindow", "SCIRun Path Settings*", 0, QApplication::UnicodeUTF8));
        scirunDataLabel_->setText(QApplication::translate("SCIRunMainWindow", "SCIRun Data", 0, QApplication::UnicodeUTF8));
        scirunDataPushButton_->setText(QApplication::translate("SCIRunMainWindow", "Set", 0, QApplication::UnicodeUTF8));
        scirunNetsLabel_->setText(QApplication::translate("SCIRunMainWindow", "SCIRun Nets", 0, QApplication::UnicodeUTF8));
        scirunNetsPushButton_->setText(QApplication::translate("SCIRunMainWindow", "Set", 0, QApplication::UnicodeUTF8));
        userDataLabel_->setText(QApplication::translate("SCIRunMainWindow", "User Data", 0, QApplication::UnicodeUTF8));
        userDataPushButton_->setText(QApplication::translate("SCIRunMainWindow", "Set", 0, QApplication::UnicodeUTF8));
        dataSetGroupBox_->setTitle(QApplication::translate("SCIRunMainWindow", "SCIRun Data Set*", 0, QApplication::UnicodeUTF8));
        dataSetLabel_->setText(QApplication::translate("SCIRunMainWindow", "Data Set", 0, QApplication::UnicodeUTF8));
        dataSetPushButton_->setText(QApplication::translate("SCIRunMainWindow", "Set", 0, QApplication::UnicodeUTF8));
        optionsGroupBox_->setTitle(QApplication::translate("SCIRunMainWindow", "SCIRun Options", 0, QApplication::UnicodeUTF8));
        optionsTabWidget_->setTabText(optionsTabWidget_->indexOf(optionsTab_), QApplication::translate("SCIRunMainWindow", "Options*", 0, QApplication::UnicodeUTF8));
        optionsTabWidget_->setTabText(optionsTabWidget_->indexOf(logTab), QApplication::translate("SCIRunMainWindow", "Log", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SCIRunMainWindow: public Ui_SCIRunMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCIRUNMAINWINDOW_H
