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


#include <Core/ImportExport/Field/FieldIEPlugin.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>
#include <boost/regex.hpp>
#include <Core/Logging/Log.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;

namespace SCIRun
{
  template class GenericIEPluginManager<Field>;
  template class GenericIEPluginManager<Matrix>;
  template class GenericIEPluginManager<NrrdData>;
}

IEPluginManagerManager::IEPluginManagerManager() {}

std::string SCIRun::fileTypeDescriptionFromDialogBoxFilter(const std::string& fileFilter)
{
  LOG_DEBUG("fileTypeDescriptionFromDialogBoxFilter received {}", fileFilter);
  boost::regex r("(.*) \\(\\*.*\\)");
  boost::smatch what;
  regex_match(fileFilter, what, r);
  LOG_DEBUG("fileTypeDescriptionFromDialogBoxFilter returning {}", std::string(what[1]));
  return what[1];
}

template <>
std::string SCIRun::defaultImportTypeForFile(const GenericIEPluginManager<Field>*)
{
  return "SCIRun Field File (*.fld)";
}

template <>
std::string SCIRun::defaultImportTypeForFile(const GenericIEPluginManager<Matrix>*)
{
  return "SCIRun Matrix File (*.mat)";
}

template <>
std::string SCIRun::defaultImportTypeForFile(const GenericIEPluginManager<NrrdData>*)
{
  return "Nrrd Files (*.nhdr *.nrrd *.png *.txt *.vtk)";
}

template <>
std::string SCIRun::defaultExportTypeForFile(const GenericIEPluginManager<Field>*)
{
  return "SCIRun Field Binary (*.fld);;SCIRun Field ASCII (*.fld)";
}

template <>
std::string SCIRun::defaultExportTypeForFile(const GenericIEPluginManager<Matrix>*)
{
  return "SCIRun Matrix Binary (*.mat);;SCIRun Matrix ASCII (*.mat)";
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Thread/Mutex.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>

#include <map>
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Datatypes;


namespace SCIRun {

static Mutex nrrdIEPluginMutex;

static std::map<std::string, NrrdIEPlugin *> *matrix_plugin_table = 0;

//----------------------------------------------------------------------

NrrdIEPlugin::NrrdIEPlugin(const std::string& pname,
                           const std::string& fextension,
                           const std::string& fmagic,
                           NrrdDataHandle (*freader)(Core::Logging::Log& pr,
                                                 const char *filename),
                           bool (*fwriter)(Core::Logging::Log& pr,
                                           NrrdDataHandle f,
                                           const char *filename))
  : pluginname(pname),
    fileextension(fextension),
    filemagic(fmagic),
    fileReader_(freader),
    fileWriter_(fwriter)
{
  nrrdIEPluginMutex.lock();

  if (!matrix_plugin_table)
  {
    matrix_plugin_table = new std::map<std::string, NrrdIEPlugin *>();
  }

  std::string tmppname = pluginname;
  int counter = 2;
  for (;;)
  {
    std::map<std::string, NrrdIEPlugin *>::iterator loc = matrix_plugin_table->find(tmppname);
    if (loc == matrix_plugin_table->end())
    {
      if (tmppname != pluginname) { ((std::string)pluginname) = tmppname; }

      (*matrix_plugin_table)[pluginname] = this;
      break;
    }
    if (*(*loc).second == *this)
    {
      std::cerr << "WARNING: NrrdIEPlugin '" << tmppname << "' duplicated.\n";
      break;
    }

    std::cout << "WARNING: Multiple NrrdIEPlugins with '" << pluginname
	 << "' name.\n";
    tmppname = pluginname + "(" + to_string(counter) + ")";
    counter++;
  }

  nrrdIEPluginMutex.unlock();
}



NrrdIEPlugin::~NrrdIEPlugin()
{
  if (matrix_plugin_table == NULL)
  {
    std::cerr << "WARNING: NrrdIEPlugin.cc: ~NrrdIEPlugin(): matrix_plugin_table is NULL\n";
    std::cerr << "         For: " << pluginname << "\n";
    return;
  }

  nrrdIEPluginMutex.lock();

  std::map<std::string, NrrdIEPlugin *>::iterator iter = matrix_plugin_table->find(pluginname);
  if (iter == matrix_plugin_table->end())
  {
    std::cerr << "WARNING: NrrdIEPlugin " << pluginname <<
      " not found in database for removal.\n";
  }
  else
  {
    matrix_plugin_table->erase(iter);
  }

  if (matrix_plugin_table->size() == 0)
  {
    delete matrix_plugin_table;
    matrix_plugin_table = 0;
  }

  nrrdIEPluginMutex.unlock();
}


bool
NrrdIEPlugin::operator==(const NrrdIEPlugin &other) const
{
  return (pluginname == other.pluginname &&
	  fileextension == other.fileextension &&
	  filemagic == other.filemagic &&
	  fileReader_ == other.fileReader_ &&
	  fileWriter_ == other.fileWriter_);
}



void
NrrdIEPluginManager::get_importer_list(std::vector<std::string> &results)
{
  if (matrix_plugin_table == NULL) return;

  nrrdIEPluginMutex.lock();
  std::map<std::string, NrrdIEPlugin *>::const_iterator itr = matrix_plugin_table->begin();
  while (itr != matrix_plugin_table->end())
  {
    if ((*itr).second->fileReader_ != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  nrrdIEPluginMutex.unlock();
}


void
NrrdIEPluginManager::get_exporter_list(std::vector<std::string> &results)
{
  if (matrix_plugin_table == NULL) return;

  nrrdIEPluginMutex.lock();
  auto itr = matrix_plugin_table->begin();
  while (itr != matrix_plugin_table->end())
  {
    if ((*itr).second->fileWriter_ != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  nrrdIEPluginMutex.unlock();
}


NrrdIEPlugin *
NrrdIEPluginManager::get_plugin(const std::string &name)
{
  if (matrix_plugin_table == NULL) return NULL;

  // Should check for invalid name.
  auto loc = matrix_plugin_table->find(name);
  if (loc == matrix_plugin_table->end())
  {
    return nullptr;
  }
  else
  {
    return (*loc).second;
  }
}


} // End namespace SCIRun


#endif
