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


///
///@author
///   Michael Callahan,
///   Department of Computer Science,
///   University of Utah
///@date  February 2001
///

#ifndef CORE_ALGORITHMS_FIELDS_FIELDDATA_CONVERTMATRICESTOMESH_H
#define CORE_ALGORITHMS_FIELDS_FIELDDATA_CONVERTMATRICESTOMESH_H 1

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {
      class SCISHARE ConvertMatricesToMesh : public Dataflow::Networks::Module,
        public Has3InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has1OutputPort<FieldPortTag>
      {
      public:
        ConvertMatricesToMesh();
        virtual void setStateDefaults() override;
        virtual void execute() override;

        INPUT_PORT(0, MeshElements, Matrix);
        INPUT_PORT(1, MeshPositions, Matrix);
        INPUT_PORT(2, MeshNormals, Matrix);
        OUTPUT_PORT(0, OutputField, Field);

        static const Core::Algorithms::AlgorithmParameterName InputFieldName;
        static const Core::Algorithms::AlgorithmParameterName InputFieldTypeName;
        static const Core::Algorithms::AlgorithmParameterName FieldBaseType;
        static const Core::Algorithms::AlgorithmParameterName DataType;
        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      private:
        void process_elements(VMesh* mesh, size_type positionRows);
      };
    }
  }
} // End namespace SCIRun

#endif
