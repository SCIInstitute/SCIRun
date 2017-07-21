#include <fstream>
#include <iostream>
#include "FilesystemSync.hpp"

namespace CPM_ES_FS_NS {

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
        std::cerr << "Failed to read contents of file " << filename << std::endl;
        item.callback(item.assetName, true, 0, nullptr);
      }
      std::free(buffer);
    }
    else
    {
      std::cerr << "Failed to open file " << filename << std::endl;
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

} // namespace CPM_ES_FS_NS

