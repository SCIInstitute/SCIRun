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

/*
 *  EcgsimFileToMatrix_Plugin.cc
 *
 *  Written by:
 *   Jeroen Stinstra
 *   Department of Computer Science
 *   University of Utah
 *
 */
 

#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/IEPlugin/EcgsimFileToMatrix_Plugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;

MatrixHandle SCIRun::EcgsimFileMatrix_reader(LoggerHandle pr, const char *filename)
{
  DenseMatrixHandle result;

  int ncols = 0;
  int nrows = 0;
  int line_ncols = 0;
  int header_rows = 0;
  int header_cols = 0;

  std::string line;
  double data;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE DIMENSIONS OF THE MATRIX
  // AND CHECK THE FILE'S INTEGRITY.

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(filename);

      // get header information
      getline(inputfile,line,'\n');
      for (size_t p = 0;p<line.size();p++)
      {
        if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
      }
      std::istringstream iss(line);
      iss >> header_rows;
      iss >> header_cols; 

      while( getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }
                  
        // replace comma's and tabs with white spaces
        for (size_t p = 0;p<line.size();p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }
        std::istringstream iss(line);
        iss.exceptions( std::ifstream::failbit | std::ifstream::badbit);

        try
        {
          line_ncols = 0;
          while(1)
          {
            iss >> data;
            line_ncols++;
          }
        }
        catch(...)
        {
        }

        if (line_ncols > 0)
        {
          nrows++;
          if (ncols > 0)
          {
            if (ncols != line_ncols)
            {
              if (pr) pr->error("Improper format of text file, not every line contains the same amount of numbers");
              return (result);
            }
          }
          else
          {
            ncols = line_ncols;
          }
        }
      }

      if(ncols*nrows != header_cols*header_rows)
      {
        if (pr) pr->error("Data does not match header information.");
        return(result);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: "+std::string(filename));
      return (result);
    }
    
    inputfile.close();
  }

  // STAGE 2 - NOW ACTUALLY READ AND STORE THE MATRIX

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    result.reset(new DenseMatrix(header_rows,header_cols));
    if (!result)
    {
      if (pr) pr->error("Could not allocate matrix");
      return(result);
    }
    
    double* dataptr = result->data();
    int k = 0;

    try
    {
      inputfile.open(filename);
    
      // get header information
      getline(inputfile,line,'\n');

      while( getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }
                                          
        // replace comma's and tabs with white spaces
        for (size_t p = 0;p<line.size();p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }
        std::istringstream iss(line);
        iss.exceptions( std::ifstream::failbit | std::ifstream::badbit);
        try
        {
          while(1)
          {
            iss >> data;
            dataptr[k++] = data;
          }
        }
        catch(...)
        {
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read data in file: " + std::string(filename));
      return (result);
    }
    
    inputfile.close();
  }
  return(result);
}


bool SCIRun::EcgsimFileMatrix_writer(LoggerHandle pr, MatrixHandle matrixInput, const char *filename)
{

  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );

  DenseMatrixHandle matrix = matrix_cast::as_dense(matrixInput);
  
  if (!matrix)
  {
    if (pr) pr->error("Empty matrix detected");
    return(false);
  }

  double* dataptr = matrix->data();
  if (!dataptr)
  {
    if (pr) pr->error("Empty matrix detected");
    return(false);
  }

  try
  {
    outputfile.open(filename);
  }
  catch (...)
  {
    if (pr) pr->error("Could not open file: "+std::string(filename));
    return (false);
  }
    
  // output header line
  //int rows = matrix->nrows();
  //int cols = matrix->ncols();
  outputfile << matrix->nrows() << " " << matrix->ncols() << std::endl;

  size_t k = 0;
  for (int p=0; p<matrix->nrows(); p++)  
  {
    for (int q=0; q<matrix->ncols(); q++)  
    {
      outputfile << dataptr[k++] << " ";
    }
    outputfile << "\n";
  }  
  
  return (true);
}



