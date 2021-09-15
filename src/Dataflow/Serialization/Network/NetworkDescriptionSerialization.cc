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


#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>
#include <boost/filesystem/operations.hpp>
#include <Dataflow/Serialization/Network/NetworkXMLSerializer.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::State;

void ToolkitFile::load(std::istream& istr)
{
  auto xmlPtr = XMLSerializer::load_xml<ToolkitFile>(istr);
  if (xmlPtr)
    (*this) = *xmlPtr;
}

void ToolkitFile::save(std::ostream& ostr) const
{
  XMLSerializer::save_xml(*this, ostr, "toolkit");
}

namespace
{
  namespace fs = boost::filesystem;

  fs::path addSlash(const fs::path& p)
  {
    auto fullBasePath(p);
    if ("." != fullBasePath.filename())
      fullBasePath += fs::path::preferred_separator;
    return fullBasePath;
  }

  fs::path diffPath(const fs::path& basePath, const fs::path& newPath)
  {
    auto fullBasePath = addSlash(basePath);
    auto tmpPath = newPath;
    fs::path diffpath;
    while (addSlash(tmpPath) != fullBasePath)
    {
      diffpath = tmpPath.stem() / diffpath;
      tmpPath = tmpPath.parent_path();
    }

    auto filename = diffpath.leaf().string() + newPath.extension().string();
    diffpath.remove_leaf() /= filename;
    return diffpath;
  }
}

ToolkitFile SCIRun::Dataflow::Networks::makeToolkitFromDirectory(const boost::filesystem::path& toolkitPath)
{
  ToolkitFile toolkit;

  for (const auto& p : boost::filesystem::recursive_directory_iterator(toolkitPath))
  {
    if (p.path().extension() == ".srn5")
    {
      auto path = diffPath(toolkitPath, p.path()).string();
      std::replace(path.begin(), path.end(), '\\', '/');
      toolkit.networks[path] = *XMLSerializer::load_xml<NetworkFile>(p.path().string());
    }
  }
  return toolkit;
}

class NetworkSerializationWrapper : public NetworkSerializationInterface
{
public:
  explicit NetworkSerializationWrapper(const NetworkXML* xml)
  {
    if (!xml)
      return;

    for (const auto& mod : xml->modules)
    {
      moduleMap_[mod.first] = { mod.second.module, makeShared<SimpleMapModuleState>(mod.second.state) };
    }
    for (const auto& conn : xml->connections)
    {
      sortedConnections_.push_back(conn);
    }
    std::sort(sortedConnections_.begin(), sortedConnections_.end());
  }

  std::map<std::string, std::pair<ModuleLookupInfo, ModuleStateHandle>> modules() const override
  {
    return moduleMap_;
  }

  std::vector<ConnectionDescription> sortedConnections() const override
  {
    return sortedConnections_;
  }
private:
  std::map<std::string, std::pair<ModuleLookupInfo, ModuleStateHandle>> moduleMap_;
  std::vector<ConnectionDescription> sortedConnections_;
};

NetworkSerializationInterfaceHandle NetworkXML::data() const
{
  return makeShared<NetworkSerializationWrapper>(this);
}
