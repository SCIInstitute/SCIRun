//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Octree for Lattice
//
// Authors: Jonathan Bronson (bronson@sci.utah.edu)
//          Joshua A. Levine (levine@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, Jonathan Bronson
//  Scientific Computing & Imaging Institute
//  University of Utah
//
//  Permission is  hereby  granted, free  of charge, to any person
//  obtaining a copy of this software and associated documentation
//  files  ( the "Software" ),  to  deal in  the  Software without
//  restriction, including  without limitation the rights to  use,
//  copy, modify,  merge, publish, distribute, sublicense,  and/or
//  sell copies of the Software, and to permit persons to whom the
//  Software is  furnished  to do  so,  subject  to  the following
//  conditions:
//
//  The above  copyright notice  and  this permission notice shall
//  be included  in  all copies  or  substantial  portions  of the
//  Software.
//
//  THE SOFTWARE IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF ANY
//  KIND,  EXPRESS OR IMPLIED, INCLUDING  BUT NOT  LIMITED  TO THE
//  WARRANTIES   OF  MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT  SHALL THE AUTHORS OR
//  COPYRIGHT HOLDERS  BE  LIABLE FOR  ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
//  USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------
//-------------------------------------------------------------------


#ifndef _OCTREE_H_
#define _OCTREE_H_
#include "BCCLattice3D.h"
#include <list>
#include <vector>
#include <cstring>
#include <string>
#include <map>
#include "vec3.h"

namespace Cleaver
{

enum vertex_index { ULF,  ULB, URF, URB, LLF, LLB, LRF, LRB, C };
enum edge_index { DULF, DULB, DURF, DURB,   //  ]_ Diagonal
                  DLLF, DLLB, DLRF, DLRB,   //  ]  Edges
                  CL, CR, CU, CD, CF, CB,   // Dual   Edges
                  UL, UR, UF, UB,           // Top    Face Edges
                  LL, LR, LF, LB,           // Bottom Face Edges
                  FL, FR, BL, BR };         // Four Column Edges
#define CC 26                               // Used When edge_index is used to lookup Oct Cells

enum tri_index { FUL, FUR, FUF, FUB,        // Triangle Faces Touching Upper Lattice Edges
                 FLL, FLR, FLF, FLB,        // Triangle Faces Touching Lower Lattice Edges
                 FFL, FFR, FBL, FBR,        // Triangle Faces Touching Four Column Edges
                 FLUF, FLUB, FLLF, FLLB,    // Triangle Faces Cutting through Left  Face
                 FRUF, FRUB, FRLF, FRLB,    // Triangle Faces Cutting through Right Face
                 FFUL, FFUR, FFLL, FFLR,    // Triangle Faces Cutting through Front Face
                 FBUL, FBUR, FBLL, FBLR,    // Triangle Faces Cutting through Back  Face
                 FUFL, FUFR, FUBL, FUBR,    // Triangle Faces Cutting through Upper Face
                 FDFL, FDFR, FDBL, FDBR};   // Triangle Faces Cutting through Lower Face

enum tet_index { TLU, TLL, TLF, TLB,        // Left  Face Tets
                 TRU, TRL, TRF, TRB,        // Right Face Tets
                 TFT, TFB, TFL, TFR,        // Front Face Tets
                 TBT, TBB, TBL, TBR,        // Back  Face Tets
                 TDF, TDB, TDL, TDR,        // Down  Face Tets
                 TUF, TUB, TUL, TUR};       // Upper Face Tets

enum cell_index {LEFT, RIGHT, UP, DOWN, FRONT, BACK, CENTER};


//these match the vertex indices in naming convention,
//but the ordering matches what is used
//to index OTCell::children in Octree::addCell()
enum children_index { CLLF, CLRF, CULF, CURF, CLLB, CLRB, CULB, CURB };





#define VERTS_PER_CELL 9     // 9 Vertices in leaf cell
#define EDGES_PER_CELL 26    // 26 Edges in leaf cell
#define FACES_PER_CELL 36    // 36 Faces in a leaf cell
#define TETS_PER_CELL  24    // 24 Lattice Tets in a leaf cell


#define EDGES_PER_VERT 14    // 14 Edges Around Each Vertex
#define FACES_PER_VERT 36    // 36 Faces Around Each Vertex
#define TETS_PER_VERT  24    // 24 Tets Around Each Vertex

#define MAX_FACES_PER_EDGE 6
#define MAX_TETS_PER_EDGE 6

#define VERTS_PER_FACE 3     //  3 Verts Around Each Face
#define EDGES_PER_FACE 3     //  3 Edges Around Each Face
#define TETS_PER_FACE  2      //  2 Tets Around Each Face

#define EDGES_PER_TET 6
#define FACES_PER_TET 4
#define VERTS_PER_TET 4

class Vertex3D;
class Edge3D;
class Face3D;
class Tet3D;
class Tet;

class OTCell
{
public:
    OTCell() : parent(NULL),vert(NULL),edge(NULL),face(NULL),tets(NULL),bg_pass_made(0) { memset(children, 0, 8*sizeof(OTCell*)); }
    ~OTCell();

    unsigned int xLocCode;  // X Locational Code
    unsigned int yLocCode;  // Y Locational Code
    unsigned int zLocCode;  // Z Locational Code
    unsigned int level;     // Cell level in hierarchy (smallest cell is level 0)
    OTCell* parent;         // Pointer to parent cell
    OTCell* children[8];    // Pointers to child cells

    Vertex3D **vert;        // 9 Vertices in leaf cell
    Edge3D **edge;          // 26 Edges in leaf cell
    Face3D **face;          // 36 Triangle Faces
    Tet3D **tets;           // 24 Lattice Tetrahedra in cell

    std::string str();       // returns printable index of cell

    int bg_pass_made;    
};

class Octree
{
public:
    Octree(int w, int h, int d, std::vector<Vertex3D*> &verts, std::vector<Tet*> &tets);
    ~Octree();

    OTCell *addCell(float x, float y, float z);
    OTCell *addCellAtLevel(float x, float y, float z, unsigned int level);
    OTCell *getCell(float x, float y, float z);
    OTCell *getDeepestCellParent(float x, float y, float z);
    OTCell *getNeighbor(const OTCell *cell, int x_offset, int y_offset, int z_offset);
    OTCell *getNeighbor(const OTCell *cell, const int offset[3]);
    OTCell *getNeighborNew(const OTCell *cell, int x_offset, int y_offset, int z_offset);
    OTCell *getNeighborNew(const OTCell *cell, const int offset[3]);

    OTCell *getNeighborAtMyLevel(const OTCell *cell, int x_dir, int y_dir, int z_dir);
    OTCell *getCommonAncestor(const OTCell *cell, unsigned int &cellLevel, unsigned int binaryDiff);
    OTCell *getRoot();

    unsigned int getNumLevels() { return nLevels; }
    unsigned int getRootLevel() { return rootLevel; }
    unsigned int getMaxVal() { return maxVal; }

    unsigned int width() { return w; }
    unsigned int height() { return h; }
    unsigned int depth() { return d; }

    std::list<OTCell*> collect_children_at_level(OTCell* pCell, unsigned int level);    
    void balance_tree();

    std::list<OTCell*> collect_all_background_grid_cells(OTCell* pCell);
    bool has_shared_face_vertex(OTCell* pCell, OTCell* qCell, int which);
    bool has_shared_edge_vertex(OTCell* pCell, OTCell* qCell, OTCell* eCell, OTCell* sCell, int wf, int we);
    void create_background_grid(const std::vector<OTCell *> &buffer_cells);
    void label_background_tets(unsigned char* labels);

    Tet* createTet(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, Vertex3D *v4, int material = -1);
    Vertex3D *vertexForPosition(const vec3 &pos);

    std::vector<Vertex3D*> &verts;
    std::vector<Tet*> &tets;

    std::map<vec3, Vertex3D*> vertex_tracker;  // keep track of hierarchical vertices we create

private:
    OTCell *root;
    unsigned int nLevels;        // Number of possible levels in the octree
    unsigned int rootLevel;      // Level of root clel (nLevels - 1)
    unsigned int maxVal;         // For converting positions to locational codes
                                 // (maxVal = 2^rootLevel)
    unsigned int w, h, d;        // data dimensions
    unsigned int bounding_size;
};

}

#endif // OCTREE_H
