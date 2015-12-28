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


#include <Core/Datatypes/Legacy/Bundle/Bundle.h>
#include <Core/Datatypes/Matrix.h>

#include <Modules/Legacy/Bundle/GetMatricesFromBundle.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Dataflow::Networks;

const ModuleLookupInfo GetMatricesFromBundle::staticInfo_("GetMatricesFromBundle", "Bundle", "SCIRun");

GetMatricesFromBundle::GetMatricesFromBundle() : Module(staticInfo_)
{

}



#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

using namespace SCIRun;

class GetMatricesFromBundle : public Module {
public:
  GetMatricesFromBundle(GuiContext*);
  virtual void execute();
  
private:
  GuiString             guimatrix1name_;
  GuiString             guimatrix2name_;
  GuiString             guimatrix3name_;
  GuiString             guimatrix4name_;
  GuiString             guimatrix5name_;
  GuiString             guimatrix6name_;
  GuiInt                guitransposenrrd1_;
  GuiInt                guitransposenrrd2_;
  GuiInt                guitransposenrrd3_;
  GuiInt                guitransposenrrd4_;
  GuiInt                guitransposenrrd5_;
  GuiInt                guitransposenrrd6_;
  GuiString             guimatrices_;
};


DECLARE_MAKER(GetMatricesFromBundle)

GetMatricesFromBundle::GetMatricesFromBundle(GuiContext* ctx)
  : Module("GetMatricesFromBundle", ctx, Filter, "Bundle", "SCIRun"),
    guimatrix1name_(get_ctx()->subVar("matrix1-name"), "matrix1"),
    guimatrix2name_(get_ctx()->subVar("matrix2-name"), "matrix2"),
    guimatrix3name_(get_ctx()->subVar("matrix3-name"), "matrix3"),
    guimatrix4name_(get_ctx()->subVar("matrix4-name"), "matrix4"),
    guimatrix5name_(get_ctx()->subVar("matrix5-name"), "matrix5"),
    guimatrix6name_(get_ctx()->subVar("matrix6-name"), "matrix6"),
    guitransposenrrd1_(get_ctx()->subVar("transposenrrd1"), 0),
    guitransposenrrd2_(get_ctx()->subVar("transposenrrd2"), 0),
    guitransposenrrd3_(get_ctx()->subVar("transposenrrd3"), 0),
    guitransposenrrd4_(get_ctx()->subVar("transposenrrd4"), 0),
    guitransposenrrd5_(get_ctx()->subVar("transposenrrd5"), 0),
    guitransposenrrd6_(get_ctx()->subVar("transposenrrd6"), 0),
    guimatrices_(get_ctx()->subVar("matrix-selection",false), "")
{
}

void
GetMatricesFromBundle::execute()
{
  // Define input handle:
  BundleHandle handle;
  
  // Get data from input port:
  get_input_handle("bundle",handle,true);
  
  if (inputs_changed_ || guimatrix1name_.changed() || 
      guimatrix2name_.changed() || guimatrix3name_.changed() || 
      guimatrix4name_.changed() || guimatrix5name_.changed() || 
      guimatrix6name_.changed() || guitransposenrrd1_.changed() ||
      guitransposenrrd2_.changed() || guitransposenrrd3_.changed() ||
      guitransposenrrd4_.changed() || guitransposenrrd5_.changed() ||
      guitransposenrrd6_.changed() || !oport_cached("bundle") || 
      !oport_cached("matrix1") || !oport_cached("matrix2") || 
      !oport_cached("matrix3") || !oport_cached("matrix4") ||
      !oport_cached("matrix5") || !oport_cached("matrix6"))
  {
    update_state(Executing);
  
    MatrixHandle fhandle;
    std::string matrix1name = guimatrix1name_.get();
    std::string matrix2name = guimatrix2name_.get();
    std::string matrix3name = guimatrix3name_.get();
    std::string matrix4name = guimatrix4name_.get();
    std::string matrix5name = guimatrix5name_.get();
    std::string matrix6name = guimatrix6name_.get();
    int transposenrrd1 = guitransposenrrd1_.get();
    int transposenrrd2 = guitransposenrrd2_.get();
    int transposenrrd3 = guitransposenrrd3_.get();
    int transposenrrd4 = guitransposenrrd4_.get();
    int transposenrrd5 = guitransposenrrd5_.get();
    int transposenrrd6 = guitransposenrrd6_.get();
    std::string matrixlist;
    
    int nummatrixs = handle->numMatrices();
    for (int p = 0; p < nummatrixs; p++)
    {
      matrixlist += "{" + handle->getMatrixName(p) + "} ";
    }

    guimatrices_.set(matrixlist);
    get_ctx()->reset();
  
    // We need to set bundle properties hence we need to detach
    handle.detach();
    
    // Send matrix1 if we found one that matches the name:
    if (handle->isMatrix(matrix1name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd1) handle->transposeNrrd(true);    
      fhandle = handle->getMatrix(matrix1name);
      send_output_handle("matrix1",fhandle);
    } 

    // Send matrix2 if we found one that matches the name:
    if (handle->isMatrix(matrix2name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd2) handle->transposeNrrd(true);    
      fhandle = handle->getMatrix(matrix2name);
      send_output_handle("matrix2",fhandle);
    } 

    // Send matrix3 if we found one that matches the name:
    if (handle->isMatrix(matrix3name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd3) handle->transposeNrrd(true);    
      fhandle = handle->getMatrix(matrix3name);
      send_output_handle("matrix3",fhandle);
    } 

    // Send matrix4 if we found one that matches the name:
    if (handle->isMatrix(matrix4name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd4) handle->transposeNrrd(true);    
      fhandle = handle->getMatrix(matrix4name);
      send_output_handle("matrix4",fhandle);
    } 

    // Send matrix5 if we found one that matches the name:
    if (handle->isMatrix(matrix5name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd5) handle->transposeNrrd(true);    
      fhandle = handle->getMatrix(matrix5name);
      send_output_handle("matrix5",fhandle);
    } 

    // Send matrix6 if we found one that matches the name:
    if (handle->isMatrix(matrix6name))
    {
      handle->transposeNrrd(false);
      if (transposenrrd6) handle->transposeNrrd(true);    
      fhandle = handle->getMatrix(matrix6name);
      send_output_handle("matrix6",fhandle);
    }     
    send_output_handle("bundle",handle);
  }
}

#endif