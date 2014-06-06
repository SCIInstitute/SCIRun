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

#ifndef SCI_project_GenericIEPlugin_h
#define SCI_project_GenericIEPlugin_h 1

#include <Core/Logging/LoggerFwd.h>
#include <Core/Thread/Mutex.h>
#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <Core/ImportExport/share.h>

namespace SCIRun {

//----------------------------------------------------------------------
template <class Data>
class GenericIEPluginInterface
{
public:
  virtual ~GenericIEPluginInterface() {}
  virtual std::string pluginname() const = 0;
  virtual std::string fileExtension() const = 0;
  virtual std::string fileMagic() const = 0;

  virtual boost::shared_ptr<Data> readFile(const std::string& filename, Core::Logging::LoggerHandle log) const = 0;
  virtual bool writeFile(boost::shared_ptr<Data> f, const std::string& filename, Core::Logging::LoggerHandle log) const = 0;
  virtual bool equals(const GenericIEPluginInterface<Data>& other) const = 0;
  virtual bool hasReader() const = 0;
  virtual bool hasWriter() const = 0;
};

template <class Data>
bool operator==(const GenericIEPluginInterface<Data>& lhs, const GenericIEPluginInterface<Data>& rhs)
{
  return lhs.equals(rhs);
}

template <class Data>
bool operator!=(const GenericIEPluginInterface<Data>& lhs, const GenericIEPluginInterface<Data>& rhs)
{
  return !(lhs == rhs);
}


template <class Data>
class IEPluginLegacyAdapter : public GenericIEPluginInterface<Data>
{
public:
  virtual std::string pluginname() const override { return pluginname_; }
  virtual std::string fileExtension() const override { return fileextension_; }
  virtual std::string fileMagic() const override { return filemagic_; }
  virtual bool hasReader() const { return filereader_ != nullptr; }
  virtual bool hasWriter() const { return filewriter_ != nullptr; }

  virtual boost::shared_ptr<Data> readFile(const std::string& filename, Core::Logging::LoggerHandle log) const override;
  virtual bool writeFile(boost::shared_ptr<Data> f, const std::string& filename, Core::Logging::LoggerHandle log) const override;
  virtual bool equals(const GenericIEPluginInterface<Data>& other) const override;

  IEPluginLegacyAdapter(const std::string &name,
    const std::string &fileextension,
    const std::string &filemagic,
    boost::shared_ptr<Data> (*freader)(Core::Logging::LoggerHandle pr, const char *filename) = 0,
    bool (*fwriter)(Core::Logging::LoggerHandle pr, boost::shared_ptr<Data> f, const char *filename) = 0);

  ~IEPluginLegacyAdapter();

  bool operator==(const IEPluginLegacyAdapter& other) const;

private:
  const std::string pluginname_;
  const std::string fileextension_;
  const std::string filemagic_;

  boost::shared_ptr<Data> (*filereader_)(Core::Logging::LoggerHandle pr, const char *filename);
  bool (*filewriter_)(Core::Logging::LoggerHandle pr,
    boost::shared_ptr<Data> f, const char *filename);
};

template <class Data>
class GenericIEPluginManager
{
public:
  static Core::Thread::Mutex& getLock();
  typedef std::map<std::string, GenericIEPluginInterface<Data>*> PluginMap;
  static PluginMap& getMap();
  static void createMap();
  static void destroyMap();

  size_t numPlugins() const;
  void get_importer_list(std::vector<std::string>& results) const;
  void get_exporter_list(std::vector<std::string>& results) const;
  GenericIEPluginInterface<Data>* get_plugin(const std::string& name) const;
private:
  static Core::Thread::Mutex lock_;
  static PluginMap* pluginTable_;
};


template <class Data>
Core::Thread::Mutex GenericIEPluginManager<Data>::lock_("fieldIE");

template <class Data>
typename GenericIEPluginManager<Data>::PluginMap* GenericIEPluginManager<Data>::pluginTable_ = 0;

template <class Data>
Core::Thread::Mutex& GenericIEPluginManager<Data>::getLock()
{
  return lock_;
}

template <class Data>
typename GenericIEPluginManager<Data>::PluginMap& GenericIEPluginManager<Data>::getMap()
{
  if (!pluginTable_)
    createMap();
  return *pluginTable_;
}

template <class Data>
void GenericIEPluginManager<Data>::createMap()
{
  if (!pluginTable_)
  {
    pluginTable_ = new typename GenericIEPluginManager<Data>::PluginMap();
  }
}

template <class Data>
size_t GenericIEPluginManager<Data>::numPlugins() const
{
  return pluginTable_ ? pluginTable_->size() : 0;
}

template <class Data>
void GenericIEPluginManager<Data>::get_importer_list(std::vector<std::string>& results) const
{
  if (!pluginTable_) 
  {
    return;
  }

  Core::Thread::Guard s(lock_.get());
  BOOST_FOREACH(const typename PluginMap::value_type& plugin, *pluginTable_)
  {
    if (plugin.second->hasReader())
      results.push_back(plugin.first);
  }
}

template <class Data>
void GenericIEPluginManager<Data>::get_exporter_list(std::vector<std::string>& results) const
{
  if (!pluginTable_) 
  {
    return;
  }

  Core::Thread::Guard s(lock_.get());
  BOOST_FOREACH(const typename PluginMap::value_type& plugin, *pluginTable_)
  {
    if (plugin.second->hasWriter())
      results.push_back(plugin.first);
  }
}

template <class Data>
GenericIEPluginInterface<Data>* GenericIEPluginManager<Data>::get_plugin(const std::string &name) const
{
  if (!pluginTable_)
    return 0;

  Core::Thread::Guard s(lock_.get());
  // Should check for invalid name.
  auto loc = pluginTable_->find(name);
  if (loc == pluginTable_->end())
  {
    return 0;
  }
  else
  {
    return loc->second;
  }
}

template <class Data>
void GenericIEPluginManager<Data>::destroyMap()
{
  delete pluginTable_;
  pluginTable_ = 0;
}


template <class Data>
IEPluginLegacyAdapter<Data>::IEPluginLegacyAdapter(const std::string& pname,
  const std::string& fextension,
  const std::string& fmagic,
  boost::shared_ptr<Data> (*freader)(Core::Logging::LoggerHandle pr, const char *filename),
  bool (*fwriter)(Core::Logging::LoggerHandle pr, boost::shared_ptr<Data> f, const char *filename))
  : pluginname_(pname),
  fileextension_(fextension),
  filemagic_(fmagic),
  filereader_(freader),
  filewriter_(fwriter)
{
  Core::Thread::Guard s(GenericIEPluginManager<Data>::getLock().get());

  GenericIEPluginManager<Data>::createMap();

  std::string tmppname = pluginname_;
  int counter = 2;
  for (;;)
  {
    auto loc = GenericIEPluginManager<Data>::getMap().find(tmppname);
    if (loc == GenericIEPluginManager<Data>::getMap().end())
    {
      if (tmppname != pluginname_) { const_cast<std::string&>(pluginname_) = tmppname; }
      GenericIEPluginManager<Data>::getMap()[pluginname_] = this;
      break;
    }
    if (*(*loc).second == *this)
    {
      std::cerr << "WARNING: FieldIEPlugin '" << tmppname << "' duplicated.\n";
      break;
    }

    std::cout << "WARNING: Multiple FieldIEPlugins with '" << pluginname_ << "' name.\n";
    tmppname = pluginname_ + "(" + boost::lexical_cast<std::string>(counter) + ")";
    counter++;
  }
}

template <class Data>
IEPluginLegacyAdapter<Data>::~IEPluginLegacyAdapter()
{
  Core::Thread::Guard s(GenericIEPluginManager<Data>::getLock().get());

  auto iter = GenericIEPluginManager<Data>::getMap().find(pluginname_);
  if (iter == GenericIEPluginManager<Data>::getMap().end())
  {
    std::cerr << "WARNING: FieldIEPlugin " << pluginname_ << " not found in database for removal.\n";
  }
  else
  {
    GenericIEPluginManager<Data>::getMap().erase(iter);
  }

  if (GenericIEPluginManager<Data>::getMap().empty())
  {
    GenericIEPluginManager<Data>::destroyMap();
  }
}

template <class Data>
boost::shared_ptr<Data> IEPluginLegacyAdapter<Data>::readFile(const std::string& filename, Core::Logging::LoggerHandle log) const
{
  return filereader_(log, filename.c_str());
}

template <class Data>
bool IEPluginLegacyAdapter<Data>::writeFile(boost::shared_ptr<Data> f, const std::string& filename, Core::Logging::LoggerHandle log) const
{
  return filewriter_(log, f, filename.c_str());
}

template <class Data>
bool IEPluginLegacyAdapter<Data>::equals(const GenericIEPluginInterface<Data>& other) const
{
  auto impl = dynamic_cast<const IEPluginLegacyAdapter<Data>*>(&other);
  if (!impl)
    return false;
  return *this == *impl;
}

template <class Data>
bool IEPluginLegacyAdapter<Data>::operator==(const IEPluginLegacyAdapter<Data>& other) const
{
  return (pluginname_ == other.pluginname_ &&
    fileextension_ == other.fileextension_ &&
    filemagic_ == other.filemagic_ &&
    filereader_ == other.filereader_ &&
    filewriter_ == other.filewriter_);
}

template <class Data>
std::string defaultImportTypeForFile(const GenericIEPluginManager<Data>* mgr = 0)
{
  return "";
}

template <>
SCISHARE std::string defaultImportTypeForFile(const GenericIEPluginManager<Field>* mgr)
{
  return "SCIRun Field File (*.fld)";
}

template <class Data>
std::string makeGuiTypesListForImport(const GenericIEPluginManager<Data>& mgr)
{
  std::vector<std::string> importers;
  mgr.get_importer_list(importers);

  std::ostringstream importtypes;
  importtypes << defaultImportTypeForFile(&mgr);

  for (size_t i = 0; i < importers.size(); i++)
  {
    auto pl = mgr.get_plugin(importers[i]);
    importtypes << ";;" << importers[i];
    if (!pl->fileExtension().empty())
    {
       importtypes << " (*." << pl->fileExtension() << ")";
    }
    else
    {
      importtypes << " (.*)";
    }
  }

  return importtypes.str();
}

template <class Data>
std::string defaultExportTypeForFile(const GenericIEPluginManager<Data>* mgr = 0)
{
  return "";
}

template <>
SCISHARE std::string defaultExportTypeForFile(const GenericIEPluginManager<Field>* mgr)
{
  return "SCIRun Field Binary (*.fld);;SCIRun Field ASCII (*.fld)";
}

template <class Data>
std::string makeGuiTypesListForExport(const GenericIEPluginManager<Data>& mgr)
{
  std::vector<std::string> exporters;
  mgr.get_exporter_list(exporters);

  std::ostringstream exporttypes;
  exporttypes << defaultExportTypeForFile(&mgr);

  for (size_t i = 0; i < exporters.size(); i++)
  {
    auto pl = mgr.get_plugin(exporters[i]);
    exporttypes << ";;" << exporters[i];
    if (!pl->fileExtension().empty())
    {
      exporttypes << " (*." << pl->fileExtension() << ")";
    }
    else
    {
      exporttypes << " (.*)";
    }
  }

  return exporttypes.str();
}

}

#endif
