#include <fstream>
#include <iostream>
#include "FilesystemDummy.hpp"

namespace CPM_ES_FS_NS {

void FilesystemDummy::update()
{
  // We make a copy so that callbacks can modify mAssets without there
  // being repercussions for our loop. Possibly use std::move?
  std::vector<AssetItem> assetsToProcess = mAssets;
  mAssets.clear();

  for (const AssetItem& item : assetsToProcess)
  {
    std::string filename = mPrefix + item.assetName;

    std::cerr << "Failed to read contents of file " << filename << std::endl;
    std::cerr << "because this is a dummy filesystem." << std::endl;
    item.callback(item.assetName, true, 0, nullptr);
  }
}

void FilesystemDummy::readFile(const std::string& assetName, const FSCallback& cb)
{
  /// Add the file loading to our 'todo' list and get back to it when we are
  /// issued a filesystem update.
  mAssets.emplace_back(assetName, cb);
}

} // namespace CPM_ES_FS_NS

