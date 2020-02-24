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


#ifndef MODULES_LEGACY_MATLAB_DATAIO_IMPORTMATRICESFROMMATLAB_H
#define MODULES_LEGACY_MATLAB_DATAIO_IMPORTMATRICESFROMMATLAB_H

#include <Modules/Legacy/Matlab/DataIO/ImportFieldsFromMatlab.h>
#include <Modules/Legacy/Matlab/DataIO/share.h>

namespace SCIRun {
namespace Modules {
namespace Matlab {

  class SCISHARE ImportMatricesFromMatlab : public MatlabFileIndexModule,
    public Has1InputPort<StringPortTag>,
    public Has7OutputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, StringPortTag>
  {
  public:
    ImportMatricesFromMatlab();
    virtual void execute() override;
    virtual void setStateDefaults() override;
    INPUT_PORT(0, Filename, String);
    OUTPUT_PORT(0, Matrix1, Matrix);
    OUTPUT_PORT(1, Matrix2, Matrix);
    OUTPUT_PORT(2, Matrix3, Matrix);
    OUTPUT_PORT(3, Matrix4, Matrix);
    OUTPUT_PORT(4, Matrix5, Matrix);
    OUTPUT_PORT(5, Matrix6, Matrix);
    OUTPUT_PORT(6, FilenameOut, String);
    enum { NUMPORTS = 6 };

    LEGACY_MATLAB_MODULE

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

  protected:
    virtual void postStateChangeInternalSignalHookup() override;
    virtual SCIRun::Core::Datatypes::DatatypeHandle processMatlabData(const SCIRun::MatlabIO::matlabarray&) const override;
    virtual int indexMatlabFile(MatlabIO::matlabconverter& converter, const MatlabIO::matlabarray& mlarray, std::string& infostring) const override;
  };
}}}

#endif
