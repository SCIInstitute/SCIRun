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


/// @file   CreateParameterBundle.cc
/// @author Jeroen G Stinstra
/// @date   17 SEP 2005

#include <Core/Algorithms/Regression/RegressionAlgo.h>

#include <Core/Datatypes/Bundle.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Module.h>

#include <sstream>
#include <string>
#include <vector>

namespace SCIRun {

/// @class CreateParameterBundle
/// @brief Sets several parameters and collects them as a single bundle.

class CreateParameterBundle : public Module {
public:
  CreateParameterBundle(GuiContext*);

  virtual void execute();

private:

  // For extracting the parameters out of the the GUI
  std::vector<std::string> converttcllist(std::string str);

  // The data of all the parameters as a long TCL list
  GuiString data_;
  GuiInt    fieldnamecount_;
  GuiString update_all_data_;

  BundleHandle oldbundle_;
};


DECLARE_MAKER(CreateParameterBundle)

CreateParameterBundle::CreateParameterBundle(GuiContext* ctx)
  : Module("CreateParameterBundle", ctx, Source, "Bundle", "SCIRun"),
    data_(get_ctx()->subVar("data")),
    fieldnamecount_(get_ctx()->subVar("new_field_count")),
    update_all_data_(get_ctx()->subVar("update_all"))
{
}

void CreateParameterBundle::execute()
{
  // TK has the problem that it only updates it's data fields when you move the
  // mouse to another part of the GUI and activate some other element. If not
  // the data is not updated. Here we force TCL to update all the data.
  // It's ugly but it solves some usability problems

  update_state(Executing);

  TCLInterface::lock();
  TCLInterface::eval(update_all_data_.get());
  TCLInterface::unlock();
  get_ctx()->reset();

  std::string data = data_.get();

  // Split the data in pieces for each parameter
  std::vector<std::string> datalist = converttcllist(data);

  BundleHandle bundle = new Bundle();

  std::string parname, partype, pardata;

  SCIRunAlgo::RegressionAlgo ralgo(this);

  for (size_t p = 0; p < datalist.size(); p++)
  {
    // Cycle to through all parameters and add then to bundle.

    std::vector<std::string> parameter = converttcllist(datalist[p]);

    parname = parameter[1];
    partype = parameter[2];
    pardata = parameter[3];

    if (partype == "boolean")
    {
      MatrixHandle matrix = new DenseMatrix(1, 1);
      double *dataptr = matrix->get_data_pointer();
      dataptr[0] = 0.0;
      if (pardata == "true") dataptr[0] = 1.0;

      if (oldbundle_.get_rep())
      if (oldbundle_->isMatrix(parname))
      {
        MatrixHandle oldmatrix = oldbundle_->getMatrix(parname);
        if (ralgo.CompareMatrices(matrix,oldmatrix)) matrix = oldmatrix;
      }

      bundle->setMatrix(parname,matrix);
    }

    if (partype == "scalar")
    {
      MatrixHandle matrix = new DenseMatrix(1, 1);
      double *dataptr = matrix->get_data_pointer();
      std::istringstream iss(pardata);
      iss >> dataptr[0];

      if (oldbundle_.get_rep())
      if (oldbundle_->isMatrix(parname))
      {
        MatrixHandle oldmatrix = oldbundle_->getMatrix(parname);
        if (ralgo.CompareMatrices(matrix,oldmatrix)) matrix = oldmatrix;
      }

      bundle->setMatrix(parname,matrix);
    }

    if (partype == "vector")
    {
      MatrixHandle matrix = new DenseMatrix(1, 3);
      std::vector<std::string> subdata = converttcllist(pardata);
      double *dataptr = matrix->get_data_pointer();

      for (size_t r=0; r<3; r++ )
      {
        std::istringstream iss(subdata[r]);
        iss >> dataptr[r];
      }

      if (oldbundle_.get_rep())
      if (oldbundle_->isMatrix(parname))
      {
        MatrixHandle oldmatrix = oldbundle_->getMatrix(parname);
        if (ralgo.CompareMatrices(matrix,oldmatrix)) matrix = oldmatrix;
      }

      bundle->setMatrix(parname,matrix);
    }

    if (partype == "tensor")
    {
      MatrixHandle matrix = new DenseMatrix(1, 9);
      std::vector<std::string> subdata = converttcllist(pardata);
      double *dataptr = matrix->get_data_pointer();
      {
        std::istringstream iss1(subdata[0]);
        iss1 >> dataptr[0];
        std::istringstream iss2(subdata[1]);
        iss2 >> dataptr[1];
        iss2 >> dataptr[3];
        std::istringstream iss3(subdata[2]);
        iss3 >> dataptr[2];
        iss3 >> dataptr[6];
        std::istringstream iss4(subdata[3]);
        iss4 >> dataptr[4];
        std::istringstream iss5(subdata[4]);
        iss5 >> dataptr[5];
        iss5 >> dataptr[7];
        std::istringstream iss6(subdata[5]);
        iss6 >> dataptr[8];
      }

      if (oldbundle_.get_rep())
      if (oldbundle_->isMatrix(parname))
      {
        MatrixHandle oldmatrix = oldbundle_->getMatrix(parname);
        if (ralgo.CompareMatrices(matrix,oldmatrix)) matrix = oldmatrix;
      }


      bundle->setMatrix(parname,matrix);
    }

    if (partype == "array")
    {
      std::vector<std::string> subdata = converttcllist(pardata);
      MatrixHandle matrix = new DenseMatrix(1, subdata.size());
      double *dataptr = matrix->get_data_pointer();

      for (size_t r=0; r<subdata.size(); r++ )
      {
        std::istringstream iss(subdata[r]);
        iss >> dataptr[r];
      }

      if (oldbundle_.get_rep())
      if (oldbundle_->isMatrix(parname))
      {
        MatrixHandle oldmatrix = oldbundle_->getMatrix(parname);
        if (ralgo.CompareMatrices(matrix,oldmatrix)) matrix = oldmatrix;
      }

      bundle->setMatrix(parname,matrix);
    }

    if (partype == "string")
    {
      StringHandle str = new String(pardata);

      if (oldbundle_.get_rep())
      if (oldbundle_->isString(parname))
      {
        StringHandle oldstring = oldbundle_->getString(parname);
        if (ralgo.CompareStrings(str,oldstring)) str = oldstring;
      }

      bundle->setString(parname,str);
    }

    if (partype == "filename")
    {
      StringHandle str = new String(pardata);

      if (oldbundle_.get_rep())
      if (oldbundle_->isString(parname))
      {
        StringHandle oldstring = oldbundle_->getString(parname);
        if (ralgo.CompareStrings(str,oldstring)) str = oldstring;
      }

      bundle->setString(parname,str);
    }
  }

  oldbundle_ = bundle;
  send_output_handle("ParameterList", bundle);
}


std::vector<std::string>
CreateParameterBundle::converttcllist(std::string str)
{
  std::string result;
  std::vector<std::string> list(0);
  long lengthlist = 0;

  // Yeah, it is TCL dependent:
  // TCL::llength determines the length of the list
  TCLInterface::lock();
  TCLInterface::eval("llength { "+str + " }",result);
  std::istringstream iss(result);
  iss >> lengthlist;
  TCLInterface::unlock();
  if (lengthlist < 0) return(list);

  list.resize(lengthlist);
  TCLInterface::lock();
  for (long p = 0;p<lengthlist;p++)
  {
    std::ostringstream oss;
    // TCL dependency:
    // TCL::lindex retrieves the p th element from the list
    oss << "lindex { " << str <<  " } " << p;
    TCLInterface::eval(oss.str(),result);
    list[p] = result;
  }
  TCLInterface::unlock();
  return(list);
}

} // End namespace SCIRun
