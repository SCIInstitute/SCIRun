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


#include <fstream>
#include <iostream>
#include "FilesystemSync.hpp"

namespace spire {

void FilesystemSync::update()
{
  // We make a copy so that callbacks can modify mAssets without there
  // being repercussions for our loop. Possibly use std::move?
  std::vector<AssetItem> assetsToProcess = mAssets;
  mAssets.clear();

  for (const AssetItem& item : assetsToProcess)
  {
    std::string filename = mPrefix + item.assetName;

    // Load the asset from disk and initiate the callback.
    std::ifstream file(filename, std::ios::binary);
    if (file)
    {
      file.seekg(0, std::ios::end);
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);

      uint8_t* buffer = static_cast<uint8_t*>(std::malloc(static_cast<size_t>(size)));
      if (file.read(reinterpret_cast<char*>(buffer), size))
      {
        item.callback(item.assetName, false, static_cast<size_t>(size), buffer);
      }
      else
      {
        logRendererError("Failed to read contents of file {}", filename);
        item.callback(item.assetName, true, 0, nullptr);
      }
      std::free(buffer);
    }
    else
    {
      logRendererWarning("Failed to open file {}", filename);
      item.callback(item.assetName, true, 0, nullptr);
    }
  }
}

void FilesystemSync::readFile(const std::string& assetName, const FSCallback& cb)
{
  /// Add the file loading to our 'todo' list and get back to it when we are
  /// issued a filesystem update.
  //\cb std::cout << "Requesting file: " << assetName << std::endl;
  mAssets.emplace_back(assetName, cb);
}

} // namespace spire
