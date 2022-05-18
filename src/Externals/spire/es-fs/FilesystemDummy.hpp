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


#ifndef SPIRE_ASYNC_FILESYSTEMDUMMY_HPP
#define SPIRE_ASYNC_FILESYSTEMDUMMY_HPP

#include <es-log/trace-log.h>
#include <vector>
#include "Filesystem.hpp"
#include <spire/scishare.h>

namespace spire {

// Use this system if you want to provide synchronous operations on disk.
// Only used when disk is present.
class FilesystemDummy : public Filesystem
{
public:
  FilesystemDummy(const std::string& prefix = "")  {mPrefix = prefix;}
  virtual ~FilesystemDummy()                       {}

  void update() override;

  void readFile(const std::string& assetName, const FSCallback& cb) override;

private:

  // Vector of assets to load when the system update arrives.
  struct AssetItem
  {
    AssetItem(const std::string& asset, const FSCallback& cb) :
        assetName(asset),
        callback(cb)
    {}

    std::string assetName;
    FSCallback  callback;
  };

  // Assets that are to be loaded.
  std::vector<AssetItem>  mAssets;
  std::string             mPrefix;
};

} // namespace spire

#endif
