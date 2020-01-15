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


#ifndef CORE_ALGORITHMS_FIELDS_CLIPMESH_CLIPMESHBYISOVALUE_H
#define CORE_ALGORITHMS_FIELDS_CLIPMESH_CLIPMESHBYISOVALUE_H 1

// Datatypes that the algorithm uses
//#include <Core/Datatypes/Mesh.h>
//#include <Core/Datatypes/Field.h>
//#include <Core/Datatypes/Matrix.h>

// Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>

// for Windows support
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
 namespace Core {
  namespace Algorithms {
   namespace Fields {

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
 using namespace SCIRun;
#endif

class SCISHARE ClipMeshByIsovalueAlgo : public AlgorithmBase
{
  public:

    /// Set defaults
    ClipMeshByIsovalueAlgo();

    /// run the algorithm
    bool run(FieldHandle input, FieldHandle& output, Datatypes::MatrixHandle& mapping) const;
    bool run(FieldHandle input, FieldHandle& output) const;
    virtual AlgorithmOutput run(const AlgorithmInput& input) const;
    static AlgorithmInputName InputField;
    static AlgorithmOutputName OutputField;
    static AlgorithmParameterName LessThanIsoValue;
    static AlgorithmParameterName ScalarIsoValue;
};

}}}} // end namespace SCIRunAlgo

#endif
