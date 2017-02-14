/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

using namespace SCIRun::Dataflow::Networks;

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
  boost::filesystem::path diffPath(const boost::filesystem::path& basePath, const boost::filesystem::path& newPath)
  {
    namespace fs = boost::filesystem;

    fs::path diffpath;

    auto tmpPath = newPath;
    while (tmpPath != basePath)
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