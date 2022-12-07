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


#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Core/Algorithms/Base/AlgorithmParameterHelper.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Algorithms;

CORE_SINGLETON_IMPLEMENTATION( Preferences );

Preferences::Preferences() :
  showModuleErrorDialogs("showModuleErrorDialogs", true),
  saveBeforeExecute("saveBeforeExecute", false),
  showModuleErrorInlineMessages("showModuleErrorInlineMessages", true),
  useNewViewSceneMouseControls("useNewViewSceneMouseControls", true),
  invertMouseZoom("invertMouseZoom", false),
  modulesSnapToGrid("modulesSnapToGrid", true),
  highlightPorts("highlightPorts", false),
  autoNotes("autoNotes", false),
  highDPIAdjustment("highDPIAdjustment", false),
  widgetSelectionCorrection("widgetSelectionCorrection", false),
  autoRotateViewerOnMouseRelease("autoRotateViewerOnMouseRelease", false),
  moduleExecuteDownstreamOnly("moduleExecuteDownstreamOnly", true),
  forceGridBackground("forceGridBackground", false),
  modulesAreDockable("modulesAreDockable", true),
  toolBarPopupShowDelay("toolBarPopupShowDelay", 200),
  toolBarPopupHideDelay("toolBarPopupHideDelay", 200),
  networkBackgroundColor("backgroundColor", "#808080"),
  postModuleAdd("postModuleAdd"),
  onNetworkLoad("onNetworkLoad"),
  applicationStart("applicationStart")
{
}

TriggeredScriptInfo::TriggeredScriptInfo(const std::string& name) :
  script(name + "_script", ""), enabled(name + "_enabled", false)
{}

boost::filesystem::path Preferences::dataDirectory() const
{
  return dataDir_;
}

std::string Preferences::setDataDirectory(const boost::filesystem::path& path)
{
  dataDir_ = path;

  if (!boost::filesystem::exists(path))
    logWarning("Data directory {} does not exist.", path.string());
  if (!boost::filesystem::is_directory(path))
    logWarning("Data directory {} is not a directory.", path.string());

  if (dataDir_.string().back() == boost::filesystem::path::preferred_separator)
  {
    dataDir_.remove_filename();
  }

  AlgorithmParameterHelper::setDataDir(dataDir_);
  AlgorithmParameterHelper::setDataDirPlaceholder(dataDirectoryPlaceholder());

  auto forwardSlashPath = boost::replace_all_copy(dataDir_.string(), "\\", "/");
  auto setDataDir = "import os; os.environ[\"SCIRUNDATADIR\"] = \"" + forwardSlashPath + "\"";
  return setDataDir;
}

boost::filesystem::path Preferences::screenshotDirectory() const
{
  return screenshotDir_;
}

void Preferences::setScreenshotDirectory(const boost::filesystem::path& path)
{
  screenshotDir_ = path;

  if (!boost::filesystem::exists(path))
    logWarning("Data directory {} does not exist.", path.string());
  if (!boost::filesystem::is_directory(path))
    logWarning("Data directory {} is not a directory.", path.string());

  if (screenshotDir_.string().back() == boost::filesystem::path::preferred_separator)
  {
    screenshotDir_.remove_filename();
  }
}

/// @todo: not sure where this should go.
std::string Preferences::dataDirectoryPlaceholder() const
{
  return "%SCIRUNDATADIR%";
}

std::vector<boost::filesystem::path> Preferences::dataPath() const
{
  return dataPath_;
}

void Preferences::addToDataPath(const boost::filesystem::path& path)
{
  dataPath_.push_back(path);
  AlgorithmParameterHelper::setDataPath(dataPath_);
}

void Preferences::setDataPath(const std::string& dirs)
{
  std::vector<std::string> paths;
  boost::split(paths, dirs, boost::is_any_of(";"));
  std::transform(paths.begin(), paths.end(), std::back_inserter(dataPath_), [](const std::string& p) { return boost::filesystem::path(p); });
  AlgorithmParameterHelper::setDataPath(dataPath_);
}
