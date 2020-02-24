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
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Util/StringUtil.h>

#include <iostream>
#include <fstream>
#include <sstream>

namespace SCIRun {

MatrixHandle SimpleTextFileHeaderMatrix_reader(ProgressReporter *pr, const char *filename);
bool SimpleTextFileHeaderMatrix_writer(ProgressReporter *pr, MatrixHandle matrix, const char *filename);

MatrixHandle SimpleTextFileHeaderMatrix_reader(ProgressReporter *pr, const char *filename)
{
  MatrixHandle result;

  SCIRun::size_type ncols = 0, ncols_from_file = 0;
  SCIRun::size_type nrows = 0, nrows_from_file = 0;
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

      bool processed_header = false;
      while( getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        // replace comma's and tabs with white spaces
        for (size_t p = 0; p < line.size(); ++p)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }

        // In this version of the simple text file to matrix plugin,
        // we are assuming that there is a header with two integer numbers
        // for the number of rows and the number of columns in the matrix
        multiple_from_string(line, values);
        line_ncols = values.size();

        if (line_ncols > 0)
        {
          if (! processed_header)
          {
            nrows_from_file = static_cast<SCIRun::size_type>(values[0]);
            ncols_from_file = static_cast<SCIRun::size_type>(values[1]);
            processed_header = true;
          }
          else {
            ++nrows;
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

      if ( (nrows_from_file > 0) && (nrows_from_file != nrows) )
      {
        nrows = nrows_from_file;
      }

      if ( (ncols_from_file > 0) && (ncols_from_file != ncols) )
      {
        ncols = ncols_from_file;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }

  // STAGE 2 - NOW ACTUALLY READ AND STORE THE MATRIX

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    result = new DenseMatrix(nrows,ncols);
    if (result.get_rep() == 0)
    {
      if (pr) pr->error("Could not allocate matrix");
      return(result);
    }

    double* dataptr = result->get_data_pointer();
    int k = 0;

    try
    {
      inputfile.open(filename);

      // read header and continue
      if (! getline(inputfile,line,'\n'))
      {
        if (pr) pr->error("Could not read first line of "+std::string(filename));
        return (result);
      }

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
      if (pr) pr->error("Could not open file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }
  return(result);
}


bool SimpleTextFileHeaderMatrix_writer(ProgressReporter *pr, MatrixHandle matrix, const char *filename)
{

  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );

  MatrixHandle temp = matrix->dense();
  matrix = temp;

  if (matrix.get_rep() == 0)
  {
    if (pr) pr->error("Empty matrix detected");
    return(false);
  }

  double* dataptr = matrix->get_data_pointer();
  if (dataptr == 0)
  {
    if (pr) pr->error("Empty matrix detected");
    return(false);
  }

  try
  {
    outputfile.open(filename);

    size_t k = 0;
    for (int p=0; p<matrix->nrows(); p++)
    {
      for (int q=0; q<matrix->ncols(); q++)
      {
        outputfile << dataptr[k++] << " ";
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

static MatrixIEPlugin SimpleTextFileHeaderMatrix_plugin("SimpleTextFileWithHeader","", "",SimpleTextFileHeaderMatrix_reader,SimpleTextFileHeaderMatrix_writer);

} // end namespace
