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

/*
 * This module reads a matlab file and converts it to a SCIRun matrix
 *
 */

#include <Modules/Legacy/Matlab/DataIO/ImportFieldsFromMatlab.h>
#include <sstream>
#include <string>
#include <vector>

#include <Core/Util/FullFileName.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/String.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>

#include <Dataflow/GuiInterface/GuiVar.h>

namespace MatlabIO {

using namespace SCIRun;

class ImportFieldsFromMatlab : public Module
{

  public:

    // Constructor
    ImportFieldsFromMatlab(GuiContext*);

    // Destructor
    virtual ~ImportFieldsFromMatlab();

    // Std functions for each module
    // execute():
    //   Execute the module and put data on the output port
    //
    // tcl_command():
    //   Handles call-backs from the TCL code

    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);

    virtual void post_read();

  private:

    // indexmatlabfile():
    //   This functions is used in the GUI interface, it loads the
    //   currently selected .mat file and returns an information
    //   string to TCL with all the names and formats of the matrices.
    //   NOTE: This Function explicitly depends on the TCL code.

    void		indexmatlabfile(bool postmsg);

    // readmatlabarray():
    //   This function reads the in the gui selected matlab array. It
    //   retrieves the current filename and the matrix selected in this
    //   file and returns an object containing the full matrix

    matlabarray readmatlabarray(int p);

    // displayerror()
    //   Relay an error during the matrixselection process directly to
    //   the user

    void displayerror(std::string str);

  private:

    enum { NUMPORTS = 6};

    // GUI variables
    GuiFilename			guifilename_;		// .mat filename (import from GUI)
    GuiString       guifilenameset_;
    GuiString				guimatrixinfotexts_;   	// A list of matrix-information strings of the contents of a .mat-file
    GuiString				guimatrixnames_;	// A list of matrix-names of the contents of a .mat-file
    GuiString				guimatrixname_;		// the name of the matrix that has been selected

};

DECLARE_MAKER(ImportFieldsFromMatlab)

// Constructor:
// Initialise all the variables shared between TCL and SCIRun
// Only filename and matrixname are used to reconstruct the
// settings of a previously created module
// matrixinfotexts and matrixnames serve as outputs to TCL.

ImportFieldsFromMatlab::ImportFieldsFromMatlab(GuiContext* ctx)
  : Module("ImportFieldsFromMatlab", ctx, Source, "DataIO", "MatlabInterface"),
    guifilename_(get_ctx()->subVar("filename")),
    guifilenameset_(get_ctx()->subVar("filename-set",false)),
    guimatrixinfotexts_(get_ctx()->subVar("matrixinfotexts",false)),
    guimatrixnames_(get_ctx()->subVar("matrixnames",false)),
    guimatrixname_(get_ctx()->subVar("matrixname"))
{
  indexmatlabfile(false);
}

// Destructor:
// All my objects have descrutors and hence nothing needs
// explicit descruction
ImportFieldsFromMatlab::~ImportFieldsFromMatlab()
{
}


void
ImportFieldsFromMatlab::post_read()
{
  FullFileName ffn(guifilename_.get());
  guifilename_.set(ffn.get_abs_filename());
}

// Execute:
// Inner workings of this module
void ImportFieldsFromMatlab::execute()
{
	StringHandle stringH;
	get_input_handle("Filename",stringH,false);
	if (stringH.get_rep())
	{
		std::string filename = stringH->get();
		guifilename_.set(filename);
		get_ctx()->reset();
    indexmatlabfile(true);
		get_ctx()->reset();
	}

  // Get the filename from TCL.
  std::string filename = guifilename_.get();

  // If the filename is empty, launch an error
  if (filename == "")
  {
    error("ImportFieldsFromMatlab: No file name was specified");
    return;
  }

  update_state(Executing);

  try
  {
    for (int p=0;p<NUMPORTS;p++)
    {
      // Now read the matrix from file
      // The next function will open, read, and close the file
      // Any error will be exported as an exception.
      // The matlab classes are all based in the matfilebase class
      // which carries the definitions of the exceptions. These
      // definitions are inherited by all other "matlab classes"

      matlabarray ma = readmatlabarray(p);

      // An empty array means something must have gone wrong
      // Or there is no data to put on this port.
      // Do not translate empty arrays, but continue to the
      // next output port.

      if (ma.isempty())
      {
        continue;
      }

      // The data is still in matlab format and the next function
      // creates a SCIRun matrix object

      SCIRun::FieldHandle mh;
      matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter *>(this));
      translate.mlArrayTOsciField(ma,mh);

      // Put the SCIRun matrix in the hands of the scheduler
			send_output_handle(p,mh,true);
    }

    SCIRun::StringHandle filenameH = new String(filename);
    send_output_handle("Filename",filenameH,true);

  }

  // in case something went wrong

  catch (matlabfile::could_not_open_file)
  {
    error("ImportFieldsFromMatlab: Could not open file");
  }
  catch (matlabfile::invalid_file_format)
  {
    error("ImportFieldsFromMatlab: Invalid file format");
  }
  catch (matlabfile::io_error)
  {
    error("ImportFieldsFromMatlab: IO error");
  }
  catch (matlabfile::out_of_range)
  {
    error("ImportFieldsFromMatlab: Out of range");
  }
  catch (matlabfile::invalid_file_access)
  {
    error("ImportFieldsFromMatlab: Invalid file access");
  }
  catch (matlabfile::empty_matlabarray)
  {
    error("ImportFieldsFromMatlab: Empty matlab array");
  }
  catch (matlabfile::matfileerror)
  {
    error("ImportFieldsFromMatlab: Internal error in reader");
  }
}



void ImportFieldsFromMatlab::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ImportFieldsFromMatlab needs a minor command");
    return;
  }

  if( args[1] == "indexmatlabfile" )
  {

    // It turns out that in the current design, SCIRun reads variables once
    // and then assumes they do not change and hence caches the data
    // Why it is done so is unclear to me, but in order to have interactive
    // GUIs I need to reset the context. (this synchronises the data again)
    get_ctx()->reset();

    // Find out what the .mat file contains
    indexmatlabfile(true);
    return;
  }
  else
  {
    // Relay data to the Module class
    Module::tcl_command(args, userdata);
  }
}


matlabarray ImportFieldsFromMatlab::readmatlabarray(int p)
{
  matlabarray marray;
  std::string filename = guifilename_.get();
  std::string guimatrixname = guimatrixname_.get();
  std::string matrixname = "";

  // guimatrixname is a list with the name of the matrices per port
  // use the TCL command lindex to select the proper string from the list

  std::ostringstream oss;
  oss << "lindex {" << guimatrixname << "} " << p;

  TCLInterface::eval(oss.str(),matrixname);

  if (matrixname == "")
  {
    // return an empty array
    return(marray);
  }

  if (matrixname == "<none>")
  {
    // return an empty array
    return(marray);
  }

  // this block contains the file IO
  // The change of errors is reasonable
  // hence errors are generated as exceptions

  // having a local matfile object here ensures
  // the file will be closed (destructor of the object).

  matlabfile  mfile;
  mfile.open(filename,"r");
  marray = mfile.getmatlabarray(matrixname);
  mfile.close();

  return(marray);
}



void ImportFieldsFromMatlab::indexmatlabfile(bool postmsg)
{

  std::string filename = "";
  std::string matrixinfotexts = "";
  std::string matrixnames = "";
  std::string newmatrixname = "";
  std::string matrixname = "";

  guimatrixinfotexts_.set(matrixinfotexts);
  guimatrixnames_.set(matrixnames);

  SCIRun::ProgressReporter* pr = 0;
  if (postmsg) pr = dynamic_cast<SCIRun::ProgressReporter* >(this);
  matlabconverter translate(pr);

  filename = guifilename_.get();

  if (filename == "")
  {
    // No file has been loaded, so reset the
    // matrix name variable
    guimatrixname_.set(newmatrixname);
    return;
  }

  matrixname = guimatrixname_.get();

  std::vector<std::string> matrixnamelist(NUMPORTS);
  bool foundmatrixname[NUMPORTS];

  for (int p=0;p<NUMPORTS;p++)
  {
    // TCL Dependent code
    std::ostringstream oss;
    oss << "lindex { " << matrixname << " } " << p;
    TCLInterface::eval(oss.str(),matrixnamelist[p]);
    foundmatrixname[p] = false;
  }

  try
  {
    matlabfile mfile;
    // Open the .mat file
    // This function also scans through the file and makes
    // sure it is amat file and counts the number of arrays

    mfile.open(filename,"r");

    // all matlab data is stored in a matlabarray object
    matlabarray ma;
    int cindex = 0;		// compability index, which matlab array fits the SCIRun matrix best?
    int maxindex = 0;		// highest index found so far

    // Scan the file and see which matrices are compatible
    // Only those will be shown (you cannot select incompatible matrices).

    std::string infotext;

    for (int p=0;p<mfile.getnummatlabarrays();p++)
    {
      ma = mfile.getmatlabarrayinfo(p); // do not load all the data fields
      if ((cindex = translate.sciFieldCompatible(ma,infotext)))
      {
        // in case we need to propose a matrix to load, select
        // the one that is most compatible with the data
        if (cindex > maxindex) { maxindex = cindex; newmatrixname = ma.getname();}

        // create tcl style list to use in the array selection process

        matrixinfotexts += std::string("{" + infotext + "} ");
        matrixnames += std::string("{" + ma.getname() + "} ");
        for (int q=0;q<NUMPORTS;q++)
        {
          if (ma.getname() == matrixnamelist[q]) foundmatrixname[q] = true;
        }
      }
    }

    matrixinfotexts += "{none} ";
    matrixnames += "{<none>} ";

    mfile.close();

    // automatically select a matrix if the current matrix name
    // cannot be found or if no matrixname has been specified

    matrixname = "";
    for (int p=0;p<NUMPORTS;p++)
    {
      if (foundmatrixname[p] == false)
      {
        if (p==0)
        {
          matrixnamelist[p] = newmatrixname;
        }
        else
        {
          matrixnamelist[p] = "<none>";
        }
      }
      matrixname += "{" + matrixnamelist[p] + "} ";
    }

    // Update TCL on the contents of this matrix
    guimatrixname_.set(matrixname);
    guimatrixinfotexts_.set(matrixinfotexts);
    guimatrixnames_.set(matrixnames);
  }

  // in case something went wrong
  // close the file and then dermine the problem

  catch (matlabfile::could_not_open_file)
  {
    displayerror("ImportFieldsFromMatlab: Could not open file");
  }
  catch (matlabfile::invalid_file_format)
  {
    displayerror("ImportFieldsFromMatlab: Invalid file format");
  }
  catch (matlabfile::io_error)
  {
    displayerror("ImportFieldsFromMatlab: IO error");
  }
  catch (matlabfile::out_of_range)
  {
    displayerror("ImportFieldsFromMatlab: Out of range");
  }
  catch (matlabfile::invalid_file_access)
  {
    displayerror("ImportFieldsFromMatlab: Invalid file access");
  }
  catch (matlabfile::empty_matlabarray)
  {
    displayerror("ImportFieldsFromMatlab: Empty matlab array");
  }
  catch (matlabfile::matfileerror)
  {
    displayerror("ImportFieldsFromMatlab: Internal error in reader");
  }
  return;
}


void ImportFieldsFromMatlab::displayerror(std::string str)
{
  // Explicit call to TCL
  TCLInterface::execute("tk_messageBox -icon error -type ok -title {ERROR} -message {" + str + "}");
}


} // End namespace MatlabIO
