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

#include <Modules/Legacy/Matlab/DataIO/ExportFieldsToMatlab.h>
#include <sstream>
#include <string>
#include <vector>

#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <boost/filesystem/operations.hpp>

#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>

//TODO: move to utility header, can be used elsewhere
template <typename... T>
auto zip(const T&... containers) -> boost::iterator_range<boost::zip_iterator<decltype(boost::make_tuple(std::begin(containers)...))>>
{
  auto zip_begin = boost::make_zip_iterator(boost::make_tuple(std::begin(containers)...));
  auto zip_end = boost::make_zip_iterator(boost::make_tuple(std::end(containers)...));
  return boost::make_iterator_range(zip_begin, zip_end);
}

using namespace SCIRun::Modules::Matlab;
using namespace SCIRun::MatlabIO;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Matlab;

ALGORITHM_PARAMETER_DEF(Matlab, FieldNames);
ALGORITHM_PARAMETER_DEF(Matlab, FieldFormats);

const ModuleLookupInfo ExportFieldsToMatlab::staticInfo_("ExportFieldsToMatlab", "Matlab", "SCIRun");

ExportFieldsToMatlab::ExportFieldsToMatlab() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(Filename);
}

void ExportFieldsToMatlab::setStateDefaults()
{
  //TODO
}

#if 0
namespace MatlabIO {

using namespace SCIRun;

class ExportFieldsToMatlab : public Module
{

  public:

    // Constructor
    ExportFieldsToMatlab(GuiContext*);

    // Destructor
    virtual ~ExportFieldsToMatlab();

    // Std functions for each module
    // execute():
    //   Execute the module and put data on the output port

    virtual void execute();

  private:

    // Support functions for converting between TCL and C++
    // converttcllist:
    // TCL lists are stings in which the elements are separated by {}.
    // In a C++ environment it is easier to have them as a STL vector
    // object. This function converts the TCL list into an STL object
    // convertdataformat:
    // In TCL the dataformat is a string this function is used to convert
    // it into its matlab enum counterpart.
    std::vector<std::string> converttcllist(std::string str);
    matlabarray::mitype convertdataformat(std::string dataformat);

    // Support functions for the GUI
    // displayerror:
    // Directly reporting an error to the user (not in the error log)
    // overwrite:
    // Ask for confirmation to overwrite the file if it already exists

    void displayerror(std::string str);
    bool overwrite();

  private:

    enum { NUMPORTS = 6};

    // GUI variables
    GuiFilename guifilename_;     // .mat filename (import from GUI)
    GuiString   guifilenameset_;
    GuiString   guimatrixname_;   // A list of the matrix names
    GuiString   guimatrixformat_; // A list of the matlabarray format (numeric array, structured array)
    GuiInt      guioverwrite_;    // Overwrite matlab file
};

// Constructor:
// Initialise all the variables shared between TCL and SCIRun
// matrixname contains a list of matrix names.
// dataformat contains a list of the format of each matrix (int32,single,double, etc...)
// matrixformat contains a list of the way the object is represented in matlab
// e.g. as a structured object or an object with the dataarray only

ExportFieldsToMatlab::ExportFieldsToMatlab(GuiContext* ctx)
  : Module("ExportFieldsToMatlab", ctx, Sink, "DataIO", "MatlabInterface"),
    guifilename_(get_ctx()->subVar("filename")),
    guifilenameset_(get_ctx()->subVar("filename-set", false)),
    guimatrixname_(get_ctx()->subVar("matrixname")),
    guimatrixformat_(get_ctx()->subVar("matrixformat")),
    guioverwrite_(get_ctx()->subVar("overwrite"), 1)
{
}
#endif

void ExportFieldsToMatlab::execute()
{
  auto filenameInputOption = getOptionalInput(Filename);
  auto fields = getRequiredDynamicInputs(InputField);

  if (needToExecute())
  {
    auto state = get_state();

    if (filenameInputOption && *filenameInputOption)
    {
      auto filename = (*filenameInputOption)->value();
      state->setValue(Variables::Filename, filename);
    }


    auto filename = state->getValue(Variables::Filename).toFilename();

    // If the filename is empty, launch an error
    if (filename.empty())
    {
      error("ExportDatatypesToMatlab: No file name was specified");
      return;
    }

    // Make sure we have a .mat extension
    if (filename.extension() != ".mat")
      filename += ".mat";

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // Make sure the path to the new file exists
    // If not make it and as well convert filename
    // to absolute path name
    if (!exists(filename))
    {
      error("Could not generate path to file");
      return;
    }
    //filename = filename.get_abs_filename();
    //guifilename_.set(filename);
    //get_ctx()->reset();


    if (!overwrite()) return;
#endif

    update_state(Executing);

    // get all the settings from the GUI

    auto fieldnames = toStringVector(state->getValue(Parameters::FieldNames).toVector());
    auto fieldformats = toStringVector(state->getValue(Parameters::FieldFormats).toVector());

    // Check the validity of the matrixnames
    matlabconverter translate(getLogger());

    for (size_t p = 0; p < fieldnames.size(); ++p)
    {
      if (!translate.isvalidmatrixname(fieldnames[p]))
      {
        error("ExportFieldsToMatlab: The matrix name specified is invalid");
        return;
      }
      for (auto q = 0; q < p; ++q)
      {
        if (fieldnames[q] == fieldnames[p])
        {
          error("ExportFieldsToMatlab: A matrix name is used twice");
          return;
        }
      }
    }

    try
    {
      matlabfile mfile;   // matlab file object contains all function for reading and writing matlab arrayd
      matlabarray ma;		// matlab style formatted array (can be stored in a matlabfile object)
      mfile.open(filename.string(),"w");   // open file for writing

      // Add an information tag to the data, so the origin of the file is known
      // There are 116 bytes of free data storage at the header of the file.
      // Do not start the file with 'SCI ', otherwise the file looks like a
      // native SCIRun file which uses the same extension.
      
      mfile.setheadertext("Matlab V5 compatible file generated by SCIRun [module ExportFieldsToMatlab version 1.1]");

      for (auto&& tup: zip(fields, fieldnames, fieldformats))
      {
        FieldHandle field;
        std::string name, format;
        boost::tie(field, name, format) = tup;
        // Convert the SCIRun matrixobject to a matlab object
        if (format == "struct array")
        {
          // translate the matrix into a matlab structured array, which
          // can also store some data from the property manager
          translate.converttostructmatrix();
        }

        if (format == "numeric array")
        {
          // only store the numeric parts of the data
          translate.converttonumericmatrix();
        }

        translate.sciFieldTOmlArray(field, ma);

        if (ma.isempty())
        {
          warning("One of the matrices is empty");
          continue; // Do not write empty matrices
        }
        // Every thing seems OK, so proceed and store the matrix in the file
        mfile.putmatlabarray(ma, name);
      }

      mfile.close();
    }

    // in case something went wrong

    catch (matlabconverter::error_type&)
    {
      error("ExportFieldsToMatlab: Error in the SCIRun to Matlab converter");
    }
    catch (matlabfile::could_not_open_file&)
    {
      error("ExportFieldsToMatlab: Could not open file");
    }
    catch (matlabfile::invalid_file_format&)
    {
      error("ExportFieldsToMatlab: Invalid file format");
    }
    catch (matlabfile::io_error&)
    {   // IO error from ferror
      error("ExportFieldsToMatlab: IO error");
    }
    catch (matlabfile::unknown_type&)
    {
      error("ExportFieldsToMatlab: Unknow type encountered");
    }
    catch (matlabfile::empty_matlabarray&)
    {
      error("ExportFieldsToMatlab: Empty Matlab array encountered");
    }
    catch (matlabfile::out_of_range&)
    {
      error("ExportFieldsToMatlab: Out of Range error");
    }
    catch (matlabfile::invalid_file_access&)
    {
      error("ExportFieldsToMatlab: Invalid file access");
    }
    catch (matlabfile::compression_error&)
    {
      error("ExportFieldsToMatlab: Compression error");
    }
    catch (matlabfile::internal_error&)
    {
      error("ExportFieldsToMatlab: Internal error");
    }
    catch (matlabfile::matfileerror&)
    {   // All other errors are classified as internal
      // matfileerrror is the base class on which all
      // other exceptions are based.
      error("ExportFieldsToMatlab: Matlab file writer error");
  }
}
}

// Additional support functions :
// To help coordinate between the GUI in TCL and
// the functions in this module on the C++ site.
// Some of the following functions are TCL specific!

// convertdataformat
// Convert the string TCL returns into a matlabarray::mitype
#if 0
matlabarray::mitype ExportFieldsToMatlab::convertdataformat(std::string dataformat)
{
  matlabarray::mitype type = matlabarray::miUNKNOWN;
  if (dataformat == "same as data")  { type = matlabarray::miSAMEASDATA; }
  else if (dataformat == "double")   { type = matlabarray::miDOUBLE; }
  else if (dataformat == "single")   { type = matlabarray::miSINGLE; }
  else if (dataformat == "uint64")   { type = matlabarray::miUINT64; }
  else if (dataformat == "int64")    { type = matlabarray::miINT64; }
  else if (dataformat == "uint32")   { type = matlabarray::miUINT32; }
  else if (dataformat == "int32")    { type = matlabarray::miINT32; }
  else if (dataformat == "uint16")   { type = matlabarray::miUINT16; }
  else if (dataformat == "int16")    { type = matlabarray::miINT16; }
  else if (dataformat == "uint8")    { type = matlabarray::miUINT8; }
  else if (dataformat == "int8")     { type = matlabarray::miINT8; }
  return (type);
}

// converttcllist:
// converts a TCL formatted list into a STL array
// of strings

std::vector<std::string> ExportFieldsToMatlab::converttcllist(std::string str)
{
  std::string result;
  std::vector<std::string> list(0);
  int lengthlist = 0;

  // Yeah, it is TCL dependent:
  // TCL::llength determines the length of the list
  TCLInterface::eval("llength { "+str + " }",result);
  std::istringstream iss(result);
  iss >> lengthlist;
  if (lengthlist < 0) return(list);

  list.resize(lengthlist);
  for (int p = 0;p<lengthlist;p++)
  {
    std::ostringstream oss;
    // TCL dependency:
    // TCL::lindex retrieves the p th element from the list
    oss << "lindex { " << str <<  " } " << p;
    TCLInterface::eval(oss.str(),result);
    list[p] = result;
  }
  return(list);
}

// overwrite:
// Ask the user whether the file should be overwritten

bool ExportFieldsToMatlab::overwrite()
{
  std::string result;
  // call overwrite Tcl function
  TCLInterface::eval(get_id()+" overwrite",result);
  if (result == std::string("0"))
  {
    warning("User chose to not save.");
    return false;
  }
  return true;
}

// displayerror:
// This function should be replaced with a more
// general function in SCIRun for displaying errors

void ExportFieldsToMatlab::displayerror(std::string str)
{
  // Explicit call to TCL
  TCLInterface::execute("tk_messageBox -icon error -type ok -title {ERROR} -message {" + str + "}");
}


} // End namespace MatlabInterface

#endif
