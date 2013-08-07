/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

/*
 *  CreateLatVol.cc:  Make an LatVol that fits the source field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 */

#ifndef MODULES_FIELDS_CREATELATVOLMESH_H
#define MODULES_FIELDS_CREATELATVOLMESH_H

#include <Dataflow/Network/Module.h>
#include <Modules/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

class SCISHARE CreateLatVolMesh : public SCIRun::Dataflow::Networks::Module,
  public Has1OutputPort<MeshPortTag>,
  public HasNoInputPorts
{
  public:
    CreateLatVolMesh();

    virtual void execute();

    OUTPUT_PORT(0, OutputSampleField, LegacyMesh);

    static Core::Algorithms::AlgorithmParameterName XSize;
    static Core::Algorithms::AlgorithmParameterName YSize;
    static Core::Algorithms::AlgorithmParameterName ZSize;
    static Core::Algorithms::AlgorithmParameterName PadPercent;
    static Core::Algorithms::AlgorithmParameterName ElementSizeNormalized;

  private:
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    GuiInt size_x_;
    GuiInt size_y_;
    GuiInt size_z_;
    GuiDouble padpercent_;
    GuiString data_at_;
    GuiString element_size_;

    enum DataTypeEnum { SCALAR, VECTOR, TENSOR };
#endif 
};

}}}

#endif