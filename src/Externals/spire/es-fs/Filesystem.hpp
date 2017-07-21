#ifndef ASYNC_IAUNS_FILESYSTEM_HPP
#define ASYNC_IAUNS_FILESYSTEM_HPP

#include <string>
#include <functional>

#include <es-acorn/Acorn.hpp>

namespace CPM_ES_FS_NS {

/// Possibly use https://github.com/d5/node.native for C++ libuv support.
typedef std::function<void(const std::string& assetName, bool error,
                           size_t bytesRead, uint8_t* buffer)> FSCallback;

class Filesystem
{
public:
  Filesystem()            {}
  virtual ~Filesystem()   {}
  
  /// Updates the filesystem (runs libuv loop and checks for existance of
  /// usable resources).
  virtual void update() = 0;

  /// Reads a file from 'disk'. This could be implemented as an asynchronous
  /// javascript and XML call just as easily as asynchronously reading from
  /// disk. Later on we may want to consider grouping and bundling assets into
  /// single files for efficient transmission over the web.
  virtual void readFile(const std::string& assetName, const FSCallback& cb) = 0;

  /// Retrieves system name that is used to update the instantiated 
  /// static filesystem component which conforms to this abstract interface.
  /// All this system does is grab the StaticFS componenent, and call the
  /// abstract update function.
  static const char* getFSSystemName();

  /// Registers file system.
  static void registerSystems(CPM_ES_ACORN_NS::Acorn& core);
};

} // namespace CPM_ES_FS_NS 

#endif

