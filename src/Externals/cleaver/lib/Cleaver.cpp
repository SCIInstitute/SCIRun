//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Mesher Class
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

#include "Cleaver.h"
#include "TetMesh.h"
#include "BCCLattice3DMesher.h"
#include "BCCLattice3D.h"
#include "ScalarField.h"
#include "Volume.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

using namespace std;

namespace Cleaver
{
    const std::string VersionNumber = "1.5.4";
    const std::string VersionDate = "Dec 20, 2013";
    const std::string Version = std::string("Version") + " " + VersionNumber + " " + VersionDate;

    const float DefaultAlphaShort = 0.357f;
    const float DefaultAlphaLong = 0.203f;

class CleaverMesherImp
{
public:
    BCCLattice3D *m_lattice;
    TetMesh *m_mesh;
};

static void interpolate_cell(OTCell *cell, BCCLattice3D *lattice);
static OTCell* addCell(BCCLattice3D *lattice, int i, int j, int k);
static void create_dual_vertex(OTCell *cell, BCCLattice3D *lattice);
static BCCLattice3D* constructLatticeFromVolume(const AbstractVolume *volume);

CleaverMesher::~CleaverMesher()
{
    cleanup();
    delete m_pimpl;
}

CleaverMesher::CleaverMesher(const AbstractVolume *volume) : m_pimpl(new CleaverMesherImp)
{
    m_pimpl->m_lattice = constructLatticeFromVolume(volume);    
    m_pimpl->m_mesh = NULL;
}

void CleaverMesher::createTetMesh(bool verbose)
{
    BCCLattice3DMesher mesher(m_pimpl->m_lattice);//, alpha_short, alpha_long);
    m_pimpl->m_mesh = mesher.mesh(true, verbose);
    if(!m_pimpl->m_mesh)
        cerr << "Failed to produce an output mesh." << endl;
}

TetMesh* CleaverMesher::getTetMesh() const
{
    return m_pimpl->m_mesh;
}

void CleaverMesher::cleanup()
{
    if(m_pimpl->m_lattice)
        delete m_pimpl->m_lattice;
    m_pimpl->m_lattice = NULL;
}

void CleaverMesher::setVolume(const AbstractVolume *volume)
{
    cleanup();
    m_pimpl->m_lattice = constructLatticeFromVolume(volume);    
}

const AbstractVolume* CleaverMesher::getVolume() const
{
    return m_pimpl->m_lattice->volume;
}

TetMesh* createMeshFromVolume(const AbstractVolume *volume, bool verbose)
{
    CleaverMesher mesher(volume);

    mesher.createTetMesh(verbose);

    return mesher.getTetMesh();
}

//==========================================================================================================
// constructLatticeFromArray()
//
// - This method takes in a vector of ScalarFields, and uses them to seed the BCC Lattice.
//==========================================================================================================
BCCLattice3D* constructLatticeFromVolume(const AbstractVolume *volume)
{
    //-----------------------------------------------------
    // Verify At Least 2 Fields Are Given As Input
    //-----------------------------------------------------
    if(volume->numberOfMaterials() < 2){
        std::cerr << "At least 2 indicator functions required to run Cleaving algorithm" << std::endl;
        return NULL;
    }

    //-----------------------------------------------------
    //           Create Storage For Lattice
    //-----------------------------------------------------
    BCCLattice3D *lattice = new BCCLattice3D(volume);


    //-----------------------------------------------------
    //            Find Dominant Materials
    //-----------------------------------------------------
    int w = lattice->width();
    int h = lattice->height();
    int d = lattice->depth();
    int m = lattice->materials();

    for(int k=0; k < d; k++){
        for(int j=0; j < h; j++){
            for(int i=0; i < w; i++){

                unsigned char dom = 0;
                float max = volume->valueAt(i,j,k,dom);

                for(int mat=1; mat < m; mat++){

                    float val = volume->valueAt(i,j,k,mat);
                    if(val > max){
                        max = val;
                        dom = mat;                        
                    }
                }

                lattice->LBL3D(i,j,k) = dom;
            }
        }
    }



    //------------------------------------------------------------
    //  Loop over each cell in the grid
    //    If it has at least two different material vertices
    //    it needs to be stored
    //------------------------------------------------------------
    for(int k=0; k < d-1; k++){
        for(int j=0; j < h-1; j++){
            for(int i=0; i < w-1; i++){

                //--------------------------------------
                // Obtain Max Material At Each Vertex
                //--------------------------------------
                // First Check Previously Touched Neighbor
                // Cells for Vertices. This way we can just
                // set pointers to those.
                // Must be careful not to delete previous vertices...

                unsigned char mLLF = lattice->LBL3D(i,j,k);
                unsigned char mLLB = lattice->LBL3D(i,j,(k+1));
                unsigned char mLRF = lattice->LBL3D((i+1),j,k);
                unsigned char mLRB = lattice->LBL3D((i+1),j,(k+1));
                unsigned char mULF = lattice->LBL3D(i,(j+1),k);
                unsigned char mULB = lattice->LBL3D(i,(j+1),(k+1));
                unsigned char mURF = lattice->LBL3D((i+1),(j+1),k);
                unsigned char mURB = lattice->LBL3D((i+1),(j+1),(k+1));

                //---------------------------------------
                // If any faces differ, add cell to Octree
                //---------------------------------------
                if( // lower face contains cut
                    mLLF != mLRF ||
                    mLLB != mLRB ||
                    mLLF != mLLB ||
                    mLRF != mLRB ||

                    // or upper face contains cut
                    mULF != mURF ||
                    mULB != mURB ||
                    mULF != mULB ||
                    mURF != mURB ||

                    // or the other four
                    mLLF != mULF ||
                    mLRF != mURF ||
                    mLLB != mULB ||
                    mLRB != mURB)
                {

                    OTCell *cell = lattice->tree->addCell(i,j,k);               
                    cell->vert = new Vertex3D*[VERTS_PER_CELL];
                    cell->edge = new Edge3D*[EDGES_PER_CELL];
                    cell->face = new Face3D*[FACES_PER_CELL];
                    cell->tets = new Tet3D*[TETS_PER_CELL];
                    memset(cell->vert, 0, VERTS_PER_CELL*sizeof(Vertex3D*));
                    memset(cell->edge, 0, EDGES_PER_CELL*sizeof(Edge3D*));
                    memset(cell->face, 0, FACES_PER_CELL*sizeof(Face3D*));
                    memset(cell->tets, 0, TETS_PER_CELL*sizeof(Tet3D*));
                    lattice->cut_cells.push_back(cell);

                    // Check If Neighbor Vertices Already Exist, Use Them If So
                    // Overwriting won't matter, as new value will be same as old

                    // check left
                    //OTCell *leftCell = lattice->tree->getNeighbor(cell, -1, 0, 0);
                    OTCell *leftCell = lattice->tree->getCell(i-1, j, k);
                    if(leftCell){
                        cell->vert[ULF] = leftCell->vert[URF];
                        cell->vert[ULB] = leftCell->vert[URB];
                        cell->vert[LLF] = leftCell->vert[LRF];
                        cell->vert[LLB] = leftCell->vert[LRB];

                        cell->edge[UL] = leftCell->edge[UR];
                        cell->edge[LL] = leftCell->edge[LR];
                        cell->edge[FL] = leftCell->edge[FR];
                        cell->edge[BL] = leftCell->edge[BR];

                        //cell->face[FLUF] = leftCell->face[FRUF];
                        //cell->face[FLUB] = leftCell->face[FRUB];
                        //cell->face[FLLF] = leftCell->face[FRLF];
                        //cell->face[FLLB] = leftCell->face[FRLB];
                    }

                    // check below
                    //OTCell *bottomCell = lattice->tree->getNeighbor(cell, 0, -1, 0);
                    OTCell *bottomCell = lattice->tree->getCell(i, j-1, k);
                    if(bottomCell){
                        cell->vert[LLF] = bottomCell->vert[ULF];
                        cell->vert[LLB] = bottomCell->vert[ULB];
                        cell->vert[LRF] = bottomCell->vert[URF];
                        cell->vert[LRB] = bottomCell->vert[URB];

                        cell->edge[LL] = bottomCell->edge[UL];
                        cell->edge[LR] = bottomCell->edge[UR];
                        cell->edge[LF] = bottomCell->edge[UF];
                        cell->edge[LB] = bottomCell->edge[UB];

                        //cell->face[FDFL] = bottomCell->face[FUFL];
                        //cell->face[FDFR] = bottomCell->face[FUFR];
                        //cell->face[FDBL] = bottomCell->face[FUBL];
                        //cell->face[FDBR] = bottomCell->face[FUBR];
                    }

                    // check in front
                    //OTCell *frontCell = lattice->tree->getNeighbor(cell, 0, 0, -1);
                    OTCell *frontCell = lattice->tree->getCell(i, j, k-1);
                    if(frontCell){
                        cell->vert[ULF] = frontCell->vert[ULB];
                        cell->vert[URF] = frontCell->vert[URB];
                        cell->vert[LLF] = frontCell->vert[LLB];
                        cell->vert[LRF] = frontCell->vert[LRB];

                        cell->edge[UF] = frontCell->edge[UB];
                        cell->edge[LF] = frontCell->edge[LB];
                        cell->edge[FL] = frontCell->edge[BL];
                        cell->edge[FR] = frontCell->edge[BR];

                        //cell->face[FFUL] = frontCell->face[FBUL];
                        //cell->face[FFUR] = frontCell->face[FBUR];
                        //cell->face[FFLL] = frontCell->face[FBLL];
                        //cell->face[FFLR] = frontCell->face[FBLR];
                    }

                    // check Lower Left
                    OTCell *LLCell = lattice->tree->getCell(i-1, j-1, k);
                    if(LLCell){
                        cell->vert[LLF] = LLCell->vert[URF];
                        cell->vert[LLB] = LLCell->vert[URB];

                        cell->edge[LL] = LLCell->edge[UR];
                    }

                    // check Lower Right
                    OTCell *LRCell = lattice->tree->getCell(i+1, j-1, k);
                    if(LRCell){
                        cell->vert[LRF] = LRCell->vert[ULF];
                        cell->vert[LRB] = LRCell->vert[ULB];

                        cell->edge[LR] = LRCell->edge[UL];
                    }

                    // check Front Lower Left
                    OTCell *LLFCell = lattice->tree->getCell(i-1, j-1, k-1);
                    if(LLFCell){
                        cell->vert[LLF] = LLFCell->vert[URB];
                    }
                    // check Front Lower Center
                    OTCell *LCFCell = lattice->tree->getCell(i, j-1, k-1);
                    if(LCFCell){
                        cell->vert[LLF] = LCFCell->vert[ULB];
                        cell->vert[LRF] = LCFCell->vert[URB];

                        cell->edge[LF] = LCFCell->edge[UB];
                    }
                    // check Front Lower Right
                    OTCell *LRFCell = lattice->tree->getCell(i+1, j-1, k-1);
                    if(LRFCell){
                        cell->vert[LRF] = LRFCell->vert[ULB];
                    }
                    // check Front Center Left
                    OTCell *CLFCell = lattice->tree->getCell(i-1, j, k-1);
                    if(CLFCell){
                        cell->vert[LLF] = CLFCell->vert[LRB];
                        cell->vert[ULF] = CLFCell->vert[URB];

                        cell->edge[FL] = CLFCell->edge[BR];
                    }
                    // check Front Center Right
                    OTCell *CRFCell = lattice->tree->getCell(i+1, j, k-1);
                    if(CRFCell){
                        cell->vert[LRF] = CRFCell->vert[LLB];
                        cell->vert[URF] = CRFCell->vert[ULB];

                        cell->edge[FR] = CRFCell->edge[BL];
                    }
                    // check Upper Front Left
                    OTCell *UFLCell = lattice->tree->getCell(i-1, j+1, k-1);
                    if(UFLCell){
                        cell->vert[ULF] = UFLCell->vert[LRB];
                    }
                    // check Upper Center Front
                    OTCell *UCFCell = lattice->tree->getCell(i, j+1, k-1);
                    if(UCFCell){
                        cell->vert[ULF] = UCFCell->vert[LLB];
                        cell->vert[URF] = UCFCell->vert[LRB];

                        cell->edge[UF] = UCFCell->edge[LB];
                    }
                    // check Upper Front Right
                    OTCell *UFRCell = lattice->tree->getCell(i+1, j+1, k-1);
                    if(UFRCell){
                        cell->vert[URF] = UFRCell->vert[LLB];
                    }


                    // Then Go through them and if any are NULL, Fill Them
                    if(!cell->vert[ULF]){
                        cell->vert[ULF] = new Vertex3D(m, cell, ULF);
                        cell->vert[ULF]->label = mULF;
                        cell->vert[ULF]->lbls[mULF] = true;
                        //cell->vert[ULF]->vals = ULF_vals;

                        cell->vert[ULF]->pos().x = cell->xLocCode;
                        cell->vert[ULF]->pos().y = cell->yLocCode + 1;
                        cell->vert[ULF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[ULB]){
                        cell->vert[ULB] = new Vertex3D(m, cell, ULB);
                        cell->vert[ULB]->label = mULB;
                        cell->vert[ULB]->lbls[mULB] = true;
                        //cell->vert[ULB]->vals = ULB_vals;

                        cell->vert[ULB]->pos().x = cell->xLocCode;
                        cell->vert[ULB]->pos().y = cell->yLocCode + 1;
                        cell->vert[ULB]->pos().z = cell->zLocCode + 1;
                    }
                    if(!cell->vert[URF]){
                        cell->vert[URF] = new Vertex3D(m, cell, URF);
                        cell->vert[URF]->label = mURF;
                        cell->vert[URF]->lbls[mURF] = true;
                        //cell->vert[URF]->vals = URF_vals;

                        cell->vert[URF]->pos().x = cell->xLocCode + 1;
                        cell->vert[URF]->pos().y = cell->yLocCode + 1;
                        cell->vert[URF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[URB]){
                        cell->vert[URB] = new Vertex3D(m, cell, URB);
                        cell->vert[URB]->label = mURB;
                        cell->vert[URB]->lbls[mURB] = true;
                        //cell->vert[URB]->vals = URB_vals;

                        cell->vert[URB]->pos().x = cell->xLocCode + 1;
                        cell->vert[URB]->pos().y = cell->yLocCode + 1;
                        cell->vert[URB]->pos().z = cell->zLocCode + 1;
                    }

                    if(!cell->vert[LLF]){
                        cell->vert[LLF] = new Vertex3D(m, cell, LLF);
                        cell->vert[LLF]->label = mLLF;
                        cell->vert[LLF]->lbls[mLLF] = true;
                        //cell->vert[LLF]->vals = LLF_vals;

                        cell->vert[LLF]->pos().x = cell->xLocCode;
                        cell->vert[LLF]->pos().y = cell->yLocCode;
                        cell->vert[LLF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[LLB]){
                        cell->vert[LLB] = new Vertex3D(m, cell, LLB);
                        cell->vert[LLB]->label = mLLB;
                        cell->vert[LLB]->lbls[mLLB] = true;
                        //cell->vert[LLB]->vals = LLB_vals;

                        cell->vert[LLB]->pos().x = cell->xLocCode;
                        cell->vert[LLB]->pos().y = cell->yLocCode;
                        cell->vert[LLB]->pos().z = cell->zLocCode + 1;
                    }
                    if(!cell->vert[LRF]){
                        cell->vert[LRF] = new Vertex3D(m, cell, LRF);
                        cell->vert[LRF]->label = mLRF;
                        cell->vert[LRF]->lbls[mLRF] = true;
                        //cell->vert[LRF]->vals = LRF_vals;

                        cell->vert[LRF]->pos().x = cell->xLocCode + 1;
                        cell->vert[LRF]->pos().y = cell->yLocCode;
                        cell->vert[LRF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[LRB]){
                        cell->vert[LRB] = new Vertex3D(m, cell, LRB);
                        cell->vert[LRB]->label = mLRB;
                        cell->vert[LRB]->lbls[mLRB] = true;
                        //cell->vert[LRB]->vals = LRB_vals;

                        cell->vert[LRB]->pos().x = cell->xLocCode + 1;
                        cell->vert[LRB]->pos().y = cell->yLocCode;
                        cell->vert[LRB]->pos().z = cell->zLocCode + 1;
                    }

                    //---------------------------------------
                    //  create interpolated center vertex
                    //---------------------------------------
                    interpolate_cell(cell,lattice);


                    //--------------------------------------------------
                    // Fill Edges That May Be Shared If They Don't Exist
                    //--------------------------------------------------
                    if(!cell->edge[UL]){
                        cell->edge[UL] = new Edge3D(true, cell, UL);
                        cell->edge[UL]->v1 = cell->vert[ULF];
                        cell->edge[UL]->v2 = cell->vert[ULB];
                    }
                    if(!cell->edge[UR]){
                        cell->edge[UR] = new Edge3D(true, cell, UR);
                        cell->edge[UR]->v1 = cell->vert[URF];
                        cell->edge[UR]->v2 = cell->vert[URB];
                    }
                    if(!cell->edge[UF]){
                        cell->edge[UF] = new Edge3D(true, cell, UF);
                        cell->edge[UF]->v1 = cell->vert[ULF];
                        cell->edge[UF]->v2 = cell->vert[URF];
                    }
                    if(!cell->edge[UB]){
                        cell->edge[UB] = new Edge3D(true, cell, UB);
                        cell->edge[UB]->v1 = cell->vert[ULB];
                        cell->edge[UB]->v2 = cell->vert[URB];
                    }
                    if(!cell->edge[LL]){
                        cell->edge[LL] = new Edge3D(true, cell, LL);
                        cell->edge[LL]->v1 = cell->vert[LLF];
                        cell->edge[LL]->v2 = cell->vert[LLB];
                    }
                    if(!cell->edge[LR]){
                        cell->edge[LR] = new Edge3D(true, cell, LR);
                        cell->edge[LR]->v1 = cell->vert[LRF];
                        cell->edge[LR]->v2 = cell->vert[LRB];
                    }
                    if(!cell->edge[LF]){
                        cell->edge[LF] = new Edge3D(true, cell, LF);
                        cell->edge[LF]->v1 = cell->vert[LLF];
                        cell->edge[LF]->v2 = cell->vert[LRF];
                    }
                    if(!cell->edge[LB]){
                        cell->edge[LB] = new Edge3D(true, cell, LB);
                        cell->edge[LB]->v1 = cell->vert[LLB];
                        cell->edge[LB]->v2 = cell->vert[LRB];
                    }
                    if(!cell->edge[FL]){
                        cell->edge[FL] = new Edge3D(true, cell, FL);
                        cell->edge[FL]->v1 = cell->vert[LLF];
                        cell->edge[FL]->v2 = cell->vert[ULF];
                    }
                    if(!cell->edge[FR]){
                        cell->edge[FR] = new Edge3D(true, cell, FR);
                        cell->edge[FR]->v1 = cell->vert[LRF];
                        cell->edge[FR]->v2 = cell->vert[URF];
                    }
                    if(!cell->edge[BL]){
                        cell->edge[BL] = new Edge3D(true, cell, BL);
                        cell->edge[BL]->v1 = cell->vert[LLB];
                        cell->edge[BL]->v2 = cell->vert[ULB];
                    }
                    if(!cell->edge[BR]){
                        cell->edge[BR] = new Edge3D(true, cell, BR);
                        cell->edge[BR]->v1 = cell->vert[LRB];
                        cell->edge[BR]->v2 = cell->vert[URB];
                    }

                    // Fill Rest
                    cell->edge[DULF] = new Edge3D(cell, DULF);
                    cell->edge[DULF]->v1 = cell->vert[C];
                    cell->edge[DULF]->v2 = cell->vert[ULF];

                    cell->edge[DULB] = new Edge3D(cell, DULB);
                    cell->edge[DULB]->v1 = cell->vert[C];
                    cell->edge[DULB]->v2 = cell->vert[ULB];

                    cell->edge[DURF] = new Edge3D(cell, DURF);
                    cell->edge[DURF]->v1 = cell->vert[C];
                    cell->edge[DURF]->v2 = cell->vert[URF];

                    cell->edge[DURB] = new Edge3D(cell, DURB);
                    cell->edge[DURB]->v1 = cell->vert[C];
                    cell->edge[DURB]->v2 = cell->vert[URB];

                    cell->edge[DLLF] = new Edge3D(cell, DLLF);
                    cell->edge[DLLF]->v1 = cell->vert[C];
                    cell->edge[DLLF]->v2 = cell->vert[LLF];

                    cell->edge[DLLB] = new Edge3D(cell, DLLB);
                    cell->edge[DLLB]->v1 = cell->vert[C];
                    cell->edge[DLLB]->v2 = cell->vert[LLB];

                    cell->edge[DLRF] = new Edge3D(cell, DLRF);
                    cell->edge[DLRF]->v1 = cell->vert[C];
                    cell->edge[DLRF]->v2 = cell->vert[LRF];

                    cell->edge[DLRB] = new Edge3D(cell, DLRB);
                    cell->edge[DLRB]->v1 = cell->vert[C];
                    cell->edge[DLRB]->v2 = cell->vert[LRB];


                    //--------------------------------------------------
                    // Fill Faces That Will Not Be Shared
                    //--------------------------------------------------
                    for(int f=0; f < 12; f++)
                        cell->face[f] = new Face3D(cell, f);

                }
            }
        }
    }

    //------------------------------------------------------------------
    // Loop over cut cells and add their one rings, so we miss no cuts
    //------------------------------------------------------------------
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){

        int i = lattice->cut_cells[c]->xLocCode;
        int j = lattice->cut_cells[c]->yLocCode;
        int k = lattice->cut_cells[c]->zLocCode;

         // Add ALL neighbors - Because this setup sucks
        for(int dz=-1; dz <= 1; dz++){
            for(int dy=-1; dy <= 1; dy++){
                for(int dx=-1; dx <= 1; dx++){

                    if(!(dx == 0 && dy == 0 && dz == 0) &&
                        i+dx >= 0 && i+dx < w-1 &&
                        j+dy >= 0 && j+dy < h-1 &&
                        k+dz >= 0 && k+dz < d-1)
                    {
                        OTCell* new_cell = addCell(lattice, i+dx, j+dy, k+dz);
                        if (new_cell) {                         
                            lattice->buffer_cells.push_back(new_cell);
                        }

                    }
                }
            }
        }

    }


    //------------------------------------------------------------------------------------
    // Loop over cut cells, populated and connect central edges and tetrahedra and faces
    //------------------------------------------------------------------------------------
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){

        OTCell *cell = lattice->cut_cells[c];
        int i = cell->xLocCode;
        int j = cell->yLocCode;
        int k = cell->zLocCode;

        OTCell *Ucell = lattice->tree->getCell(i, j+1, k);
        OTCell *Dcell = lattice->tree->getCell(i, j-1, k);
        OTCell *Lcell = lattice->tree->getCell(i-1, j, k);
        OTCell *Rcell = lattice->tree->getCell(i+1, j, k);
        OTCell *Fcell = lattice->tree->getCell(i, j, k-1);
        OTCell *Bcell = lattice->tree->getCell(i, j, k+1);

        if(Lcell && !cell->edge[CL]){

            // create edge
            Edge3D *edge = new Edge3D(true, cell, CL);
            edge->v1 = Lcell->vert[C];
            edge->v2 = cell->vert[C];
            cell->edge[CL] = edge;
            Lcell->edge[CR] = edge;

            // create 4 tets that surround edge
            cell->tets[TLU] = Lcell->tets[TRU] = new Tet3D(Lcell, TRU);    // Left Face, Upper Tet
            cell->tets[TLL] = Lcell->tets[TRL] = new Tet3D(Lcell, TRL);    // Left Face, Lower Tet
            cell->tets[TLF] = Lcell->tets[TRF] = new Tet3D(Lcell, TRF);    // Left Face, Front Tet
            cell->tets[TLB] = Lcell->tets[TRB] = new Tet3D(Lcell, TRB);    // Left Face, Back  Tet

            // create 4 faces that surround edge
            cell->face[FLUF] = Lcell->face[FRUF] = new Face3D(Lcell, FRUF);
            cell->face[FLUB] = Lcell->face[FRUB] = new Face3D(Lcell, FRUB);
            cell->face[FLLF] = Lcell->face[FRLF] = new Face3D(Lcell, FRLF);
            cell->face[FLLB] = Lcell->face[FRLB] = new Face3D(Lcell, FRLB);
        }

        if(Rcell && !cell->edge[CR]){
            Edge3D *edge = new Edge3D(true, cell, CR);
            edge->v1 = cell->vert[C];
            edge->v2 = Rcell->vert[C];
            cell->edge[CR] = edge;
            Rcell->edge[CL] = edge;

            // create 4 tets that surround edge
            cell->tets[TRU] = Rcell->tets[TLU] = new Tet3D(cell,TRU);    // Right Face, Upper Tet
            cell->tets[TRL] = Rcell->tets[TLL] = new Tet3D(cell,TRL);    // Right Face, Lower Tet
            cell->tets[TRF] = Rcell->tets[TLF] = new Tet3D(cell,TRF);    // Right Face, Front Tet
            cell->tets[TRB] = Rcell->tets[TLB] = new Tet3D(cell,TRB);    // Right Face, Back  Tet

            // create 4 faces that surround edge
            cell->face[FRUF] = Rcell->face[FLUF] = new Face3D(cell, FRUF);
            cell->face[FRUB] = Rcell->face[FLUB] = new Face3D(cell, FRUB);
            cell->face[FRLF] = Rcell->face[FLLF] = new Face3D(cell, FRLF);
            cell->face[FRLB] = Rcell->face[FLLB] = new Face3D(cell, FRLB);
        }

        if(Ucell && !cell->edge[CU]){
            Edge3D *edge = new Edge3D(true, cell, CU);
            edge->v1 = cell->vert[C];
            edge->v2 = Ucell->vert[C];
            cell->edge[CU] = edge;
            Ucell->edge[CD] = edge;

            // create 4 tets that surround edge
            cell->tets[TUF] = Ucell->tets[TDF] = new Tet3D(cell,TUF);    // Up Face, Front Tet
            cell->tets[TUB] = Ucell->tets[TDB] = new Tet3D(cell,TUB);    // Up Face, Back  Tet
            cell->tets[TUL] = Ucell->tets[TDL] = new Tet3D(cell,TUL);    // Up Face, Left  Tet
            cell->tets[TUR] = Ucell->tets[TDR] = new Tet3D(cell,TUR);    // Up Face, Right Tet

            // create 4 faces that surround edge
            cell->face[FUFL] = Ucell->face[FDFL] = new Face3D(cell, FUFL);
            cell->face[FUFR] = Ucell->face[FDFR] = new Face3D(cell, FUFR);
            cell->face[FUBL] = Ucell->face[FDBL] = new Face3D(cell, FUBL);
            cell->face[FUBR] = Ucell->face[FDBR] = new Face3D(cell, FUBR);
        }

        if(Dcell && !cell->edge[CD]){
            Edge3D *edge = new Edge3D(true, cell, CD);
            edge->v1 = Dcell->vert[C];
            edge->v2 = cell->vert[C];
            cell->edge[CD] = edge;
            Dcell->edge[CU] = edge;

            // create 4 tets that surround edge
            cell->tets[TDF] = Dcell->tets[TUF] = new Tet3D(Dcell,TUF);    // Down Face, Front Tet
            cell->tets[TDB] = Dcell->tets[TUB] = new Tet3D(Dcell,TUB);    // Down Face, Back  Tet
            cell->tets[TDL] = Dcell->tets[TUL] = new Tet3D(Dcell,TUL);    // Down Face, Left  Tet
            cell->tets[TDR] = Dcell->tets[TUR] = new Tet3D(Dcell,TUR);    // Down Face, Right Tet

            // create 4 faces that surround edge
            cell->face[FDFL] = Dcell->face[FUFL] = new Face3D(Dcell, FUFL);
            cell->face[FDFR] = Dcell->face[FUFR] = new Face3D(Dcell, FUFR);
            cell->face[FDBL] = Dcell->face[FUBL] = new Face3D(Dcell, FUBL);
            cell->face[FDBR] = Dcell->face[FUBR] = new Face3D(Dcell, FUBR);
        }

        if(Fcell && !cell->edge[CF]){
            Edge3D *edge = new Edge3D(true, cell, CF);
            edge->v1 = Fcell->vert[C];
            edge->v2 = cell->vert[C];
            cell->edge[CF] = edge;
            Fcell->edge[CB] = edge;

            // create 4 tets that surround edge
            cell->tets[TFT] = Fcell->tets[TBT] = new Tet3D(Fcell,TBT);    // Front Face, Top    Tet
            cell->tets[TFB] = Fcell->tets[TBB] = new Tet3D(Fcell,TBB);    // Front Face, Bottom Tet
            cell->tets[TFL] = Fcell->tets[TBL] = new Tet3D(Fcell,TBL);    // Front Face, Left   Tet
            cell->tets[TFR] = Fcell->tets[TBR] = new Tet3D(Fcell,TBR);    // Front Face, Right  Tet

            // create 4 faces that surround edge
            cell->face[FFUL] = Fcell->face[FBUL] = new Face3D(Fcell, FBUL);
            cell->face[FFUR] = Fcell->face[FBUR] = new Face3D(Fcell, FBUR);
            cell->face[FFLL] = Fcell->face[FBLL] = new Face3D(Fcell, FBLL);
            cell->face[FFLR] = Fcell->face[FBLR] = new Face3D(Fcell, FBLR);
        }        

        if(Bcell && !cell->edge[CB]){
            Edge3D *edge = new Edge3D(true, cell, CB);
            edge->v1 = cell->vert[C];
            edge->v2 = Bcell->vert[C];
            cell->edge[CB] = edge;
            Bcell->edge[CF] = edge;

            // create 4 tets that surround edge
            cell->tets[TBT] = Bcell->tets[TFT] = new Tet3D(cell,TBT);    // Back Face, Top    Tet
            cell->tets[TBB] = Bcell->tets[TFB] = new Tet3D(cell,TBB);    // Back Face, Bottom Tet
            cell->tets[TBL] = Bcell->tets[TFL] = new Tet3D(cell,TBL);    // Back Face, Left   Tet
            cell->tets[TBR] = Bcell->tets[TFR] = new Tet3D(cell,TBR);    // Back Face, Right  Tet

            // create 4 faces that surround edge
            cell->face[FBUL] = Bcell->face[FFUL] = new Face3D(cell, FBUL);
            cell->face[FBUR] = Bcell->face[FFUR] = new Face3D(cell, FBUR);
            cell->face[FBLL] = Bcell->face[FFLL] = new Face3D(cell, FBLL);
            cell->face[FBLR] = Bcell->face[FFLR] = new Face3D(cell, FBLR);
        }        
    }

    //----------------------------------------
    // Repeat the same for the buffer cells
    //----------------------------------------
    for(unsigned int c=0; c < lattice->buffer_cells.size(); c++){

        OTCell *cell = lattice->buffer_cells[c];
        int i = cell->xLocCode;
        int j = cell->yLocCode;
        int k = cell->zLocCode;

        OTCell *Ucell = lattice->tree->getCell(i, j+1, k);
        OTCell *Dcell = lattice->tree->getCell(i, j-1, k);
        OTCell *Lcell = lattice->tree->getCell(i-1, j, k);
        OTCell *Rcell = lattice->tree->getCell(i+1, j, k);
        OTCell *Fcell = lattice->tree->getCell(i, j, k-1);
        OTCell *Bcell = lattice->tree->getCell(i, j, k+1);

        if(Lcell && !cell->edge[CL]){

            // create edge
            Edge3D *edge = new Edge3D(true, cell, CL);
            edge->v1 = Lcell->vert[C];
            edge->v2 = cell->vert[C];
            cell->edge[CL] = edge;
            Lcell->edge[CR] = edge;

            // create 4 tets that surround edge
            cell->tets[TLU] = Lcell->tets[TRU] = new Tet3D(Lcell,TRU);    // Left Face, Upper Tet
            cell->tets[TLL] = Lcell->tets[TRL] = new Tet3D(Lcell,TRL);    // Left Face, Lower Tet
            cell->tets[TLF] = Lcell->tets[TRF] = new Tet3D(Lcell,TRF);    // Left Face, Front Tet
            cell->tets[TLB] = Lcell->tets[TRB] = new Tet3D(Lcell,TRB);    // Left Face, Back  Tet

            // create 4 faces that surround edge
            cell->face[FLUF] = Lcell->face[FRUF] = new Face3D(Lcell, FRUF);
            cell->face[FLUB] = Lcell->face[FRUB] = new Face3D(Lcell, FRUB);
            cell->face[FLLF] = Lcell->face[FRLF] = new Face3D(Lcell, FRLF);
            cell->face[FLLB] = Lcell->face[FRLB] = new Face3D(Lcell, FRLB);

        } else if (!Lcell) {
            //no left cell, this means we need two pyramids to match
            Vertex3D* v_c = cell->vert[C];
            Vertex3D* v_p0 = cell->vert[ULF];
            Vertex3D* v_p1 = cell->vert[ULB];
            Vertex3D* v_p2 = cell->vert[LLB];
            Vertex3D* v_p3 = cell->vert[LLF];

            // create tets with proper diagonal against neighbor
            if (cell == cell->parent->children[CULB] ||
                cell == cell->parent->children[CLLF] ||
                cell == cell->parent->children[CURB] ||
                cell == cell->parent->children[CLRF])
            {
                lattice->tree->createTet(v_c, v_p1, v_p3, v_p2, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p0, v_p3, v_p0->label);
            }
            else{
                lattice->tree->createTet(v_c, v_p0, v_p2, v_p1, v_p0->label);
                lattice->tree->createTet(v_c, v_p0, v_p3, v_p2, v_p0->label);
            }
        }

        if(Rcell && !cell->edge[CR]){
            Edge3D *edge = new Edge3D(true, cell, CR);
            edge->v1 = cell->vert[C];
            edge->v2 = Rcell->vert[C];
            cell->edge[CR] = edge;
            Rcell->edge[CL] = edge;

            // create 4 tets that surround edge
            cell->tets[TRU] = Rcell->tets[TLU] = new Tet3D(cell,TRU);    // Right Face, Upper Tet
            cell->tets[TRL] = Rcell->tets[TLL] = new Tet3D(cell,TRL);    // Right Face, Lower Tet
            cell->tets[TRF] = Rcell->tets[TLF] = new Tet3D(cell,TRF);    // Right Face, Front Tet
            cell->tets[TRB] = Rcell->tets[TLB] = new Tet3D(cell,TRB);    // Right Face, Back  Tet

            // create 4 faces that surround edge
            cell->face[FRUF] = Rcell->face[FLUF] = new Face3D(cell, FRUF);
            cell->face[FRUB] = Rcell->face[FLUB] = new Face3D(cell, FRUB);
            cell->face[FRLF] = Rcell->face[FLLF] = new Face3D(cell, FRLF);
            cell->face[FRLB] = Rcell->face[FLLB] = new Face3D(cell, FRLB);

        } else if (!Rcell) {
            //no right cell, this means we need two pyramids to match
            Vertex3D* v_c = cell->vert[C];
            Vertex3D* v_p0 = cell->vert[URF];
            Vertex3D* v_p1 = cell->vert[URB];
            Vertex3D* v_p2 = cell->vert[LRB];
            Vertex3D* v_p3 = cell->vert[LRF];

            if (cell == cell->parent->children[CULB] ||
                cell == cell->parent->children[CLLF] ||
                cell == cell->parent->children[CURB] ||
                cell == cell->parent->children[CLRF])
            {
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p3, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p2, v_p3, v_p0->label);
            }
            else
            {
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p2, v_p0->label);
                lattice->tree->createTet(v_c, v_p2, v_p3, v_p0, v_p0->label);
            }
        }

        if(Ucell && !cell->edge[CU]){
            Edge3D *edge = new Edge3D(true, cell, CU);
            edge->v1 = cell->vert[C];
            edge->v2 = Ucell->vert[C];
            cell->edge[CU] = edge;
            Ucell->edge[CD] = edge;

            // create 4 tets that surround edge
            cell->tets[TUF] = Ucell->tets[TDF] = new Tet3D(cell,TUF);    // Up Face, Front Tet
            cell->tets[TUB] = Ucell->tets[TDB] = new Tet3D(cell,TUB);    // Up Face, Back  Tet
            cell->tets[TUL] = Ucell->tets[TDL] = new Tet3D(cell,TUL);    // Up Face, Left  Tet
            cell->tets[TUR] = Ucell->tets[TDR] = new Tet3D(cell,TUR);    // Up Face, Right Tet

            // create 4 faces that surround edge
            cell->face[FUFL] = Ucell->face[FDFL] = new Face3D(cell, FUFL);
            cell->face[FUFR] = Ucell->face[FDFR] = new Face3D(cell, FUFR);
            cell->face[FUBL] = Ucell->face[FDBL] = new Face3D(cell, FUBL);
            cell->face[FUBR] = Ucell->face[FDBR] = new Face3D(cell, FUBR);

        } else if (!Ucell) {
            //no up cell, this means we need two pyramids to match
            Vertex3D* v_c = cell->vert[C];
            Vertex3D* v_p0 = cell->vert[ULF];
            Vertex3D* v_p1 = cell->vert[ULB];
            Vertex3D* v_p2 = cell->vert[URB];
            Vertex3D* v_p3 = cell->vert[URF];


            if (cell == cell->parent->children[CULB] ||
                cell == cell->parent->children[CURF] ||
                cell == cell->parent->children[CLLB] ||
                cell == cell->parent->children[CLRF])
            {
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p3, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p2, v_p3, v_p0->label);
            }
            else{
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p2, v_p0->label);
                lattice->tree->createTet(v_c, v_p2, v_p3, v_p0, v_p0->label);
            }

        }

        if(Dcell && !cell->edge[CD]){
            Edge3D *edge = new Edge3D(true, cell, CD);
            edge->v1 = Dcell->vert[C];
            edge->v2 = cell->vert[C];
            cell->edge[CD] = edge;
            Dcell->edge[CU] = edge;

            // create 4 tets that surround edge
            cell->tets[TDF] = Dcell->tets[TUF] = new Tet3D(Dcell,TUF);    // Down Face, Front Tet
            cell->tets[TDB] = Dcell->tets[TUB] = new Tet3D(Dcell,TUB);    // Down Face, Back  Tet
            cell->tets[TDL] = Dcell->tets[TUL] = new Tet3D(Dcell,TUL);    // Down Face, Left  Tet
            cell->tets[TDR] = Dcell->tets[TUR] = new Tet3D(Dcell,TUR);    // Down Face, Right Tet

            // create 4 faces that surround edge
            cell->face[FDFL] = Dcell->face[FUFL] = new Face3D(Dcell, FUFL);
            cell->face[FDFR] = Dcell->face[FUFR] = new Face3D(Dcell, FUFR);
            cell->face[FDBL] = Dcell->face[FUBL] = new Face3D(Dcell, FUBL);
            cell->face[FDBR] = Dcell->face[FUBR] = new Face3D(Dcell, FUBR);

        } else if (!Dcell) {
            //no down cell, this means we need two pyramids to match
            Vertex3D* v_c = cell->vert[C];
            Vertex3D* v_p0 = cell->vert[LLF];
            Vertex3D* v_p1 = cell->vert[LLB];
            Vertex3D* v_p2 = cell->vert[LRB];
            Vertex3D* v_p3 = cell->vert[LRF];

            if (cell == cell->parent->children[CULB] ||
                cell == cell->parent->children[CURF] ||
                cell == cell->parent->children[CLLB] ||
                cell == cell->parent->children[CLRF])
            {
                lattice->tree->createTet(v_c, v_p0, v_p3, v_p1, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p3, v_p2, v_p0->label);
            }
            else{
                lattice->tree->createTet(v_c, v_p0, v_p2, v_p1, v_p0->label);
                lattice->tree->createTet(v_c, v_p2, v_p0, v_p3, v_p0->label);
            }
        }

        if(Fcell && !cell->edge[CF]){
            Edge3D *edge = new Edge3D(true, cell, CF);
            edge->v1 = Fcell->vert[C];
            edge->v2 = cell->vert[C];
            cell->edge[CF] = edge;
            Fcell->edge[CB] = edge;

            // create 4 tets that surround edge
            cell->tets[TFT] = Fcell->tets[TBT] = new Tet3D(Fcell,TBT);    // Front Face, Top    Tet
            cell->tets[TFB] = Fcell->tets[TBB] = new Tet3D(Fcell,TBB);    // Front Face, Bottom Tet
            cell->tets[TFL] = Fcell->tets[TBL] = new Tet3D(Fcell,TBL);    // Front Face, Left   Tet
            cell->tets[TFR] = Fcell->tets[TBR] = new Tet3D(Fcell,TBR);    // Front Face, Right  Tet

            // create 4 faces that surround edge
            cell->face[FFUL] = Fcell->face[FBUL] = new Face3D(Fcell, FBUL);
            cell->face[FFUR] = Fcell->face[FBUR] = new Face3D(Fcell, FBUR);
            cell->face[FFLL] = Fcell->face[FBLL] = new Face3D(Fcell, FBLL);
            cell->face[FFLR] = Fcell->face[FBLR] = new Face3D(Fcell, FBLR);

        } else if (!Fcell) {
            //no front cell, this means we need two pyramids to match
            Vertex3D* v_c = cell->vert[C];
            Vertex3D* v_p0 = cell->vert[ULF];
            Vertex3D* v_p1 = cell->vert[URF];
            Vertex3D* v_p2 = cell->vert[LRF];
            Vertex3D* v_p3 = cell->vert[LLF];

            if (cell == cell->parent->children[CURF] ||
                cell == cell->parent->children[CLLF] ||
                cell == cell->parent->children[CURB] ||
                cell == cell->parent->children[CLLB])
            {
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p3, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p2, v_p3, v_p0->label);
            }
            else{
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p2, v_p0->label);
                lattice->tree->createTet(v_c, v_p2, v_p3, v_p0, v_p0->label);
            }
        }

        if(Bcell && !cell->edge[CB]){
            Edge3D *edge = new Edge3D(true, cell, CB);
            edge->v1 = cell->vert[C];
            edge->v2 = Bcell->vert[C];
            cell->edge[CB] = edge;
            Bcell->edge[CF] = edge;

            // create 4 tets that surround edge
            cell->tets[TBT] = Bcell->tets[TFT] = new Tet3D(cell,TBT);    // Back Face, Top    Tet
            cell->tets[TBB] = Bcell->tets[TFB] = new Tet3D(cell,TBB);    // Back Face, Bottom Tet
            cell->tets[TBL] = Bcell->tets[TFL] = new Tet3D(cell,TBL);    // Back Face, Left   Tet
            cell->tets[TBR] = Bcell->tets[TFR] = new Tet3D(cell,TBR);    // Back Face, Right  Tet

            // create 4 faces that surround edge
            cell->face[FBUL] = Bcell->face[FFUL] = new Face3D(cell, FBUL);
            cell->face[FBUR] = Bcell->face[FFUR] = new Face3D(cell, FBUR);
            cell->face[FBLL] = Bcell->face[FFLL] = new Face3D(cell, FBLL);
            cell->face[FBLR] = Bcell->face[FFLR] = new Face3D(cell, FBLR);

        } else if (!Bcell) {
            //no back cell, this means we need two pyramids to match
            Vertex3D* v_c = cell->vert[C];
            Vertex3D* v_p0 = cell->vert[LLB];
            Vertex3D* v_p1 = cell->vert[LRB];
            Vertex3D* v_p2 = cell->vert[URB];
            Vertex3D* v_p3 = cell->vert[ULB];

            if (cell == cell->parent->children[CURF] ||
                cell == cell->parent->children[CLLF] ||
                cell == cell->parent->children[CURB] ||
                cell == cell->parent->children[CLLB])
            {
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p2, v_p0->label);
                lattice->tree->createTet(v_c, v_p2, v_p3, v_p0, v_p0->label);
            }
            else{

                lattice->tree->createTet(v_c, v_p0, v_p1, v_p3, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p2, v_p3, v_p0->label);
            }

        }
    }


    //enforce_field_constraint(lattice);

    lattice->tree->balance_tree();
    lattice->tree->create_background_grid(lattice->buffer_cells);
    lattice->tree->label_background_tets(lattice->labels);

    lattice->setDataLoaded(true);

    return lattice;
}


static void interpolate_cell(OTCell *cell, BCCLattice3D *lattice)
{
    int m = lattice->materials();

    cell->vert[C] = new Vertex3D(m, cell, C);
    //cell->vert[C]->vals = new float[m];
    //memset(cell->vert[C]->vals, 0, m*sizeof(float));
    float i = cell->vert[C]->pos().x = cell->xLocCode + 0.5f;
    float j = cell->vert[C]->pos().y = cell->yLocCode + 0.5f;
    float k = cell->vert[C]->pos().z = cell->zLocCode + 0.5f;

    int dom = -1;
    double max = -10000;

    for(int mat=0; mat < m; mat++){

        // copy all values, in case one appears in neighbor dual
        //for(int i=0; i < 8; i++){
        //    cell->vert[C]->vals[mat] += cell->vert[i]->vals[mat];
        //}
        //cell->vert[C]->vals[mat] *= 0.125f;  // * 1/8

        bool found = false;
        for(int v=0; v < 8; v++){
            found = cell->vert[v]->lbls[mat];
            if(found)
                break;
        }

        // only allow maximums that occured on corners
        if(!found)
            continue;

        float value = lattice->volume->valueAt(i,j,k,mat);
        if(value > max || dom < 0){
            max = value;
            dom = mat;
        }
    }

    // make sure no values are equivalent to max
    /*
    for(int mat=0; mat < m; mat++){
        float value = lattice->fields->valueAt(i,j,k,mat);
        if(value == max && mat != dom){
            float diff = 1E-6*fabs(max);  diff = fmax(1E-6, diff);
            lattice->fields->setNewValueAt(max - diff, i, j, k, mat);
        }
    }
    */

    //max = cell->vert[C]->vals[dom] += 10.1;


    // now double check values aren't equivalent
    /*
    int max_count = 0;
    for(int mat=0; mat < m; mat++){
        float value = lattice->fields->valueAt(i,j,k,mat);
        if(value == max){
            max_count++;
        }
    }
    if(max_count != 1)
        cerr << "Problem!! - Dual Vertex has no clear max value" << endl;
    */

    cell->vert[C]->lbls[dom] = true;
    cell->vert[C]->label = dom;
}

static OTCell* addCell(BCCLattice3D *lattice, int i, int j, int k)
{
    //if(i == 62 && j == 7 && k == 37)
    //    std::cout  << "adding cell:" << i << "," << j << "," << k << std::endl;

    OTCell* new_cell = 0;

    // If NULL returned
    if(!lattice->tree->getCell(i, j, k)){

        // Try to Add it
        OTCell *cell = lattice->tree->addCell(i,j,k);

        // If successful, fill with data
        if(cell){
            new_cell = cell;

            int w = lattice->width();
            int h = lattice->height();
            //int d = lattice->depth();
            int m = lattice->materials();

            cell->vert = new Vertex3D*[VERTS_PER_CELL];
            cell->edge = new Edge3D*[EDGES_PER_CELL];
            cell->face = new Face3D*[FACES_PER_CELL];
            cell->tets = new Tet3D*[TETS_PER_CELL];
            memset(cell->vert, 0, VERTS_PER_CELL*sizeof(Vertex3D*));
            memset(cell->edge, 0, EDGES_PER_CELL*sizeof(Edge3D*));
            memset(cell->face, 0, FACES_PER_CELL*sizeof(Face3D*));
            memset(cell->tets, 0, TETS_PER_CELL*sizeof(Tet3D*));


            //--------------------------------------
            // Obtain Max Material At Each Vertex
            //--------------------------------------
            unsigned char mLLF = lattice->LBL3D(i,j,k);
            unsigned char mLLB = lattice->LBL3D(i,j,(k+1));
            unsigned char mLRF = lattice->LBL3D((i+1),j,k);
            unsigned char mLRB = lattice->LBL3D((i+1),j,(k+1));
            unsigned char mULF = lattice->LBL3D(i,(j+1),k);
            unsigned char mULB = lattice->LBL3D(i,(j+1),(k+1));
            unsigned char mURF = lattice->LBL3D((i+1),(j+1),k);
            unsigned char mURB = lattice->LBL3D((i+1),(j+1),(k+1));

//            float *LLF_vals = &lattice->DATA3D(i,j,k,m,0);
//            float *LLB_vals = &lattice->DATA3D(i,j,(k+1),m,0);
//            float *LRF_vals = &lattice->DATA3D((i+1),j,k,m,0);
//            float *LRB_vals = &lattice->DATA3D((i+1),j,(k+1),m,0);
//            float *ULF_vals = &lattice->DATA3D(i,(j+1),k,m,0);
//            float *ULB_vals = &lattice->DATA3D(i,(j+1),(k+1),m,0);
//            float *URF_vals = &lattice->DATA3D((i+1),(j+1),k,m,0);
//            float *URB_vals = &lattice->DATA3D((i+1),(j+1),(k+1),m,0);

            //--------------------------------------
            // Try To Use Neighbor Vertex Pointers
            //--------------------------------------
            // check left
            OTCell *leftCell = lattice->tree->getCell(i-1, j, k);
            if(leftCell){
                cell->vert[ULF] = leftCell->vert[URF];
                cell->vert[ULB] = leftCell->vert[URB];
                cell->vert[LLF] = leftCell->vert[LRF];
                cell->vert[LLB] = leftCell->vert[LRB];

                cell->edge[UL] = leftCell->edge[UR];
                cell->edge[LL] = leftCell->edge[LR];
                cell->edge[FL] = leftCell->edge[FR];
                cell->edge[BL] = leftCell->edge[BR];
            }

            // check right
            OTCell *rightCell = lattice->tree->getCell(i+1, j, k);
            if(rightCell){
                cell->vert[URF] = rightCell->vert[ULF];
                cell->vert[URB] = rightCell->vert[ULB];
                cell->vert[LRF] = rightCell->vert[LLF];
                cell->vert[LRB] = rightCell->vert[LLB];

                cell->edge[UR] = rightCell->edge[UL];
                cell->edge[LR] = rightCell->edge[LL];
                cell->edge[FR] = rightCell->edge[FL];
                cell->edge[BR] = rightCell->edge[BL];
            }

            // check below
            OTCell *bottomCell = lattice->tree->getCell(i, j-1, k);
            if(bottomCell){
                cell->vert[LLF] = bottomCell->vert[ULF];
                cell->vert[LLB] = bottomCell->vert[ULB];
                cell->vert[LRF] = bottomCell->vert[URF];
                cell->vert[LRB] = bottomCell->vert[URB];

                cell->edge[LL] = bottomCell->edge[UL];
                cell->edge[LR] = bottomCell->edge[UR];
                cell->edge[LF] = bottomCell->edge[UF];
                cell->edge[LB] = bottomCell->edge[UB];
            }

            // check above
            OTCell *topCell = lattice->tree->getCell(i, j+1, k);
            if(topCell){
                cell->vert[ULF] = topCell->vert[LLF];
                cell->vert[ULB] = topCell->vert[LLB];
                cell->vert[URF] = topCell->vert[LRF];
                cell->vert[URB] = topCell->vert[LRB];

                cell->edge[UL] = topCell->edge[LL];
                cell->edge[UR] = topCell->edge[LR];
                cell->edge[UF] = topCell->edge[LF];
                cell->edge[UB] = topCell->edge[LB];
            }

            // check in front
            OTCell *frontCell = lattice->tree->getCell(i, j, k-1);
            if(frontCell){
                cell->vert[ULF] = frontCell->vert[ULB];
                cell->vert[URF] = frontCell->vert[URB];
                cell->vert[LLF] = frontCell->vert[LLB];
                cell->vert[LRF] = frontCell->vert[LRB];

                cell->edge[UF] = frontCell->edge[UB];
                cell->edge[LF] = frontCell->edge[LB];
                cell->edge[FL] = frontCell->edge[BL];
                cell->edge[FR] = frontCell->edge[BR];
            }

            // check in back
            OTCell *backCell = lattice->tree->getCell(i, j, k+1);
            if(backCell){
                cell->vert[ULB] = backCell->vert[ULF];
                cell->vert[URB] = backCell->vert[URF];
                cell->vert[LLB] = backCell->vert[LLF];
                cell->vert[LRB] = backCell->vert[LRF];

                cell->edge[UB] = backCell->edge[UF];
                cell->edge[LB] = backCell->edge[LF];
                cell->edge[BL] = backCell->edge[FL];
                cell->edge[BR] = backCell->edge[FR];
            }

            // 8 Corners
            // check Lower Left Front
            OTCell *LLFCell = lattice->tree->getCell(i-1, j-1, k-1);
            if(LLFCell){
                cell->vert[LLF] = LLFCell->vert[URB];
            }
            // check Lower Right Front
            OTCell *LRFCell = lattice->tree->getCell(i+1, j-1, k-1);
            if(LRFCell){
                cell->vert[LRF] = LRFCell->vert[ULB];
            }
            // check Upper Left Front
            OTCell *ULFCell = lattice->tree->getCell(i-1, j+1, k-1);
            if(ULFCell){
                cell->vert[ULF] = ULFCell->vert[LRB];
            }
            // check Upper Right Front
            OTCell *UFRCell = lattice->tree->getCell(i+1, j+1, k-1);
            if(UFRCell){
                cell->vert[URF] = UFRCell->vert[LLB];
            }

            // check Lower Left Back
            OTCell *LLBCell = lattice->tree->getCell(i-1, j-1, k+1);
            if(LLBCell){
                cell->vert[LLB] = LLBCell->vert[URF];
            }
            // check Lower Right Back
            OTCell *LRBCell = lattice->tree->getCell(i+1, j-1, k+1);
            if(LRBCell){
                cell->vert[LRB] = LRBCell->vert[ULF];
            }
            // check Upper Left Back
            OTCell *ULBCell = lattice->tree->getCell(i-1, j+1, k+1);
            if(ULBCell){
                cell->vert[ULB] = ULBCell->vert[LRF];
            }
            // check Upper Right Back
            OTCell *UFBCell = lattice->tree->getCell(i+1, j+1, k+1);
            if(UFBCell){
                cell->vert[URB] = UFBCell->vert[LLF];
            }

            // 12 Edge Centers
            // check  Lower Center Front
            OTCell *LCFCell = lattice->tree->getCell(i, j-1, k-1);
            if(LCFCell){
                cell->vert[LLF] = LCFCell->vert[ULB];
                cell->vert[LRF] = LCFCell->vert[URB];

                cell->edge[LF]  = LCFCell->edge[UB];
            }
            // check Upper Center Front
            OTCell *UCFCell = lattice->tree->getCell(i, j+1, k-1);
            if(UCFCell){
                cell->vert[ULF] = UCFCell->vert[LLB];
                cell->vert[URF] = UCFCell->vert[LRB];

                cell->edge[UF]  = UCFCell->edge[LB];
            }
            // check  Center Left Front
            OTCell *CLFCell = lattice->tree->getCell(i-1, j, k-1);
            if(CLFCell){
                cell->vert[LLF] = CLFCell->vert[LRB];
                cell->vert[ULF] = CLFCell->vert[URB];

                cell->edge[FL]  = CLFCell->edge[BR];
            }
            // check  Center Right Front
            OTCell *CRFCell = lattice->tree->getCell(i+1, j, k-1);
            if(CRFCell){
                cell->vert[LRF] = CRFCell->vert[LLB];
                cell->vert[URF] = CRFCell->vert[ULB];

                cell->edge[FR]  = CRFCell->edge[BL];
            }

            // check  Lower Center Back
            OTCell *LCBCell = lattice->tree->getCell(i, j-1, k+1);
            if(LCBCell){
                cell->vert[LLB] = LCBCell->vert[ULF];
                cell->vert[LRB] = LCBCell->vert[URF];

                cell->edge[LB]  = LCBCell->edge[UF];
            }
            // check Upper Center Back
            OTCell *UCBCell = lattice->tree->getCell(i, j+1, k+1);
            if(UCBCell){
                cell->vert[ULB] = UCBCell->vert[LLF];
                cell->vert[URB] = UCBCell->vert[LRF];

                cell->edge[UB]  = UCBCell->edge[LF];
            }
            // check  Center Left Back
            OTCell *CLBCell = lattice->tree->getCell(i-1, j, k+1);
            if(CLBCell){
                cell->vert[LLB] = CLBCell->vert[LRF];
                cell->vert[ULB] = CLBCell->vert[URF];

                cell->edge[BL]  = CLBCell->edge[FR];
            }
            // check  Center Right Back
            OTCell *CRBCell = lattice->tree->getCell(i+1, j, k+1);
            if(CRBCell){
                cell->vert[LRB] = CRBCell->vert[LLF];
                cell->vert[URB] = CRBCell->vert[ULF];

                cell->edge[BR]  = CRBCell->edge[FL];
            }
            // Remaining 4
            // check Lower Center Left
            OTCell *LLCell = lattice->tree->getCell(i-1, j-1, k);
            if(LLCell){
                cell->vert[LLF] = LLCell->vert[URF];
                cell->vert[LLB] = LLCell->vert[URB];

                cell->edge[LL]  = LLCell->edge[UR];
            }
            // check Upper Center Left
            OTCell *ULCell = lattice->tree->getCell(i-1, j+1, k);
            if(ULCell){
                cell->vert[ULF] = ULCell->vert[LRF];
                cell->vert[ULB] = ULCell->vert[LRB];

                cell->edge[UL]  = ULCell->edge[LR];
            }
            // check Lower Center Right
            OTCell *LRCell = lattice->tree->getCell(i+1, j-1, k);
            if(LRCell){
                cell->vert[LRF] = LRCell->vert[ULF];
                cell->vert[LRB] = LRCell->vert[ULB];

                cell->edge[LR]  = LRCell->edge[UL];
            }
            // check Upper Center Right
            OTCell *URCell = lattice->tree->getCell(i+1, j+1, k);
            if(URCell){
                cell->vert[URF] = URCell->vert[LLF];
                cell->vert[URB] = URCell->vert[LLB];

                cell->edge[UR]  = URCell->edge[LL];
            }



            // Then Go through them and if any are NULL, Fill Them
            if(!cell->vert[ULF]){
                cell->vert[ULF] = new Vertex3D(m, cell, ULF);
                cell->vert[ULF]->label = mULF;
                cell->vert[ULF]->lbls[mULF] = true;
                //cell->vert[ULF]->vals = ULF_vals;
                cell->vert[ULF]->pos().x = cell->xLocCode;
                cell->vert[ULF]->pos().y = cell->yLocCode + 1;
                cell->vert[ULF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[ULB]){
                cell->vert[ULB] = new Vertex3D(m, cell, ULB);
                cell->vert[ULB]->label = mULB;
                cell->vert[ULB]->lbls[mULB] = true;
                //cell->vert[ULB]->vals = ULB_vals;
                cell->vert[ULB]->pos().x = cell->xLocCode;
                cell->vert[ULB]->pos().y = cell->yLocCode + 1;
                cell->vert[ULB]->pos().z = cell->zLocCode + 1;
            }
            if(!cell->vert[URF]){
                cell->vert[URF] = new Vertex3D(m, cell, URF);
                cell->vert[URF]->label = mURF;
                cell->vert[URF]->lbls[mURF] = true;
                //cell->vert[URF]->vals = URF_vals;
                cell->vert[URF]->pos().x = cell->xLocCode + 1;
                cell->vert[URF]->pos().y = cell->yLocCode + 1;
                cell->vert[URF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[URB]){
                cell->vert[URB] = new Vertex3D(m, cell, URB);
                cell->vert[URB]->label = mURB;
                cell->vert[URB]->lbls[mURB] = true;
                //cell->vert[URB]->vals = URB_vals;
                cell->vert[URB]->pos().x = cell->xLocCode + 1;
                cell->vert[URB]->pos().y = cell->yLocCode + 1;
                cell->vert[URB]->pos().z = cell->zLocCode + 1;
            }

            if(!cell->vert[LLF]){
                cell->vert[LLF] = new Vertex3D(m, cell, LLF);
                cell->vert[LLF]->label = mLLF;
                cell->vert[LLF]->lbls[mLLF] = true;
                //cell->vert[LLF]->vals = LLF_vals;

                cell->vert[LLF]->pos().x = cell->xLocCode;
                cell->vert[LLF]->pos().y = cell->yLocCode;
                cell->vert[LLF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[LLB]){
                cell->vert[LLB] = new Vertex3D(m, cell, LLB);
                cell->vert[LLB]->label = mLLB;
                cell->vert[LLB]->lbls[mLLB] = true;
                //cell->vert[LLB]->vals = LLB_vals;
                cell->vert[LLB]->pos().x = cell->xLocCode;
                cell->vert[LLB]->pos().y = cell->yLocCode;
                cell->vert[LLB]->pos().z = cell->zLocCode + 1;
            }
            if(!cell->vert[LRF]){
                cell->vert[LRF] = new Vertex3D(m, cell, LRF);
                cell->vert[LRF]->label = mLRF;
                cell->vert[LRF]->lbls[mLRF] = true;
                //cell->vert[LRF]->vals = LRF_vals;
                cell->vert[LRF]->pos().x = cell->xLocCode + 1;
                cell->vert[LRF]->pos().y = cell->yLocCode;
                cell->vert[LRF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[LRB]){
                cell->vert[LRB] = new Vertex3D(m, cell, LRB);
                cell->vert[LRB]->label = mLRB;
                cell->vert[LRB]->lbls[mLRB] = true;
                //cell->vert[LRB]->vals = LRB_vals;
                cell->vert[LRB]->pos().x = cell->xLocCode + 1;
                cell->vert[LRB]->pos().y = cell->yLocCode;
                cell->vert[LRB]->pos().z = cell->zLocCode + 1;
            }

            //---------------------------------------
            // create interpolated center vertex
            //---------------------------------------
            //if(!fields.empty())
                create_dual_vertex(cell, lattice);
            //else
            //    interpolate_cell(cell,lattice);

            // Fill Edges That May Be Shared If They Don't Exist
            if(!cell->edge[UL]){
                cell->edge[UL] = new Edge3D(true, cell, UL);
                cell->edge[UL]->v1 = cell->vert[ULF];
                cell->edge[UL]->v2 = cell->vert[ULB];
            }
            if(!cell->edge[UR]){
                cell->edge[UR] = new Edge3D(true, cell, UR);
                cell->edge[UR]->v1 = cell->vert[URF];
                cell->edge[UR]->v2 = cell->vert[URB];
            }
            if(!cell->edge[UF]){
                cell->edge[UF] = new Edge3D(true, cell, UF);
                cell->edge[UF]->v1 = cell->vert[ULF];
                cell->edge[UF]->v2 = cell->vert[URF];
            }
            if(!cell->edge[UB]){
                cell->edge[UB] = new Edge3D(true, cell, UB);
                cell->edge[UB]->v1 = cell->vert[ULB];
                cell->edge[UB]->v2 = cell->vert[URB];
            }
            if(!cell->edge[LL]){
                cell->edge[LL] = new Edge3D(true, cell, LL);
                cell->edge[LL]->v1 = cell->vert[LLF];
                cell->edge[LL]->v2 = cell->vert[LLB];
            }
            if(!cell->edge[LR]){
                cell->edge[LR] = new Edge3D(true, cell, LR);
                cell->edge[LR]->v1 = cell->vert[LRF];
                cell->edge[LR]->v2 = cell->vert[LRB];
            }
            if(!cell->edge[LF]){
                cell->edge[LF] = new Edge3D(true, cell, LF);
                cell->edge[LF]->v1 = cell->vert[LLF];
                cell->edge[LF]->v2 = cell->vert[LRF];
            }
            if(!cell->edge[LB]){
                cell->edge[LB] = new Edge3D(true, cell, LB);
                cell->edge[LB]->v1 = cell->vert[LLB];
                cell->edge[LB]->v2 = cell->vert[LRB];
            }
            if(!cell->edge[FL]){
                cell->edge[FL] = new Edge3D(true, cell, FL);
                cell->edge[FL]->v1 = cell->vert[LLF];
                cell->edge[FL]->v2 = cell->vert[ULF];
            }
            if(!cell->edge[FR]){
                cell->edge[FR] = new Edge3D(true, cell, FR);
                cell->edge[FR]->v1 = cell->vert[LRF];
                cell->edge[FR]->v2 = cell->vert[URF];
            }
            if(!cell->edge[BL]){
                cell->edge[BL] = new Edge3D(true, cell, BL);
                cell->edge[BL]->v1 = cell->vert[LLB];
                cell->edge[BL]->v2 = cell->vert[ULB];
            }
            if(!cell->edge[BR]){
                cell->edge[BR] = new Edge3D(true, cell, BR);
                cell->edge[BR]->v1 = cell->vert[LRB];
                cell->edge[BR]->v2 = cell->vert[URB];
            }

            // Fill Rest
            cell->edge[DULF] = new Edge3D(cell, DULF);
            cell->edge[DULF]->v1 = cell->vert[C];
            cell->edge[DULF]->v2 = cell->vert[ULF];

            cell->edge[DULB] = new Edge3D(cell, DULB);
            cell->edge[DULB]->v1 = cell->vert[C];
            cell->edge[DULB]->v2 = cell->vert[ULB];

            cell->edge[DURF] = new Edge3D(cell, DURF);
            cell->edge[DURF]->v1 = cell->vert[C];
            cell->edge[DURF]->v2 = cell->vert[URF];

            cell->edge[DURB] = new Edge3D(cell, DURB);
            cell->edge[DURB]->v1 = cell->vert[C];
            cell->edge[DURB]->v2 = cell->vert[URB];

            cell->edge[DLLF] = new Edge3D(cell, DLLF);
            cell->edge[DLLF]->v1 = cell->vert[C];
            cell->edge[DLLF]->v2 = cell->vert[LLF];

            cell->edge[DLLB] = new Edge3D(cell, DLLB);
            cell->edge[DLLB]->v1 = cell->vert[C];
            cell->edge[DLLB]->v2 = cell->vert[LLB];

            cell->edge[DLRF] = new Edge3D(cell, DLRF);
            cell->edge[DLRF]->v1 = cell->vert[C];
            cell->edge[DLRF]->v2 = cell->vert[LRF];

            cell->edge[DLRB] = new Edge3D(cell, DLRB);
            cell->edge[DLRB]->v1 = cell->vert[C];
            cell->edge[DLRB]->v2 = cell->vert[LRB];

            //--------------------------------------------------
            // Fill Faces That Will Not Be Shared
            //--------------------------------------------------
            for(int f=0; f < 12; f++)
                cell->face[f] = new Face3D(cell, f);
        }
    }


    return new_cell;
}

static void create_dual_vertex(OTCell *cell, BCCLattice3D *lattice)
{
    int m = lattice->materials();

    cell->vert[C] = new Vertex3D(m, cell, C);
    //cell->vert[C]->vals = new float[m];
    //memset(cell->vert[C]->vals, 0, m*sizeof(float));
    float x = cell->vert[C]->pos().x = cell->xLocCode + 0.5f;
    float y = cell->vert[C]->pos().y = cell->yLocCode + 0.5f;
    float z = cell->vert[C]->pos().z = cell->zLocCode + 0.5f;

    int dom = -1;
    double max = -10000;

    for(int mat=0; mat < m; mat++){

        // grab value from field

        //cell->vert[C]->vals[mat] = fields[mat]->valueAt(x,y,z);
        //cell->vert[C]->vals[mat] = field->value(cell->vert[C]->pos().x,
        //                                        cell->vert[C]->pos().y,
        //                                        cell->vert[C]->pos().z,mat);

        bool found = false;
        for(int i=0; i < 8; i++){
            found = cell->vert[i]->lbls[mat];
            if(found)
                break;
        }

        // only allow maximums that occured on corners
        if(!found)
            continue;

        float value = lattice->volume->valueAt(x,y,z,mat);
        if(value > max || dom < 0){
            max = value;
            dom = mat;
        }
    }

    // make sure no values are equivalent to max
    /*
    for(int mat=0; mat < m; mat++){
        float value = lattice->fields->valueAt(x,y,z,mat);
        if(value == max && mat != dom){
            float diff = 1E-6*fabs(max); diff = fmax(1E-6, diff);
            float new_value = max - diff;
            lattice->fields->setNewValueAt(new_value, x, y, z, mat);
        }
    }
    */

    //max = cell->vert[C]->vals[dom] += 10.1;

    // now double check values aren't equivalent
    /*
    int max_count = 0;
    for(int mat=0; mat < m; mat++){
        float value = lattice->fields->valueAt(x,y,z,mat);
        if(value == max){
            max_count++;
        }
    }
    if(max_count != 1)
        cerr << "Problem!! - Dual Vertex has no clear max value" << endl;
    */

    cell->vert[C]->lbls[dom] = true;
    cell->vert[C]->label = dom;
}


}
