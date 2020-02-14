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

#include <Core/Util/StringUtil.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Core/Math/MiscMath.h>

#include <iostream>
#include <fstream>
#include <sstream>


namespace SCIRun {

MatrixHandle CVTRIMappingFileMatrix_reader(ProgressReporter *pr, const char *filename);
MatrixHandle CVTRICal8FileMatrix_reader(ProgressReporter *pr, const char *filename);

MatrixHandle CVTRIMappingFileMatrix_reader(ProgressReporter *pr, const char *filename)
{
  MatrixHandle result;

  int nrows = 0;
  int line_ncols = 0;

  std::string line;

  bool header = true;
  // STAGE 1 - SCAN THE FILE TO DETERMINE THE DIMENSIONS OF THE MATRIX
  // AND CHECK THE FILE'S INTEGRITY.

  bool zero_based = false;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(filename);

      std::vector<double> values;

      while( getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        multiple_from_string(line,values);
        line_ncols = values.size();
        for (size_t j=0;j<values.size();j++) if (Round(values[j]) == 0.0) zero_based = true;

        if (line_ncols > 0)
        {
          if (header)
          {
            // Skip header file
            header = false;
            continue;
          }
          else
          {
            nrows += line_ncols;
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }

  // STAGE 2 - NOW ACTUALLY READ AND STORE THE MATRIX
  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    result = new ColumnMatrix(nrows);
    if (result.get_rep() == 0)
    {
      if (pr) pr->error("Could not allocate matrix");
      return(result);
    }

    double* dataptr = result->get_data_pointer();
    index_type k = 0;

    try
    {
      inputfile.open(filename);

      bool header = true;
      std::vector<double> values;

      while( getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        if (header)
        {
          multiple_from_string(line,values);
          if (values.size() > 0) header = false;
        }
        else
        {
          multiple_from_string(line,values);
          for (size_t j=0;j<values.size(); j++, k++)
          {
            if (zero_based) dataptr[k] = Round(values[j]);
            else dataptr[k] = Round(values[j]-1.0);
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }

  return(result);
}

MatrixHandle CVTRICal8FileMatrix_reader(ProgressReporter *pr, const char *filename)
{
  MatrixHandle result;

  int ncols = 0;
  int nrows = 0;
  int line_ncols = 0;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE DIMENSIONS OF THE MATRIX
  // AND CHECK THE FILE'S INTEGRITY.

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(filename);

      int header_cnt = 0;
      std::vector<double> values;

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
        line_ncols = 0;

        if (line_ncols > 0)
        {
          if (header_cnt < 3)
          {
            header_cnt++;
          }
          else
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
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + std::string(filename));
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

      int header_cnt = 0;
      std::vector<double> values;

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

        if (values.size() > 0)
        {
          if (header_cnt < 3)
          {
            header_cnt++;
          }
          else
          {
            for(size_t j=0; j<values.size(); j++)
            {
              dataptr[k++] = values[j];
            }
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }
  return(result);
}

static MatrixIEPlugin CVTRIMappingFileMatrix_plugin("CVRTI_MappingFileToMatrix","{.mapping}", "",CVTRIMappingFileMatrix_reader,0);
static MatrixIEPlugin CVTRICal8FileMatrix_plugin("CVRTI_CalFileToMatrix","{.cal8} {.cal}", "",CVTRICal8FileMatrix_reader,0);

} // end namespace
