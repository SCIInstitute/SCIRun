/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
 University of Utah.
 
 License for the specific language governing rights and limitations under
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

#ifndef MODULES_STRING_PRINTMATRIXINTOSTRING_H
#define MODULES_STRING_PRINTMATRIXINTOSTRING_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/String/share.h>

namespace SCIRun {
  namespace Modules {
    namespace StringManip {
      
      /// @class PrintMatrixIntoString
      /// @brief Append the rows or columns of a matrix to a base matrix.
      
      class SCISHARE PrintMatrixIntoString : public SCIRun::Dataflow::Networks::Module,
      public Has2InputPorts<StringPortTag, DynamicPortTag<MatrixPortTag>>,
      public Has1OutputPort<StringPortTag>
      {
      public:
        PrintMatrixIntoString();
        virtual void execute();
        virtual void setStateDefaults();
        virtual bool hasDynamicPorts() const { return true; }
        
        INPUT_PORT(0, Format, String);
        INPUT_PORT_DYNAMIC(1, Input, Matrix);
        OUTPUT_PORT(0, Output, String);
        
        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      };
    }}}

#endif
