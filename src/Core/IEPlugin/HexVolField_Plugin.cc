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

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

namespace SCIRun {

FieldHandle TextToHexVolField_reader(ProgressReporter *pr, const char *filename);

bool HexVolFieldToTextBaseIndexZero_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);
bool HexVolFieldToTextBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);
bool HexVolFieldToVtk_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);
bool HexVolFieldToExotxt_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);
bool HexVolFieldToExotxtBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename);

FieldHandle TextToHexVolField_reader(ProgressReporter *pr, const char *filename)
{
  FieldHandle result = 0;

  std::string hexes_fn(filename);
  std::string pts_fn(filename);

  // Check whether the .hex exists
  std::string::size_type pos = hexes_fn.find_last_of(".");
  if (pos == std::string::npos)
  {
    hexes_fn = hexes_fn + ".hex";
    try
    {
      std::ifstream inputfile;
      inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
      inputfile.open(hexes_fn.c_str());
    }
    catch (...)
    {
      if (pr) pr->error("Could not open file: " + hexes_fn);
      return (result);
    }
  }
  else
  {
    std::string base = hexes_fn.substr(0,pos);
    std::string ext  = hexes_fn.substr(pos);
    if ((ext != ".hex" ))
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        hexes_fn = base + ".hex";
        inputfile.open(hexes_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + base + ".hex");
        return (result);
      }
    }
    else
    {
      try
      {
        std::ifstream inputfile;
        inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        inputfile.open(hexes_fn.c_str());
      }
      catch (...)
      {
        if (pr) pr->error("Could not open file: " + hexes_fn);
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

  int ncols = 0;
  int nrows = 0;
  int line_ncols = 0;
  int nnodes = 0, nhexes = 0;

  // STAGE 1 - SCAN THE FILE TO DETERMINE THE NUMBER OF NODES
  // AND CHECK THE FILE'S INTEGRITY.

  bool has_header_pts = false;
  bool first_line_pts = true;

  bool has_header = false;
  bool first_line = true;

  bool has_data = false;

  std::vector<double> values;
  std::string line;

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
        for (size_t p = 0; p < line.size(); ++p)
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
              nnodes = static_cast<int>(values[0]) + 1;
            }
            else if ((line_ncols > 1))
            {
              has_header_pts = false;
              first_line_pts = false;
              ++nrows;
              ncols = line_ncols;
            }
          }
        }
        else
        {
          if (line_ncols > 0)
          {
            ++nrows;
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
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + pts_fn);
      return (result);
    }
    inputfile.close();

    if (0 == nnodes)
    {
      nnodes = nrows;
    }
    else if ( has_header_pts && (nnodes != nrows) )
    {
      if (pr) pr->warning("Number of nodes listed in header (" + boost::lexical_cast<std::string>(nnodes) +
                          ") does not match number of non-header rows in file (" + boost::lexical_cast<std::string>(nrows) + ")");
    }
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
      inputfile.open(hexes_fn.c_str());

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

        multiple_from_string(line,values);
        line_ncols = values.size();

        for (size_t j=0; j < values.size(); ++j) if (values[j] == 0.0) zero_based = true;

        if (first_line)
        {
          if (line_ncols > 0)
          {
            if (line_ncols == 1)
            {
              has_header = true;
              nhexes = static_cast<int>(values[0]) + 1;
            }
            else if (line_ncols > 7)
            {
              has_header = false;
              first_line = false;
              ++nrows;
              ncols = line_ncols;
              if (ncols == 9) has_data = true;
            }
            else
            {
              if (pr)  pr->error("Improper format of text file, the first line does not contain either a header or at least 8 entries");
              return (result);
            }
          }
        }
        else
        {
          if (line_ncols > 0)
          {
            ++nrows;
            if (ncols > 0)
            {
              if (ncols != line_ncols)
              {
                if (pr)  pr->error("Improper format of text file, not every line contains the same amount of node references");
                return (result);
              }
            }
            else
            {
              ncols = line_ncols;
              if (ncols == 9) has_data = true;
            }
          }
        }
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + hexes_fn);
      return (result);
    }
    inputfile.close();
    if (nhexes == 0)
    {
      nhexes = nrows;
    }
    else if ( has_header && (nhexes != nrows) )
    {
      if (pr) pr->warning("Number of elements listed in header (" + boost::lexical_cast<std::string>(nhexes) +
                          ") does not match number of non-header rows in file (" + boost::lexical_cast<std::string>(nrows) + ")");
    }
  }

  FieldInformation fi("HexVolMesh",-1,"double");
  if (has_data) fi.make_constantdata();
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();
  VField *field = result->vfield();

  mesh->node_reserve(nnodes);
  mesh->elem_reserve(nhexes);

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );
    try
    {
      inputfile.open(pts_fn.c_str());

      std::vector<double> vdata(3);

      for (int i = 0; i < nnodes && getline(inputfile, line, '\n'); ++i)
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

        multiple_from_string(line,vdata);

        if (vdata.size() == 3) mesh->add_point(Point(vdata[0],vdata[1],vdata[2]));
        if (vdata.size() == 2) mesh->add_point(Point(vdata[0],vdata[1],0.0));
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  std::vector<double> fvalues;

  {
    std::ifstream inputfile;
    inputfile.exceptions( std::ifstream::badbit );

    try
    {
      inputfile.open(hexes_fn.c_str());

      VMesh::Node::array_type vdata;
      vdata.resize(8);

      std::vector<VMesh::index_type> ivalues;

      for (int i = 0; i < nhexes && getline(inputfile, line, '\n'); ++i)
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

        multiple_from_string(line,ivalues);

        for (size_t j = 0; j < ivalues.size() && j < 8; ++j)
        {
          if (zero_based) vdata[j] = ivalues[j];
          else vdata[j] = ivalues[j]-1;
        }
        if (ivalues.size() > 8) fvalues.push_back(ivalues[8]);

        if (ivalues.size() > 7) mesh->add_elem(vdata);
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and process file: " + hexes_fn);
      return (result);
    }
    inputfile.close();
  }

  if (has_data)
  {
    field->resize_values();
    field->set_values(fvalues);
  }
  return (result);
}

bool HexVolFieldToTextBaseIndexZero_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
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
        outputfile << p.x() << " " << p.y() << " " << p.z() << std::endl;
        ++nodeIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: " + pts_fn);
      return (false);
    }
    outputfile.close();
  }

  // Hex file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string hexes_fn(filename);
    std::string::size_type pos = hexes_fn.find_last_of(".");
    std::string base = hexes_fn.substr(0, pos);
    std::string ext  = hexes_fn.substr(pos);
    const char* fileExt = ".hex";

    if (pos == std::string::npos)
    {
      hexes_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      hexes_fn = base + fileExt;
    }

    try
    {
      outputfile.open(hexes_fn.c_str());

      VMesh::Cell::iterator cellIter;
      VMesh::Cell::iterator cellIterEnd;
      VMesh::Cell::size_type cellSize;
      VMesh::Node::array_type cellNodes(8);

      mesh->begin(cellIter);
      mesh->end(cellIterEnd);
      mesh->size(cellSize);

#if DEBUG
      std::cerr << "Number of hexes = " << cellSize << std::endl;
#endif

      while (cellIter != cellIterEnd) {
        mesh->get_nodes(cellNodes, *cellIter);
        outputfile << cellNodes[0] << " "
                   << cellNodes[1] << " "
                   << cellNodes[2] << " "
                   << cellNodes[3] << " "
                   << cellNodes[4] << " "
                   << cellNodes[5] << " "
                   << cellNodes[6] << " "
                   << cellNodes[7] << std::endl;
        ++cellIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: " + hexes_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}

bool HexVolFieldToTextBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
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
        outputfile << p.x() << " " << p.y() << " " << p.z() << std::endl;
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

  // Hex file
  {
    std::ofstream outputfile;
    outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    std::string hexes_fn(filename);
    std::string::size_type pos = hexes_fn.find_last_of(".");
    std::string base = hexes_fn.substr(0, pos);
    std::string ext  = hexes_fn.substr(pos);
    const char* fileExt = ".hex";

    if (pos == std::string::npos)
    {
      hexes_fn += fileExt;
    }
    else if (ext != fileExt)
    {
      hexes_fn = base + fileExt;
    }

    try
    {
      outputfile.open(hexes_fn.c_str());

      VMesh::Cell::iterator cellIter;
      VMesh::Cell::iterator cellIterEnd;
      VMesh::Cell::size_type cellSize;
      VMesh::Node::array_type cellNodes(4);

      mesh->begin(cellIter);
      mesh->end(cellIterEnd);
      mesh->size(cellSize);
      int baseIndex = 1;

#if DEBUG
      std::cerr << "Number of hexes = " << cellSize << std::endl;
#endif

      while (cellIter != cellIterEnd) {
        mesh->get_nodes(cellNodes, *cellIter);
        outputfile << cellNodes[0] + baseIndex << " "
                   << cellNodes[1] + baseIndex << " "
                   << cellNodes[2] + baseIndex << " "
                   << cellNodes[3] + baseIndex << " "
                   << cellNodes[4] + baseIndex << " "
                   << cellNodes[5] + baseIndex << " "
                   << cellNodes[6] + baseIndex << " "
                   << cellNodes[7] + baseIndex << std::endl;
        ++cellIter;
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and write to file: "+ hexes_fn);
      return (false);
    }
    outputfile.close();
  }
  return true;
}

bool HexVolFieldToVtk_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  // VTK file format (PDF file): http://www.vtk.org/VTK/img/file-formats.pdf
  VMesh *mesh = fh->vmesh();
  VField *field = fh->vfield();

  if (! mesh->is_hexvolmesh())
  {
    if (pr) pr->error("Exporter only supports HexVol fields.");
    return false;
  }

  // TODO: extend with tensor support
  if (! (field->is_scalar() || field->is_vector()) )
  {
    if (pr) pr->error("Export to VTK file is only supported for fields with scalar and vector data");
    return false;
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
  else if (ext != fileExt)
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
          << "vtk output" << std::endl
          << "ASCII" << std::endl << std::endl
          << "DATASET UNSTRUCTURED_GRID" << std::endl;

    VMesh::Node::iterator nodeIter;
    VMesh::Node::iterator nodeIterEnd;
    VMesh::Node::size_type nodeSize;

    mesh->begin(nodeIter);
    mesh->end(nodeIterEnd);
    mesh->size(nodeSize);

    outputfile << "POINTS " << nodeSize << " double" << std::endl;
    while (nodeIter != nodeIterEnd)
    {
      Point p;
      mesh->get_center(p, *nodeIter);
      outputfile << std::setprecision(9) << p.x() << " " << p.y() << " " << p.z() << std::endl;
      ++nodeIter;
    }

    VMesh::Elem::iterator elemIter;
    VMesh::Elem::iterator elemIterEnd;
    VMesh::Elem::size_type elemSize;
    VMesh::Node::array_type elemNodes(8);

    mesh->begin(elemIter);
    mesh->end(elemIterEnd);
    mesh->size(elemSize);

#if DEBUG
    std::cerr << "Number of hexes = " << elemSize << std::endl;
#endif

    outputfile << std::endl << "CELLS " << elemSize <<  " " << elemSize * 8 + elemSize << std::endl;
    while (elemIter != elemIterEnd) {
      mesh->get_nodes(elemNodes, *elemIter);
      outputfile << "8 "
                 << elemNodes[0] << " "
                 << elemNodes[1] << " "
                 << elemNodes[2] << " "
                 << elemNodes[3] << " "
                 << elemNodes[4] << " "
                 << elemNodes[5] << " "
                 << elemNodes[6] << " "
                 << elemNodes[7] << std::endl;
      ++elemIter;
    }

    // VTK_HEXAHEDRON = 12
    outputfile << std::endl << "CELL_TYPES " << elemSize << std::endl;
    for (index_type i = 0; i < elemSize; i++) {
      outputfile << 12 << std::endl;
    }

    if ( field->is_lineardata() ) // node centered
    {
      outputfile << std::endl << "POINT_DATA " << nodeSize << std::endl;
      // TODO: extend to support tensors
      if (field->is_scalar())
      {
        outputfile << "SCALARS scalars double " << 1 << std::endl;
        outputfile << "LOOKUP_TABLE default" << std::endl;
        mesh->begin(nodeIter);
        mesh->end(nodeIterEnd);
        double min = DBL_MAX;
        double max = DBL_MIN;

        while (nodeIter != nodeIterEnd)
        {
          double val;
          field->get_value(val, *nodeIter);
          if (val < min) min = val;
          if (val > max) max = val;

          outputfile << std::setprecision(9) << val << std::endl;
          ++nodeIter;
        }
      }
      else if (field->is_vector())
      {
        outputfile << "VECTORS vectors double" << std::endl;
        mesh->begin(nodeIter);
        mesh->end(nodeIterEnd);
        Vector val;
        while (nodeIter != nodeIterEnd)
        {
          field->get_value(val, *nodeIter);
          outputfile << std::setprecision(9) << val.x() << " " << val.y() << " " << val.z() << std::endl;
          ++nodeIter;
        }
      }
      else // redundant, but here for completeness
      {
        if (pr) pr->error("Field node centered data type is not supported for VTK export.");
        return false;
      }
    }
    else if ( field->is_constantdata() ) // cell centered
    {
      outputfile << std::endl << "CELL_DATA " << elemSize << std::endl;
      if (field->is_scalar())
      {
        outputfile << "SCALARS scalars double " << 1 << std::endl;
        outputfile << "LOOKUP_TABLE default" << std::endl;
        mesh->begin(elemIter);
        mesh->end(elemIterEnd);
        double min = DBL_MAX;
        double max = DBL_MIN;

        while (elemIter != elemIterEnd)
        {
          double val;
          field->get_value(val, *elemIter);
          if (val < min) min = val;
          if (val > max) max = val;

          outputfile << std::setprecision(9) << val << std::endl;
          ++elemIter;
        }
      }
      else if (field->is_vector())
      {
        outputfile << "VECTORS vectors double" << std::endl;
        mesh->begin(elemIter);
        mesh->end(elemIterEnd);
        Vector val;
        while (elemIter != elemIterEnd)
        {
          field->get_value(val, *elemIter);
          outputfile << std::setprecision(9) << val.x() << " " << val.y() << " " << val.z() << std::endl;
          ++elemIter;
        }
      }
      else // redundant, but here for completeness
      {
        if (pr) pr->error("Field cell centered data type is not supported for VTK export.");
        return false;
      }
    }
    else
    {
      if (pr)
      {
        std::ostringstream oss;
        oss << "Field basis order (" << field->basis_order() << ") is not supported for VTK export.";
        pr->error(oss.str());
      }
      return false;
    }
  }
  catch (...)
  {
    if (pr) pr->error("Could not open and write to file: " + vtk_fn);
    return false;
  }
  outputfile.close();
  return true;
}

bool HexVolFieldToExotxt_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  if (fh->get_type_description(Field::MESH_TD_E)->get_name().find("HexVolField") !=
      std::string::npos) {
    if (pr) pr->error("input field wasn't a HexVolField (type_name=" + fh->get_type_description(Field::MESH_TD_E)->get_name() + ")");
    return false;
  }

  VMesh::Node::iterator niter;
  VMesh::Node::iterator niter_end;
  VMesh::Node::size_type nsize;
  mesh->begin(niter);
  mesh->end(niter_end);
  mesh->size(nsize);
  VMesh::Cell::size_type csize;
  VMesh::Cell::iterator citer;
  VMesh::Cell::iterator citer_end;
  VMesh::Node::array_type cell_nodes(8);
  mesh->size(csize);
  mesh->begin(citer);
  mesh->end(citer_end);

  FILE *f_out = fopen(filename, "wt");
  if (!f_out) {
    if (pr) pr->error("error opening output file" + std::string(filename));
    return false;
  }

  int node_size = (unsigned)(nsize);
  int hex_size = (unsigned)(csize);
  //cerr << "Number of points = "<< nsize <<"\n";

  fprintf( f_out, "! Database Title                             exo2txt                                                                 \n" );
  fprintf( f_out, "cubit(temp.g): 05/06/2005: 16:52:36                                             \n" );
  fprintf( f_out, "! Database initial variables\n" );
  fprintf( f_out, "         3      3.01               ! dimensions, version number\n" );
  fprintf( f_out, "      %d      %d         1     ! nodes, elements, element blocks\n", node_size, hex_size );
  fprintf( f_out, "           0         0               ! #node sets, #side sets\n" );
  fprintf( f_out, "         0         0               ! len: node set list, dist fact length\n" );
  fprintf( f_out, "         0         0         0     ! side sets len: element, node , dist fact\n" );
  fprintf( f_out, "! Coordinate names\n" );
  fprintf( f_out, "x                                y                                z                               \n" );
  fprintf( f_out, "! Coordinates\n" );

  while(niter != niter_end)
  {
    Point p;
    mesh->get_center(p, *niter);
    fprintf( f_out, "%lf %lf %lf\n", p.x(), p.y(), p.z() );
    ++niter;
  }

  fprintf( f_out, "! Node number map\n" );
  fprintf( f_out, "sequence 1..numnp\n" );
  fprintf( f_out, "! Element number map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element order map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element block    1\n" );
  fprintf( f_out, "         1%10d      HEX8      ! ID, elements, name\n", hex_size );
  fprintf( f_out, "         8         0      ! nodes per element, attributes\n" );
  fprintf( f_out, "! Connectivity\n" );

  //scerr << "Number of hexes = "<< csize <<"\n";
  while(citer != citer_end)
  {
    mesh->get_nodes(cell_nodes, *citer);
    fprintf(f_out, "%d %d %d %d %d %d %d %d\n",
            (int)cell_nodes[0],
            (int)cell_nodes[1],
            (int)cell_nodes[2],
            (int)cell_nodes[3],
            (int)cell_nodes[4],
            (int)cell_nodes[5],
            (int)cell_nodes[6],
            (int)cell_nodes[7]);
    ++citer;
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

  return true;
}

bool HexVolFieldToExotxtBaseIndexOne_writer(ProgressReporter *pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  if (fh->get_type_description(Field::MESH_TD_E)->get_name().find("HexVolField") !=
      std::string::npos) {
    if (pr) pr->error("input field wasn't a HexVolField (type_name=" + fh->get_type_description(Field::MESH_TD_E)->get_name() + ")");
    return false;
  }

  VMesh::Node::iterator niter;
  VMesh::Node::iterator niter_end;
  VMesh::Node::size_type nsize;
  mesh->begin(niter);
  mesh->end(niter_end);
  mesh->size(nsize);
  VMesh::Cell::size_type csize;
  VMesh::Cell::iterator citer;
  VMesh::Cell::iterator citer_end;
  VMesh::Node::array_type cell_nodes(8);
  mesh->size(csize);
  mesh->begin(citer);
  mesh->end(citer_end);

  FILE *f_out = fopen(filename, "wt");
  if (!f_out) {
    if (pr) pr->error("error opening output file" + std::string(filename));
    return false;
  }

  int node_size = (unsigned)(nsize);
  int hex_size = (unsigned)(csize);
  //cerr << "Number of points = "<< nsize <<"\n";

  fprintf( f_out, "! Database Title                             exo2txt                                                                 \n" );
  fprintf( f_out, "cubit(temp.g): 05/06/2005: 16:52:36                                             \n" );
  fprintf( f_out, "! Database initial variables\n" );
  fprintf( f_out, "         3      3.01               ! dimensions, version number\n" );
  fprintf( f_out, "      %d      %d         1     ! nodes, elements, element blocks\n", node_size, hex_size );
  fprintf( f_out, "           0         0               ! #node sets, #side sets\n" );
  fprintf( f_out, "         0         0               ! len: node set list, dist fact length\n" );
  fprintf( f_out, "         0         0         0     ! side sets len: element, node , dist fact\n" );
  fprintf( f_out, "! Coordinate names\n" );
  fprintf( f_out, "x                                y                                z                               \n" );
  fprintf( f_out, "! Coordinates\n" );

  while(niter != niter_end)
  {
    Point p;
    mesh->get_center(p, *niter);
    fprintf( f_out, "%lf %lf %lf\n", p.x(), p.y(), p.z() );
    ++niter;
  }

  fprintf( f_out, "! Node number map\n" );
  fprintf( f_out, "sequence 1..numnp\n" );
  fprintf( f_out, "! Element number map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element order map\n" );
  fprintf( f_out, "sequence 1..numel\n" );
  fprintf( f_out, "! Element block    1\n" );
  fprintf( f_out, "         1%10d      HEX8      ! ID, elements, name\n", hex_size );
  fprintf( f_out, "         8         0      ! nodes per element, attributes\n" );
  fprintf( f_out, "! Connectivity\n" );

  //scerr << "Number of hexes = "<< csize <<"\n";
  int baseIndex = 1;
  while(citer != citer_end)
  {
    mesh->get_nodes(cell_nodes, *citer);
    fprintf(f_out, "%d %d %d %d %d %d %d %d\n",
            (int)cell_nodes[0]+baseIndex,
            (int)cell_nodes[1]+baseIndex,
            (int)cell_nodes[2]+baseIndex,
            (int)cell_nodes[3]+baseIndex,
            (int)cell_nodes[4]+baseIndex,
            (int)cell_nodes[5]+baseIndex,
            (int)cell_nodes[6]+baseIndex,
            (int)cell_nodes[7]+baseIndex);
    ++citer;
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

  return true;
}

static FieldIEPlugin HexVolField_plugin("HexVolField","{.hex} {.pts} {.pos}", "", TextToHexVolField_reader, HexVolFieldToTextBaseIndexZero_writer);
static FieldIEPlugin HexVolFieldBaseIndexOne_plugin("HexVolField[BaseIndex 1]", "{.hex} {.pts}", "", 0, HexVolFieldToTextBaseIndexOne_writer);
static FieldIEPlugin HexVolFieldVtk_plugin("HexVolFieldToVtk", "{.vtk}", "", 0, HexVolFieldToVtk_writer);
static FieldIEPlugin HexVolFieldToExotxt_plugin("HexVolFieldToExotxt", "{.ex2}", "", 0, HexVolFieldToExotxt_writer);
static FieldIEPlugin HexVolFieldToExotxtBaseIndexOne_plugin("HexVolFieldToExotxt[BaseIndex 1]", "{.ex2}", "", 0, HexVolFieldToExotxtBaseIndexOne_writer);

} // end namespace
