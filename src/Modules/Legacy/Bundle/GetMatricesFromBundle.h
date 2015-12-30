/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_LEGACY_BUNDLE_GETMATRICESFROMBUNDLE_H__
#define MODULES_LEGACY_BUNDLE_GETMATRICESFROMBUNDLE_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Bundle/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Bundles {

      /// @class GetMatricesFromBundle
      /// @brief This module retrieves a matrix object from a bundle.

      class SCISHARE GetMatricesFromBundle : public Dataflow::Networks::Module,
        public Has1InputPort<BundlePortTag>,
        public Has7OutputPorts<BundlePortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>
      {
      public:
        GetMatricesFromBundle();
        virtual void setStateDefaults() override;
        virtual void execute() override;

        INPUT_PORT(0, InputBundle, Bundle);
        OUTPUT_PORT(0, OutputBundle, Bundle);
        OUTPUT_PORT(1, matrix1, Matrix);
        OUTPUT_PORT(2, matrix2, Matrix);
        OUTPUT_PORT(3, matrix3, Matrix);
        OUTPUT_PORT(4, matrix4, Matrix);
        OUTPUT_PORT(5, matrix5, Matrix);
        OUTPUT_PORT(6, matrix6, Matrix);

        static const Core::Algorithms::AlgorithmParameterName MatrixNameList;
        static const Core::Algorithms::AlgorithmParameterName MatrixNames[];
        static const int NUM_BUNDLE_OUT = 6; //TODO: get from class def

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      private:
        std::string makeMatrixNameList(const Core::Datatypes::Bundle& bundle) const;
      };

    }
  }
}

#endif
