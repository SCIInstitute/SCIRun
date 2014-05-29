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

#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Util/StringUtil.h>

#include <map>


using namespace SCIRun;


namespace SCIRun {

// Use Core/Util/share.h.
// Core_Util is the first library to be loaded at runtime.
#include <Core/Util/share.h>
extern SCISHARE Mutex fieldIEPluginMutex;

static std::map<std::string, FieldIEPlugin *> *field_plugin_table = 0;

//----------------------------------------------------------------------
FieldIEPlugin::FieldIEPlugin(const std::string& pname,
			     const std::string& fextension,
			     const std::string& fmagic,
			     FieldHandle (*freader)(ProgressReporter *pr,
						    const char *filename),
			     bool (*fwriter)(ProgressReporter *pr,
					     FieldHandle f,
					     const char *filename))
  : pluginname(pname),
    fileextension(fextension),
    filemagic(fmagic),
    filereader(freader),
    filewriter(fwriter)
{
  fieldIEPluginMutex.lock();

  if (!field_plugin_table)
  {
    field_plugin_table = new std::map<std::string, FieldIEPlugin *>();
  }

  std::string tmppname = pluginname;
  int counter = 2;
  for (;;)
  {
    std::map<std::string, FieldIEPlugin *>::iterator loc = field_plugin_table->find(tmppname);
    if (loc == field_plugin_table->end())
    {
      if (tmppname != pluginname) { const_cast<std::string&>(pluginname) = tmppname; }
      (*field_plugin_table)[pluginname] = this;
      break;
    }
    if (*(*loc).second == *this)
    {
      std::cerr << "WARNING: FieldIEPlugin '" << tmppname << "' duplicated.\n";
      break;
    }

    std::cout << "WARNING: Multiple FieldIEPlugins with '" << pluginname
	 << "' name.\n";
    tmppname = pluginname + "(" + to_string(counter) + ")";
    counter++;
  }

  fieldIEPluginMutex.unlock();
}



FieldIEPlugin::~FieldIEPlugin()
{
  if (field_plugin_table == NULL)
  {
    std::cerr << "WARNING: FieldIEPlugin.cc: ~FieldIEPlugin(): field_plugin_table is NULL\n";
    std::cerr << "         For: " << pluginname << "\n";
    return;
  }

  fieldIEPluginMutex.lock();

  std::map<std::string, FieldIEPlugin *>::iterator iter = field_plugin_table->find(pluginname);
  if (iter == field_plugin_table->end())
  {
    std::cerr << "WARNING: FieldIEPlugin " << pluginname << 
      " not found in database for removal.\n";
  }
  else
  {
    field_plugin_table->erase(iter);
  }

  if (field_plugin_table->size() == 0)
  {
    delete field_plugin_table;
    field_plugin_table = 0;
  }

  fieldIEPluginMutex.unlock();
}


bool
FieldIEPlugin::operator==(const FieldIEPlugin &other) const
{
  return (pluginname == other.pluginname &&
	  fileextension == other.fileextension &&
	  filemagic == other.filemagic &&
	  filereader == other.filereader &&
	  filewriter == other.filewriter);
}



void
FieldIEPluginManager::get_importer_list(std::vector<std::string> &results)
{
  if (field_plugin_table == 0) return;

  fieldIEPluginMutex.lock();
  std::map<std::string, FieldIEPlugin *>::const_iterator itr = field_plugin_table->begin();
  while (itr != field_plugin_table->end())
  {
    if ((*itr).second->filereader != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  fieldIEPluginMutex.unlock();
}


void
FieldIEPluginManager::get_exporter_list(std::vector<std::string> &results)
{
  if (field_plugin_table == 0) return;

  fieldIEPluginMutex.lock();
  std::map<std::string, FieldIEPlugin *>::const_iterator itr = field_plugin_table->begin();
  while (itr != field_plugin_table->end())
  {
    if ((*itr).second->filewriter != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  fieldIEPluginMutex.unlock();
}

 
FieldIEPlugin *
FieldIEPluginManager::get_plugin(const std::string &name)
{
  if (field_plugin_table == 0) return NULL;

  // Should check for invalid name.
  std::map<std::string, FieldIEPlugin *>::iterator loc = field_plugin_table->find(name);
  if (loc == field_plugin_table->end())
  {
    return 0;
  }
  else
  {
    return (*loc).second;
  }
}


} // End namespace SCIRun


