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

#ifndef CORE_ALGORITHMS_FINTEELEMENTS_BUILDFEGRIDMAPPING_H
#define CORE_ALGORITHMS_FINTEELEMENTS_BUILDFEGRIDMAPPING_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixOperations.h>

// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

// for Windows support
#include <Core/Algorithms/FiniteElements/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// Generate mapping matrices for the case that certain nodes contain the same
// value, e.g. a period domain or a domain with a floating conductor.
// This function will generate a mapping from the mesh (geom) to the computational
// grid (grid). Depending on which vector with values need to be adapted 
// (sources or potentials) several mapping matrices are constructed.

class SCISHARE BuildFEGridMappingAlgo : public AlgoBase 
{
  public:
    // Set default values
    BuildFEGridMappingAlgo()
    {
      add_bool("build_potential_gridtogeom",true);
      add_bool("build_potential_geomtogrid",true);
      add_bool("build_current_gridtogeom",true);
      add_bool("build_current_geomtogrid",true);
    }
  
    // Griduting the mapping between the Gridutational Grid and the
    // Geometry grid: We have two versions, one for potentials where potentials
    // are averaged when projected onto the grid and where they are copied when
    // projecting back on the mesh, and one for currents, where currents are 
    // added together when nodes are merged and where currents are distributed
    // over the nodes when translating back to the geometry. 
    // More over potentials are assumed to be continuous over the boundary
    // and currents are assumed as sources that need to be divided or added
    // together.
    bool run(MatrixHandle& NodeLink, 
             MatrixHandle& PotentialGeomToGrid, 
             MatrixHandle& PotentialGridToGeom,
             MatrixHandle& CurrentGeomToGrid, 
             MatrixHandle& CurrentGridToGeom);  

    // Special version for not linking elements that
    // are not in the same domain
    bool run(FieldHandle&  DomainField,
             MatrixHandle& NodeLink, 
             MatrixHandle& PotentialGeomToGrid, 
             MatrixHandle& PotentialGridToGeom,
             MatrixHandle& CurrentGeomToGrid, 
             MatrixHandle& CurrentGridToGeom);  
};

}

#endif
