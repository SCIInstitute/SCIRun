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


#ifndef SPIRE_RENDER_GEOMPROMISE_HPP
#define SPIRE_RENDER_GEOMPROMISE_HPP

#include <es-log/trace-log.h>
#include <cstring>
#include <es-cereal/ComponentSerialize.hpp>
#include <spire/scishare.h>

namespace ren {

struct GeomPromise
{
  // -- Data --
  static const int MaxAssetName = 64;

  /// Name of the shader that was promised.
  char assetName[MaxAssetName];

  // Indicator variable. Never serialized out and is used to indicate when
  // we need to re-issue a request to the filesystem.
  bool requestInitiated;

  // -- Functions --
  GeomPromise()
  {
    requestInitiated = false;
    assetName[0] = 0;
  }

  static const char* getName()    {return "ren:GeomPromise";}

  void setAssetName(const char* name)
  {
    size_t nameLen = std::strlen(name);
    if (nameLen < MaxAssetName - 1)
    {
      std::strncpy(assetName, name, MaxAssetName);
    }
    else
    {
      logRendererError("GeomPromise: Unable to set name: {}. Name must be {} characters or shorter.",
        name, MaxAssetName - 1);
    }
  }

  bool serialize(spire::ComponentSerialize& s, uint64_t /* entityID */)
  {
    std::string asset = assetName;
    s.serialize("name", asset);

    // If we are deserializing, then we need to absolutely sure that
    // requestInitiated is false (although it should be from the constructor
    // and the fact that we did not serialize it).
    if (s.isDeserializing()) requestInitiated = false;
    return true;
  }

};

} // namespace ren

#endif
