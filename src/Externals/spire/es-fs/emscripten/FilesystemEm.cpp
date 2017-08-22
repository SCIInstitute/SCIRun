#include <fstream>
#include <iostream>
#include <emscripten/emscripten.h>
#include "FilesystemEm.hpp"

namespace CPM_ES_FS_NS {

void FilesystemEm::update()
{
}

void FilesystemEm::readFile(const std::string& assetName, const FSCallback& cb)
{
  // Issue the request asynchronously. We will get a callback either way.
  std::string preloadURL = mPreloadPrefix + assetName;

  // Check to see if we already have the file preloaded with the javascript.
  // Otherwise, asynchronously cache it.
  FILE* f = fopen(preloadURL.c_str(), "rb");

  if (!f)
  {
    /// Add the file loading to our 'todo' list and get back to it when we are
    /// issued a filesystem update.
    AssetItem* item = new AssetItem(assetName, cb);

    std::string url = mPrefix + assetName;
    std::cout << "Fetching " << url << std::endl;
    emscripten_async_wget_data(url.c_str(), reinterpret_cast<void*>(item), onLoad, onError);
  }
  else
  {
    // Load the asset immediately and issue the callback.
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* contents = static_cast<uint8_t*>(malloc(fsize));
    if (contents)
    {
      fread(contents, fsize, 1, f);
      cb(assetName, false, fsize, contents);
      free(contents);
    }
    else
    {
      std::cerr << "Unable to read preloaded file: " << assetName << std::endl;
    }
    fclose(f);
  }
}


void FilesystemEm::onLoad(void* ourData, void* data, int dataSize)
{
  AssetItem* item = reinterpret_cast<AssetItem*>(ourData);
  item->callback(item->assetName, false, static_cast<size_t>(dataSize),
                 reinterpret_cast<uint8_t*>(data));
  delete item;
}

void FilesystemEm::onError(void* ourData)
{
  AssetItem* item = reinterpret_cast<AssetItem*>(ourData);
  std::cerr << "Failed to read contents of file " << item->assetName << std::endl;
  item->callback(item->assetName, true, 0, nullptr);
  delete item;
}

} // namespace CPM_ES_FS_NS 

