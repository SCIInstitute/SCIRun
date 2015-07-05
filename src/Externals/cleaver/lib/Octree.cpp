//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Octree for Lattice
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
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
//  Portions of this code derived from MERL Octree source:
//
//-------------------------------------------------------------------
//  Copyright 2002 Mitsubishi Electric Research Laboratories.
//  All Rights Reserved.
//
//  Permission to use, copy, modify  and  distribute this software
//  and its documentation for educational, research and non-profit
//  purposes,  without fee, and  without a  written  agreement  is
//  hereby  granted, provided  that the above copyright notice and
//  the following three paragraphs appear in all copies.
//
//  To  request  permission  to  incorporate  this  software   into
//  commercial products contact MERL - Mitsubishi Electric Research
//  Laboratories, 201 Broadway, Cambridge, MA 02139.
//
//  IN  NO  EVENT SHALL MERL  BE LIABLE  TO ANY  PARTY  FOR DIRECT,
//  INDIRECT,SPECIAL,INCIDENTAL, OR CONSEQUENTIAL DAMAGES INCLUDING
//  LOST PROFITS, ARISING  OUT OF  THE USE OF THIS SOFTWARE AND ITS
//  DOCUMENTATION, EVEN IF MERL HAS BEEN ADVISED OF THE POSSIBILITY
//  OF SUCH DAMAGES.
//
//  MERL  SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
//  LIMITED  TO, THE  IMPLIED  WARRANTIES  OF  MERCHANTABILITY  AND
//  FITNESS FOR  A  PARTICULAR   PURPOSE.  THE  SOFTWARE   PROVIDED
//  HEREUNDER IS  ON AN "AS IS"  BASIS, AND MERL HAS NO OBLIGATIONS
//  TO  PROVIDE  MAINTENANCE,  SUPPORT,  UPDATES,  ENHANCEMENTS, OR
//  MODIFICATIONS.
//-------------------------------------------------------------------

//-------------------------------------------------------------------

#include "Octree.h"
#include <cmath>
#include <cstdlib>
#include <sstream>
#include "TetMesh.h"
#include "Util.h"


using namespace Cleaver;


const int FACE_NEIGHBOR_INDICES[6] = {-4, 4, -2, 2, -1, 1};
const int FACE_NEIGHBOR_OFFSETS[6][3] = {
    {-1, 0, 0},      //left
    { 1, 0, 0},      //right
    { 0, 0,-1},      //front
    { 0, 0, 1},      //back
    { 0,-1, 0},      //down
    { 0, 1, 0}       //up
};

const int EDGE_FACE_NEIGHBOR_OFFSETS[6][4][3] = {
    { { 0,-1, 0}, { 0, 1, 0}, { 0, 0,-1}, { 0, 0, 1} },  // left
    { { 0,-1, 0}, { 0, 1, 0}, { 0, 0,-1}, { 0, 0, 1} },  // right
    { {-1, 0, 0}, { 1, 0, 0}, { 0,-1, 0}, { 0, 1, 0} },  // front
    { {-1, 0, 0}, { 1, 0, 0}, { 0,-1, 0}, { 0, 1, 0} },  // back
    { {-1, 0, 0}, { 1, 0, 0}, { 0, 0,-1}, { 0, 0, 1} },  // down
    { {-1, 0, 0}, { 1, 0, 0}, { 0, 0,-1}, { 0, 0, 1} }   // up
};
const int TET_TO_VERT_INDICES[24][3] = {
    {ULF, ULB, C}, {LLF, LLB, C}, {ULF, LLF, C}, {ULB, LLB, C},   //left
    {URF, URB, C}, {LRF, LRB, C}, {URF, LRF, C}, {URB, LRB, C},   //right
    {ULF, URF, C}, {LLF, LRF, C}, {ULF, LLF, C}, {URF, LRF, C},   //front
    {ULB, URB, C}, {LLB, LRB, C}, {ULB, LLB, C}, {URB, LRB, C},   //back
    {LLF, LRF, C}, {LLB, LRB, C}, {LLF, LLB, C}, {LRF, LRB, C},   //lower
    {ULF, URF, C}, {ULB, URB, C}, {ULF, ULB, C}, {URF, URB, C}    //upper
};


const bool bgParity[6][4] = {
  {false,true,true,false},   // left
  {true,false,false,true},   // right
  {false,true,true,false},   // front
  {true,false,false,true},   // back
  {true,false,false,true},   // down
  {false,true,true,false}    // up
};

//   bgBiParity[face][edge]
const bool bgBiParity[6][4] = {
  {true,false,false,true},    // left
  {false,true,true,false},    // right
  {true,false,false,true},    // front
  {false,true,true,false},    // back
  {false,true,true,false},    // down
  {true,false,false,true}     // up
};

//   bgQuadParity[face][edge]
const bool bgQuadParity[6][4] = {
  {false,true,true,false},    // left
  {true,false,false,true},    // right
  {false,true,true,false},    // front
  {true,false,false,true},    // back
  {true,false,false,true},    // down
  {false,true,true,false}     // up
};


using namespace std;

bool g_bMatchDataDimensionsExactly = true;

unsigned int max(unsigned int a, unsigned int b, unsigned int c)
{
    if(a > b){
        if(a > c){
            return a;
        }
        else{
            return c;
        }
    }
    else{
        if(b > c){
            return b;
        }
        else{
            return c;
        }
    }
}

Octree::Octree(int w, int h, int d, std::vector<Vertex3D*> &verts, std::vector<Tet*> &tets) : verts(verts), tets(tets), root(new OTCell()), w(w), h(h), d(d)
{
    // find smallest power of two that can hold this data
    int n = 0;
    int size = 1;
    while(size < w || size < h || size < d){
        size *= 2;
        n++;
    }

    this->bounding_size = size;
    this->nLevels   = n+1;               // Number of possible levels in the octree
    this->rootLevel = this->nLevels - 1; // Level of root clel (QT_N_LEVELS - 1)
    this->maxVal    = pow2(this->rootLevel); // For converting positions to locational codes
                                         // (QT_MAX_VAL = 2^QT_ROOT_LEVEL)
    this->root->level = this->rootLevel;
    this->root->xLocCode = 0;
    this->root->yLocCode = 0;
    this->root->zLocCode = 0;
}

// Octree Deconstructor
Octree::~Octree()
{   
    // verts / edges / faces / tets should be deleted by now
    if(root)
        delete root;
}

// Octree Cell Deconstructor
OTCell::~OTCell()
{
    // delete my children
    for(int i=0; i < 8; i++)
    {
        if(children[i])
        {
            delete children[i];
        }
    }
}

OTCell* Octree::addCell(float x, float y, float z)
{
    unsigned int xLocCode = (unsigned int)x; // (x * this->maxVal);
    unsigned int yLocCode = (unsigned int)y; // (y * this->maxVal);
    unsigned int zLocCode = (unsigned int)z; // (z * this->maxVal);

    // figure out where this cell should go
    OTCell *pCell = this->root;
    unsigned int nextLevel = this->rootLevel - 1;
    unsigned int n = nextLevel + 1;
    unsigned int childBranchBit;
    unsigned int childIndex;

    while(n--){
         childBranchBit = 1 << nextLevel;
         childIndex = ((xLocCode & childBranchBit) >> (nextLevel))
                    + (((yLocCode & childBranchBit) >> (nextLevel)) << 1)
                    + (((zLocCode & childBranchBit) >> (nextLevel)) << 2);
         --nextLevel;

        if(!(pCell->children[childIndex])){
            OTCell *newCell = new OTCell();
            newCell->level = pCell->level - 1;
            newCell->xLocCode = pCell->xLocCode | (childBranchBit & xLocCode);
            newCell->yLocCode = pCell->yLocCode | (childBranchBit & yLocCode);
            newCell->zLocCode = pCell->zLocCode | (childBranchBit & zLocCode);
            newCell->parent = pCell;
            pCell->children[childIndex] = newCell;
        }

        pCell = pCell->children[childIndex];
    }

    // return newly created leaf-cell, or existing one
    return pCell;
}


//this is a bit of cheater function.  It will create all branches for the
//tree down to a specific level the x,y,z location is the location for some
//leaf, which won't be created unless level == 0.  It is not particularly
//safe to use unless you know what you're doing.
OTCell* Octree::addCellAtLevel(float x, float y, float z, unsigned int level)
{
    unsigned int xLocCode = (unsigned int)x; // (x * this->maxVal);
    unsigned int yLocCode = (unsigned int)y; // (y * this->maxVal);
    unsigned int zLocCode = (unsigned int)z; // (z * this->maxVal);

    // figure out where this cell should go
    OTCell *pCell = this->root;
    unsigned int nextLevel = this->rootLevel - 1;
    unsigned int n = nextLevel + 1;
    unsigned int childBranchBit;
    unsigned int childIndex;

    while(n-- && pCell->level > level){
         childBranchBit = 1 << nextLevel;
         childIndex = ((xLocCode & childBranchBit) >> (nextLevel))
                    + (((yLocCode & childBranchBit) >> (nextLevel)) << 1)
                    + (((zLocCode & childBranchBit) >> (nextLevel)) << 2);
         --nextLevel;

        if(!(pCell->children[childIndex])){
            OTCell *newCell = new OTCell();
            newCell->level = pCell->level - 1;
            newCell->xLocCode = pCell->xLocCode | (childBranchBit & xLocCode);
            newCell->yLocCode = pCell->yLocCode | (childBranchBit & yLocCode);
            newCell->zLocCode = pCell->zLocCode | (childBranchBit & zLocCode);
            newCell->parent = pCell;
            pCell->children[childIndex] = newCell;
        }

        pCell = pCell->children[childIndex];
    }

    // return newly created leaf-cell, or existing one
    return pCell;
}

OTCell* Octree::getCell(float x, float y, float z)
{
    if(x < 0 || y < 0 || z < 0)
        return NULL;
    else if(x >= w || y >= h || z >= d)
        return NULL;


    unsigned int xLocCode = (unsigned int) x; //(x * this->maxVal);
    unsigned int yLocCode = (unsigned int) y; //(y * this->maxVal);
    unsigned int zLocCode = (unsigned int) z; //(z * this->maxVal);

    // branch to appropriate cell
    OTCell *pCell = this->root;
    unsigned int nextLevel = this->rootLevel - 1;

    while(pCell && pCell->level > 0){
        unsigned int childBranchBit = 1 << nextLevel;
        unsigned int childIndex = ((xLocCode  & childBranchBit) >> (nextLevel))
                                + (((yLocCode  & childBranchBit) >> (nextLevel)) << 1)
                                + (((zLocCode & childBranchBit) >> nextLevel) << 2);
        --nextLevel;
        pCell = (pCell->children[childIndex]);
    }

    // return desired cell (or NULL)
    return pCell;
}


OTCell* Octree::getDeepestCellParent(float x, float y, float z)
{
    if(x < 0 || y < 0 || z < 0)
        return NULL;
    else if(x >= w || y >= h || z >= d)
        return NULL;

    unsigned int xLocCode = (unsigned int) x; //(x * this->maxVal);
    unsigned int yLocCode = (unsigned int) y; //(y * this->maxVal);
    unsigned int zLocCode = (unsigned int) z; //(z * this->maxVal);

    // branch to appropriate cell
    OTCell *pCell = this->root;
    OTCell *qCell = this->root;
    unsigned int nextLevel = this->rootLevel - 1;

    while(pCell && pCell->level > 0){
        unsigned int childBranchBit = 1 << nextLevel;
        unsigned int childIndex = ((xLocCode  & childBranchBit) >> (nextLevel))
                                + (((yLocCode  & childBranchBit) >> (nextLevel)) << 1)
                                + (((zLocCode & childBranchBit) >> nextLevel) << 2);
        --nextLevel;
        pCell = (pCell->children[childIndex]);
        if (pCell) {
            qCell = pCell;
        }
    }

    // return desired cell (or NULL)  // JRB 12/6/11: There's no way for this function to return NULL
    return qCell;
}

OTCell* Octree::getNeighborNew(const OTCell *cell, const int offset[3])
{
    return getNeighborNew(cell, offset[0], offset[1], offset[2]);
}

OTCell* Octree::getNeighborNew(const OTCell *cell, int x_offset, int y_offset, int z_offset)
{
    // if neighbor pointer exists, use it quickly
    /*
    if(cell->neighbor[x_offset+1][y_offset+1][z_offset+1])
    {
        return cell->neighbor[x_offset+1][y_offset+1][z_offset+1];
    }
    // otherwise, call getCell()
    else
    {
        return getCell(cell->xLocCode+x_offset,
                       cell->yLocCode+y_offset,
                       cell->zLocCode+z_offset);
    }
    */
    return NULL;
}

OTCell* Octree::getNeighbor(const OTCell *cell, const int offset[3])
{
    return getCell(cell->xLocCode + offset[0],
                   cell->yLocCode + offset[1],
                   cell->zLocCode + offset[2]);
}

OTCell* Octree::getNeighbor(const OTCell *cell, int x_offset, int y_offset, int z_offset)
{
    //JAL temporarily replaced getNeighbor with this
    return getCell(cell->xLocCode+x_offset,
                   cell->yLocCode+y_offset,
                   cell->zLocCode+z_offset);

/*
    if((cell->xLocCode + x_offset) < 0 ||
       (cell->yLocCode + y_offset) < 0 ||
       (cell->zLocCode + z_offset) < 0)
        return NULL;

    // Get cell's x,y, and z locational codes
    unsigned int xLocCode = cell->xLocCode;
    unsigned int yLocCode = cell->yLocCode;
    unsigned int zLocCode = cell->zLocCode;

    // Get Neighbors x,y, and z locational codes
    unsigned int binaryCellSize = 1;    // << cell->level; We only care about leaves
    unsigned int xNeighborLocCode = xLocCode + (x_offset * binaryCellSize);
    unsigned int yNeighborLocCode = yLocCode + (y_offset * binaryCellSize);
    unsigned int zNeighborLocCode = zLocCode + (z_offset * binaryCellSize);

    // Determine smallest common ancestor
    unsigned int x_diff = xLocCode ^ xNeighborLocCode;
    unsigned int y_diff = yLocCode ^ yNeighborLocCode;
    unsigned int z_diff = zLocCode ^ zNeighborLocCode;
    //unsigned int diff = max(x_diff, y_diff, z_diff);

    //OTCell *pCell = cell;
    unsigned int cellLevel, nextLevel;
    cellLevel = nextLevel = cell->level;

    // Get Common Ancestor
    unsigned int x_nextLevel = cell->level;
    unsigned int y_nextLevel = cell->level;
    unsigned int z_nextLevel = cell->level;
    OTCell *x_pCell = getCommonAncestor(cell, x_nextLevel, x_diff);
    OTCell *y_pCell = getCommonAncestor(cell, y_nextLevel, y_diff);
    OTCell *z_pCell = getCommonAncestor(cell, z_nextLevel, z_diff);
    x_nextLevel--;
    y_nextLevel--;
    z_nextLevel--;

    OTCell *pCell = 0;
    if(x_pCell->level > y_pCell->level){
        if(x_pCell->level > z_pCell->level){
            pCell = x_pCell;
            nextLevel = x_nextLevel;
        }
        else{
            pCell = z_pCell;
            nextLevel = z_nextLevel;
        }
    }
    else{
        if(y_pCell->level > z_pCell->level){
            pCell = y_pCell;
            nextLevel = y_nextLevel;
        }
        else{
            pCell = z_pCell;
            nextLevel = z_nextLevel;
        }
    }

    // Traverse Down To Neighbor
    unsigned int n = nextLevel - cellLevel + 1;
    while (n--){
        unsigned int childBranchBit = 1 << nextLevel;
        unsigned int childIndex = ((xLocCode  & childBranchBit) >> (nextLevel))
                                + (((yLocCode  & childBranchBit) >> (nextLevel)) << 1)
                                + (((zLocCode & childBranchBit) >> (nextLevel)) << 2);
        --nextLevel;
        pCell = (pCell->children[childIndex]);
        if(pCell == NULL)
            break;
    }

    // return neighbor
    return pCell;
*/
}


OTCell* Octree::getNeighborAtMyLevel(const OTCell *cell, int x_dir, int y_dir, int z_dir) {
    unsigned int shift = 1 << cell->level;
    unsigned int x = cell->xLocCode + x_dir*shift;
    unsigned int y = cell->yLocCode + y_dir*shift;
    unsigned int z = cell->zLocCode + z_dir*shift;

    if(x < 0 || y < 0 || z < 0)
        return NULL;
    else if(x >= w || y >= h || z >= d)
        return NULL;

    unsigned int xLocCode = (unsigned int) x; //(x * this->maxVal);
    unsigned int yLocCode = (unsigned int) y; //(y * this->maxVal);
    unsigned int zLocCode = (unsigned int) z; //(z * this->maxVal);

    // branch to appropriate cell
    OTCell *pCell = this->root;
    unsigned int nextLevel = this->rootLevel - 1;

    while(pCell && pCell->level > cell->level){
        unsigned int childBranchBit = 1 << nextLevel;
        unsigned int childIndex = ((xLocCode  & childBranchBit) >> (nextLevel))
                                + (((yLocCode  & childBranchBit) >> (nextLevel)) << 1)
                                + (((zLocCode & childBranchBit) >> nextLevel) << 2);
        --nextLevel;
        pCell = (pCell->children[childIndex]);
    }

    // return desired cell (or NULL)
    return pCell;


}

OTCell* Octree::getCommonAncestor(const OTCell *cell, unsigned int &cellLevel, unsigned int binaryDiff)
{
    OTCell *pCell = const_cast<OTCell *>(cell);
    while((binaryDiff) & (1 << cellLevel)){
        pCell = pCell->parent;
        cellLevel++;
    }
    return pCell;
}

OTCell* Octree::getRoot()
{
    return this->root;
}



list<OTCell*> Octree::collect_children_at_level(OTCell* pCell, unsigned int level) {
    list<OTCell*> kids;
    if (pCell != 0) {
        if (pCell->level > level) {
            for (int i=0; i<8; i++) {
                list<OTCell*> sub_kids = collect_children_at_level(pCell->children[i], level);
                kids.insert(kids.end(), sub_kids.begin(), sub_kids.end());
            }
        } else if (pCell->level == level) {
            kids.insert(kids.end(), pCell);
        }
    }

    return kids;

}

// weak balance condition.
void Octree::balance_tree()
{
    // start at the bottom, move up
    for(unsigned int i=0; i < nLevels; i++)
    {
        list<OTCell*> nodes = collect_children_at_level(this->root, i);
        list<OTCell*>::iterator n_iter = nodes.begin();

        // look at each node at this level
        while (n_iter != nodes.end())
        {
            OTCell *cell = *n_iter;

            // examine each neighbor but corners
            for(int x_dir=-1; x_dir <= 1; x_dir++)
            {
                for(int y_dir=-1; y_dir <= 1; y_dir++)
                {
                    for(int z_dir=-1; z_dir <= 1; z_dir++)
                    {
                        // skip corners
		      if(std::abs(x_dir) + std::abs(y_dir) + std::abs(z_dir) == 3)
                            continue;

                        // skip self
		      if(std::abs(x_dir) + std::abs(y_dir) + std::abs(z_dir) == 0)
                            continue;

                        unsigned int shift = 1 << cell->level;

                        unsigned int cur_x = cell->xLocCode;
                        unsigned int cur_y = cell->yLocCode;
                        unsigned int cur_z = cell->zLocCode;

                        unsigned int nbr_x = cur_x + shift*x_dir;
                        unsigned int nbr_y = cur_y + shift*y_dir;
                        unsigned int nbr_z = cur_z + shift*z_dir;

                        // if outside, move on
                        if(nbr_x < 0 || nbr_y < 0 || nbr_z < 0)
                            continue;
                        else if(nbr_x >= bounding_size || nbr_y >= bounding_size || nbr_z >= bounding_size)
                            continue;

                        // otherwise add neighbor parent
                        addCellAtLevel(nbr_x, nbr_y, nbr_z, cell->level+1);
                    }
                }

            }

            n_iter++;
        }
    }
}

list<OTCell*> Octree::collect_all_background_grid_cells(OTCell* pCell) {
    list<OTCell*> kids;
    if (pCell != 0) {
        int kid_count = 0;
        for (int i=0; i<8; i++) {
            if (pCell->children[i]) {
                kid_count++;

                list<OTCell*> sub_kids = collect_all_background_grid_cells(pCell->children[i]);
                kids.insert(kids.end(), sub_kids.begin(), sub_kids.end());
            }
        }


        if (kid_count == 0 && pCell->level > 0) {
            //i am a non lowest level leaf   Definitely have work to do.
            kids.insert(kids.end(), pCell);
        } else if (kid_count > 0 && kid_count < 8) {
            //i have some children, but not all of them.  i will have to stencil part of myself too
            kids.insert(kids.end(), pCell);
        } else {
            //either kid_count == 8,
            //this means all my kids are here and they will take care of themselves!
            //or kid_count == 0 && pCell->level == 0,
            //this means i am a low level leaf, and hence am a cut cell.  Other code
            //will take care of me
        }

    }

    return kids;
}


bool Octree::has_shared_face_vertex(OTCell *pCell, OTCell *qCell, int which) {
    //there can be a shared face vertex IF
    //pCell is not a leaf
    //qCell is not a leaf and has children sharing the face.



    if (FACE_NEIGHBOR_OFFSETS[which][0] < 0) {
        //check p's left face
        //this means check if any of p's left kids exist or any of q's right kids
        return  pCell->children[CLLF] || pCell->children[CULF] ||
                pCell->children[CLLB] || pCell->children[CULB] ||
                (qCell &&
                 (qCell->children[CLRF] || qCell->children[CURF] ||
                  qCell->children[CLRB] || qCell->children[CURB]));
    } else if (FACE_NEIGHBOR_OFFSETS[which][0] > 0) {
        //check p's right face
        //this means check if any of p's right kids exist or any of q's left kids
        return  pCell->children[CLRF] || pCell->children[CURF] ||
                pCell->children[CLRB] || pCell->children[CURB] ||
                (qCell &&
                 (qCell->children[CLLF] || qCell->children[CULF] ||
                  qCell->children[CLLB] || qCell->children[CULB]));
    } else if (FACE_NEIGHBOR_OFFSETS[which][1] < 0) {
        //check p's bottom face
        //this means check if any of p's bottom kids exist or any of q's top kids
        return  pCell->children[CLLF] || pCell->children[CLRF] ||
                pCell->children[CLLB] || pCell->children[CLRB] ||
                (qCell &&
                 (qCell->children[CULF] || qCell->children[CURF] ||
                  qCell->children[CULB] || qCell->children[CURB]));
    } else if (FACE_NEIGHBOR_OFFSETS[which][1] > 0) {
        //check p's top face
        //this means check if any of p's top kids exist or any of q's bottom kids
        return  pCell->children[CULF] || pCell->children[CURF] ||
                pCell->children[CULB] || pCell->children[CURB] ||
                (qCell &&
                 (qCell->children[CLLF] || qCell->children[CLRF] ||
                  qCell->children[CLLB] || qCell->children[CLRB]));
    } else if (FACE_NEIGHBOR_OFFSETS[which][2] < 0) {
        //check p's front face
        //this means check if any of p's front kids exist or any of q's back kids
        return  pCell->children[CLLF] || pCell->children[CLRF] ||
                pCell->children[CULF] || pCell->children[CURF] ||
                (qCell &&
                 (qCell->children[CLLB] || qCell->children[CLRB] ||
                  qCell->children[CULB] || qCell->children[CURB]));
    } else if (FACE_NEIGHBOR_OFFSETS[which][2] > 0) {
        //check p's back face
        //this means check if any of p's back kids exist or any of q's front kids
        return  pCell->children[CLLB] || pCell->children[CLRB] ||
                pCell->children[CULB] || pCell->children[CURB] ||
                (qCell &&
                 (qCell->children[CLLF] || qCell->children[CLRF] ||
                  qCell->children[CULF] || qCell->children[CURF]));
    }



    //these cells have the same xyz
    return false;

}


bool Octree::has_shared_edge_vertex(OTCell *pCell, OTCell *qCell, OTCell *eCell, OTCell *sCell, int f, int e) {
    //need to check for the pair of children upon each edge

    // LEFT FACE
    if (FACE_NEIGHBOR_OFFSETS[f][0] < 0)
    {
        // LOWER EDGE
        if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] < 0)
        {
            return pCell->children[CLLF] || pCell->children[CLLB] ||
                   (qCell && (qCell->children[CLRF] || qCell->children[CLRB])) ||
                   (eCell && (eCell->children[CULF] || eCell->children[CULB])) ||
                   (sCell && (sCell->children[CURF] || sCell->children[CURB]));
        }
        // UPPER EDGE
        else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] > 0)
        {
            return pCell->children[CULF] || pCell->children[CULB] ||
                   (qCell && (qCell->children[CURF] || qCell->children[CURB])) ||
                   (eCell && (eCell->children[CLLF] || eCell->children[CLLB])) ||
                   (sCell && (sCell->children[CLRF] || sCell->children[CLRB]));
        }
        // FRONT EDGE
        else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] < 0)
        {
            return pCell->children[CLLF] || pCell->children[CULF] ||
                   (qCell && (qCell->children[CLRF] || qCell->children[CURF])) ||
                   (eCell && (eCell->children[CLLB] || eCell->children[CULB])) ||
                   (sCell && (sCell->children[CLRB] || sCell->children[CURB]));
        }
        // BACK EDGE
        else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] > 0)
        {
            return pCell->children[CLLB] || pCell->children[CULB] ||
                   (qCell && (qCell->children[CLRB] || qCell->children[CURB])) ||
                   (eCell && (eCell->children[CLLF] || eCell->children[CULF])) ||
                   (sCell && (sCell->children[CLRF] || sCell->children[CURF]));
        }
    }
    // RIGHT FACE
    else if (FACE_NEIGHBOR_OFFSETS[f][0] > 0)
    {
        if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] < 0)
        {
            //check right, lower edge
            return pCell->children[CLRF] || pCell->children[CLRB] ||
                   (qCell && (qCell->children[CLLF] || qCell->children[CLLB])) ||
                   (eCell && (eCell->children[CURF] || eCell->children[CURB])) ||
                   (sCell && (sCell->children[CULF] || sCell->children[CULB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] > 0) {
            //check right, upper edge
            return pCell->children[CURF] || pCell->children[CURB] ||
                   (qCell && (qCell->children[CULF] || qCell->children[CULB])) ||
                   (eCell && (eCell->children[CLRF] || eCell->children[CLRB])) ||
                   (sCell && (sCell->children[CLLF] || sCell->children[CLLB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] < 0) {
            //check right, front edge
            return pCell->children[CLRF] || pCell->children[CURF] ||
                   (qCell && (qCell->children[CLLF] || qCell->children[CULF])) ||
                   (eCell && (eCell->children[CLRB] || eCell->children[CURB])) ||
                   (sCell && (sCell->children[CLLB] || sCell->children[CULB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] > 0) {
            //check right, back edge
            return pCell->children[CLRB] || pCell->children[CURB] ||
                   (qCell && (qCell->children[CLLB] || qCell->children[CULB])) ||
                   (eCell && (eCell->children[CLRF] || eCell->children[CURF])) ||
                   (sCell && (sCell->children[CLLF] || sCell->children[CULF]));
        }
    }
    // BOTTOM FACE
    else if (FACE_NEIGHBOR_OFFSETS[f][1] < 0) {
        if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] < 0) {
            //check lower, left edge
            return pCell->children[CLLF] || pCell->children[CLLB] ||
                   (qCell && (qCell->children[CULF] || qCell->children[CULB])) ||
                   (eCell && (eCell->children[CLRF] || eCell->children[CLRB])) ||
                   (sCell && (sCell->children[CURF] || sCell->children[CURB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] > 0) {
            //check lower, right edge
            return pCell->children[CLRF] || pCell->children[CLRB] ||
                   (qCell && (qCell->children[CURF] || qCell->children[CURB])) ||
                   (eCell && (eCell->children[CLLF] || eCell->children[CLLB])) ||
                   (sCell && (sCell->children[CULF] || sCell->children[CULB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] < 0) {
            //check lower, front edge
            return pCell->children[CLLF] || pCell->children[CLRF] ||
                   (qCell && (qCell->children[CULF] || qCell->children[CURF])) ||
                   (eCell && (eCell->children[CLLB] || eCell->children[CLRB])) ||
                   (sCell && (sCell->children[CULB] || sCell->children[CURB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] > 0) {
            //check lower, back edge
            return pCell->children[CLLB] || pCell->children[CLRB] ||
                   (qCell && (qCell->children[CULB] || qCell->children[CURB])) ||
                   (eCell && (eCell->children[CLLF] || eCell->children[CLRF])) ||
                   (sCell && (sCell->children[CULF] || sCell->children[CURF]));
        }
    }
    // TOP FACE
    else if (FACE_NEIGHBOR_OFFSETS[f][1] > 0) {
        if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] < 0) {
            //check upper, left edge
            return pCell->children[CULF] || pCell->children[CULB] ||
                   (qCell && (qCell->children[CLLF] || qCell->children[CLLB])) ||
                   (eCell && (eCell->children[CURF] || eCell->children[CURB])) ||
                   (sCell && (sCell->children[CLRF] || sCell->children[CLRB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] > 0) {
            //check upper, right edge
            return pCell->children[CURF] || pCell->children[CURB] ||
                   (qCell && (qCell->children[CLRF] || qCell->children[CLRB])) ||
                   (eCell && (eCell->children[CULF] || eCell->children[CULB])) ||
                   (sCell && (sCell->children[CLLF] || sCell->children[CLLB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] < 0) {
            //check upper, front edge
            return pCell->children[CULF] || pCell->children[CURF] ||
                   (qCell && (qCell->children[CLLF] || qCell->children[CLRF])) ||
                   (eCell && (eCell->children[CULB] || eCell->children[CURB])) ||
                   (sCell && (sCell->children[CLLB] || sCell->children[CLRB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2] > 0) {
            //check upper, back edge
            return pCell->children[CULB] || pCell->children[CURB] ||
                   (qCell && (qCell->children[CLLB] || qCell->children[CLRB])) ||
                   (eCell && (eCell->children[CULF] || eCell->children[CURF])) ||
                   (sCell && (sCell->children[CLLF] || sCell->children[CLRF]));
        }
    }
    // FRONT FACE
    else if (FACE_NEIGHBOR_OFFSETS[f][2] < 0) {
        if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] < 0) {
            //check front, left edge
            return pCell->children[CLLF] || pCell->children[CULF] ||
                   (qCell && (qCell->children[CLLB] || qCell->children[CULB])) ||
                   (eCell && (eCell->children[CLRF] || eCell->children[CURF])) ||
                   (sCell && (sCell->children[CLRB] || sCell->children[CURB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] > 0) {
            //check front, right edge
            return pCell->children[CLRF] || pCell->children[CURF] ||
                   (qCell && (qCell->children[CLRB] || qCell->children[CURB])) ||
                   (eCell && (eCell->children[CLLF] || eCell->children[CULF])) ||
                   (sCell && (sCell->children[CLLB] || sCell->children[CULB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] < 0) {
            //check front, lower edge
            return pCell->children[CLLF] || pCell->children[CLRF] ||
                   (qCell && (qCell->children[CLLB] || qCell->children[CLRB])) ||
                   (eCell && (eCell->children[CULF] || eCell->children[CURF])) ||
                   (sCell && (sCell->children[CULB] || sCell->children[CURB]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] > 0) {
            //check front, upper edge
            return pCell->children[CULF] || pCell->children[CURF] ||
                   (qCell && (qCell->children[CULB] || qCell->children[CURB])) ||
                   (eCell && (eCell->children[CLLF] || eCell->children[CLRF])) ||
                   (sCell && (sCell->children[CLLB] || sCell->children[CLRB]));
        }
    }
    // BACK FACE
    else if (FACE_NEIGHBOR_OFFSETS[f][2] > 0) {
        if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] < 0) {
            //check back, left edge
            return pCell->children[CLLB] || pCell->children[CULB] ||
                   (qCell && (qCell->children[CLLF] || qCell->children[CULF])) ||
                   (eCell && (eCell->children[CLRB] || eCell->children[CURB])) ||
                   (sCell && (sCell->children[CLRF] || sCell->children[CURF]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0] > 0) {
            //check back, right edge
            return pCell->children[CLRB] || pCell->children[CURB] ||
                   (qCell && (qCell->children[CLRF] || qCell->children[CURF])) ||
                   (eCell && (eCell->children[CLLB] || eCell->children[CULB])) ||
                   (sCell && (sCell->children[CLLF] || sCell->children[CULF]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] < 0) {
            //check back, lower edge
            return pCell->children[CLLB] || pCell->children[CLRB] ||
                   (qCell && (qCell->children[CLLF] || qCell->children[CLRF])) ||
                   (eCell && (eCell->children[CULB] || eCell->children[CURB])) ||
                   (sCell && (sCell->children[CULF] || sCell->children[CURF]));
        } else if (EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1] > 0) {
            //check back, upper edge
            return pCell->children[CULB] || pCell->children[CURB] ||
                   (qCell && (qCell->children[CULF] || qCell->children[CURF])) ||
                   (eCell && (eCell->children[CLLB] || eCell->children[CLRB])) ||
                   (sCell && (sCell->children[CLLF] || sCell->children[CLRF]));
        }
    }

    return false;
}



void Octree::create_background_grid(const std::vector<OTCell *> &buffer_cells) {

    // first store list of lattice vertices, so we don't recompute them
    // Can add them when they're created for increased efficiency
    vertex_tracker.clear();
    for(unsigned int i=0; i < buffer_cells.size(); i++)
    {
        OTCell *cell = buffer_cells[i];

        if(cell->vert)
        {
            for(int v=0; v < VERTS_PER_CELL; v++)
            {
                Vertex3D *vertex = cell->vert[v];
                if(vertex)
                    vertex_tracker[vertex->pos()] = vertex;
            }
        }
    }

    //nodes is the list of things that need be tetrahedralized.
    list<OTCell*> nodes = this->collect_all_background_grid_cells(this->root);
    list<OTCell*>::iterator n_iter = nodes.begin();
    while (n_iter != nodes.end())
    {
        OTCell *cell = *n_iter;

        if(cell->vert)
        {
            for(int v=0; v < VERTS_PER_CELL; v++)
            {
                Vertex3D *vertex = cell->vert[v];
                vertex_tracker[vertex->pos()] = vertex;
            }
        }

        n_iter++;
    }


    //check if the neighbors of each node exist
    n_iter = nodes.begin();
    while (n_iter != nodes.end()) {

        OTCell* pCell = *n_iter;
        //i am a leaf, check my neighbors to link up to them using shewchuk rules

        // skip real leaves
        if(pCell->level == 0)
        {
            pCell->bg_pass_made = 1;
            n_iter++;
            continue;
        }

        // skip cells that are not in bounding box of data
        if(g_bMatchDataDimensionsExactly)
        {
            unsigned int shift = 1 << pCell->level;
            if(pCell->xLocCode > this->w || pCell->yLocCode > this->h || pCell->zLocCode > this->d ||
                    pCell->xLocCode+shift > this->w || pCell->yLocCode+shift > this->h || pCell->zLocCode+shift > this->d)
            {
                pCell->bg_pass_made = 1;
                n_iter++;
                continue;
            }
        }

        //collect my face neighbors, fn[], and my edge neighbors, en[], and shared neighbors sn[]
        OTCell* fn[6] = {0};
        OTCell* en[6][4] = {{0}};
        OTCell* sn[6][4] = {{0}};
        for (int i=0; i<6; i++) {
            int off_x = FACE_NEIGHBOR_OFFSETS[i][0];
            int off_y = FACE_NEIGHBOR_OFFSETS[i][1];
            int off_z = FACE_NEIGHBOR_OFFSETS[i][2];

            OTCell* qCell = this->getNeighborAtMyLevel(pCell, off_x, off_y, off_z);
            fn[i] = qCell;

            for (int j=0; j<4; j++) {
                int off_x2 = EDGE_FACE_NEIGHBOR_OFFSETS[i][j][0];
                int off_y2 = EDGE_FACE_NEIGHBOR_OFFSETS[i][j][1];
                int off_z2 = EDGE_FACE_NEIGHBOR_OFFSETS[i][j][2];

                OTCell* eCell = this->getNeighborAtMyLevel(pCell, off_x2, off_y2, off_z2);
                en[i][j] = eCell;

                int off_x3 = off_x + off_x2;
                int off_y3 = off_y + off_y2;
                int off_z3 = off_z + off_z2;

                OTCell* sCell = this->getNeighborAtMyLevel(pCell, off_x3, off_y3, off_z3);
                sn[i][j] = sCell;
            }
        }

        int cell_width = pow2((int)pCell->level) + 0.5f;
        int half_cell_width = cell_width/2;

        //compute the center of pCell
        vec3 v_c;
        v_c.x = pCell->xLocCode + half_cell_width;
        v_c.y = pCell->yLocCode + half_cell_width;
        v_c.z = pCell->zLocCode + half_cell_width;


        //check each of my faces and see if my neighbor has a split
        for (int f=0; f<6; f++) {
            int off_x = FACE_NEIGHBOR_OFFSETS[f][0];
            int off_y = FACE_NEIGHBOR_OFFSETS[f][1];
            int off_z = FACE_NEIGHBOR_OFFSETS[f][2];
            OTCell* qCell = fn[f];

            if (!has_shared_face_vertex(pCell, qCell, f)) {
                //if qCell != null, then the octant is the same size as pCell
                if (qCell && (qCell->level == pCell->level)) {
                    //compute the center of qCell
                    vec3 v_c2;
                    v_c2.x = v_c.x + off_x*cell_width;
                    v_c2.y = v_c.y + off_y*cell_width;
                    v_c2.z = v_c.z + off_z*cell_width;

                    //compute the location of shared vertex d centered in s
                    vec3 v_d;
                    v_d.x = (v_c.x + v_c2.x) / 2.0;
                    v_d.y = (v_c.y + v_c2.y) / 2.0;
                    v_d.z = (v_c.z + v_c2.z) / 2.0;

                    //we can safely skip checking qCell for these kinds of tets
                    //since they span two tetrahedra of equal size
                    //if qCell has already done the work!
                    if (!qCell->bg_pass_made) {

                        //check each edges of s for midpoints, create bcc tet or bisected bcc
                        for (int e=0; e<4; e++) {
                            OTCell* eCell = en[f][e];
                            OTCell* sCell = sn[f][e];

                            int off_x2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0];
                            int off_y2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1];
                            int off_z2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2];

                            vec3 v_m;                            
                            v_m.x = v_d.x + off_x2*half_cell_width;
                            v_m.y = v_d.y + off_y2*half_cell_width;
                            v_m.z = v_d.z + off_z2*half_cell_width;

                            int off_x3 = off_x + off_x2;
                            int off_y3 = off_y + off_y2;
                            int off_z3 = off_z + off_z2;

                            //figure out endpoints of edge

                            vec3 v_e0, v_e1;
                            v_e0.x = v_e1.x = v_m.x;
                            v_e0.y = v_e1.y = v_m.y;
                            v_e0.z = v_e1.z = v_m.z;

                            if (off_x3 && off_y3) {
                                //edge lies in z direction
                                v_e0.z += half_cell_width; v_e1.z -= half_cell_width;
                            } else if (off_x3 && off_z3) {
                                //edge lies in y direction
                                v_e0.y += half_cell_width; v_e1.y -= half_cell_width;
                            } else if (off_y3 && off_z3) {
                                //edge lies in x direction
                                v_e0.x += half_cell_width; v_e1.x -= half_cell_width;
                            }


                            //---------------------------------------------------//
                            //                   JRB Version                     //
                            //---------------------------------------------------//
                            if (has_shared_edge_vertex(pCell, qCell, eCell, sCell, f, e))
                            {
                                //create bisected BCC tets eX, m, c, c2
                                Vertex3D *vert_c  = vertexForPosition(v_c);
                                Vertex3D *vert_c2 = vertexForPosition(v_c2);
                                Vertex3D *vert_m  = vertexForPosition(v_m);
                                Vertex3D *vert_e0 = vertexForPosition(v_e0);
                                Vertex3D *vert_e1 = vertexForPosition(v_e1);


                                if(bgBiParity[f][e])
                                {
                                    createTet(vert_e0, vert_m, vert_c2, vert_c);
                                    createTet(vert_e1, vert_m, vert_c, vert_c2);
                                }
                                else
                                {
                                    createTet(vert_e0, vert_m, vert_c, vert_c2);
                                    createTet(vert_e1, vert_m, vert_c2, vert_c);
                                }
                            }
                            else
                            {
                                //create BCC tet c, c2, e0, e1
                                Vertex3D *vert_c  = vertexForPosition(v_c);
                                Vertex3D *vert_c2 = vertexForPosition(v_c2);
                                Vertex3D *vert_e0 = vertexForPosition(v_e0);
                                Vertex3D *vert_e1 = vertexForPosition(v_e1);

                                if(bgParity[f][e])
                                    createTet(vert_c, vert_c2, vert_e0, vert_e1);
                                else
                                    createTet(vert_c, vert_c2, vert_e1, vert_e0);

                            }
                        }
                    }

                } else {
                    //the face s between pCell and qCell doesn't exist
                    //either qCell is bigger or pCell is boundary.  In either case create two half pyramids
                    //the diagonal must adjoin a corner or the center of pCell's parent.

                    vec3 v_p0, v_p1, v_p2, v_p3;

                    v_p0.x = v_p1.x = v_p2.x = v_p3.x = v_c.x;
                    v_p0.y = v_p1.y = v_p2.y = v_p3.y = v_c.y;
                    v_p0.z = v_p1.z = v_p2.z = v_p3.z = v_c.z;

                    if (off_x) {
                        //add y and z offsets to get the corners
                        v_p0.x = v_p1.x = v_p2.x = v_p3.x = v_c.x + off_x*half_cell_width;

                        // choose order based on which of the parent cell's octants we're in
                        int branchBit = 1 << pCell->level;
                        if(((pCell->yLocCode & branchBit) >> (pCell->level)) ^ ((pCell->zLocCode & branchBit) >> (pCell->level)))
                        {
                            v_p0.y += half_cell_width; v_p0.z -= half_cell_width;
                            v_p1.y -= half_cell_width; v_p1.z -= half_cell_width;
                            v_p2.y -= half_cell_width; v_p2.z += half_cell_width;
                            v_p3.y += half_cell_width; v_p3.z += half_cell_width;
                        }
                        else
                        {
                            v_p0.y += half_cell_width; v_p0.z += half_cell_width;
                            v_p1.y += half_cell_width; v_p1.z -= half_cell_width;
                            v_p2.y -= half_cell_width; v_p2.z -= half_cell_width;
                            v_p3.y -= half_cell_width; v_p3.z += half_cell_width;
                        }
                    } else if (off_y) {
                        v_p0.y = v_p1.y = v_p2.y = v_p3.y = v_c.y + off_y*half_cell_width;

                        // choose order based on which of the parent cell's octants we're in
                        int branchBit = 1 << pCell->level;
                        if(((pCell->xLocCode & branchBit) >> (pCell->level)) ^ ((pCell->zLocCode & branchBit) >> (pCell->level)))
                        {
                            v_p0.x += half_cell_width; v_p0.z -= half_cell_width;
                            v_p1.x -= half_cell_width; v_p1.z -= half_cell_width;
                            v_p2.x -= half_cell_width; v_p2.z += half_cell_width;
                            v_p3.x += half_cell_width; v_p3.z += half_cell_width;
                        }
                        else
                        {
                            v_p0.x += half_cell_width; v_p0.z += half_cell_width;
                            v_p1.x += half_cell_width; v_p1.z -= half_cell_width;
                            v_p2.x -= half_cell_width; v_p2.z -= half_cell_width;
                            v_p3.x -= half_cell_width; v_p3.z += half_cell_width;
                        }
                    } else if (off_z) {
                        v_p0.z = v_p1.z = v_p2.z = v_p3.z = v_c.z + off_z*half_cell_width;

                        // choose order based on which of the parent cell's octants we're in
                        int branchBit = 1 << pCell->level;
                        if(((pCell->xLocCode & branchBit) >> (pCell->level)) ^ ((pCell->yLocCode & branchBit) >> (pCell->level)))
                        {
                            v_p0.x += half_cell_width; v_p0.y -= half_cell_width;
                            v_p1.x -= half_cell_width; v_p1.y -= half_cell_width;
                            v_p2.x -= half_cell_width; v_p2.y += half_cell_width;
                            v_p3.x += half_cell_width; v_p3.y += half_cell_width;
                        }
                        else
                        {
                            v_p0.x += half_cell_width; v_p0.y += half_cell_width;
                            v_p1.x += half_cell_width; v_p1.y -= half_cell_width;
                            v_p2.x -= half_cell_width; v_p2.y -= half_cell_width;
                            v_p3.x -= half_cell_width; v_p3.y += half_cell_width;
                        }
                    }

                    //---------------------------------------------------//
                    //                   JRB Version                     //
                    //---------------------------------------------------//
                    //check each edges of s for midpoints
                    bool split = false;
                    for (int e=0; e<4; e++) {
                        OTCell* eCell = en[f][e];
                        OTCell* sCell = sn[f][e];

                        split = split || (has_shared_edge_vertex(pCell, qCell, eCell, sCell, f, e));
                    }

                    if(!split)
                    {
                        Vertex3D *vert_c  = vertexForPosition(v_c);
                        Vertex3D *vert_p0 = vertexForPosition(v_p0);
                        Vertex3D *vert_p1 = vertexForPosition(v_p1);
                        Vertex3D *vert_p2 = vertexForPosition(v_p2);
                        Vertex3D *vert_p3 = vertexForPosition(v_p3);

                        // todo - change math above so this logic can be turned into
                        // if(off_x < 0 || off_y < 0 || off_z < 0)
                        if(off_x < 0 || off_z < 0){
                            createTet(vert_c, vert_p0, vert_p1, vert_p2);
                            createTet(vert_c, vert_p2, vert_p3, vert_p0);
                        }
                        else if(off_x > 0 || off_z > 0){
                            createTet(vert_c, vert_p0, vert_p2, vert_p1);
                            createTet(vert_c, vert_p2, vert_p0, vert_p3);
                        }
                        else if(off_y < 0){
                            createTet(vert_c, vert_p0, vert_p2, vert_p1);
                            createTet(vert_c, vert_p2, vert_p0, vert_p3);
                        }
                        else if(off_y > 0){
                            createTet(vert_c, vert_p0, vert_p1, vert_p2);
                            createTet(vert_c, vert_p2, vert_p3, vert_p0);
                        }


                    }
                    else{

                        //compute the location of vertex d centered in s
                        vec3 v_d;
                        v_d.x = v_c.x + off_x*half_cell_width;
                        v_d.y = v_c.y + off_y*half_cell_width;
                        v_d.z = v_c.z + off_z*half_cell_width;

                        // for each edge
                        for (int e=0; e<4; e++) {
                            OTCell* eCell = en[f][e];
                            OTCell* sCell = sn[f][e];

                            int off_x2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0];
                            int off_y2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1];
                            int off_z2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2];

                            vec3 v_m;
                            v_m.x = v_d.x + off_x2*half_cell_width;
                            v_m.y = v_d.y + off_y2*half_cell_width;
                            v_m.z = v_d.z + off_z2*half_cell_width;

                            int off_x3 = off_x + off_x2;
                            int off_y3 = off_y + off_y2;
                            int off_z3 = off_z + off_z2;

                            //figure out endpoints of edge

                            vec3 v_e0, v_e1;
                            v_e0.x = v_e1.x = v_m.x;
                            v_e0.y = v_e1.y = v_m.y;
                            v_e0.z = v_e1.z = v_m.z;

                            int childIndex0 = 0;
                            int childIndex1 = 0;

                            if (off_x3 && off_y3) {
                                //edge lies in z direction
                                v_e0.z += half_cell_width; v_e1.z -= half_cell_width;

                                //set the indices for checking if pCell has a child here
                                if (off_x3 > 0)
                                    childIndex0 = childIndex0 | 1;
                                if (off_y3 > 0)
                                    childIndex0 = childIndex0 | 2;

                                childIndex1 = childIndex0;
                                childIndex0 = childIndex0 | 4;
                            } else if (off_x3 && off_z3) {
                                //edge lies in y direction
                                v_e0.y += half_cell_width; v_e1.y -= half_cell_width;

                                //set the indices for checking if pCell has a child here
                                if (off_x3 > 0)
                                    childIndex0 = childIndex0 | 1;
                                if (off_z3 > 0)
                                    childIndex0 = childIndex0 | 4;

                                childIndex1 = childIndex0;
                                childIndex0 = childIndex0 | 2;
                            } else if (off_y3 && off_z3) {
                                //edge lies in x direction
                                v_e0.x += half_cell_width; v_e1.x -= half_cell_width;

                                //set the indices for checking if pCell has a child here
                                if (off_y3 > 0)
                                    childIndex0 = childIndex0 | 2;
                                if (off_z3 > 0)
                                    childIndex0 = childIndex0 | 4;

                                childIndex1 = childIndex0;
                                childIndex0 = childIndex0 | 1;
                            }

                            // if edge has split, put quadrisected
                            if(has_shared_edge_vertex(pCell, qCell, eCell, sCell, f, e))
                            {
                                Vertex3D *vert_c  = vertexForPosition(v_c);
                                Vertex3D *vert_d  = vertexForPosition(v_d);
                                Vertex3D *vert_m  = vertexForPosition(v_m);
                                Vertex3D *vert_e0 = vertexForPosition(v_e0);
                                Vertex3D *vert_e1 = vertexForPosition(v_e1);

                                //first check v_e0
                                if (!pCell->children[childIndex0])
                                {
                                    //there is no child here, create quadrisected!
                                    if(bgParity[f][e])
                                        createTet(vert_e0, vert_m, vert_c, vert_d);
                                    else
                                        createTet(vert_e0, vert_m, vert_d, vert_c);

                                }
                                //next check v_e1
                                if (!pCell->children[childIndex1])
                                {
                                    //there is no child here, create quadrisected!
                                    if(bgParity[f][e])
                                        createTet(vert_e1, vert_m, vert_d, vert_c);
                                    else
                                        createTet(vert_e1, vert_m, vert_c, vert_d);
                                }
                            }
                            // else create bisected BCC c, d, e0, e1
                            else
                            {
                                Vertex3D *vert_c  = vertexForPosition(v_c);
                                Vertex3D *vert_d  = vertexForPosition(v_d);
                                Vertex3D *vert_e0 = vertexForPosition(v_e0);
                                Vertex3D *vert_e1 = vertexForPosition(v_e1);


                                if(bgParity[f][e])
                                    createTet(vert_e0, vert_e1, vert_c, vert_d);
                                else
                                    createTet(vert_e0, vert_e1, vert_d, vert_c);

                            }
                        }
                    }
                }

            }
            else {
                //compute the center of qCell purely to make v_d
                vec3 v_c2;
                v_c2.x = v_c.x + off_x*cell_width;
                v_c2.y = v_c.y + off_y*cell_width;
                v_c2.z = v_c.z + off_z*cell_width;

                //compute the location of shared vertex d centered in s
                vec3 v_d;
                v_d.x = (v_c.x + v_c2.x) / 2.0;
                v_d.y = (v_c.y + v_c2.y) / 2.0;
                v_d.z = (v_c.z + v_c2.z) / 2.0;

                // loop over edges
                for (int e=0; e<4; e++) {
                    OTCell* eCell = en[f][e];
                    OTCell* sCell = sn[f][e];

                    int off_x2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][0];
                    int off_y2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][1];
                    int off_z2 = EDGE_FACE_NEIGHBOR_OFFSETS[f][e][2];

                    vec3 v_m;
                    v_m.x = v_d.x + off_x2*half_cell_width;
                    v_m.y = v_d.y + off_y2*half_cell_width;
                    v_m.z = v_d.z + off_z2*half_cell_width;

                    int off_x3 = off_x + off_x2;
                    int off_y3 = off_y + off_y2;
                    int off_z3 = off_z + off_z2;

                    //figure out endpoints of edge

                    vec3 v_e0, v_e1;
                    v_e0.x = v_e1.x = v_m.x;
                    v_e0.y = v_e1.y = v_m.y;
                    v_e0.z = v_e1.z = v_m.z;

                    int childIndex0 = 0;
                    int childIndex1 = 0;

                    if (off_x3 && off_y3) {
                        //edge lies in z direction
                        v_e0.z += half_cell_width; v_e1.z -= half_cell_width;

                        //set the indices for checking if pCell has a child here
                        if (off_x3 > 0)
                            childIndex0 = childIndex0 | 1;
                        if (off_y3 > 0)
                            childIndex0 = childIndex0 | 2;

                        childIndex1 = childIndex0;
                        childIndex0 = childIndex0 | 4;
                    } else if (off_x3 && off_z3) {
                        //edge lies in y direction
                        v_e0.y += half_cell_width; v_e1.y -= half_cell_width;

                        //set the indices for checking if pCell has a child here
                        if (off_x3 > 0)
                            childIndex0 = childIndex0 | 1;
                        if (off_z3 > 0)
                            childIndex0 = childIndex0 | 4;

                        childIndex1 = childIndex0;
                        childIndex0 = childIndex0 | 2;
                    } else if (off_y3 && off_z3) {
                        //edge lies in x direction
                        v_e0.x += half_cell_width; v_e1.x -= half_cell_width;

                        //set the indices for checking if pCell has a child here
                        if (off_y3 > 0)
                            childIndex0 = childIndex0 | 2;
                        if (off_z3 > 0)
                            childIndex0 = childIndex0 | 4;

                        childIndex1 = childIndex0;
                        childIndex0 = childIndex0 | 1;
                    }

                    //if one of these doesn't exist there is no possibility
                    //that there are midpoints on the edges of s, by the balancing condition

                    //---------------------------------------------------//
                    //                   JRB Version                     //
                    //---------------------------------------------------//
                    if (has_shared_edge_vertex(pCell, qCell, eCell, sCell, f, e))
                    {
                        Vertex3D *vert_c  = vertexForPosition(v_c);
                        Vertex3D *vert_d  = vertexForPosition(v_d);
                        Vertex3D *vert_m  = vertexForPosition(v_m);
                        Vertex3D *vert_e0 = vertexForPosition(v_e0);
                        Vertex3D *vert_e1 = vertexForPosition(v_e1);

                        //first check v_e0
                        if (!pCell->children[childIndex0])
                        {
                            //there is no child here, create quadrisected!
                            if(bgQuadParity[f][e])
                                createTet(vert_e0, vert_m, vert_c, vert_d);
                            else
                                createTet(vert_e0, vert_m, vert_d, vert_c);
                        }
                        //next check v_e1
                        if (!pCell->children[childIndex1])
                        {
                            //there is no child here, create quadrisected!
                            if(bgQuadParity[f][e])
                                createTet(vert_e1, vert_m, vert_d, vert_c);
                            else
                                createTet(vert_e1, vert_m, vert_c, vert_d);
                        }
                    }
                    else
                    {
                        //create bisected BCC c, d, e0, e1
                        Vertex3D *vert_c  = vertexForPosition(v_c);
                        Vertex3D *vert_d  = vertexForPosition(v_d);
                        Vertex3D *vert_e0 = vertexForPosition(v_e0);
                        Vertex3D *vert_e1 = vertexForPosition(v_e1);

                        if(bgBiParity[f][e])
                            createTet(vert_e0, vert_e1, vert_d, vert_c);
                        else
                            createTet(vert_e0, vert_e1, vert_c, vert_d);
                    }

                }
            }
        }

        pCell->bg_pass_made = 1;


        n_iter++;
    }

    vertex_tracker.clear();   
}


void Octree::label_background_tets(unsigned char *labels)
{
    for (unsigned int i=0; i< tets.size(); i++)
    {
        // if flag value, we need to label it
        if (tets[i]->mat_label == -1) {

            //find integer coordinates and fill in the label;
            int which_vert = -1;
            double x = 0;
            double y = 0;
            double z = 0;
            //this magic number check is ok since background tets either have integer or 1/2 coordinates
            for (int j=0; j<4; j++) {

                if (modf(tets[i]->verts[j]->pos().x, &x) < 1e-5 &&
                    modf(tets[i]->verts[j]->pos().y, &y) < 1e-5 &&
                    modf(tets[i]->verts[j]->pos().z, &z) < 1e-5) {
                    if (x < (w+1) && y < (h+1) && z < (d+1)) {
                        which_vert = j;
                        j = 4;
                    }
                }
            }

            if (which_vert >= 0) {
                // JAL: Octree needs a different label call
                // because of the mismatch in w,h,d between
                // Octree and BCCLattice3D
                int label = LBL3DOCTREE(int(x), int(y), int(z));
                tets[i]->mat_label = label;
            } else {
                //scream bloody murder
            }
        }
    }
}

std::string OTCell::str()
{
    stringstream ss;
    ss << "[" << this->xLocCode << ", " << this->yLocCode << ", " << this->zLocCode << "]";
    return ss.str();
}


//-----------------------------------------------------------------------------------
// - createTet()
//
//  Since creating an output Tet for the mesh always involves the same procedure,
// it is helpful to have a function dedicated to this task. The calling code will
// pass in the 4 vertices making up the output Tet, add them to the global lists
// if necessary, and copy adjacency information appropriately.
//-----------------------------------------------------------------------------------
Tet* Octree::createTet(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, Vertex3D *v4, int material)
{
    //----------------------------
    //  Create Tet + Add to List
    //----------------------------
    Tet *tet = new Tet(v1, v2, v3, v4, material);
    tet->tm_index = tets.size();
    tets.push_back(tet);

    //------------------------
    //   Add Verts To List
    //------------------------
    if(v1->tm_v_index < 0){
        v1->tm_v_index = verts.size();
        verts.push_back(v1);
    }
    if(v2->tm_v_index < 0){
        v2->tm_v_index = verts.size();
        verts.push_back(v2);
    }
    if(v3->tm_v_index < 0){
        v3->tm_v_index = verts.size();
        verts.push_back(v3);
    }
    if(v4->tm_v_index < 0){
        v4->tm_v_index = verts.size();
        verts.push_back(v4);
    }

    return tet;
}

//-----------------------------------------------------------------------------------
// - vertexForPosition()
//
//  This method takes the given coordinate and looks up a MAP to find background
//  cell vertex that has already been created for this position. IF no such vertex
//  is found, a new one is created, added to the map, and returned.
//-----------------------------------------------------------------------------------
Vertex3D* Octree::vertexForPosition(const vec3 &position)
{
    Vertex3D *vertex;
    map<vec3, Vertex3D*>::iterator res = vertex_tracker.find(position);

    // create new one if necessary
    if (res == vertex_tracker.end())
    {
        vertex = new Vertex3D();
        vertex->pos() = position;        
        vertex_tracker[position] = vertex;
    }
    // or return existing one
    else
    {        
        vertex = res->second;
    }

    return vertex;
}
