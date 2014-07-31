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

#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/ImportExport/Nrrd/NrrdIEPlugin.h>
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>


// This file contains plugins to read and write files in MathWork's Matlab
// file format.


namespace SCIRun {

using namespace MatlabIO;

// Write a matrix as matlab object
MatrixHandle
MatlabMatrix_reader(ProgressReporter *pr, const char *filename)
{
  // Create a matlab file object, which will be linked to input file
  matlabfile mf;
  // Instantiate the matlab converter between SCIRun objects and Matlab objects
  matlabconverter mc(pr);
  // Define a matlab array for in the matlab file
  matlabarray ma;
  
  // The engine will generate an information string which we will ignore
  std::string dummytext;
  
  // Define the output handle
  MatrixHandle mh = 0;
  
  try
  {
    // Open the file
    mf.open(std::string(filename),"r");
    // Get the number of matlab objects in the file
    int numarrays = mf.getnummatlabarrays();
    // Find the first one that is suitable
    for (int p=0;p<numarrays;p++)
    {
      // Extract only the object's header
      ma = mf.getmatlabarrayinfo(p);
      // Check whether we could convert it into a matrix object
      if (mc.sciMatrixCompatible(ma,dummytext)) 
      { 
        // Read the full object
        ma = mf.getmatlabarray(p);
        // Convert the object into a SCIRun object
        mc.mlArrayTOsciMatrix(ma,mh); break; 
      }
    }
    mf.close();
  }
  catch (...)
  {
    mh = 0;
  }

  return(mh);
}

bool
MatlabMatrix_writer(ProgressReporter *pr,
			   MatrixHandle mh, const char *filename)
{
  matlabfile mf;
  matlabconverter mc(pr);
  matlabarray ma;
  std::string name;
 
  try
  {
    // Make sure we convert it to a numeric matrix, remove all annotation
    // This is the most common case
    mc.converttonumericmatrix();
    // Convert the matrix into a matlab object
    mc.sciMatrixTOmlArray(mh,ma);
    // If the SCIRun object has a name, use it
    mh->get_property("name",name);
    // Set a default name if none was supplied
    if ((name=="")||(!mc.isvalidmatrixname(name))) name = "scirunmatrix";
    // Open the file
    mf.open(std::string(filename),"w");
    // Write the file
    mf.putmatlabarray(ma,name);
    // Close the file
    mf.close();
  }
  catch (...)
  {
    return(false);
  }
  return(true);
}
  


NrrdDataHandle
MatlabNrrd_reader(ProgressReporter *pr, const char *filename)
{
  matlabfile mf;
  matlabconverter mc(pr);
  matlabarray ma;
  std::string dummytext;
  NrrdDataHandle mh;
  
  mh = 0;
  
  try
  {
    // Open the matlab file
    mf.open(std::string(filename),"r");
    // Check how many matlab objects are in the file
    int numarrays = mf.getnummatlabarrays();
    // Find the first one we can convert
    for (int p=0;p<numarrays;p++)
    {
      // Get the header of the object only
      ma = mf.getmatlabarrayinfo(p);
      // If it is compatible with a SCIRun nrrd, get it
      if (mc.sciNrrdDataCompatible(ma,dummytext)) 
      { 
        // Read the full object from disk
        ma = mf.getmatlabarray(p);
        // Convert the full object
        mc.mlArrayTOsciNrrdData(ma,mh); break; 
      }
    }
    mf.close();
  }
  catch (...)
  {
    // If something went wrong
    mh = 0;
  }

  return(mh);
}

bool
MatlabNrrd_writer(ProgressReporter *pr,
			   NrrdDataHandle mh, const char *filename)
{
  matlabfile mf;
  matlabconverter mc(pr);
  matlabarray ma;
  std::string name;
 
  try
  {
    // Make sure we make it into a numeric array, no annotion and headers
    mc.converttonumericmatrix();
    // Convert the object
    mc.sciNrrdDataTOmlArray(mh,ma);
    // Check if the object has a name
    mh->get_property("name",name);
    // if no name available give it one
    if ((name=="")||(!mc.isvalidmatrixname(name))) name = "scirunnrrd";
    // Open the file
    mf.open(std::string(filename),"w");
    // Write the object to file
    mf.putmatlabarray(ma,name);
    // Close the file
    mf.close();
  }
  catch (...)
  {
    // in case we crashed somehow
    return(false);
  }
  return(true);
}

         
FieldHandle
MatlabField_reader(ProgressReporter *pr, const char *filename)
{
  matlabfile mf;
  // Make sure that errors are forwarded in the conversion process
  matlabconverter mc(pr);
  matlabarray ma;
  std::string dummytext;
  FieldHandle mh;
  
  mh = 0;
  
  try
  {
    // Open the file
    mf.open(std::string(filename),"r");
    // How many object do we have in the file
    int numarrays = mf.getnummatlabarrays();
    // Check the objects and find the first one that is compatible with the
    // Field object
    for (int p=0;p<numarrays;p++)
    {
      // Get the header of the object
      ma = mf.getmatlabarrayinfo(p);
      // Is this object compatible?
      if (mc.sciFieldCompatible(ma,dummytext)) 
      {
        // If so get the full object
        ma = mf.getmatlabarray(p);
        // Convert the object to SCIRun
        mc.mlArrayTOsciField(ma,mh); break; 
      }
    }
    // Close file
    mf.close();
  }
  catch (...)
  {
    // Something went wrong...
    mh = 0;
  }

  return(mh);
}

bool
MatlabField_writer(ProgressReporter *pr,
			   FieldHandle mh, const char *filename)
{
  matlabfile mf;
  // Make sure that errors are forwarded in the conversion process
  matlabconverter mc(pr);
  matlabarray ma;
  std::string name;
 
  try
  {
    // We want all the annotation. A field without annotation is hard to use
    mc.converttostructmatrix();
    // Convert the object
    mc.sciFieldTOmlArray(mh,ma);
    // Get the name
    mh->get_property("name",name);
    // If no name, set a default
    if ((name=="")||(!mc.isvalidmatrixname(name))) name = "scirunfield";
    // Write the object to the file
    mf.open(std::string(filename),"w");
    mf.putmatlabarray(ma,name);
    mf.close();
  }
  catch (...)
  {
    // there is no way to signal an error to a module upstream
    return(false);
  }
  return(true);
}

static MatrixIEPlugin MatlabMatrix_plugin("Matlab Matrix",".mat", "*.mat", MatlabMatrix_reader, MatlabMatrix_writer);
static FieldIEPlugin MatlabField_plugin("Matlab Field",".mat", "*.mat",MatlabField_reader,MatlabField_writer);   
static NrrdIEPlugin MatlabNrrd_plugin("Matlab Matrix",".mat", "*.mat",MatlabNrrd_reader,MatlabNrrd_writer);     
  
} // end namespace SCIRun

