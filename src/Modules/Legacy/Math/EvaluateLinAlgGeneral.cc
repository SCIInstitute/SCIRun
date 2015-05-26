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

#include <Modules/Legacy/Math/EvaluateLinAlgGeneral.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
//#include <Core/Parser/LinAlgEngine.h>

//#include <Core/Util/StringUtil.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo EvaluateLinearAlgebraGeneral::staticInfo_("EvaluateLinearAlgebraGeneral", "Math", "SCIRun");

#if 0
namespace SCIRun {

/// @class EvaluateLinAlgGeneral
/// @brief This module performs a user defined linear algebra operation on up to five input matrices. 

class EvaluateLinAlgGeneral : public Module
{
public:
  EvaluateLinAlgGeneral(GuiContext* ctx);
  virtual ~EvaluateLinAlgGeneral() {}

  virtual void execute();
  virtual void presave();

private:
  GuiString guifunction_;

};


DECLARE_MAKER(EvaluateLinAlgGeneral)
#endif

EvaluateLinearAlgebraGeneral::EvaluateLinearAlgebraGeneral() : Module(staticInfo_)
    //guifunction_(get_ctx()->subVar("function"), "o1 = i1 * i2;")
{
  INITIALIZE_PORT(i1);
  INITIALIZE_PORT(i2);
  INITIALIZE_PORT(i3);
  INITIALIZE_PORT(i4);
  INITIALIZE_PORT(i5);
  INITIALIZE_PORT(o1);
  INITIALIZE_PORT(o2);
  INITIALIZE_PORT(o3);
  INITIALIZE_PORT(o4);
  INITIALIZE_PORT(o5);
}

void EvaluateLinearAlgebraGeneral::setStateDefaults()
{
}

void
EvaluateLinearAlgebraGeneral::execute()
{
#if 0
  std::vector<MatrixHandle> imatrix(5);
  std::vector<MatrixHandle> omatrix(5);
  
  for (int i=0; i<5; i++)
  {
    std::string matrixname = std::string("i")+to_string(i+1);
    get_input_handle(matrixname,imatrix[i],false);
  }
  
  TCLInterface::eval(get_id()+" update_text");

  if (inputs_changed_ || guifunction_.changed() || !oport_cached("o1")
        || !oport_cached("o2") || !oport_cached("o3")
        || !oport_cached("o4") || !oport_cached("o5"))
  {
    // Inform module that execution started
    update_state(Executing);
    
    NewLinAlgEngine engine;
    engine.set_progress_reporter(this);
    
    for (size_t p = 0; p < 5; p++)
    {
      std::string matrixname = std::string("o")+to_string(p+1);
      if (oport_connected(matrixname))
      {
        if(!(engine.add_output_matrix(matrixname))) return;
      }
      
      if (imatrix[p].get_rep())
      {
        std::string matrixname = std::string("i")+to_string(p+1);
        if (!(engine.add_input_matrix(matrixname,imatrix[p]))) return;
      }
    }
    
    std::string function = guifunction_.get();

    if(!(engine.add_expressions(function))) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping 
    // over every data point

    if (!(engine.run()))
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      error("");
      error("This module does not fully support backwards compatibility:");
      error("C++/C functions are not supported in by this module anymore.");
      error("Please review documentation to explore available functionality and grammar of this module.");
      error("We are sorry for this inconvenience, but we do not longer support dynamically compiling in SCIRun.");
      ///-----------------------
      
      return;
    }
    
    for (size_t p = 0; p < 5; p++)
    {
      std::string matrixname = std::string("o")+to_string(p+1);
      engine.get_matrix(matrixname,omatrix[p]);
    }

    send_output_handle("o1", omatrix[0]);
    send_output_handle("o2", omatrix[1]);
    send_output_handle("o3", omatrix[2]);
    send_output_handle("o4", omatrix[3]);
    send_output_handle("o5", omatrix[4]);
  }
#endif
}
