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
 *  EcgsimToTriSurf_IEPlugin.cc
 *
 *  Written by:
 *   Jeroen Stinstra
 *   Department of Computer Science
 *   University of Utah
 *
 */

#include <Core/IEPlugin/EcgsimFileToTriSurf_Plugin.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>

#include <iostream>
#include <fstream>

using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;

FieldHandle SCIRun::EcgsimFileToTriSurf_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result;

  int ncols = 4;
  int nrows = 0;
  int line_ncols = 0;
  int num_pts = 0;
  int num_fac = 0;

  std::string line;
  double data;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool first_line = true;
  bool is_facs = false;

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

        if (first_line)
        {
          if (line_ncols == 1)
          {
            if(is_facs)
            {
              num_fac = int(data);
            }
            else
            {
              num_pts = int(data);
            }
            first_line = false;
          }
          else
          {
            if (pr)  pr->error("Improper format of text file, header missing.");
            return (result);
          }
        }
        else
        {
          if (line_ncols > 0)
          {
            nrows++;

            if (ncols != line_ncols)
            {
              if (pr)  pr->error("Improper format of text file, not every line contains the same amount of coordinates");
              return (result);
            }
            // If on last point, switch to facs
            else if(!is_facs && nrows == num_pts)
            {
              first_line = true;
              is_facs = true;
            }
            // If on last fac, ignore remaining data
            else if(is_facs && nrows == num_fac)
            {
              if (pr) pr->remark("Reached last element, ignoring rest of data in file");
              while( getline(inputfile,line,'\n')) {}
            }
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }


   // STAGE 2
   // Generate field from


  // Now create field
  FieldInformation fi("TriSurfMesh",-1,"double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();

  mesh->node_reserve(num_pts);
  mesh->elem_reserve(num_fac);

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(filename);

      std::vector<double> vdata(3);
      int k = 0;

      // remove header
      getline(inputfile,line,'\n');

      int line_count;
      for(line_count = 0; line_count < num_pts; line_count++ )
      {
        getline(inputfile,line,'\n');
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
          k = 0;
          while(k < 4)
          {
            iss >> data;
            if(k > 0)
            {
              vdata[k-1] = data;
            }
            k++;
          }
        }
        catch(...)
        {
        }

        if (k == 4) mesh->add_point(Point(vdata[0],vdata[1],vdata[2]));
      }


      unsigned int idata;
      VMesh::Node::array_type ndata;
      ndata.resize(3);

      k = 0;

      // remove header
      getline(inputfile,line,'\n');

      for(line_count = 0; line_count < num_fac; line_count++ )
      {
        getline(inputfile,line,'\n');
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
          k = 0;
          while(k < 4)
          {
            iss >> idata;
            if(k>0)
            {
              ndata[k-1] = VMesh::Node::index_type(idata-1);
            }

            k++;
          }
        }
        catch(...)
        {
        }

        if (k == 4)
        {
          mesh->add_elem(ndata);
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read data from file: " + std::string(filename));
      return (result);
    }
    inputfile.close();
  }

  return (result);
}
