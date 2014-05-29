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

#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/ImportExport/ColorMap/ColorMapIEPlugin.h>
#include <Core/Thread/Mutex.h>
#include <map>

using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Datatypes;

namespace SCIRun {

static Mutex colormapIEPluginMutex("colorMapIE"); 

static std::map<std::string, ColorMapIEPlugin *> *colormap_plugin_table = 0;

//----------------------------------------------------------------------
ColorMapIEPlugin::ColorMapIEPlugin(const std::string& pname,
				   const std::string& fextension,
				   const std::string& fmagic,
				   ColorMapHandle (*freader)(Core::Logging::Log& pr,
							     const char *filename),
				   bool (*fwriter)(Core::Logging::Log& pr,
						   ColorMapHandle f,
						   const char *filename))
  : pluginname(pname),
    fileextension(fextension),
    filemagic(fmagic),
    filereader(freader),
    filewriter(fwriter)
{
  colormapIEPluginMutex.lock();
  if (!colormap_plugin_table)
  {
    colormap_plugin_table = new std::map<std::string, ColorMapIEPlugin *>();
  }

  std::string tmppname = pluginname;
  int counter = 2;
  while (1)
  {
    std::map<std::string, ColorMapIEPlugin *>::iterator loc = colormap_plugin_table->find(tmppname);
    if (loc == colormap_plugin_table->end())
    {
      if (tmppname != pluginname) { ((std::string)pluginname) = tmppname; }
      (*colormap_plugin_table)[pluginname] = this;
      break;
    }
    if (*(*loc).second == *this)
    {
      std::cerr << "WARNING: ColorMapIEPlugin '" << tmppname << "' duplicated.\n";
      break;
    }

    std::cout << "WARNING: Multiple ColorMapIEPlugins with '" << pluginname
	 << "' name.\n";
    tmppname = pluginname + "(" + to_string(counter) + ")";
    counter++;
  }

  colormapIEPluginMutex.unlock();
}



ColorMapIEPlugin::~ColorMapIEPlugin()
{
  if (colormap_plugin_table == NULL)
  {
    std::cerr << "WARNING: ColorMapIEPlugin.cc: ~ColorMapIEPlugin(): colormap_plugin_table is NULL\n";
    std::cerr << "         For: " << pluginname << "\n";
    return;
  }

  colormapIEPluginMutex.lock();

  std::map<std::string, ColorMapIEPlugin *>::iterator iter = colormap_plugin_table->find(pluginname);
  if (iter == colormap_plugin_table->end())
  {
    std::cerr << "WARNING: ColorMapIEPlugin " << pluginname << 
      " not found in database for removal.\n";
  }
  else
  {
    colormap_plugin_table->erase(iter);
  }

  if (colormap_plugin_table->size() == 0)
  {
    delete colormap_plugin_table;
    colormap_plugin_table = 0;
  }

  colormapIEPluginMutex.unlock();
}


bool
ColorMapIEPlugin::operator==(const ColorMapIEPlugin &other) const
{
  return (pluginname == other.pluginname &&
	  fileextension == other.fileextension &&
	  filemagic == other.filemagic &&
	  filereader == other.filereader &&
	  filewriter == other.filewriter);
}



void
ColorMapIEPluginManager::get_importer_list(std::vector<std::string> &results)
{
  if (colormap_plugin_table == NULL) return;

  colormapIEPluginMutex.lock();
  std::map<std::string, ColorMapIEPlugin *>::const_iterator itr = colormap_plugin_table->begin();
  while (itr != colormap_plugin_table->end())
  {
    if ((*itr).second->filereader != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  colormapIEPluginMutex.unlock();
}


void
ColorMapIEPluginManager::get_exporter_list(std::vector<std::string> &results)
{
  if (colormap_plugin_table == NULL) return;

  colormapIEPluginMutex.lock();
  std::map<std::string, ColorMapIEPlugin *>::const_iterator itr = colormap_plugin_table->begin();
  while (itr != colormap_plugin_table->end())
  {
    if ((*itr).second->filewriter != NULL)
    {
      results.push_back((*itr).first);
    }
    ++itr;
  }
  colormapIEPluginMutex.unlock();
}

 
ColorMapIEPlugin *
ColorMapIEPluginManager::get_plugin(const std::string &name)
{
  if (colormap_plugin_table == NULL) return NULL;

  // Should check for invalid name.
  std::map<std::string, ColorMapIEPlugin *>::iterator loc = colormap_plugin_table->find(name);
  if (loc == colormap_plugin_table->end())
  {
    return NULL;
  }
  else
  {
    return (*loc).second;
  }
}


} // End namespace SCIRun


