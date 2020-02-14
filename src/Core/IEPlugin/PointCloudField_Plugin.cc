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


#include <Core/IEPlugin/PointCloudField_Plugin.h>
#include <Core/ImportExport/Field/FieldIEPlugin.h>
#include <Core/IEPlugin/IEPluginInit.h>

#include <Core/Logging/LoggerInterface.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/PointCloudMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Utils/Legacy/StringUtil.h>

using namespace SCIRun;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;

FieldHandle SCIRun::TextToPointCloudField_reader(LoggerHandle pr, const char *filename)
{
  FieldHandle result;
  std::string pts_fn(filename);

	// Check whether the .pts file exists
	std::string::size_type pos = pts_fn.find_last_of(".");
	if (pos == std::string::npos)
	{
    if (pr) pr->error("Could not open file: " + pts_fn);
    return (result);
	}
	else
	{
		std::string base = pts_fn.substr(0,pos);
		std::string ext  = pts_fn.substr(pos);
		if ((ext != ".pts" )||(ext != ".pos")||(ext != ".txt"))
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
          try
          {
            std::ifstream inputfile;
            inputfile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
            pts_fn = base + ".txt";
            inputfile.open(pts_fn.c_str());
          }
          catch (...)
          {
            if (pr) pr->error("Could not open file: " + pts_fn);
            return (result);
          }
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
  int num_pts = 0;

  std::string line;

  // Read file, get number of rows, cols...

	bool has_header_pts = false;
	bool first_line_pts = true;

  std::vector<double> values;

  FieldInformation fi("PointCloudMesh", "ConstantBasis", "double");
  result = CreateField(fi);

  VMesh *mesh = result->vmesh();
  VField *field = result->vfield();

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
              num_pts = static_cast<int>(values[0]) + 1;
              continue;
            }
            else if ((line_ncols > 1))
            {
              has_header_pts = false;
              first_line_pts = false;
              nrows++;
              ncols = line_ncols;
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
        // fill in 3D or 2D points by row
        if (values.size() == 3) mesh->add_point(Point(values[0], values[1], values[2]));
        if (values.size() == 2) mesh->add_point(Point(values[0], values[1], 0.0));
      }
      if ( has_header_pts && (num_pts != nrows) )
      {
        if (pr) pr->warning("Number of points listed in header (" + boost::lexical_cast<std::string>(num_pts) +
                            ") does not match number of non-header rows in file (" + boost::lexical_cast<std::string>(nrows) + ")");
      }
    }
    catch (...)
    {
      if (pr) pr->error("Could not open and read file: " + pts_fn);
      return (result);
    }
    inputfile.close();
  }

  field->resize_values();
  return (result);
}

bool SCIRun::PointCloudFieldToText_writer(LoggerHandle pr, FieldHandle fh, const char *filename)
{
  VMesh *mesh = fh->vmesh();

  std::string pts_fn(filename);
  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );

	// Check whether the .pts file exists
	std::string::size_type pos = pts_fn.find_last_of(".");
	if (pos == std::string::npos)
  {
    pts_fn += ".pts";
  }
  else
  {
    std::string base = pts_fn.substr(0, pos);
    std::string ext  = pts_fn.substr(pos);
		if ((ext != ".pts" )||(ext != ".pos" )||(ext != ".txt" ))
		{
      pts_fn = base + ".pts";
    }
	}

  try
  {
    outputfile.open(pts_fn.c_str());

    // these appear to be reasonable formatting flags for output
    auto ff = outputfile.flags();
    ff |= outputfile.showpoint; // write floating-point values including always the decimal point
    ff |= outputfile.fixed; // write floating point values in fixed-point notation
    outputfile.precision(15);
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
  return (true);
}
