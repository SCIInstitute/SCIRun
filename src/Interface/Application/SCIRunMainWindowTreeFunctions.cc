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
#include <Interface/Application/GuiLogger.h>
#include <Interface/Application/SCIRunMainWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/Connection.h>
#include <Interface/Application/MainWindowCollaborators.h>
#include <Interface/Application/TreeViewCollaborators.h>
#include <Interface/Application/GuiCommands.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h> //TODO
#include <Dataflow/Network/NetworkFwd.h>
#include <Core/Logging/Log.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>


using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Commands;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;

namespace
{
  QColor favesColor()
  {
    return Qt::yellow;
  }
  QColor packageColor()
  {
    return Qt::yellow;
  }
  QColor categoryColor()
  {
    return Qt::green;
  }

  const QString bullet = "* ";
  const QString hash = "# ";
  const QString favoritesText = bullet + "Favorites";
  const QString clipboardHistoryText = hash + "Clipboard History";
  const QString savedSubsText = hash + "Saved Fragments";
  const QString recentText = bullet + "Recent Modules";
  const QString frequentText = bullet + "Frequent Modules";
}

void SCIRunMainWindow::addFavoriteMenu(QTreeWidget* tree)
{
  auto faves = new QTreeWidgetItem();
  faves->setText(0, favoritesText);
  faves->setForeground(0, favesColor());

  tree->addTopLevelItem(faves);
}

QTreeWidgetItem* SCIRunMainWindow::getTreeMenu(QTreeWidget* tree, const QString& text)
{
  for (int i = 0; i < tree->topLevelItemCount(); ++i)
  {
    auto top = tree->topLevelItem(i);
    if (top->text(0) == text)
    {
      return top;
    }
  }
  return nullptr;
}

QTreeWidgetItem* SCIRunMainWindow::getFavoriteMenu()
{
  return getTreeMenu(userModuleSelectorTreeWidget_, favoritesText);
}

QTreeWidgetItem* SCIRunMainWindow::getClipboardHistoryMenu()
{
  return getTreeMenu(userModuleSelectorTreeWidget_, clipboardHistoryText);
}

QTreeWidgetItem* SCIRunMainWindow::getRecentModulesMenu()
{
  return getTreeMenu(userModuleSelectorTreeWidget_, recentText);
}

QTreeWidgetItem* SCIRunMainWindow::getFrequentModulesMenu()
{
  return getTreeMenu(userModuleSelectorTreeWidget_, frequentText);
}

QTreeWidgetItem* SCIRunMainWindow::getSavedSubnetworksMenu()
{
  return getTreeMenu(userModuleSelectorTreeWidget_, savedSubsText);
}

void SCIRunMainWindow::addSnippet(const QString& code, QTreeWidgetItem* snips)
{
  auto snipItem = new QTreeWidgetItem();
  snipItem->setText(0, code);
  snips->addChild(snipItem);
}

void SCIRunMainWindow::readCustomSnippets(QTreeWidgetItem* snips)
{
  QFile inputFile("patterns.txt");
  if (inputFile.open(QIODevice::ReadOnly))
  {
    GuiLogger::logInfoQ("Pattern file opened: " + inputFile.fileName());
    QTextStream in(&inputFile);
    while (!in.atEnd())
    {
      QString line = in.readLine();
      addSnippet(line, snips);
      GuiLogger::logInfoQ("Pattern read: " + line);
    }
    inputFile.close();
  }
}

void SCIRunMainWindow::addSnippetMenu(QTreeWidget* tree)
{
  auto snips = new QTreeWidgetItem();
  snips->setText(0, bullet + "Typical Patterns");
  snips->setForeground(0, favesColor());

  //hard-code a few popular ones.

  addSnippet("[ReadField*->ShowField->ViewScene]", snips);
  addSnippet("[CreateLatVol->ShowField->ViewScene]", snips);
  addSnippet("[ReadField*->ReportFieldInfo]", snips);
  addSnippet("[ReadMatrix*->ReportMatrixInfo]", snips);
  addSnippet("[CreateStandardColorMap->RescaleColorMap->ShowField->ViewScene]", snips);
  addSnippet("[GetFieldBoundary->FairMesh->ShowField]", snips);

  readCustomSnippets(snips);

  tree->addTopLevelItem(snips);
}

void SCIRunMainWindow::addFrequentMenu(QTreeWidget* tree)
{
  auto freqs = new QTreeWidgetItem();
  freqs->setText(0, frequentText);
  freqs->setForeground(0, favesColor());
  tree->addTopLevelItem(freqs);

  std::for_each(frequentModulesSettings_.constKeyValueBegin(), frequentModulesSettings_.constKeyValueEnd(),
    [&](auto&& p) { frequentModules_.emplace(p.first, p.second.toInt()); });

  for (const auto& m : topNMostFrequentModules())
  {
    auto mod = new QTreeWidgetItem();
    mod->setText(0, m);
    freqs->addChild(mod);
  }
  connect(actionClearFrequentModules_, &QAction::triggered, this, &SCIRunMainWindow::clearFrequentModules);
}

void SCIRunMainWindow::addRecentMenu(QTreeWidget* tree)
{
  auto recent = new QTreeWidgetItem();
  recent->setText(0, recentText);
  recent->setForeground(0, favesColor());
  tree->addTopLevelItem(recent);

  for (const auto& m : recentModules_)
  {
    auto mod = new QTreeWidgetItem();
    mod->setText(0, m);
    mod->setData(0, clipboardKey, m.split(' ')[2]);
    recent->insertChild(0, mod);
  }
  connect(actionClearRecentModules_, &QAction::triggered, this, &SCIRunMainWindow::clearRecentModules);
}

void SCIRunMainWindow::addSavedSubnetworkMenu(QTreeWidget* tree)
{
  auto savedSubnetworks = new QTreeWidgetItem();
  savedSubnetworks->setText(0, savedSubsText);
  savedSubnetworks->setData(0, Qt::UserRole, saveFragmentData_);
  savedSubnetworks->setForeground(0, favesColor());
  tree->addTopLevelItem(savedSubnetworks);
}

void SCIRunMainWindow::addClipboardHistoryMenu(QTreeWidget* tree)
{
  auto clips = new QTreeWidgetItem();
  clips->setText(0, clipboardHistoryText);
  clips->setForeground(0, favesColor());
  tree->addTopLevelItem(clips);
}

QTreeWidgetItem* SCIRunMainWindow::addFavoriteItem(QTreeWidgetItem* faves, QTreeWidgetItem* module)
{
  guiLogDebug("Adding item to favorites: {}", module->text(0).toStdString());
  auto copy = new QTreeWidgetItem(*module);
  copy->setData(0, Qt::CheckStateRole, QVariant());
  if (copy->foreground(0) == CLIPBOARD_COLOR)
  {
    copy->setFlags(copy->flags() | Qt::ItemIsEditable);
  }
  faves->addChild(copy);
  return copy;
}

void SCIRunMainWindow::fillTreeWidget(QTreeWidget* tree, const ModuleDescriptionMap& moduleMap, const QStringList& favoriteModuleNames)
{
  auto faves = getFavoriteMenu();
  for (const auto& package : moduleMap)
  {
    const auto& packageName = package.first;
    auto packageItem = new QTreeWidgetItem();
    packageItem->setText(0, QString::fromStdString(packageName));
    packageItem->setForeground(0, packageColor());
    tree->addTopLevelItem(packageItem);
    size_t totalModules = 0;
    for (const auto& category : package.second)
    {
      const auto& categoryName = category.first;
      auto categoryItem = new QTreeWidgetItem();
      categoryItem->setText(0, QString::fromStdString(categoryName));
      categoryItem->setForeground(0, categoryColor());
      packageItem->addChild(categoryItem);
      for (const auto& module : category.second)
      {
        const auto& moduleName = module.first;
        auto moduleItem = new QTreeWidgetItem();
        auto name = QString::fromStdString(moduleName);
        moduleItem->setText(0, name);
        if (favoriteModuleNames.contains(name))
        {
          moduleItem->setCheckState(0, Qt::Checked);
          addFavoriteItem(faves, moduleItem);
        }
        else
        {
          moduleItem->setCheckState(0, Qt::Unchecked);
        }
        moduleItem->setText(1, QString::fromStdString(module.second.moduleStatus_));
        moduleItem->setForeground(1, Qt::lightGray);
        moduleItem->setText(2, QString::fromStdString(module.second.moduleInfo_));
        moduleItem->setForeground(2, Qt::lightGray);
        moduleItem->setData(0, hasUIDataFlag, module.second.hasUI_);
        categoryItem->addChild(moduleItem);
        totalModules++;
      }
      categoryItem->setText(1, "Category Module Count = " + QString::number(category.second.size()));
      categoryItem->setForeground(1, Qt::magenta);
    }
    packageItem->setText(1, "Package Module Count = " + QString::number(totalModules));
    packageItem->setForeground(1, Qt::magenta);
  }
}

void SCIRunMainWindow::sortFavorites()
{
  auto faves = getFavoriteMenu();
  faves->sortChildren(0, Qt::AscendingOrder);
}
