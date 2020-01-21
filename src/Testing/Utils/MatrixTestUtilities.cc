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


#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>

using namespace SCIRun;

FieldHandle SCIRun::TestUtils::loadFieldFromFile(const boost::filesystem::path& filename)
{
  FieldHandle handle;
  PiostreamPtr stream = auto_istream(filename.string(), nullptr);
  if (!stream)
  {
    throw "Error reading file '" + filename.string();
  }

  Pio(*stream, handle);

  if (!handle || stream->error())
  {
    throw "File read error";
  }
  return handle;
}

bool SCIRun::TestUtils::compareNodes(FieldHandle expected, FieldHandle actual)
{
  Core::Algorithms::Fields::GetMeshNodesAlgo getfieldnodes_algo;
  Core::Datatypes::DenseMatrixHandle output_nodes, exp_result_nodes;
  const double epsilon = 1e-8;
  try
  {
    getfieldnodes_algo.run(actual, output_nodes);
    getfieldnodes_algo.run(expected, exp_result_nodes);
  }
  catch (...)
  {
    std::cerr << " ERROR: could not get field nodes from input files. " << std::endl;
    return false;
  }

  if (output_nodes->ncols() != exp_result_nodes->ncols() || output_nodes->nrows() != exp_result_nodes->nrows())
  {
    std::cerr << " ERROR: number of nodes is different than expected. " << std::endl;
    return false;
  }

  for (int idx = 0; idx < exp_result_nodes->nrows(); ++idx)
  {
    for (int c = 0; c < 3; ++c)
    {
      if (fabs((*exp_result_nodes)(idx, c) - (*output_nodes)(idx, c)) > epsilon)
      {
        std::cerr << " ERROR: node (" << idx << "," << c << ") coordinate value " << (*output_nodes)(idx, c) <<
          " is different than expected: " << (*exp_result_nodes)(idx, c) << std::endl;
        std::cerr << "Full expected nodes: \n" << *exp_result_nodes << "\n\nFull actual nodes:\n" << *output_nodes << std::endl;
        return false;
      }
    }
  }
  return true;
}
