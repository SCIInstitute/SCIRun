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
 *  FieldIEPlugin:  Data structure needed to make a SCIRun FieldIE Plugin
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2004
 *
 */

#ifndef SCI_project_FieldIEPlugin_h
#define SCI_project_FieldIEPlugin_h 1

#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/ImportExport/GenericIEPlugin.h>
#include <Core/Thread/Mutex.h>
#include <boost/lexical_cast.hpp>

#include <Core/ImportExport/share.h>

namespace SCIRun {

typedef GenericIEPluginInterface<Field> FieldIEPlugin;

template <class Data>
class SCISHARE IEPluginLegacyAdapter : public GenericIEPluginInterface<Data>
{
public:
  virtual std::string pluginname() const override { return pluginname_; }
  virtual std::string fileExtension() const override { return fileextension_; }
  virtual std::string fileMagic() const override { return filemagic_; }

  virtual boost::shared_ptr<Data> readFile(const std::string& filename, Core::Logging::Log& log) const override;
  virtual bool writeFile(boost::shared_ptr<Data> f, const std::string& filename, Core::Logging::Log& log) const override;
  virtual bool equals(const GenericIEPluginInterface<Data>& other) const override;

  const std::string pluginname_;
  const std::string fileextension_;
  const std::string filemagic_;

  boost::shared_ptr<Data> (*filereader_)(Core::Logging::Log& pr, const char *filename);
  bool (*filewriter_)(Core::Logging::Log& pr,
    boost::shared_ptr<Data> f, const char *filename);

  IEPluginLegacyAdapter(const std::string &name,
    const std::string &fileextension,
    const std::string &filemagic,
    boost::shared_ptr<Data> (*freader)(Core::Logging::Log& pr, const char *filename) = 0,
    bool (*fwriter)(Core::Logging::Log& pr, boost::shared_ptr<Data> f, const char *filename) = 0);

  ~IEPluginLegacyAdapter();

  bool operator==(const IEPluginLegacyAdapter& other) const;
};

typedef IEPluginLegacyAdapter<Field> FieldIEPluginLegacyAdapter;

typedef GenericIEPluginManager<Field> FieldIEPluginManager;

template <class Data>
IEPluginLegacyAdapter<Data>::IEPluginLegacyAdapter(const std::string& pname,
  const std::string& fextension,
  const std::string& fmagic,
  boost::shared_ptr<Data> (*freader)(Core::Logging::Log& pr, const char *filename),
  bool (*fwriter)(Core::Logging::Log& pr, boost::shared_ptr<Data> f, const char *filename))
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
boost::shared_ptr<Data> IEPluginLegacyAdapter<Data>::readFile(const std::string& filename, Core::Logging::Log& log) const
{
  return filereader_(log, filename.c_str());
}

template <class Data>
bool IEPluginLegacyAdapter<Data>::writeFile(boost::shared_ptr<Data> f, const std::string& filename, Core::Logging::Log& log) const
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



}

#endif
