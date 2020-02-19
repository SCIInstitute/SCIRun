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


#ifndef ASYNC_SPIRE_FILESYSTEM_HPP
#define ASYNC_SPIRE_FILESYSTEM_HPP

#include <es-log/trace-log.h>
#include <string>
#include <functional>
#include <es-acorn/Acorn.hpp>
#include <spire/scishare.h>

namespace spire {

/// Possibly use https://github.com/d5/node.native for C++ libuv support.
typedef std::function<void(const std::string& assetName, bool error,
                           size_t bytesRead, uint8_t* buffer)> FSCallback;

class SCISHARE Filesystem
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
  static void registerSystems(spire::Acorn& core);
};

} // namespace spire

#endif
