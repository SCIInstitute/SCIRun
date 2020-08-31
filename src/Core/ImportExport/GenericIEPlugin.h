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


#ifndef SCI_project_GenericIEPlugin_h
#define SCI_project_GenericIEPlugin_h 1

#include <Core/Logging/LoggerFwd.h>
#include <Core/Thread/Mutex.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <Core/ImportExport/share.h>

namespace SCIRun 
{

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
  virtual bool hasReader() const override { return filereader_ != nullptr; }
  virtual bool hasWriter() const override { return filewriter_ != nullptr; }

  virtual boost::shared_ptr<Data> readFile(const std::string& filename, Core::Logging::LoggerHandle log) const override;
  virtual bool writeFile(boost::shared_ptr<Data> f, const std::string& filename, Core::Logging::LoggerHandle log) const override;
  virtual bool equals(const GenericIEPluginInterface<Data>& other) const override;

  IEPluginLegacyAdapter(const std::string &name,
    const std::string &fileextension,
    const std::string &filemagic,
    boost::shared_ptr<Data> (*freader)(Core::Logging::LoggerHandle pr, const char *filename) = nullptr,
    bool (*fwriter)(Core::Logging::LoggerHandle pr, boost::shared_ptr<Data> f, const char *filename) = nullptr);

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
class PluginMap
{
public:
  PluginMap() : lock_("IE plugin map"), pluginTable_(nullptr) {}
  Core::Thread::Mutex& getLock();
  using Map = std::map<std::string, GenericIEPluginInterface<Data>*>;
  Map& getMap();
  void createMap();
  void destroyMap();
  size_t numPlugins() const;
private:
  Core::Thread::Mutex lock_;
  Map* pluginTable_;
};

template <class Data>
class GenericIEPluginManager
{
public:
  size_t numPlugins() const { return map_.numPlugins(); }
  void get_importer_list(std::vector<std::string>& results) const;
  void get_exporter_list(std::vector<std::string>& results) const;
  GenericIEPluginInterface<Data>* get_plugin(const std::string& name) const;
  static PluginMap<Data>& getMap();
protected:
  static PluginMap<Data> map_;
};

template <class Data>
PluginMap<Data>& GenericIEPluginManager<Data>::getMap()
{
  return map_;
}

template <class Data>
Core::Thread::Mutex& PluginMap<Data>::getLock()
{
  return lock_;
}

template <class Data>
typename PluginMap<Data>::Map& PluginMap<Data>::getMap()
{
  if (!pluginTable_)
    createMap();
  return *pluginTable_;
}

template <class Data>
void PluginMap<Data>::createMap()
{
  if (!pluginTable_)
  {
    pluginTable_ = new Map();
  }
}

template <class Data>
PluginMap<Data> GenericIEPluginManager<Data>::map_;

template <class Data>
size_t PluginMap<Data>::numPlugins() const
{
  return pluginTable_ ? pluginTable_->size() : 0;
}

template <class Data>
void GenericIEPluginManager<Data>::get_importer_list(std::vector<std::string>& results) const
{
  if (0 == map_.numPlugins())
  {
    return;
  }

  Core::Thread::Guard s(map_.getLock().get());
  for (const auto& plugin : map_.getMap())
  {
    if (plugin.second->hasReader())
      results.push_back(plugin.first);
  }
}

template <class Data>
void GenericIEPluginManager<Data>::get_exporter_list(std::vector<std::string>& results) const
{
  if (0 == map_.numPlugins())
  {
    return;
  }

  Core::Thread::Guard s(map_.getLock().get());
  for (const auto& plugin : map_.getMap())
  {
    if (plugin.second->hasWriter())
      results.push_back(plugin.first);
  }
}

template <class Data>
GenericIEPluginInterface<Data>* GenericIEPluginManager<Data>::get_plugin(const std::string &name) const
{
  if (0 == map_.numPlugins())
    return nullptr;

  Core::Thread::Guard s(map_.getLock().get());
  // Should check for invalid name.
  auto loc = map_.getMap().find(name);
  if (loc == map_.getMap().end())
  {
    return nullptr;
  }
  else
  {
    return loc->second;
  }
}

template <class Data>
void PluginMap<Data>::destroyMap()
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
  Core::Thread::Guard s(GenericIEPluginManager<Data>::getMap().getLock().get());

  GenericIEPluginManager<Data>::getMap().createMap();

  std::string tmppname = pluginname_;
  int counter = 2;
  for (;;)
  {
    auto loc = GenericIEPluginManager<Data>::getMap().getMap().find(tmppname);
    if (loc == GenericIEPluginManager<Data>::getMap().getMap().end())
    {
      if (tmppname != pluginname_) { const_cast<std::string&>(pluginname_) = tmppname; }
      GenericIEPluginManager<Data>::getMap().getMap()[pluginname_] = this;
      break;
    }
    if (*(*loc).second == *this)
    {
      //TODO: inject new logger std::cerr << "WARNING: IEPlugin '" << tmppname << "' duplicated.\n";
      break;
    }

    //TODO: inject new logger std::cout << "WARNING: Multiple IEPlugins with '" << pluginname_ << "' name.\n";
    tmppname = pluginname_ + "(" + boost::lexical_cast<std::string>(counter) + ")";
    counter++;
  }
}

template <class Data>
IEPluginLegacyAdapter<Data>::~IEPluginLegacyAdapter()
{
  Core::Thread::Guard s(GenericIEPluginManager<Data>::getMap().getLock().get());

  auto iter = GenericIEPluginManager<Data>::getMap().getMap().find(pluginname_);
  if (iter == GenericIEPluginManager<Data>::getMap().getMap().end())
  {
    //TODO: inject new logger std::cerr << "WARNING: IEPlugin " << pluginname_ << " not found in database for removal.\n";
  }
  else
  {
    GenericIEPluginManager<Data>::getMap().getMap().erase(iter);
  }

  if (GenericIEPluginManager<Data>::getMap().getMap().empty())
  {
    GenericIEPluginManager<Data>::getMap().destroyMap();
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
std::string defaultImportTypeForFile(const GenericIEPluginManager<Data>* mgr = nullptr)
{
  return "";
}

template <>
SCISHARE std::string defaultImportTypeForFile(const GenericIEPluginManager<Field>* mgr);

template <>
SCISHARE std::string defaultImportTypeForFile(const GenericIEPluginManager<Core::Datatypes::Matrix>* mgr);

template <>
SCISHARE std::string defaultImportTypeForFile(const GenericIEPluginManager<NrrdData>* mgr);

SCISHARE std::string fileTypeDescriptionFromDialogBoxFilter(const std::string& fileFilter);

template <class Data>
std::string dialogBoxFilterFromFileTypeDescription(const GenericIEPluginManager<Data>& mgr, const std::string& name)
{
  if (name.find("*") != std::string::npos) // user has set state variable with full filter string
    return name;
  std::ostringstream filter;
  auto pl = mgr.get_plugin(name);
  auto ext = pl ? pl->fileExtension() : std::string();
  filter << name << " (" << (!ext.empty() ? ext : "*.*") << ")";
  return filter.str();
}

template <class Data>
std::function<std::string(const std::string&)> dialogBoxFilterFromFileTypeDescription(const GenericIEPluginManager<Data>& mgr)
{
  return [&mgr](const std::string& name) { return dialogBoxFilterFromFileTypeDescription(mgr, name); };
}

template <class Data>
std::string printPluginDescriptionsForFilter(const GenericIEPluginManager<Data>& mgr, const std::string& defaultType, const std::vector<std::string>& pluginNames)
{
  std::ostringstream types;
  types << defaultType;

  // Qt dialog-specific formatting
  for (const auto& name : pluginNames)
  {
    types << ";;" << dialogBoxFilterFromFileTypeDescription(mgr, name);
  }
  return types.str();
}

template <class Data>
std::string makeGuiTypesListForImport(const GenericIEPluginManager<Data>& mgr)
{
  std::vector<std::string> importers;
  mgr.get_importer_list(importers);

  return printPluginDescriptionsForFilter(mgr, defaultImportTypeForFile(&mgr), importers);
}

template <class Data>
std::string defaultExportTypeForFile(const GenericIEPluginManager<Data>* mgr = nullptr)
{
  return "";
}

template <>
SCISHARE std::string defaultExportTypeForFile(const GenericIEPluginManager<Field>* mgr);

template <>
SCISHARE std::string defaultExportTypeForFile(const GenericIEPluginManager<Core::Datatypes::Matrix>* mgr);

template <class Data>
std::string makeGuiTypesListForExport(const GenericIEPluginManager<Data>& mgr)
{
  std::vector<std::string> exporters;
  mgr.get_exporter_list(exporters);
  return printPluginDescriptionsForFilter(mgr, defaultExportTypeForFile(&mgr), exporters);
}

}

#endif
