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


#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/Util/StringUtil.h>

#include <sci_debug.h>

#include <iostream>
#include <fstream>
#include <sstream>

namespace SCIRun {

FieldHandle StructHexVolField_reader(ProgressReporter *pr, const char *filename);
bool StructHexVolField_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

FieldHandle StructHexVolField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle result = 0;
  std::string pts_fn(filename);

  std::string::size_type pos = pts_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    pts_fn = pts_fn + ".pts";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(pts_fn.c_str());
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + pts_fn);
      return (result);
    }
  }
  else
  {
    std::string base = pts_fn.substr(0,pos);
    std::string ext  = pts_fn.substr(pos);
    if ((ext != ".pts" )||(ext != ".pos"))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        pts_fn = base + ".pts";
        inputfile.open(pts_fn.c_str());
      }
      catch (...)
      {
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
          pts_fn = base + ".pos";
          inputfile.open(pts_fn.c_str());
        }
        catch (...)
        {
          if (pr) pr->error("Could not open file: " + base + ".pts");
          return (result);
        }
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(pts_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + pts_fn);
        return (result);
      }
    }
  }

  int ncols = 3;
  int nrows = 0;
  int line_ncols = 0;

  std::string line;
  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool first_line_pts = true;

  std::vector<double> values;
  int ni = 0, nj = 0, nk = 0;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(pts_fn.c_str());

      while( getline(inputfile,line,'\n'))
      {
        if (line.size() > 0)
        {
          // block out comments
          if ((line[0] == '#')||(line[0] == '%')) continue;
        }

        // replace comma's and tabs with white spaces
        for (size_t p = 0; p < line.size(); p++)
        {
          if ((line[p] == '\t')||(line[p] == ',')||(line[p]=='"')) line[p] = ' ';
        }

        if (first_line_pts)
        {
          // For the StructHexVolMesh converter, we need to assume that the first line
          // is a header.
          if (line.find(".") != std::string::npos) {
            if (pr) pr->warning("Improper format of text file - make sure this first line is a header specifying ni nj nk (white-space separated");
          }

          std::vector<int> headerValues;
          multiple_from_string(line, headerValues);
          line_ncols = headerValues.size();
          if (headerValues.size() == 3) {
            ni = headerValues[0];
            nj = headerValues[1];
            nk = headerValues[2];
          }
          else {
            if (pr)  pr->error("Improper format of text file header");
            return (result);
          }
          first_line_pts = false;
        }
        else
        {
          multiple_from_string(line, values);
          line_ncols = values.size();
          if (line_ncols > 0)
          {
            nrows++;
            if (ncols > 0)
            {
              if (ncols != line_ncols)
              {
                if (pr)  pr->error("Improper format of text file, not every line contains the same amount of coordinates");
                return (result);
              }
            }
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  FieldInformation fi("StructHexVolMesh", "HexTrilinearLgn", "double");
  MeshHandle mh = CreateMesh(fi, ni, nj, nk);
  result = CreateField(fi, mh);
  VMesh* mesh = mh->vmesh();
  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(pts_fn.c_str());

      // Points file: x/y/z coords of each point, one per line, entries
      // separated by white space.
      for (int i = 0; i < ni; ++i) {
        for (int j = 0; j < nj; ++j) {
          for (int k = 0; k < nk; ++k) {
            if (getline(inputfile, line, '\n')) {
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
            }

            multiple_from_string(line, values);
            VMesh::Node::index_type idx;
            mesh->to_index(idx, i, j, k);
            mesh->set_point(Point(values[0],values[1],values[2]), idx);
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }
  return (result);
}

bool StructHexVolField_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  // Points file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string pts_fn(filename);
    std::string::size_type pos = pts_fn.find_last_of(".");
    std::string base = pts_fn.substr(0, pos);
    std::string ext  = pts_fn.substr(pos);
    const char* fileExt = ".pts";

    if (pos == std::string::npos)
    {
      pts_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      pts_fn = base + fileExt;
    }

    try
    {
      outputfile.open(pts_fn.c_str());

      // these appear to be reasonable formatting flags for output
      std::ios_base::fmtflags ff;
      ff = outputfile.flags();
      ff |= outputfile.showpoint; // write floating-point values including always the decimal point
      ff |= outputfile.fixed; // write floating point values in fixed-point notation
      outputfile.flags(ff);

      std::vector<Mesh::index_type> dims;
      mesh->get_dimensions(dims);

      VMesh::Node::iterator nodeIter;
      VMesh::Node::iterator nodeIterEnd;
      VMesh::Node::size_type nodeSize;

      mesh->begin(nodeIter);
      mesh->end(nodeIterEnd);
      mesh->size(nodeSize);

      // require header
      outputfile << dims[0] << " " << dims[1] << " " << dims[2] << "\n";

      while (nodeIter != nodeIterEnd)
      {
        Point p;
        mesh->get_center(p, *nodeIter);
        outputfile << p.x() << " " << p.y() << " " << p.z() << "\n";
        ++nodeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write file: "+ pts_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}

static FieldIEPlugin StructHexVolField_plugin("StructHexVolField", "{.pts} {.pos}", "", StructHexVolField_reader, StructHexVolField_writer);

} // end namespace
