/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


/*
 *  Persistent.h: Base class for persistent objects...
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2004
 *
 */

#include <Core/Thread/Mutex.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>

#include <map>

using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Datatypes;

namespace SCIRun {

static Mutex matrixIEPluginMutex("matrixIE");

static std::map<std::string, MatrixIEPlugin *> *matrix_plugin_table = 0;

//----------------------------------------------------------------------

MatrixIEPlugin::MatrixIEPlugin(const std::string& pname,
                               const std::string& fextension,
                               const std::string& fmagic,
                               MatrixHandle (*freader)(Core::Logging::Log& pr,
                                                       const char *filename),
                               bool (*fwriter)(Core::Logging::Log& pr,
                                               MatrixHandle f,
                                               const char *filename))
  : pluginname_(pname),
    fileExtension_(fextension),
    fileMagic_(fmagic),
    fileReader_(freader),
    fileWriter_(fwriter)
{
  matrixIEPluginMutex.lock();

  if (!matrix_plugin_table)
  {
    matrix_plugin_table = new std::map<std::string, MatrixIEPlugin *>();
  }

  std::string tmppname = pluginname_;
  int counter = 2;
  for (;;)
  {
    std::map<std::string, MatrixIEPlugin *>::iterator loc = matrix_plugin_table->find(tmppname);
    if (loc == matrix_plugin_table->end())
    {
      if (tmppname != pluginname_) { ((std::string)pluginname_) = tmppname; }

      (*matrix_plugin_table)[pluginname_] = this;
      break;
    }
    if (*(*loc).second == *this)
    {
      std::cerr << "WARNING: MatrixIEPlugin '" << tmppname << "' duplicated.\n";
      break;
    }

    std::cout << "WARNING: Multiple MatrixIEPlugins with '" << pluginname_
	 << "' name.\n";
    tmppname = pluginname_ + "(" + to_string(counter) + ")";
    counter++;
  }

  matrixIEPluginMutex.unlock();
}



MatrixIEPlugin::~MatrixIEPlugin()
{
  if (matrix_plugin_table == NULL)
  {
    std::cerr << "WARNING: MatrixIEPlugin.cc: ~MatrixIEPlugin(): matrix_plugin_table is NULL\n";
    std::cerr << "         For: " << pluginname_ << "\n";
    return;
  }

  matrixIEPluginMutex.lock();

  std::map<std::string, MatrixIEPlugin *>::iterator iter = matrix_plugin_table->find(pluginname_);
  if (iter == matrix_plugin_table->end())
  {
    std::cerr << "WARNING: MatrixIEPlugin " << pluginname_ << 
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

  matrixIEPluginMutex.unlock();
}


bool
MatrixIEPlugin::operator==(const MatrixIEPlugin &other) const
{
  return (pluginname_ == other.pluginname_ &&
	  fileExtension_ == other.fileExtension_ &&
	  fileMagic_ == other.fileMagic_ &&
	  fileReader_ == other.fileReader_ &&
	  fileWriter_ == other.fileWriter_);
}



void
MatrixIEPluginManager::get_importer_list(std::vector<std::string> &results)
{
  if (matrix_plugin_table == NULL) return;

  matrixIEPluginMutex.lock();
  std::map<std::string, MatrixIEPlugin *>::const_iterator itr = matrix_plugin_table->begin();
  while (itr != matrix_plugin_table->end())
  {
    if ((*itr).second->fileReader_ != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  matrixIEPluginMutex.unlock();
}


void
MatrixIEPluginManager::get_exporter_list(std::vector<std::string> &results)
{
  if (matrix_plugin_table == NULL) return;

  matrixIEPluginMutex.lock();
  std::map<std::string, MatrixIEPlugin *>::const_iterator itr = matrix_plugin_table->begin();
  while (itr != matrix_plugin_table->end())
  {
    if ((*itr).second->fileWriter_ != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  matrixIEPluginMutex.unlock();
}

 
MatrixIEPlugin *
MatrixIEPluginManager::get_plugin(const std::string &name)
{
  if (matrix_plugin_table == NULL) return NULL;

  // Should check for invalid name.
  std::map<std::string, MatrixIEPlugin *>::iterator loc = matrix_plugin_table->find(name);
  if (loc == matrix_plugin_table->end())
  {
    return NULL;
  }
  else
  {
    return (*loc).second;
  }
}


} // End namespace SCIRun


