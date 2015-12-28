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

#include <Modules/Legacy/Bundle/InsertMatricesIntoBundle.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Bundles;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

const ModuleLookupInfo InsertMatricesIntoBundle::staticInfo_("InsertMatricesIntoBundle", "Bundle", "SCIRun");

InsertMatricesIntoBundle::InsertMatricesIntoBundle()
  : Module(staticInfo_)
{
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Bundle.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

using namespace SCIRun;

class InsertMatricesIntoBundle : public Module {
public:
  InsertMatricesIntoBundle(GuiContext*);
  virtual void execute();

private:
  GuiString     guimatrix1name_;
  GuiString     guimatrix2name_;
  GuiString     guimatrix3name_;
  GuiString     guimatrix4name_;
  GuiString     guimatrix5name_;
  GuiString     guimatrix6name_;
  
  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;
  
  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertMatricesIntoBundle)

InsertMatricesIntoBundle::InsertMatricesIntoBundle(GuiContext* ctx)
  : Module("InsertMatricesIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
    guimatrix1name_(get_ctx()->subVar("matrix1-name"), "matrix1"),
    guimatrix2name_(get_ctx()->subVar("matrix2-name"), "matrix2"),
    guimatrix3name_(get_ctx()->subVar("matrix3-name"), "matrix3"),
    guimatrix4name_(get_ctx()->subVar("matrix4-name"), "matrix4"),
    guimatrix5name_(get_ctx()->subVar("matrix5-name"), "matrix5"),
    guimatrix6name_(get_ctx()->subVar("matrix6-name"), "matrix6"),
    guireplace1_(get_ctx()->subVar("replace1"),1),
    guireplace2_(get_ctx()->subVar("replace2"),1),
    guireplace3_(get_ctx()->subVar("replace3"),1),
    guireplace4_(get_ctx()->subVar("replace4"),1),
    guireplace5_(get_ctx()->subVar("replace5"),1),
    guireplace6_(get_ctx()->subVar("replace6"),1),
    guibundlename_(get_ctx()->subVar("bundlename",false), "")
{
}

void InsertMatricesIntoBundle::execute()
{
  BundleHandle  handle;
  MatrixHandle matrix1, matrix2, matrix3;
  MatrixHandle matrix4, matrix5, matrix6;

  get_input_handle("bundle",handle,false);
  get_input_handle("matrix1",matrix1,false);
  get_input_handle("matrix2",matrix2,false);
  get_input_handle("matrix3",matrix3,false);
  get_input_handle("matrix4",matrix4,false);
  get_input_handle("matrix5",matrix5,false);
  get_input_handle("matrix6",matrix6,false);
  
  if (inputs_changed_ || guimatrix1name_.changed() || 
      guimatrix2name_.changed() || guimatrix3name_.changed() || 
      guimatrix4name_.changed() || guimatrix5name_.changed() || 
      guimatrix6name_.changed() || 
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||
      guibundlename_.changed() || !oport_cached("bundle"))
  {
    update_state(Executing);
  
    std::string matrix1name = guimatrix1name_.get();
    std::string matrix2name = guimatrix2name_.get();
    std::string matrix3name = guimatrix3name_.get();
    std::string matrix4name = guimatrix4name_.get();
    std::string matrix5name = guimatrix5name_.get();
    std::string matrix6name = guimatrix6name_.get();
    std::string bundlename = guibundlename_.get();

    if (handle.get_rep())
    {
      handle.detach();
    }
    else
    {
      handle = new Bundle();
      if (handle.get_rep() == 0)
      {
        error("Could not allocate new bundle");
        return;
      }
    }

    if (matrix1.get_rep()
        &&(guireplace1_.get()||!(handle->isMatrix(matrix1name)))) 
      handle->setMatrix(matrix1name,matrix1);

    if (matrix2.get_rep()
        &&(guireplace2_.get()||!(handle->isMatrix(matrix2name)))) 
      handle->setMatrix(matrix2name,matrix2);

    if (matrix3.get_rep()
        &&(guireplace3_.get()||!(handle->isMatrix(matrix3name)))) 
      handle->setMatrix(matrix3name,matrix3);

    if (matrix4.get_rep()
        &&(guireplace4_.get()||!(handle->isMatrix(matrix4name)))) 
      handle->setMatrix(matrix4name,matrix4);

    if (matrix5.get_rep()
        &&(guireplace5_.get()||!(handle->isMatrix(matrix5name)))) 
      handle->setMatrix(matrix5name,matrix5);

    if (matrix6.get_rep()
        &&(guireplace6_.get()||!(handle->isMatrix(matrix6name)))) 
      handle->setMatrix(matrix6name,matrix6);
                      
    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle", handle);
  }

}

#endif