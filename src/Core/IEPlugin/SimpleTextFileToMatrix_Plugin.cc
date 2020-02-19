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


/*
 *  SimpleTextFileToMatrix_Plugin.cc
 *
 *  Written by:
 *   Jeroen Stinstra
 *   Department of Computer Science
 *   University of Utah
 *
 */

// This is a plugin is meant to read simple text files with pure data
// in ascii format into a SCIRun matrix.

#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/IEPlugin/SimpleTextFileToMatrix_Plugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Logging/LoggerInterface.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;

MatrixHandle SCIRun::SimpleTextFileMatrix_reader(LoggerHandle pr, const char *filename)
{
  DenseMatrixHandle result;

  SCIRun::size_type ncols = 0;
  SCIRun::size_type nrows = 0;
  SCIRun::size_type line_ncols = 0;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE DIMENSIONS OF THE MATRIX
  // AND CHECK THE FILE'S INTEGRITY.

  std::vector<double> values;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(filename);

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

        multiple_from_string(line,values);
        line_ncols = values.size();

        if (line_ncols > 0)
        {
          nrows++;
          if (ncols > 0)
          {
            if (ncols != line_ncols)
            {
              if (pr)  pr->error("Improper format of text file, not every line contains the same amount of numbers");
              return (result);
            }
          }
          else
          {
            ncols = line_ncols;
          }
        }
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

    result.reset(new DenseMatrix(nrows,ncols));
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

        multiple_from_string(line,values);
        for (size_t j=0; j<values.size();j++) dataptr[k++] = values[j];
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: "+std::string(filename));
      return (result);
    }
    inputfile.close();
  }
  return(result);
}

bool SCIRun::SimpleTextFileMatrix_writer(LoggerHandle pr, MatrixHandle matrix, const char *filename)
{
  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );

  DenseMatrixHandle dense = convertMatrix::toDense(matrix);

  if (!dense)
  {
    if (pr) pr->error("Empty matrix detected");
    return(false);
  }

  double* dataptr = dense->data();
  if (dataptr == 0)
  {
    if (pr) pr->error("Empty matrix detected");
    return(false);
  }

  try
  {
    outputfile.open(filename);

    size_t k = 0;
    for (int p=0; p<dense->nrows(); p++)
    {
      for (int q=0; q<dense->ncols(); q++)
      {
        double val = dataptr[k++];
        if (is_integral_value(val))
          outputfile << static_cast<size_type>(val);
        else
          outputfile << val;
        outputfile << " ";
      }
      outputfile << "\n";
    }
  }
  catch (...)
  {
    if (pr) pr->error("Could not open and write to file: "+std::string(filename));
    return (false);
  }
  return (true);
}
