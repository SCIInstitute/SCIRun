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

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/IEPlugin/TriSurfField_Plugin.h>
#include <Core/Utils/Legacy/StringUtil.h>
#include <Core/Algorithms/Legacy/DataIO/VTKToTriSurfReader.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

FieldHandle SCIRun::VtkToTriSurfField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle outputField;
  VTKToTriSurfReader reader(pr);

  if (! reader.run(filename, outputField) )
  {
    if (pr)
      pr->error("Convert VTK file to SCIRun TriSurf field failed.");
    return nullptr;
  }
  return outputField;
}

FieldHandle SCIRun::TextToTriSurfField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result = 0;

  std::string fac_fn(filename);
  std::string pts_fn(filename);

  // Check whether the .fac or .tri file exists
  std::string::size_type pos = fac_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    fac_fn = fac_fn + ".fac";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(fac_fn.c_str());
    }

    catch (...)
    {
      if (pr) pr->error("Could not open file: "+fac_fn);
      return (result);
    }
  }
  else
  {
    std::string base = fac_fn.substr(0,pos);
    std::string ext  = fac_fn.substr(pos);
    if ((ext != ".fac" )||(ext != ".tri"))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        fac_fn = base + ".fac";
        inputfile.open(fac_fn.c_str());
      }
      catch (...)
      {
        try
        {
          std::ifstream inputfile;
          inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
          fac_fn = base + ".tri";
          inputfile.open(fac_fn.c_str());
        }

        catch (...)
        {
          if (pr) pr->error("Could not open file: "+base + ".fac");
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
        inputfile.open(fac_fn.c_str());
      }

      catch (...)
      {
        if (pr) pr->error("Could not open file: "+fac_fn);
        return (result);
      }
    }
  }


  pos = pts_fn.find_last_of(".");
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
      if (pr) pr->error("Could not open file: "+pts_fn);
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
          if (pr) pr->error("Could not open file: "+base + ".pts");
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
        if (pr) pr->error("Could not open file: "+pts_fn);
        return (result);
      }
    }
  }


  int ncols = 0;
  int nrows = 0;
  int line_ncols = 0;
  int num_elems = 0, num_nodes = 0;

  std::string line;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool has_header_pts = false;
  bool first_line_pts = true;

  bool has_header = false;
  bool first_line = true;

  std::vector<double> values;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(pts_fn.c_str());

      while ( getline(inputfile,line,'\n') )
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

        if (first_line_pts)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header_pts = true;
              num_nodes =  static_cast<int>(values[0]) + 1;
            }
            else if ((line_ncols == 3)||(line_ncols == 2))
            {
              has_header_pts = false;
              first_line_pts = false;
              nrows++;
              ncols = line_ncols;
            }
            else
            {
              if (pr)  pr->error("Improper format of text file, some lines contain more than 3 entries");
              return (result);
            }
          }
        }
        else
        {
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
            else
            {
              ncols = line_ncols;
            }
          }
        }
      }
    }
    catch (...) {
      if (pr) pr->error("Could not open file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_nodes) {
    num_nodes = nrows;
  }

  nrows = 0;
  ncols = 0;
  line_ncols = 0;

  bool zero_based = false;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(fac_fn.c_str());

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

        for (size_t j=0; j<values.size(); j++) if (values[j] == 0.0) zero_based = true;

        if (first_line)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
              num_elems =  static_cast<int>(values[0]) + 1;
            }
            else if (line_ncols == 3)
            {
              has_header = false;
              first_line = false;
              nrows++;
              ncols = line_ncols;
            }
            else
            {
              if (pr)  pr->error("Improper format of text file, some lines do not contain 3 entries");
              return (result);
            }
          }
        }
        else
        {
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
            else
            {
              ncols = line_ncols;
            }
          }
        }
      }
    }
    catch (...) {
      if (pr) pr->error("Could not open file: " + fac_fn);
      return (result);
    }
    inputfile.close();
  }

  if (0 == num_elems)
  {
    num_elems = nrows;
  }

  FieldInformation fi("TriSurfMesh", 1,"double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();

  mesh->node_reserve(num_nodes);
  mesh->elem_reserve(num_elems);

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(pts_fn.c_str());

      std::vector<double> vdata(3);

      for(int i = 0; i < num_nodes && getline(inputfile,line,'\n'); ++i)
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

        if (values.size() == 3) mesh->add_point(Point(values[0],values[1],values[2]));
        if (values.size() == 2) mesh->add_point(Point(values[0],values[1],0.0));
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: "+std::string(pts_fn));
      return (result);
    }
    inputfile.close();
  }

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(fac_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(3);

      std::vector<VMesh::index_type> ivalues;

      for (int i = 0; i < num_elems && getline(inputfile,line,'\n'); ++i)
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

        multiple_from_string(line,ivalues);
        for (size_t j = 0; j < ivalues.size() && j < 3; j++)
        {
          if (zero_based) vdata[j] = ivalues[j];
          else vdata[j] = ivalues[j]-1;
        }

        if (ivalues.size() > 2) mesh->add_elem(vdata);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: "+std::string(fac_fn));
      return (result);
    }
    inputfile.close();
  }

  return (result);
}

FieldHandle SCIRun::MToTriSurfField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result = 0;

  FieldInformation fi("TriSurfMesh", "TriLinearLgn", "double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();

  std::ifstream instr(filename);

  if (instr.fail()) {
    if (pr) pr->error("could not open file " + std::string(filename));
    return 0;
  }

  unsigned int line = 1;
  while (! instr.eof()) {
    std::string type;
    instr >> type;
    if (type[0] == '#') {
      //cerr << "skipping comment line" << endl;
      char c = instr.get();
      while(c != '\n') {
        //cerr << c << endl;
        c = instr.get();
      }
      line++;
      continue;
    }
    unsigned int  idx;
    instr >> idx;
    if (type == "Vertex") {
      double x, y, z;
      instr >> x >> y >> z;
      mesh->add_point(Point(x,y,z));
      //cerr << "Added point #"<< i <<": ("
      //   << x << ", " << y << ", " << z << ")" << endl;
    } else if (type == "Face") {
      VMesh::Node::array_type n(3);
      unsigned int n1, n2, n3;
      instr >> n1 >> n2 >> n3;
      n1 -= 1; n[0] = n1;
      n2 -= 1; n[1] = n2;
      n3 -= 1; n[2] = n3;

      mesh->add_elem(n);
      //cerr << "Added face #"<< i <<": ("
      //   << n1 << ", " << n2 << ", " << n3 << ")" << endl;
    } else {
      if (instr.eof()) break;
      std::ostringstream oss;
      oss << "parsing error:" << type << ": at line: " << line;
      if (pr) pr->error(oss.str());
      return 0;
    }
    line++;
  }
  return (result);
}

bool SCIRun::TriSurfFieldToTextBaseIndexZero_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
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

      VMesh::Node::iterator nodeIter;
      VMesh::Node::iterator nodeIterEnd;
      VMesh::Node::size_type nodeSize;

      mesh->begin(nodeIter);
      mesh->end(nodeIterEnd);
      mesh->size(nodeSize);

     // N.B: not writing header

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
      if (pr) pr->error("Could not open and write to file: "+ pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Faces file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string facs_fn(filename);
    std::string::size_type pos = facs_fn.find_last_of(".");
    std::string base = facs_fn.substr(0, pos);
    std::string ext  = facs_fn.substr(pos);
    const char* fileExt = ".fac";

    if (pos == std::string::npos)
    {
      facs_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      facs_fn = base + fileExt;
    }

    try
    {
      outputfile.open(facs_fn.c_str());

      VMesh::Face::iterator faceIter;
      VMesh::Face::iterator faceIterEnd;
      VMesh::Face::size_type faceSize;
      VMesh::Node::array_type faceNodes(3);

      mesh->begin(faceIter);
      mesh->end(faceIterEnd);
      mesh->size(faceSize);

      while (faceIter != faceIterEnd) {
        mesh->get_nodes(faceNodes, *faceIter);
        outputfile << faceNodes[0] << " " << faceNodes[1] << " " << faceNodes[2] << "\n";
        ++faceIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: "+ facs_fn);
      return (false);
    }
    outputfile.close();
  }
  return (true);
}

bool SCIRun::TriSurfFieldToTextBaseIndexOne_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
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
    else
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

      VMesh::Node::iterator nodeIter;
      VMesh::Node::iterator nodeIterEnd;
      VMesh::Node::size_type nodeSize;

      mesh->begin(nodeIter);
      mesh->end(nodeIterEnd);
      mesh->size(nodeSize);

     // N.B: not writing header

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
      if (pr) pr->error("Could not open and write to file: "+ pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Faces file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string facs_fn(filename);
    std::string::size_type pos = facs_fn.find_last_of(".");
    std::string base = facs_fn.substr(0, pos);
    std::string ext  = facs_fn.substr(pos);
    const char* fileExt = ".fac";

    if (pos == std::string::npos)
    {
      facs_fn += fileExt;
    }
    else
    {
      facs_fn = base + fileExt;
    }

    try
    {
      outputfile.open(facs_fn.c_str());

      VMesh::Face::iterator faceIter;
      VMesh::Face::iterator faceIterEnd;
      VMesh::Face::size_type faceSize;
      VMesh::Node::array_type faceNodes(3);

      mesh->begin(faceIter);
      mesh->end(faceIterEnd);
      mesh->size(faceSize);

      int baseIndex = 1;

      while (faceIter != faceIterEnd) {
        mesh->get_nodes(faceNodes, *faceIter);
        outputfile << faceNodes[0] + baseIndex << " "
                   << faceNodes[1] + baseIndex << " "
                   << faceNodes[2] + baseIndex << "\n";
        ++faceIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: "+ facs_fn);
      return (false);
    }
    outputfile.close();
  }
  return (true);
}

bool SCIRun::TriSurfFieldToM_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
  std::string m_fn(filename);
  std::string::size_type pos = m_fn.find_last_of(".");
  std::string base = m_fn.substr(0, pos);
  std::string ext  = m_fn.substr(pos);
  const char* fileExt = ".m";

  if (pos == std::string::npos)
  {
    m_fn += fileExt;
  }
  else
  {
    m_fn = base + fileExt;
  }

  try
  {
    outputfile.open(m_fn.c_str());

    // these appear to be reasonable formatting flags for output
    std::ios_base::fmtflags ff;
    ff = outputfile.flags();
    ff |= outputfile.showpoint; // write floating-point values including always the decimal point
    ff |= outputfile.fixed; // write floating point values in fixed-point notation
    outputfile.flags(ff);

    VMesh::Node::iterator nodeIter;
    VMesh::Node::iterator nodeIterEnd;
    VMesh::Node::size_type nodeSize;

    mesh->begin(nodeIter);
    mesh->end(nodeIterEnd);
    mesh->size(nodeSize);

    unsigned int count = 1;
    while (nodeIter != nodeIterEnd)
    {
      Point p;
      mesh->get_center(p, *nodeIter);
      outputfile << "Vertex " << count++ << " " << p.x() << " " << p.y() << " " << p.z() << std::endl;
      ++nodeIter;
    }

    VMesh::Face::iterator faceIter;
    VMesh::Face::iterator faceIterEnd;
    VMesh::Face::size_type faceSize;
    VMesh::Node::array_type faceNodes(3);

    mesh->size(faceSize);
    mesh->begin(faceIter);
    mesh->end(faceIterEnd);
    count = 1;
    while (faceIter != faceIterEnd) {
      mesh->get_nodes(faceNodes, *faceIter);
      outputfile << "Face " << count++ << " " << faceNodes[0] << " " << faceNodes[1] << " " << faceNodes[2] << std::endl;
      ++faceIter;
    }
  }
  catch (...)
  {
    if (pr) pr->error("Could not open and write to file: "+ m_fn);
    return (false);
  }
  outputfile.close();
  return (true);
}

bool SCIRun::TriSurfFieldToVtk_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  const TypeDescription *mtd = fh->get_type_description(Field::MESH_TD_E);
  if (mtd->get_name().find("TriSurfMesh") == std::string::npos) {
    if (pr) pr->error("Input field didn't have a TriSurfMesh (type_name=" + mtd->get_name() + ")");
    return (false);
  }

  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
  std::string vtk_fn(filename);
  std::string::size_type pos = vtk_fn.find_last_of(".");
  std::string base = vtk_fn.substr(0, pos);
  std::string ext  = vtk_fn.substr(pos);
  const char* fileExt = ".vtk";

  if (pos == std::string::npos)
  {
    vtk_fn += fileExt;
  }
  else
  {
    vtk_fn = base + fileExt;
  }

  try
  {
    outputfile.open(vtk_fn.c_str());

    // these appear to be reasonable formatting flags for output
    std::ios_base::fmtflags ff;
    ff = outputfile.flags();
    ff |= outputfile.showpoint; // write floating-point values including always the decimal point
    ff |= outputfile.fixed; // write floating point values in fixed-point notation
    outputfile.flags(ff);

    outputfile << "# vtk DataFile Version 3.0" << std::endl
               << "vtk output\nASCII"<< std::endl
              << "DATASET POLYDATA" << std::endl << std::endl;

    VMesh::Node::iterator nodeIter;
    VMesh::Node::iterator nodeIterEnd;
    VMesh::Node::size_type nodeSize;

    mesh->begin(nodeIter);
    mesh->end(nodeIterEnd);
    mesh->size(nodeSize);

    outputfile << "POINTS " << nodeSize << " float" << std::endl;
    while (nodeIter != nodeIterEnd)
    {
      Point p;
      mesh->get_center(p, *nodeIter);
      outputfile << p.x() << " " << p.y() << " " << p.z() << std::endl;
      ++nodeIter;
    }

    VMesh::Face::iterator faceIter;
    VMesh::Face::iterator faceIterEnd;
    VMesh::Face::size_type faceSize;
    VMesh::Node::array_type faceNodes(3);

    mesh->size(faceSize);
    mesh->begin(faceIter);
    mesh->end(faceIterEnd);

    outputfile << "POLYGONS " << faceSize << " " << faceSize * 4 << std::endl;
    while (faceIter != faceIterEnd) {
      mesh->get_nodes(faceNodes, *faceIter);
      outputfile << "3 " << faceNodes[0] << " " << faceNodes[1] << " " << faceNodes[2] << std::endl;
      ++faceIter;
    }
  }
  catch (...)
  {
    if (pr) pr->error("Could not open and write to file: "+ vtk_fn);
    return (false);
  }
  outputfile.close();
  return (true);
}

bool SCIRun::TriSurfFieldToExotxt_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  if (fh->get_type_description(Field::MESH_TD_E)->get_name().find("TriSurfField") !=
      std::string::npos)
  {
    if (pr) pr->error("input field wasn't a TriSurfField (type_name="
      + fh->get_type_description(Field::MESH_TD_E)->get_name() + ")");
    return false;
  }

  VMesh *mesh = fh->vmesh();
  VMesh::Node::iterator niter;
  VMesh::Node::iterator niter_end;
  VMesh::Node::size_type nsize;
  mesh->begin(niter);
  mesh->end(niter_end);
  mesh->size(nsize);
  VMesh::Face::size_type fsize;
  VMesh::Face::iterator fiter;
  VMesh::Face::iterator fiter_end;
  VMesh::Node::array_type fac_nodes(3);
  mesh->size(fsize);
  mesh->begin(fiter);
  mesh->end(fiter_end);

  FILE *f_out = fopen(filename, "wt");
  if (!f_out) {
    if (pr) pr->error("opening output file " + std::string(filename));
    return (false);
  }

  int node_size = (unsigned)(nsize);
  int tri_size = (unsigned)(fsize);
  std::cerr << "Number of points = "<< nsize <<"\n";

  fprintf( f_out, "! Database Title                             exo2txt                                                                 \n" );
  fprintf( f_out, "cubit(temp.g): 05/06/2005: 16:52:36                                             \n" );
  fprintf( f_out, "! Database initial variables\n" );
  fprintf( f_out, "         3      3.01               ! dimensions, version number\n" );
  fprintf( f_out, "      %d      %d         1     ! nodes, elements, element blocks\n", node_size, tri_size );
  fprintf( f_out, "           0         0               ! #node sets, #side sets\n" );
  fprintf( f_out, "         0         0               ! len: node set list, dist fact length\n" );
  fprintf( f_out, "         0         0         0     ! side sets len: element, node , dist fact\n" );
  fprintf( f_out, "! Coordinate names\n" );
  fprintf( f_out, "x                                y                                z                               \n" );
  fprintf( f_out, "! Coordinates\n" );

  while(niter != niter_end) {
    Point p;
    mesh->get_center(p, *niter);
    fprintf(f_out, "%lf %lf %lf\n", p.x(), p.y(), p.z());
    ++niter;
  }
  fprintf( f_out, "! Node number map\n" );
  fprintf( f_out, "sequence 1..numnp\n" );
  fprintf( f_out, "! Element number map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element order map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element block    1\n" );
  fprintf( f_out, "         1%10d      TRI3      ! ID, elements, name\n", tri_size );
  fprintf( f_out, "         3         0      ! nodes per element, attributes\n" );
  fprintf( f_out, "! Connectivity\n" );

  std::cerr << "Number of tris = "<< fsize <<"\n";
  while(fiter != fiter_end) {
    mesh->get_nodes(fac_nodes, *fiter);
    fprintf(f_out, "%d %d %d\n",
      (int)fac_nodes[0],
      (int)fac_nodes[1],
      (int)fac_nodes[2]);
    ++fiter;
  }

  fprintf( f_out, "! Properties\n" );
  fprintf( f_out, "         1            ! Number of ELEMENT BLOCK Properties\n" );
  fprintf( f_out, "! Property Name: \n" );
  fprintf( f_out, "ID                              \n" );
  fprintf( f_out, "! Property Value(s): \n" );
  fprintf( f_out, "         1\n" );
  fprintf( f_out, "         0            ! Number of NODE SET Properties\n" );
  fprintf( f_out, "         0            ! Number of SIDE SET Properties\n" );
  fprintf( f_out, "! QA Records\n" );
  fprintf( f_out, "         1      ! QA records\n" );
  fprintf( f_out, "exo2txt                         \n" );
  fprintf( f_out, " 1.13                           \n" );
  fprintf( f_out, "20050506                        \n" );
  fprintf( f_out, "16:54:44                        \n" );
  fprintf( f_out, "! Information Records\n" );
  fprintf( f_out, "         0      ! information records\n" );
  fprintf( f_out, "! Variable names\n" );
  fprintf( f_out, "         0         0         0      ! global, nodal, element variables\n" );

  fclose(f_out);

  return (true);
}

bool SCIRun::TriSurfFieldToExotxtBaseIndexOne_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  if (fh->get_type_description(Field::MESH_TD_E)->get_name().find("TriSurfField") !=
      std::string::npos)
  {
    if (pr) pr->error("input field wasn't a TriSurfField (type_name="
      + fh->get_type_description(Field::MESH_TD_E)->get_name() + ")");
    return (false);
  }

  VMesh *mesh = fh->vmesh();
  VMesh::Node::iterator niter;
  VMesh::Node::iterator niter_end;
  VMesh::Node::size_type nsize;
  mesh->begin(niter);
  mesh->end(niter_end);
  mesh->size(nsize);
  VMesh::Face::size_type fsize;
  VMesh::Face::iterator fiter;
  VMesh::Face::iterator fiter_end;
  VMesh::Node::array_type fac_nodes(3);
  mesh->size(fsize);
  mesh->begin(fiter);
  mesh->end(fiter_end);

  FILE *f_out = fopen(filename, "wt");
  if (!f_out) {
    if (pr) pr->error("opening output file " + std::string(filename));
    return false;
  }

  int node_size = (unsigned)(nsize);
  int tri_size = (unsigned)(fsize);
  //cerr << "Number of points = "<< nsize <<"\n";

  fprintf( f_out, "! Database Title                             exo2txt                                                                 \n" );
  fprintf( f_out, "cubit(temp.g): 05/06/2005: 16:52:36                                             \n" );
  fprintf( f_out, "! Database initial variables\n" );
  fprintf( f_out, "         3      3.01               ! dimensions, version number\n" );
  fprintf( f_out, "      %d      %d         1     ! nodes, elements, element blocks\n", node_size, tri_size );
  fprintf( f_out, "           0         0               ! #node sets, #side sets\n" );
  fprintf( f_out, "         0         0               ! len: node set list, dist fact length\n" );
  fprintf( f_out, "         0         0         0     ! side sets len: element, node , dist fact\n" );
  fprintf( f_out, "! Coordinate names\n" );
  fprintf( f_out, "x                                y                                z                               \n" );
  fprintf( f_out, "! Coordinates\n" );

  while(niter != niter_end) {
    Point p;
    mesh->get_center(p, *niter);
    fprintf(f_out, "%lf %lf %lf\n", p.x(), p.y(), p.z());
    ++niter;
  }
  fprintf( f_out, "! Node number map\n" );
  fprintf( f_out, "sequence 1..numnp\n" );
  fprintf( f_out, "! Element number map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element order map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element block    1\n" );
  fprintf( f_out, "         1%10d      TRI3      ! ID, elements, name\n", tri_size );
  fprintf( f_out, "         3         0      ! nodes per element, attributes\n" );
  fprintf( f_out, "! Connectivity\n" );

  //cerr << "Number of tris = "<< fsize <<"\n";
  int baseIndex = 1;
  while(fiter != fiter_end) {
    mesh->get_nodes(fac_nodes, *fiter);
    fprintf(f_out, "%d %d %d\n",
      (int)fac_nodes[0]+baseIndex,
      (int)fac_nodes[1]+baseIndex,
      (int)fac_nodes[2]+baseIndex);
    ++fiter;
  }

  fprintf( f_out, "! Properties\n" );
  fprintf( f_out, "         1            ! Number of ELEMENT BLOCK Properties\n" );
  fprintf( f_out, "! Property Name: \n" );
  fprintf( f_out, "ID                              \n" );
  fprintf( f_out, "! Property Value(s): \n" );
  fprintf( f_out, "         1\n" );
  fprintf( f_out, "         0            ! Number of NODE SET Properties\n" );
  fprintf( f_out, "         0            ! Number of SIDE SET Properties\n" );
  fprintf( f_out, "! QA Records\n" );
  fprintf( f_out, "         1      ! QA records\n" );
  fprintf( f_out, "exo2txt                         \n" );
  fprintf( f_out, " 1.13                           \n" );
  fprintf( f_out, "20050506                        \n" );
  fprintf( f_out, "16:54:44                        \n" );
  fprintf( f_out, "! Information Records\n" );
  fprintf( f_out, "         0      ! information records\n" );
  fprintf( f_out, "! Variable names\n" );
  fprintf( f_out, "         0         0         0      ! global, nodal, element variables\n" );

  fclose(f_out);

  return (true);
}
