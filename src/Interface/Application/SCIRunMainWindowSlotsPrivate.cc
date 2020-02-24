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


#include <es-log/trace-log.h>
#include <QtGui>
#include <functional>
#include <boost/bind.hpp>
#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/algorithm/string.hpp>
#include <Core/Utils/Legacy/MemoryUtil.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/DeveloperConsole.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/PreferencesWindow.h>
#include <Interface/Application/TagManagerWindow.h>
#include <Interface/Application/ShortcutsInterface.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Application/MacroEditor.h>
#include <Interface/Application/NetworkEditorControllerGuiProxy.h>
#include <Interface/Application/NetworkExecutionProgressBar.h>
#include <Interface/Application/DialogErrorControl.h>
#include <Interface/Modules/Base/RemembersFileDialogDirectory.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Interface/Application/ModuleWizard/ModuleWizard.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h> //DOH! see TODO in setController
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Dataflow/Network/SimpleSourceSink.h>  //TODO: encapsulate!!!
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Parallel.h>
#include <Core/Application/Version.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Core/Utils/CurrentFileName.h>

#ifdef BUILD_WITH_PYTHON
#include <Interface/Application/PythonConsoleWidget.h>
#include <Core/Python/PythonInterpreter.h>
#endif
#include <Dataflow/Serialization/Network/XMLSerializer.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;


void SCIRunMainWindow::saveNetwork()
{
  if (currentFile_.isEmpty())
    saveNetworkAs();
  else
    saveNetworkFile(currentFile_);
}

void SCIRunMainWindow::saveNetworkAs()
{
  auto filename = QFileDialog::getSaveFileName(this, "Save Network...", latestNetworkDirectory_.path(), "*.srn5");
  if (!filename.isEmpty())
    saveNetworkFile(filename);
}

void SCIRunMainWindow::loadNetwork()
{
  if (okToContinue())
  {
    auto filename = QFileDialog::getOpenFileName(this, "Load Network...", latestNetworkDirectory_.path(), "*.srn5");
    loadNetworkFile(filename);
  }
}

void SCIRunMainWindow::checkAndLoadNetworkFile(const QString& filename)
{
  if (okToContinue())
  {
    loadNetworkFile(filename);
  }
}

void SCIRunMainWindow::importLegacyNetwork()
{
  if (okToContinue())
  {
    auto filename = QFileDialog::getOpenFileName(this, "Import Old Network...", latestNetworkDirectory_.path(), "*.srn");
    importLegacyNetworkFile(filename);
  }
}

bool SCIRunMainWindow::newNetwork()
{
  if (okToContinue())
  {
    networkEditor_->clear();
    provenanceWindow_->clear();
    setCurrentFile("");
		networkEditor_->viewport()->update();
    return true;
  }
  return false;
}

void SCIRunMainWindow::loadRecentNetwork()
{
  if (okToContinue())
  {
    auto action = qobject_cast<QAction*>(sender());
    if (action)
      loadNetworkFile(action->data().toString());
  }
}

//TODO: hook up to modules' state_changed_sig_t via GlobalStateManager
//TODO: pass a boolean here to avoid updating total modules when only connections are made--saves a lock
void SCIRunMainWindow::networkModified()
{
  setWindowModified(true);
  networkProgressBar_->updateTotalModules(networkEditor_->numModules());
}

void SCIRunMainWindow::filterModuleNamesInTreeView(const QString& start)
{
  ShowAll show;
  visitTree(moduleSelectorTreeWidget_, show);

  HideItemsNotMatchingString::SearchType searchType;
  if(filterActionGroup_->checkedAction()->text().contains("Starts with"))
    searchType = HideItemsNotMatchingString::SearchType::STARTS_WITH;
  if(filterActionGroup_->checkedAction()->text().contains("wildcards"))
    searchType = HideItemsNotMatchingString::SearchType::WILDCARDS;
  else if(filterActionGroup_->checkedAction()->text().contains("fuzzy search"))
    searchType = HideItemsNotMatchingString::SearchType::FUZZY_SEARCH;
  else if(filterActionGroup_->checkedAction()->text().contains("Filter UI only"))
    searchType = HideItemsNotMatchingString::SearchType::HIDE_NON_UI;

  HideItemsNotMatchingString func(searchType, start);

  //note: goofy double call, first to hide the leaves, then hide the categories.
  visitTree(moduleSelectorTreeWidget_, func);
  visitTree(moduleSelectorTreeWidget_, func);
}

void SCIRunMainWindow::makePipesCubicBezier()
{
  networkEditor_->setConnectionPipelineType(static_cast<int>(ConnectionDrawType::CUBIC));
}

void SCIRunMainWindow::makePipesEuclidean()
{
  networkEditor_->setConnectionPipelineType(static_cast<int>(ConnectionDrawType::EUCLIDEAN));
}

void SCIRunMainWindow::makePipesManhattan()
{
  networkEditor_->setConnectionPipelineType(static_cast<int>(ConnectionDrawType::MANHATTAN));
}

void SCIRunMainWindow::setDragMode(bool toggle)
{
  if (toggle)
  {
    networkEditor_->setMouseAsDragMode();
    statusBar()->showMessage("Mouse in drag mode", 2000);
  }
  actionSelectMode_->setChecked(!actionDragMode_->isChecked());
}

void SCIRunMainWindow::setSelectMode(bool toggle)
{
  if (toggle)
  {
    networkEditor_->setMouseAsSelectMode();
    statusBar()->showMessage("Mouse in select mode", 2000);
  }
  actionDragMode_->setChecked(!actionSelectMode_->isChecked());
}

void SCIRunMainWindow::switchMouseMode()
{
  if (actionDragMode_->isChecked())
  {
    setSelectMode(true);
    actionSelectMode_->setChecked(true);
  }
  else // select->drag
  {
    setDragMode(true);
    actionDragMode_->setChecked(true);
  }
}

void SCIRunMainWindow::zoomNetwork()
{
  auto action = qobject_cast<QAction*>(sender());
  if (action)
  {
    const auto name = action->text();
    if (name == "Zoom In")
    {
      networkEditor_->zoomIn();
    }
    else if (name == "Zoom Out")
    {
      networkEditor_->zoomOut();
    }
    else if (name == "Reset Network Zoom")
    {
      networkEditor_->zoomReset();
    }
    else if (name == "Zoom Best Fit")
    {
      networkEditor_->zoomBestFit();
    }
  }
  else
  {
    qDebug() << "Sender was null or not an action";
  }
}

void SCIRunMainWindow::filterDoubleClickedModuleSelectorItem(QTreeWidgetItem* item)
{
  if (item && item->childCount() == 0)
    Q_EMIT moduleItemDoubleClicked();
}

void SCIRunMainWindow::setExecutor(int type)
{
  LOG_DEBUG("Executor of type {} selected", type);
  networkEditor_->getNetworkEditorController()->setExecutorType(type);
}

void SCIRunMainWindow::setGlobalPortCaching(bool enable)
{
  LOG_DEBUG("Global port caching flag set to {}", (enable ? "true" : "false"));
  //TODO: encapsulate better
  SimpleSink::setGlobalPortCachingFlag(enable);
}

void SCIRunMainWindow::readDefaultNotePosition(int index)
{
  Q_EMIT defaultNotePositionChanged(defaultNotePositionGetter_->position()); //TODO: unit test.
}

void SCIRunMainWindow::readDefaultNoteSize(int index)
{
  Q_EMIT defaultNoteSizeChanged(defaultNotePositionGetter_->size()); //TODO: unit test.
}

void SCIRunMainWindow::runScript()
{
  if (okToContinue())
  {
    QString filename = QFileDialog::getOpenFileName(this, "Load Script...", latestNetworkDirectory_.path(), "*.py");
    if (!filename.isEmpty())
      runPythonScript(filename);
  }
}

void SCIRunMainWindow::runMacro()
{
#ifdef BUILD_WITH_PYTHON
  auto index = sender()->property(MacroEditor::Index).toInt();
  auto script = macroEditor_->macroForButton(index);
  NetworkEditor::InEditingContext iec(networkEditor_);
  PythonInterpreter::Instance().run_script(script.toStdString());
#endif
}

void SCIRunMainWindow::updateMacroButton(int index, const QString& name)
{
  static std::vector<QAction*> buttons { actionRunMacro1_, actionRunMacro2_, actionRunMacro3_, actionRunMacro4_, actionRunMacro5_ };
  if (index >= MacroEditor::MIN_MACRO_INDEX && index <= MacroEditor::MAX_MACRO_INDEX)
  {
    auto str = tr("Run Macro %0").arg(index);
    if (!name.isEmpty())
      str += ": " + name;
    buttons[index - 1]->setToolTip(str);
  }
}

void SCIRunMainWindow::showModuleSelectorContextMenu(const QPoint& pos)
{
  auto globalPos = moduleSelectorTreeWidget_->mapToGlobal(pos);
	auto item = moduleSelectorTreeWidget_->selectedItems()[0];
	auto subnetData = item->data(0, Qt::UserRole).toString();
  if (saveFragmentData_ == subnetData)
  {
    QMenu menu;
		menu.addAction("Export fragment list...", this, SLOT(exportFragmentList()));
    menu.addAction("Import fragment list...", this, SLOT(importFragmentList()));
    menu.addAction("Clear", this, SLOT(clearFragmentList()));
  	menu.exec(globalPos);
  }
	else if (!subnetData.isEmpty())
	{
  	QMenu menu;
		menu.addAction("Rename", this, SLOT(renameSavedSubnetwork()))->setProperty("ID", subnetData);
		menu.addAction("Delete", this, SLOT(removeSavedSubnetwork()))->setProperty("ID", subnetData);
  	menu.exec(globalPos);
	}
}

void SCIRunMainWindow::clearFragmentList()
{
  savedSubnetworksNames_.clear();
  savedSubnetworksXml_.clear();
  auto menu = getSavedSubnetworksMenu(moduleSelectorTreeWidget_);
  auto count = menu->childCount();
  for (int i = 0; i < count; ++i)
  {
    delete menu->child(0);
  }
}

using NetworkFragmentXMLMap = std::map<std::string, std::pair<std::string, std::string>>;

class NetworkFragmentXML
{
public:
  NetworkFragmentXMLMap fragments;
private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & BOOST_SERIALIZATION_NVP(fragments);
  }
};

void SCIRunMainWindow::importFragmentList()
{
  auto filename = QFileDialog::getOpenFileName(this, "Import Network Fragments...", latestNetworkDirectory_.path(), "*.srn5fragment");
  if (!filename.isEmpty())
  {
    auto frags = XMLSerializer::load_xml<NetworkFragmentXML>(filename.toStdString());
    QMap<QString, QVariant> names, xmls;
    for (const auto& frag : frags->fragments)
    {
      auto key = QString::fromStdString(frag.first);
      names.insert(key, QString::fromStdString(frag.second.first));
      xmls.insert(key, QString::fromStdString(frag.second.second));
    }
    addFragmentsToMenu(names, xmls);
    savedSubnetworksNames_.unite(names);
    savedSubnetworksXml_.unite(xmls);
    showStatusMessage("Fragment list imported: " + filename, 2000);
  }
}

void SCIRunMainWindow::exportFragmentList()
{
  auto filename = QFileDialog::getSaveFileName(this, "Export Network Fragments...", latestNetworkDirectory_.path(), "*.srn5fragment");
  if (!filename.isEmpty())
  {
    NetworkFragmentXML data;
    auto keys = savedSubnetworksNames_.keys();
    for (auto&& tup : zip(savedSubnetworksNames_, savedSubnetworksXml_, keys))
    {
      QVariant name, xml;
      QString key;
      boost::tie(name, xml, key) = tup;
      data.fragments[key.toStdString()] = { name.toString().toStdString(), xml.toString().toStdString() };
    }
    XMLSerializer::save_xml(data, filename.toStdString(), "fragments");
    showStatusMessage("Fragment list exported: " + filename, 2000);
  }
}

void SCIRunMainWindow::handleCheckedModuleEntry(QTreeWidgetItem* item, int column)
{
  if (item && 0 == column)
  {
    moduleSelectorTreeWidget_->setCurrentItem(item);

    auto faves = item->foreground(0) == CLIPBOARD_COLOR ? getSavedSubnetworksMenu(moduleSelectorTreeWidget_) : getFavoriteMenu(moduleSelectorTreeWidget_);

    if (item->checkState(0) == Qt::Checked)
    {
      if (faves)
      {
        auto fave = addFavoriteItem(faves, item);
        faves->sortChildren(0, Qt::AscendingOrder);
        if (item->foreground(0) != CLIPBOARD_COLOR)
          favoriteModuleNames_ << item->text(0);
        else
        {
					setupSubnetItem(fave, true, "");
        }
      }
    }
    else
    {
      if (faves && item->foreground(0) != CLIPBOARD_COLOR)
      {
        favoriteModuleNames_.removeAll(item->text(0));
        for (int i = 0; i < faves->childCount(); ++i)
        {
          auto child = faves->child(i);
          if (child->text(0) == item->text(0))
            faves->removeChild(child);
        }
      }
    }
  }
}

void SCIRunMainWindow::removeSavedSubnetwork()
{
	auto toDelete = sender()->property("ID").toString();
  savedSubnetworksNames_.remove(toDelete);
  savedSubnetworksXml_.remove(toDelete);
	auto tree = getSavedSubnetworksMenu(moduleSelectorTreeWidget_);
	for (int i = 0; i < tree->childCount(); ++i)
	{
		auto subnet = tree->child(i);
		if (toDelete == subnet->data(0, Qt::UserRole).toString())
		{
			delete tree->takeChild(i);
			break;
		}
	}
}

void SCIRunMainWindow::renameSavedSubnetwork()
{
  auto toRename = sender()->property("ID").toString();
  bool ok;
  auto text = QInputDialog::getText(this, tr("Rename fragment"), tr("Enter new fragment name:"), QLineEdit::Normal, savedSubnetworksNames_[toRename].toString(), &ok);
  if (ok && !text.isEmpty())
  {
    savedSubnetworksNames_[toRename] = text;
    auto tree = getSavedSubnetworksMenu(moduleSelectorTreeWidget_);
    for (int i = 0; i < tree->childCount(); ++i)
    {
      auto subnet = tree->child(i);
      if (toRename == subnet->data(0, Qt::UserRole).toString())
      {
				subnet->setText(0, text);
        break;
      }
    }
  }
}

void SCIRunMainWindow::displayAcknowledgement()
{
  QMessageBox::information(this, "NIH/NIGMS Center for Integrative Biomedical Computing Acknowledgment",
    "CIBC software and the data sets provided on this web site are Open Source software projects that are principally funded through the SCI Institute's NIH/NCRR CIBC. For us to secure the funding that allows us to continue providing this software, we must have evidence of its utility. Thus we ask users of our software and data to acknowledge us in their publications and inform us of these publications. Please use the following acknowledgment and send us references to any publications, presentations, or successful funding applications that make use of the NIH/NCRR CIBC software or data sets we provide. <p> <i>This project was supported by the National Institute of General Medical Sciences of the National Institutes of Health under grant number P41GM103545.</i>");
}

void SCIRunMainWindow::helpWithToolkitCreation()
{
  QMessageBox::information(this, "Temp",
    "<b>Help with toolkit creation--for power users</b>"
    "<p> First, gather all network files for your toolkit into a single directory. This directory may contain "
    "one level of subdirectories. Next, "
    "in your build directory (you must SCIRun from source), locate the executable named <code>bundle_toolkit</code>. "
    " The usage is:"
    "<pre>bundle_toolkit OUTPUT_FILE [DIRECTORY_TO_SCAN]</pre>"
    "where OUTPUT_FILE is the desired name of your toolkit bundle. If no directory is specified, the current directory is scanned."
    "<p>For further assistance, visit https://github.com/SCIInstitute/FwdInvToolkit/wiki."
);
}

void SCIRunMainWindow::addToPathFromGUI()
{
  auto dir = QFileDialog::getExistingDirectory(this, tr("Add Directory to Data Path"), ".");
	addToDataDirectory(dir);
}

void SCIRunMainWindow::reportIssue()
{
  if (QMessageBox::Ok == QMessageBox::information(this, "Report Issue",
    "Click OK to be taken to SCIRun's Github issue reporting page.\n\nFor bug reports, please follow the template.", QMessageBox::Ok|QMessageBox::Cancel))
  {
    QDesktopServices::openUrl(QUrl("https://github.com/SCIInstitute/SCIRun/issues/new/choose", QUrl::TolerantMode));
  }
}

void SCIRunMainWindow::setFocusOnFilterLine()
{
  moduleFilterLineEdit_->setFocus(Qt::ShortcutFocusReason);
  statusBar()->showMessage(tr("Module filter activated"), 2000);
}

//disable these
void SCIRunMainWindow::addModuleKeyboardAction()
{
  //TODO
  auto item = moduleSelectorTreeWidget_->currentItem();
  if (item && item->childCount() == 0)
    std::cout << "Current module: " << item->text(0).toStdString() << std::endl;
}

void SCIRunMainWindow::selectModuleKeyboardAction()
{
  moduleSelectorTreeWidget_->setFocus(Qt::ShortcutFocusReason);
  statusBar()->showMessage(tr("Module selection activated"), 2000);
}

void SCIRunMainWindow::modulesSnapToChanged()
{
  bool snapTo = prefsWindow_->modulesSnapToCheckBox_->isChecked();
  Preferences::Instance().modulesSnapToGrid.setValue(snapTo);
}

void SCIRunMainWindow::highlightPortsChanged()
{
  bool val = prefsWindow_->portSizeEffectsCheckBox_->isChecked();
  Preferences::Instance().highlightPorts.setValue(val);
}

void SCIRunMainWindow::resetWindowLayout()
{
  configurationDockWidget_->hide();
  provenanceWindow_->hide();
  moduleSelectorDockWidget_->show();
  moduleSelectorDockWidget_->setFloating(false);
  addDockWidget(Qt::LeftDockWidgetArea, moduleSelectorDockWidget_);
}

void SCIRunMainWindow::launchNewUserWizard()
{
  NewUserWizard wiz(this);
  wiz.exec();
}

void SCIRunMainWindow::launchPythonWizard()
{
#ifdef BUILD_WITH_PYTHON
  PythonWizard *wiz = new PythonWizard( [this](const  QString& code) {pythonConsole_->runWizardCommand(code); }, this);
  wiz->show();
#endif
}

void SCIRunMainWindow::adjustModuleDock(int state)
{
  bool dockable = prefsWindow_->dockableModulesCheckBox_->isChecked();
  actionPinAllModuleUIs_->setEnabled(dockable);
  Preferences::Instance().modulesAreDockable.setValueWithSignal(dockable);
}

void SCIRunMainWindow::updateDockWidgetProperties(bool isFloating)
{
  auto dock = qobject_cast<QDockWidget*>(sender());
  if (dock && isFloating)
  {
    dock->setWindowFlags(Qt::Window);
    dock->show();
  }
}

void SCIRunMainWindow::changeExecuteActionIconToStop()
{
  actionExecuteAll_->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
	actionExecuteAll_->setText("Halt Execution");
}

void SCIRunMainWindow::changeExecuteActionIconToPlay()
{
  actionExecuteAll_->setIcon(QPixmap(":/general/Resources/new/general/run.png"));
	actionExecuteAll_->setText("Execute All");
}

void SCIRunMainWindow::openLogFolder()
{
  auto logPath = QString::fromStdString(Application::Instance().logDirectory().string());
  QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
}

void SCIRunMainWindow::adjustExecuteButtonAppearance()
{
  switch (prefsWindow_->actionTextIconCheckBox_->checkState())
  {
  case 0:
    prefsWindow_->actionTextIconCheckBox_->setText("Execute Button Text");
		executeButton_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    break;
  case 1:
    prefsWindow_->actionTextIconCheckBox_->setText("Execute Button Icon");
		executeButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    break;
  case 2:
    prefsWindow_->actionTextIconCheckBox_->setText("Execute Button Text+Icon");
		executeButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    break;
  }
}

void SCIRunMainWindow::alertForNetworkCycles(int code)
{
  if (code == -1)
  {
    QMessageBox::warning(this, "Network graph has a cycle", "Your network contains a cycle. The execution scheduler cannot handle cycles at this time. Please ensure all cycles are broken before executing.");
    networkEditor_->resetNetworkDueToCycle();
  }
}

void SCIRunMainWindow::addModuleToWindowList(const QString& modId, bool hasUI)
{
  if (menuCurrent_->actions().isEmpty())
    menuCurrent_->setEnabled(true);
  auto modAction = new QAction(this);
  modAction->setText(modId);
  modAction->setEnabled(hasUI);
  connect(modAction, SIGNAL(triggered()), networkEditor_, SLOT(moduleWindowAction()));
  currentModuleActions_.insert(modId, modAction);
  menuCurrent_->addAction(modAction);

  if (modId.contains("Subnet"))
  {
    if (menuCurrentSubnets_->actions().isEmpty())
      menuCurrentSubnets_->setEnabled(true);

    auto subnetMenu = new QMenu(modId, this);
    auto showAction = new QAction(subnetMenu);
    showAction->setText("Show");
    subnetMenu->addAction(showAction);
    auto renameAction = new QAction(subnetMenu);
    renameAction->setText("Rename...");
    subnetMenu->addAction(renameAction);

    connect(showAction, SIGNAL(triggered()), networkEditor_, SLOT(subnetMenuActionTriggered()));
    connect(renameAction, SIGNAL(triggered()), networkEditor_, SLOT(subnetMenuActionTriggered()));
    currentSubnetActions_.insert(modId, subnetMenu);
    menuCurrentSubnets_->addMenu(subnetMenu);
  }
}

void SCIRunMainWindow::removeModuleFromWindowList(const ModuleId& modId)
{
  auto name = QString::fromStdString(modId.id_);
  auto action = currentModuleActions_[name];
  menuCurrent_->removeAction(action);
  currentModuleActions_.remove(name);
  if (menuCurrent_->actions().isEmpty())
    menuCurrent_->setEnabled(false);

  if (modId.id_.find("Subnet") != std::string::npos)
  {
    auto subnet = currentSubnetActions_[name];
    if (subnet)
      menuCurrentSubnets_->removeAction(subnet->menuAction());
    currentSubnetActions_.remove(name);
    if (menuCurrentSubnets_->actions().isEmpty())
      menuCurrentSubnets_->setEnabled(false);
  }

}

void SCIRunMainWindow::showTagHelp()
{
	TagManagerWindow::showHelp(this);
}

void SCIRunMainWindow::toggleTagLayer(bool toggle)
{
	networkEditor_->tagLayer(toggle, AllTags);
	if (toggle)
		showStatusMessage("Tag layer active: all");
	else
		showStatusMessage("Tag layer inactive", 1000);
}

void SCIRunMainWindow::toggleMetadataLayer(bool toggle)
{
	networkEditor_->metadataLayer(toggle);
	//TODO: extract methods
	if (toggle)
		showStatusMessage("Metadata layer active");
	else
		showStatusMessage("Metadata layer inactive", 1000);
}

void SCIRunMainWindow::showKeyboardShortcutsDialog()
{
  if (!shortcuts_)
  {
    shortcuts_ = new ShortcutsInterface(this);
  }
  shortcuts_->show();
}

void SCIRunMainWindow::runNewModuleWizard()
{
  auto wizard = new ClassWizard(this);
	wizard->show();
}

void SCIRunMainWindow::copyVersionToClipboard()
{
  QApplication::clipboard()->setText(QString::fromStdString(VersionInfo::GIT_VERSION_TAG));
  statusBar()->showMessage("Version string copied to clipboard.", 2000);
}

void SCIRunMainWindow::updateClipboardHistory(const QString& xml)
{
  auto clips = getClipboardHistoryMenu(moduleSelectorTreeWidget_);

  auto clip = new QTreeWidgetItem();
  clip->setText(0, "clipboard " + QDateTime::currentDateTime().toString("ddd MMMM d yyyy hh:mm:ss.zzz"));
  clip->setToolTip(0, "todo: xml translation");
  clip->setData(0, clipboardKey, xml);
  clip->setForeground(0, CLIPBOARD_COLOR);

  const int clipMax = 5;
  if (clips->childCount() == clipMax)
    clips->removeChild(clips->child(0));

  clip->setCheckState(0, Qt::Unchecked);
  clips->addChild(clip);
}

void SCIRunMainWindow::showSnippetHelp()
{
  QMessageBox::information(this, "Patterns",
    "Patterns are strings that encode a linear subnetwork. They can vastly shorten network construction time. They take the form [A->B->...->C] where A, B, C, etc are module names, and the arrow represents a connection between adjacent modules. "
    "\n\nThey are available in the module selector and work just like the single module entries there: double-click or drag onto the "
    "network editor to insert the entire snippet. A '*' at the end of the module name will open the UI for that module.\n\nCustom patterns can be created by editing the file patterns.txt (if not present, create it) in the same folder as the SCIRun executable. Enter one pattern per line in the prescribed format, then restart SCIRun for them to appear."
    "\n\nFor feedback, please comment on this issue: https://github.com/SCIInstitute/SCIRun/issues/1263"
    );
}

void SCIRunMainWindow::showClipboardHelp()
{
  QMessageBox::information(this, "Clipboard",
    "The network editor clipboard works on arbitrary network selections (modules and connections). A history of five copied items is kept under \"Clipboard History\" in the module selector. "
    "\n\nTo cut/copy/paste, see the Edit menu and the corresponding hotkeys."
    "\n\nClipboard history items can be starred like module favorites. When starred, they are saved as fragments under \"Saved Fragments,\" which are preserved in application settings. "
    "\n\nThe user may edit the text of the saved fragment items to give them informative names, which are also saved. Hover over them to see a tooltip representation of the saved fragment."
    "\n\nRight-click on the fragment item to rename or delete it."
    );
}

void SCIRunMainWindow::loadPythonAPIDoc()
{
  openPythonAPIDoc();
}

void SCIRunMainWindow::showTriggerHelp()
{
	QMessageBox::information(this, "Triggered Scripts",
    "The triggered scripts interface allows the user to inject Python code that executes whenever a specific event happens. Currently the available events are post-module-add (manually, not "
    "via network loading), and post-network-load (after user loads a file)."
    "\n\nExamples can be found in the GUI when you first load the dialog. The scripts are saved at the application level and can be enabled/disabled."
     );
}

void SCIRunMainWindow::showModuleFuzzySearchHelp()
{
  QMessageBox::information(this, "Module Selector Fuzzy Search",
                           "\nFuzzy search lets you skip characters as long as the are in order."
                           "\nExample: Searching 'rdfl' will find the module 'ReadField'."
                           "\n\nSearches are case insensitive except when you type an upper case character."
                           "\nExample: Searching 'RdFl' or 'Rdfl' will reduce total matches while still finding 'ReadField'."
                           "\nTip: Type the first letters of words in capitals, such as 'SFG' to find 'ShowFieldGlyphs'."
                           "\n\nSpaces between words will do multiple searches to further reduce matched modules."
                           "\nExample: Search 'mesh get' to search both 'mesh' and 'get' in the same module name."
                           "\n\nAll symbols are igonred except *'s which will make the search switch to the wildcards option instead of fuzzy search."
    );
}

void SCIRunMainWindow::toolkitDownload()
{
  auto action = qobject_cast<QAction*>(sender());

	static std::vector<ToolkitDownloader*> downloaders;
  downloaders.push_back(new ToolkitDownloader(action, statusBar(), this));
}

void SCIRunMainWindow::loadToolkit()
{
  auto filename = QFileDialog::getOpenFileName(this, "Load Toolkit...", latestNetworkDirectory_.path(), "*.toolkit");
  loadToolkitsFromFile(filename);
}

void SCIRunMainWindow::loadToolkitsFromFile(const QString& filename)
{
  if (!filename.isEmpty())
  {
    ToolkitUnpackerCommand command;
    command.set(Variables::Filename, filename.toStdString());

    {
      auto file = command.get(Variables::Filename).toFilename();
      auto stem = QString::fromStdString(file.leaf().stem().string());
      auto dir = QString::fromStdString(file.parent_path().string());
      auto added = toolkitDirectories_.contains(stem);
      if (added && toolkitDirectories_[stem] == dir)
      {
        auto replace = QMessageBox::warning(this, "Toolkit already loaded",
          "Toolkit " + filename + " is already loaded. Do you wish to overwrite?", QMessageBox::Yes | QMessageBox::No);

        if (QMessageBox::No == replace)
          return;
        else
        {
          auto path = QString::fromStdString(file.string());
          auto toRemove = toolkitMenus_[path];
          if (toRemove)
          {
            menuToolkits_->removeAction(toolkitMenus_[path]->menuAction());
            toolkitMenus_.remove(path);
            toolkitFiles_.removeAll(filename);
            importedToolkits_.removeAll(filename);
          }
          else
            qDebug() << "logical error in toolkit removal";
        }
      }
    }

    if (command.execute())
    {
      statusBar()->showMessage(tr("Toolkit imported: ") + filename, 2000);
      if (!toolkitFiles_.contains(filename))
      {
        toolkitFiles_ << filename;
      }
      importedToolkits_ << filename;
    }
    else
    {
      statusBar()->showMessage(tr("Toolkit import failed: ") + filename, 2000);
      toolkitFiles_.removeAll(filename);
    }
  }
}

void SCIRunMainWindow::openToolkitFolder()
{
  auto path = sender()->property("path").toString();
  QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void SCIRunMainWindow::removeToolkit()
{
  auto filename = sender()->property("filename").toString();
  int r = QMessageBox::warning(this, tr("SCIRun 5"), "Remove toolkit " + filename + "?", QMessageBox::Yes | QMessageBox::No);
  if (QMessageBox::Yes == r)
  {
    menuToolkits_->removeAction(qobject_cast<QMenu*>(sender()->parent())->menuAction());
    auto fullpath = sender()->property("fullpath").toString();
    toolkitFiles_.removeAll(fullpath);
    importedToolkits_.removeAll(fullpath);
  }
}

void SCIRunMainWindow::openToolkitNetwork()
{
  if (okToContinue())
  {
    QTemporaryFile temp;
    if (temp.open())
    {
      auto network = sender()->property("network").toString();
      QTextStream stream(&temp);
      stream << network;
    }
    loadNetworkFile(temp.fileName(), true);
  }
}

void SCIRunMainWindow::launchNewInstance()
{
#ifdef __APPLE__
  auto appFilepath = Core::Application::Instance().executablePath();

#ifdef BUILD_BUNDLE
  auto execName = appFilepath / "SCIRun";
#else
  auto execName = appFilepath.parent_path().parent_path() / "SCIRun/SCIRun_test";
#endif

  auto command = "open -n " + execName.string() + " &";

  system( command.c_str() );
#endif
}

void SCIRunMainWindow::maxCoreValueChanged(int value)
{
  qDebug() << __FUNCTION__ << value;
  Core::Thread::Parallel::SetMaximumCores(value);
}

void SCIRunMainWindow::toggleFullScreen()
{
  if (isFullScreen())
    showNormal();
  else
    showFullScreen();
}
