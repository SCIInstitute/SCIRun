#ifndef IAUNS_ASYNC_FILESYSTEMEMSCRIPTEN_HPP
#define IAUNS_ASYNC_FILESYSTEMEMSCRIPTEN_HPP

#include <vector>
#include "../Filesystem.hpp"

namespace CPM_ES_FS_NS {

// Use this system if you want to provide synchronous operations on disk.
// Only used when disk is present.
class FilesystemEm : public Filesystem
{
public:
  FilesystemEm(const std::string& prefix, const std::string& preloadPrefix)
  {
    mPrefix = prefix;
    mPreloadPrefix = preloadPrefix;
  }

  virtual ~FilesystemEm()                       {}

  void update() override;

  void readFile(const std::string& assetName, const FSCallback& cb) override;

private:

  static void onLoad(void* ourData, void* data, int dataSize);
  static void onError(void* ourData);

  // Vector of assets to load when the system update arrives.
  struct AssetItem
  {
    AssetItem(const std::string& file, const FSCallback& cb) :
        assetName(file),
        callback(cb)
    {}

    std::string assetName;
    FSCallback  callback;
  };

  std::string   mPrefix;
  std::string   mPreloadPrefix;
};

} // namespace CPM_ES_FS_NS

#endif 
