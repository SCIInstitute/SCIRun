//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- BCC Lattice
//
// Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, Jonathan Bronson
//  Scientific Computing  &  Imaging Institute
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

#ifndef BCCLATTICE3D_H
#define BCCLATTICE3D_H

#include <cstring>
#include <vector>
#include "Octree.h"
#include "vec3.h"
#include "Vertex.h"

namespace Cleaver
{

class AbstractVolume;
class ScalarField;
class Poly3D;
class Tet3D;
class Face3D;
class Edge3D;
class Octree;
class OTCell;
class BCCLattice3D;
class Tet;

class Edge3D : public Geometry{

public:
    Edge3D():v1(0),v2(0),cut(0),isLong(false),evaluated(false){}
    Edge3D(OTCell *cell, int index):cell(cell), v1(0), v2(0), cut(0), edge_index(index), isLong(false), evaluated(false){}
    Edge3D(bool isLong, OTCell *cell, int index):cell(cell), v1(0), v2(0), cut(0), edge_index(index), isLong(isLong), evaluated(false){}
    ~Edge3D();

    OTCell *cell;    
    Vertex3D *v1, *v2;      // two ordered vertices adjacent to edge
    Vertex3D *cut;

    unsigned char edge_index;
    bool isLong:1;
    bool evaluated:1;

    float length()
    {
        return (float)L2(v1->pos() - v2->pos());
    }

    bool contains(Vertex3D *vertex)
    {
        if(this->v1 == vertex || this->v2 == vertex)
            return true;
        else
            return false;
    }

    bool containsBoth(Vertex3D *v1, Vertex3D *v2)
    {
        if((this->v1 == v1 || this->v2 == v1) &&
           (this->v1 == v2 || this->v2 == v2))
            return true;
        else
            return false;
    }


};

class Face3D : public Geometry{
public:
    Face3D(Vertex3D *t):triple(t),evaluated(true){ }
    Face3D():triple(NULL),evaluated(false){ }
    Face3D(OTCell *cell, int index):cell(cell), triple(NULL), face_index(index), evaluated(false){ }
    ~Face3D();

    OTCell *cell;
    Vertex3D *triple;
    unsigned char face_index;
    bool evaluated:1;
};

class Tet3D : public Geometry{
public:
    Tet3D():quad(NULL),tm_index(0),key(0),evaluated(false),stenciled(false){}
    Tet3D(int index):quad(NULL),tm_index(0),tet_index(index),key(0),evaluated(false),stenciled(false){}
    Tet3D(OTCell *cell,int index):cell(cell),quad(NULL), tm_index(0),tet_index(index),key(0),evaluated(false),stenciled(false){}
    ~Tet3D();

    OTCell *cell;    
    Vertex3D *quad;

    int *tm_index;
    unsigned char tet_index;
    unsigned char key;

    bool evaluated:1;
    bool stenciled:1;
};

class BCCLattice3D
{
public:
    //BCCLattice3D(int width, int height, int depth, int materials);
    //BCCLattice3D(int width, int height, int depth, int materials, float *data);
    BCCLattice3D(const AbstractVolume *volume);
    ~BCCLattice3D();

    const AbstractVolume *volume;
    Octree *tree;              // octree structure
    unsigned char *labels;     // dominant materials

    std::vector<OTCell *> cut_cells;    // cells that actually have cuts in them
    std::vector<OTCell *> buffer_cells; // cells that are adjacent to ones with cuts

    int materials() const { return m_iNumMaterials; }
    int width() const { return m_iWidth; }
    int height() const { return m_iHeight; }
    int depth() const { return m_iDepth; }

    float a_long;
    float a_short;
    float theta_long;           // angle corresponding to alpha_long
    float theta_short;          // angle corresponding to alpha_short
    float long_length;
    float short_length;
    static int MaxNumTets;

    unsigned char keyFromAdjacentEdges(Edge3D *edges[6]);
    unsigned char generalizedKey(const Tet3D *tet);

    bool isKeyValid(unsigned char key);


    void getCellsAroundVertex(const OTCell *cell, int index, OTCell *cells[8]);
    void getCellsAroundVertex(Vertex3D *vertex, OTCell *cell_list[8]);

    void getEdgesAroundVertex(const OTCell *cell, int index, Edge3D *edges[14]);
    void getEdgesAroundVertex(const Vertex3D *vertex, Edge3D *edges[14]);
    void getFacesAroundVertex(const OTCell *cell, int index, Face3D *faces[36]);
    void getFacesAroundVertex(const Vertex3D *vertex, Face3D *faces[36]);
    void getTetsAroundVertex(const OTCell *cell, int index, Tet3D *tets[24]);
    void getTetsAroundVertex(const Vertex3D *vertex, Tet3D *tets[24]);

    void getFacesAroundEdge(const Edge3D *edge, Face3D *faces[6], unsigned int *count);
    void getTetsAroundEdge(const Edge3D *edge, Tet3D *tets[6], unsigned int *count);

    void getVertsAroundFace(const OTCell *cell, int face_index, Vertex3D *verts[3]);
    void getVertsAroundFace(const Face3D *face, Vertex3D *verts[3]);
    void getEdgesAroundFace(const OTCell *cell, int face_index, Edge3D *edges[3]);
    void getEdgesAroundFace(const Face3D *face, Edge3D *edges[3]);
    void getTetsAroundFace(const Face3D *face, Tet3D *tets[2]);

    void getVertsAroundTet(const Tet3D *tet, Vertex3D *verts[4]);
    void getEdgesAroundTet(const Tet3D *tet, Edge3D *edges[6]);
    void getFacesAroundTet(const Tet3D *tet, Face3D *faces[4]);

    void getAdjacencyLists(const Face3D *face, Vertex3D *verts[3], Edge3D *edges[3]);
    void getAdjacencyLists(const Tet3D *tet, Vertex3D *verts[4], Edge3D *edges[6], Face3D *faces[4]);

    void getRightHandedVertexList(const Tet3D *tet, Vertex3D *verts[15]);
    Vertex3D* getGeneralizedVertex(const Tet3D *tet, const int index);

    bool contains(const Face3D *face, Vertex3D *vertex);
    bool contains(const Face3D *face, Edge3D *edge);
    bool contains(const Tet3D *tet, Vertex3D *vertex);
    bool contains(const Tet3D *tet, const Edge3D *edge);
    bool contains(const Tet3D *tet, const Face3D *face);

    Tet3D* getInnerTet(const Face3D *face, const Vertex3D *warp_vertex, const vec3 warp_pt);
    Tet3D* getInnerTet(const Edge3D *edge, const Vertex3D *warp_vertex, const vec3 warp_pt);
    Tet3D* getOppositeTet(const Tet3D *tet, const Face3D *face);

    void getFacesAroundEdgeOnTet(const Tet3D *tet, const Edge3D *edge, Face3D *faces[2]);


    //-------- State Setters -------------//
    void setDataLoaded(bool state){ m_bDataLoaded = state; }
    void setCutsComputed(bool state){ m_bCutsComputed = state; }
    void setTriplesComputed(bool state){ m_bTriplesComputed = state; }
    void setQuadsComputed(bool state){ m_bQuadsComputed = state; }
    void setGeneralized(bool state){ m_bGeneralized = state; }
    void setStenciled(bool state){ m_bStenciled = state; }

    void setPhase1Complete(bool state){ m_bPhase1Complete = state; }
    void setPhase2Complete(bool state){ m_bPhase2Complete = state; }
    void setPhase3Complete(bool state){ m_bPhase3Complete = state; }

    void setPhase2ViolationsFound(bool state){ m_bPhase2ViolationsFound = state; }
    void setPhase3ViolationsFound(bool state){ m_bPhase3ViolationsFound = state; }

    void setPadded(bool state){ m_bPadded = state; }

    void setDebugMode(bool state)
    {
        m_bDebugMode = state;
        m_currentCell = 0;
        m_currentVertex = 0;
    }

    //-------- State Getters -------------//
    bool dataLoaded(){ return m_bDataLoaded; }
    bool cutsComputed(){ return m_bCutsComputed; }
    bool triplesComputed(){ return m_bTriplesComputed; }
    bool quadsComputed(){ return m_bQuadsComputed; }
    bool generalized(){ return m_bGeneralized; }
    bool stenciled(){ return m_bStenciled; }

    bool phase1Complete(){ return m_bPhase1Complete; }
    bool phase2Complete(){ return m_bPhase2Complete; }
    bool phase3Complete(){ return m_bPhase3Complete; }

    bool padded(){ return m_bPadded; }

    vec3 origin(){ return m_origin; }
    vec3* frame(){ return m_frame; }

    std::vector<Vertex3D*> *verts;
    std::vector<Tet*> *tets;


private:
    int m_iNumMaterials;    // number of materials in volume
    int m_iWidth;           // width of grid
    int m_iHeight;          // height of grid
    int m_iDepth;           // depth of grid

    vec3 m_origin;  // origin
    vec3 m_frame[3];   // coordinate frame

    //---- State of Data -----//
    bool m_bDataLoaded;
    bool m_bCutsComputed;
    bool m_bTriplesComputed;
    bool m_bQuadsComputed;
    bool m_bGeneralized;
    bool m_bStenciled;
    bool m_bPhase1Complete;
    bool m_bPhase2Complete;
    bool m_bPhase3Complete;
    bool m_bPhase2ViolationsFound;
    bool m_bPhase3ViolationsFound;
    bool m_bPadded;

    bool m_bDebugMode;
    bool m_bStepComplete;
    bool m_bSubStepComplete;
    //------------------------//

    //----- Debug Data -------//
    int m_currentCell;
    int m_currentVertex;
    int m_currentEdge;
    int m_currentFace;
    int m_currentTet;
    int m_currentStep;
    int m_currentSubStep;
    //------------------------//
};


#define DATA3D(a,b,c,m,mat) data[m*(a + b*w + c*w*h) + mat]
#define LBL3D(a,b,c)    labels[(a + b*w + c*w*h)]
#define LBL3DOCTREE(a,b,c)    labels[(a + b*(w+1) + c*(w+1)*(h+1))]
#define DUAL3D(a,b,c)   dual[a + b*(w-1) + c*(w-1)*(h-1)]

#define MAIN_HORI(a,b,c) main_hori[a + b*(w-1) + c*(w-1)*h]
#define MAIN_VERT(a,b,c) main_vert[a + b*w + c*w*(h-1)]
#define MAIN_DEEP(a,b,c) main_deep[a + b*w + c*(w-1)*(h-1)]

#define DUAL_HORI(a,b,c) dual_hori[a + b*(w-1) + c*(w-1)*h]
#define DUAL_VERT(a,b,c) dual_vert[a + b*w + c*w*(h-1)]
#define DUAL_DEEP(a,b,c) dual_deep[a + b*w + c*(w-1)*(h-1)]


#define DIAG_ULF(a,b,c) diagULF[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_ULB(a,b,c) diagULB[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_URF(a,b,c) diagURF[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_URB(a,b,c) diagURB[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_LLF(a,b,c) diagLLF[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_LLB(a,b,c) diagLLB[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_LRF(a,b,c) diagLRF[a + b*(w-1) + c*(w-1)*(h-1)]
#define DIAG_LRB(a,b,c) diagLRB[a + b*(w-1) + c*(w-1)*(h-1)]

}

#endif // BCCLATTICE3D_H
