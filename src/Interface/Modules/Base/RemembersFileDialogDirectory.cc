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


#include <Interface/Modules/Base/RemembersFileDialogDirectory.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QDir>
#include <QLineEdit>
#include <QDebug>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

void RemembersFileDialogDirectory::setStartingDir(const QString& dir)
{
  startingDirectory_ = dir;
}

QString RemembersFileDialogDirectory::startingDirectory_(".");
QString RemembersFileDialogDirectory::lastUsedDirectory_;

QString RemembersFileDialogDirectory::dialogDirectory()
{
  if (currentDirectory_.isEmpty())
  {
    if (!lastUsedDirectory_.isEmpty())
    {
      currentDirectory_ = lastUsedDirectory_;
    }
    else
    {
      currentDirectory_ = startingDirectory_;
    }
  }
  return currentDirectory_;
}

void RemembersFileDialogDirectory::updateRecentFile(const QString& recentFile)
{
  lastUsedDirectory_ = currentDirectory_ = QDir(recentFile).absolutePath();
}

QString RemembersFileDialogDirectory::pullFilename(ModuleStateHandle state, QLineEdit* fileNameLineEdit, std::function<std::string(const std::string&)> filterFromFiletype)
{
  auto filenameVar = state->getValue(Variables::Filename);
  auto file = QString::fromStdString(filenameVar.toString());
  fileNameLineEdit->setText(file);
  if (!file.isEmpty())
    updateRecentFile(QString::fromStdString(filenameVar.toFilename().string()));

  if (filterFromFiletype)
    return QString::fromStdString(filterFromFiletype(state->getValue(Variables::FileTypeName).toString()));
  return "";
}
