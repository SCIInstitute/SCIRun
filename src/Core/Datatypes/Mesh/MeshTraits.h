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


/// @todo Documentation Core/Datatypes/Mesh/MeshTraits.h

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#ifndef CORE_DATATYPES_MESHTRAITS_H
#define CORE_DATATYPES_MESHTRAITS_H

#include <vector>
#include <Core/Datatypes/Mesh/FieldFwd.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/Datatypes/Mesh/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <class VirtualMeshType>
  class MeshFacade;

  template <class VirtualMeshType>
  class MeshTraits
  {
  public:
    typedef SCIRun::mask_type                  mask_type;
    typedef SCIRun::index_type                 under_type;
    typedef SCIRun::index_type                 index_type;
    typedef SCIRun::index_type                 size_type;
    typedef std::vector<index_type>            array_type;
    typedef std::vector<size_type>             dimension_type;

    typedef boost::shared_ptr<MeshFacade<VirtualMeshType>> MeshFacadeHandle;
  };

  // We reserve the last unsigned int value as a marker for bad mesh
  // indices.  This is useful for example when there are no neighbors,
  // or when elements are deleted.
#define MESH_NO_NEIGHBOR -1

}}}

#endif
