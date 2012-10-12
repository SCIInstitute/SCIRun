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
 *  CreateLatVol.cc:  Make an ImageField that fits the source field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 */

#ifndef MODULES_FIELDS_CREATELATVOLBASIC_H
#define MODULES_FIELDS_CREATELATVOLBASIC_H

#include <Dataflow/Network/Module.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

class CreateLatVolBasic : public SCIRun::Dataflow::Networks::Module,
  public Has1OutputPort<FieldPortTag>
{
  public:
    CreateLatVolBasic();

    virtual void execute();

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