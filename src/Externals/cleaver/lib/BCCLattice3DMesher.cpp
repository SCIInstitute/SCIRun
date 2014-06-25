//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- BCC Lattice Mesher Class
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2011, 2012, 2013, Jonathan Bronson
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

#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include "BCCLattice3DMesher.h"
#include "GeneralizedInterfaceTable.h"
#include "GeneralizedMaterialTable.h"
#include "GeneralizedStencilTable.h"
#include "GeneralizedVertexTable.h"
#include "Volume.h"

using namespace std;
using namespace Cleaver;

#ifndef PI
#define PI 3.14159265
#endif

//===================================================
//  mesh()
//
//  Single call to run the entire meshing routine.
//===================================================
TetMesh* BCCLattice3DMesher::mesh(bool snap, bool verbose)
{
    //---------------------------------
    //          Start Timer
    //---------------------------------
    unsigned long time_start = clock();

    //--------------------------------
    //   Grab Current User Settings
    //--------------------------------
    lattice->a_long = alpha_long;
    lattice->a_short = alpha_short;
    lattice->long_length = 1.0;
    lattice->short_length = (float)L2(vec3(0.5, 0.5, 0.5));

    //--------------------------------
    //   Compute All Intersections
    //--------------------------------
    compute_all_cuts();
    if(verbose)
        cout << "Cuts Computed" << endl;
    compute_all_trips();
    if(verbose)
        cout << "Triples Computed" << endl;
    compute_all_quads();
    if(verbose)
        cout << "Quads Computed" << endl;

    //--------------------------------
    //  Free Label Data From Memory
    //--------------------------------
    delete[] lattice->labels;
    lattice->labels = NULL;
    for(unsigned int i=0; i < lattice->cut_cells.size(); i++)
    {
        Vertex3D *dual = lattice->cut_cells[i]->vert[C];
        delete[] dual->lbls;
        dual->lbls = NULL;
    }
    for(unsigned int i=0; i < lattice->buffer_cells.size(); i++)
    {
        Vertex3D *dual = lattice->buffer_cells[i]->vert[C];        
        delete[] dual->lbls;        
        dual->lbls = NULL;
    }

    //--------------------------------
    //    Generalize Lattice Tets
    //--------------------------------
    generalize_tets();
    if(verbose)
        cout << "Tets Generalized" << endl;

    //------------------------
    //     Snap To Verts
    //------------------------
    if(snap)
    {
        warp_violating_cuts();
        if(verbose)
            cout << "Phase 1 Complete"  << endl;

        //------------------------
        //     Snap to Edges
        //------------------------
        detect_trips_violating_edges();
        detect_quads_violating_edges();
        warp_violating_trips();
        if(verbose)
            cout << "Phase 2 Complete"  << endl;

        //------------------------
        //     Snap to Faces
        //------------------------
        detect_quads_violating_faces();
        warp_violating_quads();
        if(verbose)
            cout << "Phase 3 Complete"  << endl;
    }

    //-------------------------
    //  Fill in Tet Stencils
    //-------------------------
    tm = new TetMesh(*lattice->verts,*lattice->tets);
    //tm->include_bg_tets = false;
    fill_all_stencils();

    if(verbose)
        std::cout << "Stenciling Complete"  << std::endl;
    if(verbose)
        std::cout << "Mesh Construction Complete" << std::endl;

    //-------------------
    //    Stop Timer
    //-------------------
    unsigned long time_stop = clock();

    tm->time = double(time_stop - time_start) / CLOCKS_PER_SEC;

    if(verbose)
        std::cout << "Total Time: " << tm->time << " s" << std::endl;
    return tm;
}

//=========================================================
// compute_all_cuts()
//
// This method loops over the lattice and computes all
// cutpoints (material transitions). These are saved as
// vertices, and stored in two lists. An edge list for
// algorithmic lookup, and a global list used solely for
// visualization.
//=========================================================
void BCCLattice3DMesher::compute_all_cuts()
{
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){

        OTCell *cell = lattice->cut_cells[c];

        for(int e=0; e < EDGES_PER_CELL; e++)
        {
            Edge3D *edge = cell->edge[e];
            if(!edge)
            {
                cerr << "Problem:  Material Transitions found on boundary." << endl;
                cerr << "Rerun with padding" << endl;
                exit(0);
            }

            if(!edge->evaluated)
                compute_cut(cell->edge[e]);
        }
    }

    lattice->setCutsComputed(true);
}


//==================================================
//  compute_all_trips()
//
// This method iterates over the lattice and finds
// any triangle that has three material transitions.
// Such a triangle MUST contain a triplepoint. A
// pointer to this triple is stored in a list and
// a call is made to detect if the triple violates.
//==================================================
void BCCLattice3DMesher::compute_all_trips()
{
    for(unsigned int c = 0; c < lattice->cut_cells.size(); c++){

        OTCell *cell = lattice->cut_cells[c];

        for(int f=0; f < FACES_PER_CELL; f++)
        {
            Face3D *face = cell->face[f];

            if(!face->evaluated)
                compute_triple(face);
        }
    }

    lattice->setTriplesComputed(true);
}


//==================================================
//  compute_all_quads()
//
// This method iterates over the lattice and finds
// any tet that has four material transitions.
// Such a tet MUST contain a quadpoint. A
// pointer to this triple is stored in a list and
// a call is made to detect if the triple violates.
//==================================================
void BCCLattice3DMesher::compute_all_quads()
{
    for(unsigned int c = 0; c < lattice->cut_cells.size(); c++){

        OTCell *cell = lattice->cut_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            Tet3D *tet = cell->tets[t];

            if(!tet->evaluated)
                compute_quadruple(tet);
        }
    }

    lattice->setQuadsComputed(true);
}


//=========================================================
// compute_cut()
//
// This method takes in two vertices and the edge that
// spans them, and if present, computes the location of
// the cutpoint along that edge, and fills the structures
// with the appropriate information.
//=========================================================
void BCCLattice3DMesher::compute_cut(Edge3D *edge)
{
    Vertex3D *v1 = edge->v1;
    Vertex3D *v2 = edge->v2;

    edge->evaluated = true;
    if(!isTransition(v1->lbls, v2->lbls))
        return;

    int a_mat = v1->label;
    int b_mat = v2->label;

    double a1 = lattice->volume->valueAt(v1->pos(), a_mat); //  v1->vals[a_mat];
    double a2 = lattice->volume->valueAt(v2->pos(), a_mat); //  v2->vals[a_mat];
    double b1 = lattice->volume->valueAt(v1->pos(), b_mat); //  v1->vals[b_mat];
    double b2 = lattice->volume->valueAt(v2->pos(), b_mat); //  v2->vals[b_mat];
    double top = (a1 - b1);
    double bot = (b2 - a2 + a1 - b1);
    double t = top / bot;

    Vertex3D *cut = new Vertex3D(lattice->materials());    
    t = max(t, 0.0);
    t = min(t, 1.0);
    cut->pos() = v1->pos()*(1-t) + v2->pos()*t;



    if(t < 0.5)
        cut->closestGeometry = v1;
    else
        cut->closestGeometry = v2;

    //cut->vals[a_mat] = mat_val;
    //cut->vals[b_mat] = mat_val;
    cut->label = a_mat;         // doesn't really matter which
    cut->lbls[v1->label] = true;
    cut->lbls[v2->label] = true;

    // check violating condition
    double measure;
    if(edge->isLong)
        measure = lattice->a_long;
    else
        measure = lattice->a_short;

    if(t < measure || t > (1 - measure))
        cut->violating = true;
    else
        cut->violating = false;

    //lattice->cuts.push_back(cut);
    edge->cut = cut;
    //cut->m_edge = edge;
    cut->order() = 1;
}

void construct_plane(const vec3 &p1, const vec3 &p2, const vec3 &p3, float &a, float &b, float &c, float &d)
{
    vec3 n = normalize(((p2 - p1).cross(p3 - p1)));
    a = (float)n.x;
    b = (float)n.y;
    c = (float)n.z;
    d = (float)-n.dot(p1);
}

int LATTICE_FACE_AXIS[36] =
{
    1, 1, 1, 1, //  FUL, FUR, FUF, FUB,        // Triangle Faces Touching Upper Lattice Edges
    1, 1, 1, 1, //  FLL, FLR, FLF, FLB,        // Triangle Faces Touching Lower Lattice Edges
    2, 2, 2, 2, //  FFL, FFR, FBL, FBR,        // Triangle Faces Touching Four Column Edges
    2, 2, 2, 2, //  FLUF, FLUB, FLLF, FLLB,    // Triangle Faces Cutting through Left  Face
    2, 2, 2, 2, //  FRUF, FRUB, FRLF, FRLB,    // Triangle Faces Cutting through Right Face
    1, 1, 1, 1, //  FFUL, FFUR, FFLL, FFLR,    // Triangle Faces Cutting through Front Face
    1, 1, 1, 1, //  FBUL, FBUR, FBLL, FBLR,    // Triangle Faces Cutting through Back  Face
    2, 2, 2, 2, //  FUFL, FUFR, FUBL, FUBR,    // Triangle Faces Cutting through Upper Face
    2, 2, 2, 2  //  FDFL, FDFR, FDBL, FDBR     // Triangle Faces Cutting through Lower Face
};

bool is_point_in_triangle(vec3 a, vec3 b, vec3 c, vec3 p)
{
    // Compute vectors
    vec3 v0 = c - a;
    vec3 v1 = b - a;
    vec3 v2 = p - a;

    // Compute dot products
    double dot00 = dot(v0, v0);
    double dot01 = dot(v0, v1);
    double dot02 = dot(v0, v2);
    double dot11 = dot(v1, v1);
    double dot12 = dot(v1, v2);

    // Compute barycentric coordinates
    double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return ((u >= 0) && (v >= 0) && (u + v < 1));
}

void force_point_in_triangle(vec3 a, vec3 b, vec3 c, vec3 &p)
{
    // Compute vectors
    vec3 v0 = c - a;
    vec3 v1 = b - a;
    vec3 v2 = p - a;

    // Compute dot products
    double dot00 = dot(v0, v0);
    double dot01 = dot(v0, v1);
    double dot02 = dot(v0, v2);
    double dot11 = dot(v1, v1);
    double dot12 = dot(v1, v2);

    // Compute barycentric coordinates
    double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    double w = 1 - u - v;

    vec3 test = (1 - u - v)*a + v*b + u*c;

    // Check if point is in triangle
    u = max(0.0, u);
    v = max(0.0, v);
    w = max(0.0, w);

    double L1 = u + v + w;
    if(L1 > 0){
        u /= L1;
        v /= L1;
    }

    p = (1 - u - v)*a + v*b + u*c;
}

//==================================================
//  compute_triple()
//
// This method computes a triple point for a given
// triangle. It takes as input the 3 vertices and
// three edges that compose the triangle face. The
// elements should be ordered such that vert[i] is
// opposite to edge[i].
//==================================================
void BCCLattice3DMesher::compute_triple(Face3D *face)
{
    Vertex3D *verts[3];
    Edge3D *edges[3];

    lattice->getAdjacencyLists(face, verts, edges);

    face->evaluated = true;
    if(!edges[0]->cut || !edges[1]->cut || !edges[2]->cut)
        return;

    fixTriangleOrdering(edges, verts);

    //-------------------------------------------------------
    // get coordinates and create Vertex
    //-------------------------------------------------------
    Vertex3D *v1 = verts[0];
    Vertex3D *v2 = verts[1];
    Vertex3D *v3 = verts[2];

    vec3 result = vec3::zero;

    float a1,b1,c1,d1;
    float a2,b2,c2,d2;
    float a3,b3,c3,d3;

    // get materials
    int m1 = v1->label;
    int m2 = v2->label;
    int m3 = v3->label;

    int axis = LATTICE_FACE_AXIS[face->face_index];

    if(axis == 1)
    {
        vec3 p1_m1 = vec3(v1->pos().x, lattice->volume->valueAt(v1->pos(), m1), v1->pos().z);
        vec3 p2_m1 = vec3(v2->pos().x, lattice->volume->valueAt(v2->pos(), m1), v2->pos().z);
        vec3 p3_m1 = vec3(v3->pos().x, lattice->volume->valueAt(v3->pos(), m1), v3->pos().z);

        vec3 p1_m2 = vec3(v1->pos().x, lattice->volume->valueAt(v1->pos(), m2), v1->pos().z);
        vec3 p2_m2 = vec3(v2->pos().x, lattice->volume->valueAt(v2->pos(), m2), v2->pos().z);
        vec3 p3_m2 = vec3(v3->pos().x, lattice->volume->valueAt(v3->pos(), m2), v3->pos().z);

        vec3 p1_m3 = vec3(v1->pos().x, lattice->volume->valueAt(v1->pos(), m3), v1->pos().z);
        vec3 p2_m3 = vec3(v2->pos().x, lattice->volume->valueAt(v2->pos(), m3), v2->pos().z);
        vec3 p3_m3 = vec3(v3->pos().x, lattice->volume->valueAt(v3->pos(), m3), v3->pos().z);


        //cout << "axis 1" << endl;
        construct_plane(p1_m1, p2_m1, p3_m1, a1,b1,c1,d1);
        construct_plane(p1_m2, p2_m2, p3_m2, a2,b2,c2,d2);
        construct_plane(p1_m3, p2_m3, p3_m3, a3,b3,c3,d3);

        double A[2][2];
        double b[2];

        A[0][0] = (a3/b3 - a1/b1);  //(a3/c3 - a1/c1);
        A[0][1] = (c3/b3 - c1/b1);  //(b3/c3 - b1/c1);
        A[1][0] = (a3/b3 - a2/b2);  //(a3/c3 - a2/c2);
        A[1][1] = (c3/b3 - c2/b2);  //(b3/c3 - b2/c2);

        b[0] = (d1/b1 - d3/b3);     //(d1/c1 - d3/c3);
        b[1] = (d2/b2 - d3/b3);     //(d2/c2 - d3/c3);

        // solve using cramers rule
        vec3 result2d = vec3::zero;

        double det = A[0][0]*A[1][1] - A[0][1]*A[1][0];

        result2d.x = (b[0]*A[1][1] - b[1]*A[0][1])/ det;
        result2d.y = (b[1]*A[0][0] - b[0]*A[1][0])/ det;


        // intersect triangle plane with line (from result2d point along axis)
        vec3 origin(result2d.x, 0, result2d.y);
        vec3 ray(0, 1, 0);
        bool success = plane_intersect(v1, v2, v3, origin, ray, result);
        if(!success)
        {
            cout << "Failed to Project Triple BACK into 3D: Using Barycenter" << endl;
            result = (1.0/3.0)*(v1->pos() + v2->pos() + v3->pos());
        }

    }
    else if(axis == 2)
    {
        vec3 p1_m1 = vec3(v1->pos().x, v1->pos().y, lattice->volume->valueAt(v1->pos(), m1));
        vec3 p2_m1 = vec3(v2->pos().x, v2->pos().y, lattice->volume->valueAt(v2->pos(), m1));
        vec3 p3_m1 = vec3(v3->pos().x, v3->pos().y, lattice->volume->valueAt(v3->pos(), m1));

        vec3 p1_m2 = vec3(v1->pos().x, v1->pos().y, lattice->volume->valueAt(v1->pos(), m2));
        vec3 p2_m2 = vec3(v2->pos().x, v2->pos().y, lattice->volume->valueAt(v2->pos(), m2));
        vec3 p3_m2 = vec3(v3->pos().x, v3->pos().y, lattice->volume->valueAt(v3->pos(), m2));

        vec3 p1_m3 = vec3(v1->pos().x, v1->pos().y, lattice->volume->valueAt(v1->pos(), m3));
        vec3 p2_m3 = vec3(v2->pos().x, v2->pos().y, lattice->volume->valueAt(v2->pos(), m3));
        vec3 p3_m3 = vec3(v3->pos().x, v3->pos().y, lattice->volume->valueAt(v3->pos(), m3));

        //cout << "axis 2" << endl;
        construct_plane(p1_m1, p2_m1, p3_m1, a1,b1,c1,d1);
        construct_plane(p1_m2, p2_m2, p3_m2, a2,b2,c2,d2);
        construct_plane(p1_m3, p2_m3, p3_m3, a3,b3,c3,d3);

        double A[2][2];
        double b[2];

        A[0][0] = (a3/c3 - a1/c1);
        A[0][1] = (b3/c3 - b1/c1);
        A[1][0] = (a3/c3 - a2/c2);
        A[1][1] = (b3/c3 - b2/c2);

        b[0] = (d1/c1 - d3/c3);
        b[1] = (d2/c2 - d3/c3);

        // solve using cramers rule
        vec3 result2d = vec3::zero;

        double det = A[0][0]*A[1][1] - A[0][1]*A[1][0];

        result2d.x = (b[0]*A[1][1] - b[1]*A[0][1])/ det;
        result2d.y = (b[1]*A[0][0] - b[0]*A[1][0])/ det;

        // intersect triangle plane with line (from result2d point along axis)
        vec3 origin(result2d.x, result2d.y, 0);
        vec3 ray(0, 0, 1);
        bool success = plane_intersect(v1, v2, v3, origin, ray, result);
        if(!success)
        {
            cout << "Failed to Project Triple BACK into 3D. Using barycenter: " << endl;
            result = (1.0/3.0)*(v1->pos() + v2->pos() + v3->pos());
        }

    }

    force_point_in_triangle(v1->pos(), v2->pos(), v3->pos(), result);

    Vertex3D *triple = new Vertex3D(lattice->materials());
    triple->pos() = result;
    triple->lbls[verts[0]->label] = true;
    triple->lbls[verts[1]->label] = true;
    triple->lbls[verts[2]->label] = true;

    triple->order() = TRIP;
    triple->violating = false;
    triple->closestGeometry = NULL;    
    face->triple = triple;

    // check if its violating
    check_triple_violating_lattice(face);
    return;
}


//==================================================
//  compute_quadruple()
//
// This method computes a quad point for a given
// tetrahedra. It takes as input the 4 vertices and
// six edges that compose the tetrahedra.
//
// The elements should be ordered such that vert[i] is
// opposite to face[i].
// -dual        dual vert 1
// -left        dual vert 2
// -right       primal vert 1
// -primal      primal vett 2
// -left
// -right
//==================================================
void BCCLattice3DMesher::compute_quadruple(Tet3D* tet)
{
    Vertex3D *verts[4];
    Edge3D *edges[6];
    Face3D *faces[4];

    lattice->getAdjacencyLists(tet, verts, edges, faces);


    fixTetrahedronOrdering(faces, edges, verts);

    tet->evaluated = true;
    if(!(edges[0]->cut && edges[0]->cut->order() == 1 &&
       edges[1]->cut && edges[1]->cut->order() == 1 &&
       edges[2]->cut && edges[2]->cut->order() == 1 &&
       edges[3]->cut && edges[3]->cut->order() == 1 &&
       edges[4]->cut && edges[4]->cut->order() == 1 &&
       edges[5]->cut && edges[5]->cut->order() == 1))
        return;

    // Create Matrix A and solve for Inverse
    double M[16];
    double inv[16];

    int m1 = verts[0]->label;
    int m2 = verts[1]->label;
    int m3 = verts[2]->label;
    int m4 = verts[3]->label;

    // Fill Material Values
    M[ 0] = lattice->volume->valueAt(verts[0]->pos(), m1);
    M[ 1] = lattice->volume->valueAt(verts[0]->pos(), m2);
    M[ 2] = lattice->volume->valueAt(verts[0]->pos(), m3);
    M[ 3] = lattice->volume->valueAt(verts[0]->pos(), m4);
    M[ 4] = lattice->volume->valueAt(verts[1]->pos(), m1);
    M[ 5] = lattice->volume->valueAt(verts[1]->pos(), m2);
    M[ 6] = lattice->volume->valueAt(verts[1]->pos(), m3);
    M[ 7] = lattice->volume->valueAt(verts[1]->pos(), m4);
    M[ 8] = lattice->volume->valueAt(verts[2]->pos(), m1);
    M[ 9] = lattice->volume->valueAt(verts[2]->pos(), m2);
    M[10] = lattice->volume->valueAt(verts[2]->pos(), m3);
    M[11] = lattice->volume->valueAt(verts[2]->pos(), m4);
    M[12] = lattice->volume->valueAt(verts[3]->pos(), m1);
    M[13] = lattice->volume->valueAt(verts[3]->pos(), m2);
    M[14] = lattice->volume->valueAt(verts[3]->pos(), m3);
    M[15] = lattice->volume->valueAt(verts[3]->pos(), m4);


    // Solve Inverse
    double a0 = M[ 0]*M[ 5] - M[ 1]*M[ 4];
    double a1 = M[ 0]*M[ 6] - M[ 2]*M[ 4];
    double a2 = M[ 0]*M[ 7] - M[ 3]*M[ 4];
    double a3 = M[ 1]*M[ 6] - M[ 2]*M[ 5];
    double a4 = M[ 1]*M[ 7] - M[ 3]*M[ 5];
    double a5 = M[ 2]*M[ 7] - M[ 3]*M[ 6];
    double b0 = M[ 8]*M[13] - M[ 9]*M[12];
    double b1 = M[ 8]*M[14] - M[10]*M[12];
    double b2 = M[ 8]*M[15] - M[11]*M[12];
    double b3 = M[ 9]*M[14] - M[10]*M[13];
    double b4 = M[ 9]*M[15] - M[11]*M[13];
    double b5 = M[10]*M[15] - M[11]*M[14];

    double det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;

    if(det == 0)
    {
        std::cerr << "Failed to compute Quadruple. Determinant Zero. Using Barycenter." << std::endl;

        Vertex3D *quad = new Vertex3D(lattice->materials());

        quad->lbls[verts[0]->label] = true;
        quad->lbls[verts[1]->label] = true;
        quad->lbls[verts[2]->label] = true;
        quad->lbls[verts[3]->label] = true;

        quad->pos() = 0.25*verts[0]->pos() + 0.25*verts[1]->pos() + 0.25*verts[2]->pos() + 0.25*verts[3]->pos();
        quad->order() = QUAD;
        quad->violating = false;
        tet->quad = quad;
        quad->closestGeometry = NULL;
        return;
    }

    double invDet = 1/det;

    inv[ 0] = ( M[ 5]*b5 - M[ 6]*b4 + M[ 7]*b3)*invDet;
    inv[ 1] = (-M[ 1]*b5 + M[ 2]*b4 - M[ 3]*b3)*invDet;
    inv[ 2] = ( M[13]*a5 - M[14]*a4 + M[15]*a3)*invDet;
    inv[ 3] = (-M[ 9]*a5 + M[10]*a4 - M[11]*a3)*invDet;
    inv[ 4] = (-M[ 4]*b5 + M[ 6]*b2 - M[ 7]*b1)*invDet;
    inv[ 5] = ( M[ 0]*b5 - M[ 2]*b2 + M[ 3]*b1)*invDet;
    inv[ 6] = (-M[12]*a5 + M[14]*a2 - M[15]*a1)*invDet;
    inv[ 7] = ( M[ 8]*a5 - M[10]*a2 + M[11]*a1)*invDet;
    inv[ 8] = ( M[ 4]*b4 - M[ 5]*b2 + M[ 7]*b0)*invDet;
    inv[ 9] = (-M[ 0]*b4 + M[ 1]*b2 - M[ 3]*b0)*invDet;
    inv[10] = ( M[12]*a4 - M[13]*a2 + M[15]*a0)*invDet;
    inv[11] = (-M[ 8]*a4 + M[ 9]*a2 - M[11]*a0)*invDet;
    inv[12] = (-M[ 4]*b3 + M[ 5]*b1 - M[ 6]*b0)*invDet;
    inv[13] = ( M[ 0]*b3 - M[ 1]*b1 + M[ 2]*b0)*invDet;
    inv[14] = (-M[12]*a3 + M[13]*a1 - M[14]*a0)*invDet;
    inv[15] = ( M[ 8]*a3 - M[ 9]*a1 + M[10]*a0)*invDet;

    // Multiply inv*1
    double slam[4];
    //transposed to correct matrix for these to work
    slam[0] = inv[ 0] + inv[ 4] + inv[ 8] + inv[12];
    slam[1] = inv[ 1] + inv[ 5] + inv[ 9] + inv[13];
    slam[2] = inv[ 2] + inv[ 6] + inv[10] + inv[14];
    slam[3] = inv[ 3] + inv[ 7] + inv[11] + inv[15];

    double lambda[4], L=0;
    L = slam[0] + slam[1] + slam[2] + slam[3];
    lambda[0] = slam[0] / L;
    lambda[1] = slam[1] / L;
    lambda[2] = slam[2] / L;
    lambda[3] = slam[3] / L;

    // Check that point didn't fall outside face 1
    if(lambda[0] < 0){
        Vertex3D *quad = new Vertex3D(lattice->materials());

        quad->lbls[verts[0]->label] = true;
        quad->lbls[verts[1]->label] = true;
        quad->lbls[verts[2]->label] = true;
        quad->lbls[verts[3]->label] = true;

        quad->order() = QUAD;
        quad->pos() = faces[0]->triple->pos();
        quad->violating = faces[0]->triple->violating;
        quad->closestGeometry = faces[0]->triple->closestGeometry;
        tet->quad = quad;
        return;
    }
    // Check that point didn't fall outside face 2
    if(lambda[1] < 0){
        Vertex3D *quad = new Vertex3D(lattice->materials());

        quad->lbls[verts[0]->label] = true;
        quad->lbls[verts[1]->label] = true;
        quad->lbls[verts[2]->label] = true;
        quad->lbls[verts[3]->label] = true;

        quad->order() = QUAD;
        quad->pos() = faces[1]->triple->pos();
        quad->violating = faces[1]->triple->violating;
        quad->closestGeometry = faces[1]->triple->closestGeometry;
        tet->quad = quad;
        return;
    }
    // Check that point didn't fall outside face 3
    if(lambda[2] < 0){
        Vertex3D *quad = new Vertex3D(lattice->materials());

        quad->lbls[verts[0]->label] = true;
        quad->lbls[verts[1]->label] = true;
        quad->lbls[verts[2]->label] = true;
        quad->lbls[verts[3]->label] = true;

        quad->order() = QUAD;
        quad->pos() = faces[2]->triple->pos();
        quad->violating = faces[2]->triple->violating;
        quad->closestGeometry = faces[2]->triple->closestGeometry;
        tet->quad = quad;
        return;
    }
    // Check that point didn't fall outside face 4
    if(lambda[3] < 0){
        Vertex3D *quad = new Vertex3D(lattice->materials());

        quad->lbls[verts[0]->label] = true;
        quad->lbls[verts[1]->label] = true;
        quad->lbls[verts[2]->label] = true;
        quad->lbls[verts[3]->label] = true;

        quad->order() = QUAD;
        quad->pos() = faces[3]->triple->pos();
        quad->violating = faces[3]->triple->violating;
        quad->closestGeometry = faces[3]->triple->closestGeometry;
        tet->quad = quad;
        return;
    }

    //-------------------------------------------------------
    // If it's inside, get coordinates and create Vertex
    //-------------------------------------------------------
    Vertex3D *quad = new Vertex3D(lattice->materials());
    quad->pos().x = lambda[0]*(verts[0]->pos().x - verts[3]->pos().x)
                + lambda[1]*(verts[1]->pos().x - verts[3]->pos().x)
                + lambda[2]*(verts[2]->pos().x - verts[3]->pos().x)
                + verts[3]->pos().x;
    quad->pos().y = lambda[0]*(verts[0]->pos().y - verts[3]->pos().y)
            + lambda[1]*(verts[1]->pos().y - verts[3]->pos().y)
            + lambda[2]*(verts[2]->pos().y - verts[3]->pos().y)
            + verts[3]->pos().y;
    quad->pos().z = lambda[0]*(verts[0]->pos().z - verts[3]->pos().z)
            + lambda[1]*(verts[1]->pos().z - verts[3]->pos().z)
            + lambda[2]*(verts[2]->pos().z - verts[3]->pos().z)
            + verts[3]->pos().z;

    quad->lbls[verts[0]->label] = true;
    quad->lbls[verts[1]->label] = true;
    quad->lbls[verts[2]->label] = true;
    quad->lbls[verts[3]->label] = true;

    quad->order() = QUAD;
    quad->violating = false;
    quad->closestGeometry = NULL;
    tet->quad = quad;

    // check if its violating
    check_quadruple_violating_lattice(tet);

    return;
}


//=====================================================================================
//  check_cut_violating_lattice()
//
// This method users the rules that dictate whether a cutpoint violates a lattice
// vertex. A cut is violating a vertex if it is within alpha distance to that vertex.
// Only one vertex can be violated at a time, and which vertex is violated is stored.
//=====================================================================================
void BCCLattice3DMesher::check_cut_violating_lattice(Edge3D *edge)
{
    Vertex3D *cut = edge->cut;
    vec3 a = edge->v1->pos();
    vec3 b = edge->v2->pos();
    vec3 c = cut->pos();

    //cut->t = L2(c - a) / L2(b - a);
    double t = L2(c - a) / L2(b - a);

    float alpha;

    if(edge->isLong)
        alpha = lattice->a_long * (lattice->long_length / edge->length());
    else
        alpha = lattice->a_short * (lattice->short_length / edge->length());

    // Check Violations
    if (t <= alpha){
        cut->violating = true;
        cut->closestGeometry = edge->v1;
    }
    else if(t >= (1 - alpha)){
        cut->violating = true;
        cut->closestGeometry = edge->v2;
    }
    else{
        cut->violating = false;
    }
}



//=====================================================================================
//  check_triple_violating_lattice
//
// This method generalizes the rules that dictate whether a cutpoint violates a lattice
// vertex. Similarly, we also want to know when a Triple Point violates such a vertex
// so that it can be included in the warping rules. The generalization follows by
// extending lines from the alpha points, to their opposite corners. The intersection
// of these two edges forms a region of violation for the triple point and a vertex.
// This test must be performed three times, once for each vertex in the triangle. Only
// one vertex can be violated at a time, and which vertex is violated is stored.
//
//=====================================================================================
void BCCLattice3DMesher::check_triple_violating_lattice(Face3D *face)
{
    Edge3D *edges[3];
    Vertex3D *verts[3];
    Vertex3D *triple = face->triple;
    triple->violating = false;
    triple->closestGeometry = NULL;

    //TODO:  VERIFY THESE ARE ORDERED PROPERLY
    lattice->getAdjacencyLists(face, verts, edges);

    fixTriangleOrdering(edges, verts);

    vec3 v0 = verts[0]->pos();
    vec3 v1 = verts[1]->pos();
    vec3 v2 = verts[2]->pos();
    vec3 trip = triple->pos();

    // check v0
    if(!triple->violating){
        vec3 e1 = normalize(v0 - v2);       vec3 e2 = normalize(v0 - v1);
        vec3 t1 = normalize(trip - v2);     vec3 t2 = normalize(trip - v1);

        double alpha1 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        double alpha2 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());

        vec3 c1 = v0*(1-alpha1) + alpha1*v1;
        vec3 c2 = v0*(1-alpha2) + alpha2*v2;

        c1 = normalize(c1 - v2);
        c2 = normalize(c2 - v1);

        if(dot(e1, t1) >= dot(e1, c1) &&
           dot(e2, t2) >= dot(e2, c2)){
            triple->violating = true;
            triple->closestGeometry = verts[0];
            return;
        }
    }

    // check v1
    if(!triple->violating){
        vec3 e1 = normalize(v1 - v0);       vec3 e2 = normalize(v1 - v2);
        vec3 t1 = normalize(trip - v0);     vec3 t2 = normalize(trip - v2);

        double alpha1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short  * (lattice->short_length / edges[0]->length());
        double alpha2 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short  * (lattice->short_length / edges[2]->length());

        vec3 c1 = v1*(1-alpha1) + alpha1*v2;
        vec3 c2 = v1*(1-alpha2) + alpha2*v0;

        c1 = normalize(c1 - v0);
        c2 = normalize(c2 - v2);

        if(dot(e1, t1) >= dot(e1, c1) &&
           dot(e2, t2) >= dot(e2, c2)){
            triple->violating = true;
            triple->closestGeometry = verts[1];
            return;
        }
    }

    // check v2
    if(!triple->violating){
        vec3 e1 = normalize(v2 - v1);      vec3 e2 = normalize(v2 - v0);
        vec3 t1 = normalize(trip - v1);    vec3 t2 = normalize(trip - v0);

        double alpha1 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        double alpha2 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());

        vec3 c1 = v2*(1-alpha1) + alpha1*v0;
        vec3 c2 = v2*(1-alpha2) + alpha2*v1;

        c1 = normalize(c1 - v1);
        c2 = normalize(c2 - v0);

        if(dot(e1, t1) >= dot(e1, c1) &&
           dot(e2, t2) >= dot(e2, c2)){
            triple->violating = true;
            triple->closestGeometry = verts[2];
            return;
        }
    }
}



//=====================================================================================
//  check_quadruple_violating_lattice
//
// This method generalizes the rules that dictate whether a triplepoint violates a lattice
// vertex. Similarly, we also want to know when a Quadruple Point violates such a vertex
// so that it can be included in the warping rules. The generalization follows by
// extending lines from the alpha points, to their opposite corners. The intersection
// of these three edges forms a region of violation for the quadruple point and a vertex.
// This test must be performed four times, once for each vertex in the Lattice Tet. Only
// one vertex can be violated at a time, and which vertex is violated is stored.
//=====================================================================================
void BCCLattice3DMesher::check_quadruple_violating_lattice(Tet3D *tet)
{
    Vertex3D *quad = tet->quad;
    quad->violating = false;

    Vertex3D *verts[4];
    Edge3D *edges[6];
    Face3D *faces[4];

    lattice->getAdjacencyLists(tet, verts, edges, faces);    

    vec3 v1 = verts[0]->pos();
    vec3 v2 = verts[1]->pos();
    vec3 v3 = verts[2]->pos();
    vec3 v4 = verts[3]->pos();
    vec3 q  = quad->pos();

    // check v1 - using edges e1, e2, e3
    if(!quad->violating){
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        vec3 ev1 = (1 - t1)*v1 + t1*v2;
        vec3  n1 = normalize(cross(v3 - ev1, v4 - ev1));
        vec3  q1 = q - ev1;
        double d1 = dot(n1, q1);

        float t2 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        vec3 ev2 = (1 - t2)*v1 + t2*v3;
        vec3  n2 = normalize(cross(v4 - ev2, v2 - ev2));
        vec3  q2 = q - ev2;
        double d2 = dot(n2, q2);

        double t3 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()): lattice->a_short * (lattice->short_length / edges[2]->length());   // edge violating coefficient
        vec3 ev3 = (1 - t3)*v1 + t3*v4;                            // edge violation crosspoint
        vec3  n3 = normalize(cross(v2 - ev3, v3 - ev3));           // normal to plane
        vec3  q3 = q - ev3;                                        // quadruple in locall coordinate fram
        double d3 = dot(n3, q3);                                   // distance from quad to plane

        if(d1 < 0 && d2 < 0 && d3 < 0){
            quad->violating = true;
            quad->closestGeometry = verts[0];
        }
    }

      // check v2 - using edges e1(v1), e4(v3), e6(v4)
    if(!quad->violating){
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        vec3 ev1 = (1 - t1)*v2 + t1*v1;
        vec3  n1 = normalize(cross(v4 - ev1, v3 - ev1));
        vec3  q1 = q - ev1;
        double d1 = dot(n1, q1);

        double t2 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());
        vec3 ev2 = (1 - t2)*v2 + t2*v4;
        vec3  n2 = normalize(cross(v3 - ev2, v1 - ev2));
        vec3  q2 = q - ev2;
        double d2 = dot(n2, q2);

        double t3 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        vec3 ev3 = (1 - t3)*v2 + t3*v3;
        vec3  n3 = normalize(cross(v1 - ev3, v4 - ev3));
        vec3  q3 = q - ev3;
        double d3 = dot(n3, q3);

        if(d1 < 0 && d2 < 0 && d3 < 0){
            quad->violating = true;
            quad->closestGeometry = verts[1];
        }
    }

    // check v3 - using edges e2, e4, e5
    if(!quad->violating){
        double t1 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()): lattice->a_short * (lattice->short_length / edges[1]->length());
        vec3 ev1 = (1 - t1)*v3  + t1*v1;
        vec3  n1 = normalize(cross(v2 - ev1, v4 - ev1));
        vec3  q1 = q - ev1;
        double d1 = dot(n1, q1);

        double t2 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());
        vec3 ev2 = (1 - t2)*v3  + t2*v4;
        vec3  n2 = normalize(cross(v1 - ev2, v2 - ev2));
        vec3  q2 = q - ev2;
        double d2 = dot(n2, q2);

        double t3 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        vec3 ev3 = (1 - t3)*v3  + t3*v2;
        vec3  n3 = normalize(cross(v4 - ev3, v1 - ev3));
        vec3  q3 = q - ev3;
        double d3 = dot(n3, q3);

        if(d1 < 0 && d2 < 0 && d3 < 0){
            quad->violating = true;
            quad->closestGeometry = verts[2];
        }
    }

     // check v4 - using edges e3, e5, e6
    if(!quad->violating){
        double t1 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        vec3 ev1 = (1 - t1)*v4  + t1*v1;
        vec3  n1 = normalize(cross(v3 - ev1, v2 - ev1));
        vec3  q1 = q - ev1;
        double d1 = dot(n1, q1);

        double t2 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());
        vec3 ev2 = (1 - t2)*v4  + t2*v3;
        vec3  n2 = normalize(cross(v2 - ev2, v1 - ev2));
        vec3  q2 = q - ev2;
        double d2 = dot(n2, q2);

        double t3 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());
        vec3 ev3 = (1 - t3)*v4 + t3*v2;
        vec3  n3 = normalize(cross(v1 - ev3, v3 - ev3));
        vec3  q3 = q - ev3;
        double d3 = dot(n3, q3);

        if(d1 < 0 && d2 < 0 && d3 < 0){
            quad->violating = true;
            quad->closestGeometry = verts[3];
        }
    }
}


//======================================================
//  generalize_tets()
//
// This method turns any non 6-cut case lattice tet into
// a generalized 6-cut case, by filling in pointers for
// the cuts / triples / and quads that don't exist, with
// corresponding lower order vertices that do.
//======================================================
void BCCLattice3DMesher::generalize_tets()
{
    //--------------------------------------------------
    //      Generalize All Tets In Cut Cells
    //--------------------------------------------------
    for(unsigned int c = 0; c < lattice->cut_cells.size(); c++){

        OTCell *cell = lattice->cut_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            Tet3D *tet = cell->tets[t];

            // if no quad, start generalization
            if(tet && !tet->quad)
            {
                // look up generalization
                Vertex3D *verts[4];
                Edge3D *edges[6];
                Face3D *faces[4];

                lattice->getAdjacencyLists(tet, verts, edges, faces);
                unsigned char key = tet->key = lattice->keyFromAdjacentEdges(edges);


                if(!lattice->isKeyValid(key)){
                    cout << "BAD TET KEY: " << key << endl;
                    exit(-1);
                }

                int new_index[11];
                if(parity_flip[tet->tet_index])
                {
                    for(int i=0; i < 11; i++)
                        new_index[i] = vertexTableEven[key][i+4];
                }
                else
                {
                    for(int i=0; i < 11; i++)
                        new_index[i] = vertexTableOdd[key][i+4];
                }

                Vertex3D *v[15];
                for(int i=0; i < 4; i++)
                    v[i] = verts[i];

                v[4] = edges[0]->cut;
                v[5] = edges[1]->cut;
                v[6] = edges[2]->cut;
                v[7] = edges[3]->cut;
                v[8] = edges[4]->cut;
                v[9] = edges[5]->cut;
                v[10] = faces[0]->triple;
                v[11] = faces[1]->triple;
                v[12] = faces[2]->triple;
                v[13] = faces[3]->triple;
                v[14] = tet->quad;

                // before applying changes, we can check if we agree existing decision
                // if no quad, some cuts are also missing
                for(int e=0; e < 6; e++){
                    // debug check
                    if(edges[e]->cut != NULL)
                    {
                        if(edges[e]->cut != v[new_index[e]])
                            cout << "Warning: Cut Generalization Disagrees with neighbor tet!" << endl;
                    }
                    edges[e]->cut = v[new_index[e]];
                }

                // if no quad, some triples are also missing
                for(int f=0; f < 4; f++){
                    // debug check
                    if(faces[f]->triple != NULL)
                    {
                        if(faces[f]->triple != v[new_index[f+6]])
                            cout << "Warning: Face Generalization Disagrees with neighbor tet!" << endl;
                    }
                    faces[f]->triple = v[new_index[f+6]];
                }

                // set quad, which we know is missing
                tet->quad = v[new_index[_ABCD-4]];

                // verify that this tet is now generalized
                Vertex3D *v2[15];
                memset(v2, 0, 15*sizeof(Vertex3D*));
                lattice->getRightHandedVertexList(tet, v2);
                for(int i=0; i < 15; i++)
                {
                    if(v2[i] == NULL)
                    {
                        cout << "Problem! Tet failed to generalize" << endl;
                        exit(-1);
                    }
                }

            }
            // end generalization
            else{
                // set key for non-degenerate case
                Edge3D *test_edges[EDGES_PER_TET];
                lattice->getEdgesAroundTet(tet, test_edges);
                tet->key = lattice->keyFromAdjacentEdges(test_edges);
            }
        }
    }


    //--------------------------------------------------
    //      Generalize All Tets In Buffer Cells
    //--------------------------------------------------
    for(unsigned int c = 0; c < lattice->buffer_cells.size(); c++){

        OTCell *cell = lattice->buffer_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            Tet3D *tet = cell->tets[t];
            if(!tet)
                continue;

            // if no quad, start generalization
            if(!tet->quad)
            {
                // look up generalization
                Vertex3D *verts[4];
                Edge3D *edges[6];
                Face3D *faces[4];

                lattice->getAdjacencyLists(tet, verts, edges, faces);
                unsigned char key = tet->key = lattice->keyFromAdjacentEdges(edges);

                if(!lattice->isKeyValid(key)){
                    cout << "BAD TET KEY: " << key << endl;
                    exit(-1);
                }

                int new_index[11];
                if(parity_flip[tet->tet_index])
                {
                    for(int i=0; i < 11; i++)
                        new_index[i] = vertexTableEven[key][i+4];
                }
                else
                {
                    for(int i=0; i < 11; i++)
                        new_index[i] = vertexTableOdd[key][i+4];
                }

                Vertex3D *v[15];
                for(int i=0; i < 4; i++)
                    v[i] = verts[i];

                v[4] = edges[0]->cut;
                v[5] = edges[1]->cut;
                v[6] = edges[2]->cut;
                v[7] = edges[3]->cut;
                v[8] = edges[4]->cut;
                v[9] = edges[5]->cut;
                v[10] = faces[0]->triple;
                v[11] = faces[1]->triple;
                v[12] = faces[2]->triple;
                v[13] = faces[3]->triple;
                v[14] = tet->quad;

                // before applying changes, we can check if we agree existing decision
                // if no quad, some cuts are also missing
                for(int e=0; e < 6; e++){
                    // debug check
                    if(edges[e]->cut != NULL)
                    {
                        if(edges[e]->cut != v[new_index[e]])
                            cout << "Warning: Cut Generalization Disagrees with neighbor tet!" << endl;
                    }
                    edges[e]->cut = v[new_index[e]];
                }

                // if no quad, some triples are also missing
                for(int f=0; f < 4; f++){
                    // debug check
                    if(faces[f]->triple != NULL)
                    {
                        if(faces[f]->triple != v[new_index[f+6]])
                            cout << "Warning: Face Generalization Disagrees with neighbor tet!" << endl;
                    }
                    faces[f]->triple = v[new_index[f+6]];
                }

                // set quad, which we know is missing
                tet->quad = v[new_index[_ABCD-4]];

                // verify that this tet is now generalized
                Vertex3D *v2[15];
                memset(v2, 0, 15*sizeof(Vertex3D*));
                lattice->getRightHandedVertexList(tet, v2);
                for(int i=0; i < 15; i++)
                {
                    if(v2[i] == NULL)
                    {
                        cout << "Unhandled Problem! Tet failed to generalize" << endl;
                        exit(-1);
                    }
                }

            }
            // end generalization
            else{
                // set key for non-degenerate case
                Edge3D *test_edges[EDGES_PER_TET];
                lattice->getEdgesAroundTet(tet, test_edges);
                tet->key = lattice->keyFromAdjacentEdges(test_edges);
            }
        }
    }

    lattice->setGeneralized(true);
}


//==========================================
//  fill_all_stencils()
//
// This method iterates over the lattice and
// uses the material transitions and cuts to
// generate a key that looksup what tetrahedra
// stencil must be used.
//==========================================
void BCCLattice3DMesher::fill_all_stencils()
{
    //--------------------------------------
    //      fill cut stencils
    //--------------------------------------
    for(unsigned int c = 0; c < lattice->cut_cells.size(); c++)
    {
        OTCell *cell = lattice->cut_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            Tet3D *tet = cell->tets[t];
            if(!tet->stenciled)
                fill_stencil(tet);
        }
    }

    //--------------------------------------
    //      fill buffer stencils
    //--------------------------------------
    for(unsigned int c = 0; c < lattice->buffer_cells.size(); c++)
    {
        OTCell *cell = lattice->buffer_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            Tet3D *tet = cell->tets[t];
            if(tet && !tet->stenciled)
                fill_stencil(tet);
        }
    }

    lattice->setStenciled(true);
}

//======================================================
//  fill_stencil()
//
// This method fills a single lattice cell with the
// appropriate stencil to match its cuts, trips, quad
// The output tets are stored in the passed in Tet3D.
//======================================================
void BCCLattice3DMesher::fill_stencil(Tet3D *tet)
{
    Vertex3D *verts[15];
    lattice->getRightHandedVertexList(tet, verts);
    unsigned char key = 63; // tet->key;

    if(parity_flip[tet->tet_index])
    {

        for(int t=0; t < 24; t++)
        {
            //--------------------------------------
            //  Procede Only If Should Output Tets
            //--------------------------------------
            if(stencilTableOdd[key][t][0] == _O)
                break;

            //-----------------------
            //     Get Vertices
            //-----------------------
            Vertex3D *v1 = verts[stencilTableOdd[key][t][0]]->root();  // grabbing root ensures uniqueness
            Vertex3D *v2 = verts[stencilTableOdd[key][t][1]]->root();
            Vertex3D *v3 = verts[stencilTableOdd[key][t][2]]->root();
            Vertex3D *v4 = verts[stencilTableOdd[key][t][3]]->root();
            Vertex3D *vM = verts[materialTableOdd[key][t]]->root();

            //-----------------------
            //  Check If Degenerate
            //-----------------------
            if(v1 == v2 || v1 == v3 || v1 == v4 || v2 == v3 || v2 == v4 || v3 == v4)
                continue;

            //----------------------------
            //  Create Tet + Add to List
            //----------------------------
            Tet *st = lattice->tree->createTet(v1, v2, v3, v4, (int)vM->label);
            st->key = tet->key;
        }

    }
    else
    {

        for(int t=0; t < 24; t++)
        {
            //--------------------------------------
            //  Procede Only If Should Output Tets
            //--------------------------------------
            if(stencilTableEven[key][t][0] == _O)
                break;

            //-----------------------
            //     Get Vertices
            //-----------------------
            Vertex3D *v1 = verts[stencilTableEven[key][t][0]]->root();
            Vertex3D *v2 = verts[stencilTableEven[key][t][1]]->root();
            Vertex3D *v3 = verts[stencilTableEven[key][t][2]]->root();
            Vertex3D *v4 = verts[stencilTableEven[key][t][3]]->root();
            Vertex3D *vM = verts[materialTableEven[key][t]]->root();

            //-----------------------
            //  Check If Degenerate
            //-----------------------
            if(v1 == v2 || v1 == v3 || v1 == v4 || v2 == v3 || v2 == v4 || v3 == v4)
                continue;

            //----------------------------
            //  Create Tet + Add to List
            //----------------------------
            Tet *st = lattice->tree->createTet(v1, v2, v3, v4, vM->label);
            st->key = tet->key;
        }

    }


    tet->stenciled = true;
}


//===================================================
// warp_violating_cuts()
//
// This method warps vertices to remove violations.
// It uses the new rule set, defined in the rules.pdf
// found on the project gforge.
//===================================================
void BCCLattice3DMesher::warp_violating_cuts()
{
    //---------------------------------------------------
    //  Apply vertex warping to all vertices in lattice
    //---------------------------------------------------
    for (unsigned int i=0; i < lattice->cut_cells.size(); i++)
    {
        OTCell *cell = lattice->cut_cells[i];

        for(int v=0; v < VERTS_PER_CELL; v++)
        {
            if(!cell->vert[v]->warped)
                warp_vertex(cell->vert[v]);
        }
    }

    //---------------------------------------------------
    // Buffer Cells can only have cuts on central edges.
    // Therefore, we only need to check the dual vertex
    //---------------------------------------------------
    for (unsigned int i=0; i < lattice->buffer_cells.size(); i++)
    {
        OTCell *cell = lattice->buffer_cells[i];

        if(!cell->vert[C]->warped)
            warp_vertex(cell->vert[C]);
    }

    lattice->setPhase1Complete(true);
}



//===================================================
// warp_vertex()
//
// This method warps the given vertex to remove
// violations. It uses the new rule set, defined in
// the rules.pdf found on the project gforge.
//===================================================
void BCCLattice3DMesher::warp_vertex(Vertex3D *vertex)
{
    std::vector<Edge3D *>   viol_edges;      // violating cut edges
    std::vector<Face3D *>   viol_faces;      // violating triple-points
    std::vector<Tet3D *>    viol_tets;       // violating quadruple-points

    std::vector<Edge3D *>   part_edges;      // participating cut edges
    std::vector<Face3D *>   part_faces;      // participating triple-points
    std::vector<Tet3D *>    part_tets;       // participating quadruple-points

    std::vector<Edge3D*>    snap_list;


    vertex->warped = true;

    //---------------------------------------------------------
    //   Add Participating & Violating CutPoints  (Edges)
    //---------------------------------------------------------
    Edge3D *edges[EDGES_PER_VERT];
    lattice->getEdgesAroundVertex(vertex, edges);

    for(int e=0; e < EDGES_PER_VERT; e++)
    {
        // TODO: If cut == NULL, Generealization didn't work for this case (Probably a buffer edge)
        if(edges[e] && edges[e]->cut && edges[e]->cut->order() == CUT)
        {
            if(edges[e]->cut->violating && edges[e]->cut->closestGeometry == vertex)
                viol_edges.push_back(edges[e]);
            else
                part_edges.push_back(edges[e]);
        }
    }


    //---------------------------------------------------------
    // Add Participating & Violating TriplePoints   (Faces)
    //---------------------------------------------------------
    Face3D *faces[FACES_PER_VERT];
    lattice->getFacesAroundVertex(vertex, faces);

    for(int f=0; f < FACES_PER_VERT; f++)
    {
        // TODO: If triple == NULL, Generalization didn't work for this case (probably a buffer face)
        if(faces[f] && faces[f]->triple && faces[f]->triple->order() == TRIP)
        {
            if(faces[f]->triple->violating && faces[f]->triple->closestGeometry == vertex)
                viol_faces.push_back(faces[f]);
            else
                part_faces.push_back(faces[f]);
        }
    }

    //---------------------------------------------------------
    // Add Participating & Violating QuaduplePoints   (Tets)
    //---------------------------------------------------------
    Tet3D *tets[TETS_PER_VERT];
    lattice->getTetsAroundVertex(vertex, tets);

    for(int t=0; t < TETS_PER_VERT; t++)
    {
        // TODO: If quad == NULL, Generalization didn't work for this case (probably a buffer tet)
        if(tets[t] && tets[t]->quad && tets[t]->quad->order() == QUAD)
        {
            if(tets[t]->quad->violating && tets[t]->quad->closestGeometry == vertex)
                viol_tets.push_back(tets[t]);
            else
                part_tets.push_back(tets[t]);
        }
    }

    // If no violations, move to next vertex     JRB Changed 1/11/211
    if(!viol_edges.empty() || !viol_faces.empty() || !viol_tets.empty())
    {
        vec3 warp_point = vec3::zero;

        // If 1 Quadpoint is Violating
        if(viol_tets.size() == 1)
            warp_point = viol_tets[0]->quad->pos();
        // If 1 Triplepoint is Violating
        else if(viol_faces.size() == 1)
            warp_point = viol_faces[0]->triple->pos();
        else
            warp_point = centerOfMass(viol_edges, viol_faces, viol_tets);

        //---------------------------------------
        //  Conform Quadruple Pt (if it exists)
        //---------------------------------------
        for(unsigned int q=0; q < part_tets.size(); q++)
        {
            conformQuadruple(part_tets[q], vertex, warp_point);
        }

        //---------------------------------------------------------
        // Project Any TriplePoints That Survived On A Warped Face
        //---------------------------------------------------------
        for(unsigned int f=0; f < part_faces.size(); f++){

            // If There Remains a triple
            if(part_faces[f]->triple->order() == TRIP){

                Tet3D *innerTet = lattice->getInnerTet(part_faces[f], vertex, warp_point);
                Vertex3D *q = innerTet->quad;

                Edge3D *edges[3];
                lattice->getEdgesAroundFace(part_faces[f], edges);

                // conform triple if it's also a quad (would not end up in part_quads list)
                if(q->isEqualTo(part_faces[f]->triple)){
                    conformQuadruple(innerTet, vertex, warp_point);
                }
                // coincide with Quad if it conformed to face
                else if(q->order() == QUAD && q->conformedFace == part_faces[f])
                {
                    part_faces[f]->triple->pos_next() = q->pos_next();
                    part_faces[f]->triple->conformedEdge = NULL;
                }
                // coincide with Quad if it conformed to one of faces edges
                else if(q->order() == QUAD && (q->conformedEdge == edges[0] ||
                                               q->conformedEdge == edges[1] ||
                                               q->conformedEdge == edges[2]))
                {
                  part_faces[f]->triple->pos_next() = q->pos_next();
                  part_faces[f]->triple->conformedEdge = q->conformedEdge;
                }
                // otherwise intersect lattice face with Q-T interface
                else{
                    part_faces[f]->triple->pos_next() = projectTriple(part_faces[f], q, vertex, warp_point);
                    conformTriple(part_faces[f], vertex, warp_point);
                }


                // Sanity Check for NaN
                if(part_faces[f]->triple->pos_next() != part_faces[f]->triple->pos_next())
                {
                    cerr << "Fatal Error:  Triplepoint set to NaN: Failed to project triple, using InnerTet.  Cell = [" << vertex->cell->xLocCode << "," << vertex->cell->yLocCode << "," << vertex->cell->zLocCode << "]" << endl;
                    exit(1445);
                }

                // Sanity Check for Zero
                if(part_faces[f]->triple->pos_next() == vec3::zero)
                {
                    cerr << "Fatal Error:  Triplepoint set to vec3::zero == (0,0,0)" << endl;
                    exit(1452);
                }
            }
        }

        //------------------------------------------------------
        // Project Any Cutpoints That Survived On A Warped Edge
        //------------------------------------------------------
        for(unsigned int e=0; e < part_edges.size(); e++){

            // If There remains a cut
            if(part_edges[e]->cut->order() == CUT){

                Tet3D *innertet = lattice->getInnerTet(part_edges[e], vertex, warp_point);

                Face3D *faces[6] = {0};
                unsigned int face_count = 0;
                lattice->getFacesAroundEdge(part_edges[e], faces, &face_count);

                bool handled = false;
                for(unsigned int f=0; f < face_count; f++)
                {
                    // if triple conformed to this edge, use it's position
                    if(faces[f]->triple->order() == TRIP && faces[f]->triple->conformedEdge == part_edges[e]){
                        part_edges[e]->cut->pos_next() = faces[f]->triple->pos_next();
                        handled = true;
                        break;
                    }
                }

                if(handled && part_edges[e]->cut->pos_next() == vec3::zero)
                    cerr << "Conformed Cut Problem!" << endl;

                // TODO: What about conformedVertex like quadpoint?
                // otherwise compute projection with innerTet
                if(!handled)
                    part_edges[e]->cut->pos_next() = projectCut(part_edges[e], innertet, vertex, warp_point);


                if(part_edges[e]->cut->pos_next() == vec3::zero)
                    cerr << "Cut Projection Problem!" << endl;
            }
        }

        //------------------------------------
        //   Move Vertices To New Positions
        //------------------------------------
        vertex->pos() = warp_point;

        /*
        for(int tst=0; tst < VERTS_PER_CELL; tst++)
            if(vertex->cell->vert[tst]->pos() == vec3::zero || vertex->cell->vert[tst]->pos() != vertex->cell->vert[tst]->pos())
                cerr << "Uncaught Exception: pos == " << (vertex->cell->vert[tst]->pos().toString()).c_str() << endl;
        */

        for (unsigned int e=0; e < part_edges.size(); e++){
            if(part_edges[e]->cut->order() == CUT){
                part_edges[e]->cut->pos() = part_edges[e]->cut->pos_next();
                this->check_cut_violating_lattice(part_edges[e]);
            }
        }
        for (unsigned int f=0; f < part_faces.size(); f++){
            if(part_faces[f]->triple->order() == TRIP){
                part_faces[f]->triple->pos() = part_faces[f]->triple->pos_next();
                this->check_triple_violating_lattice(part_faces[f]);
            }
        }
        for (unsigned int q=0; q < part_tets.size(); q++){
            if(part_tets[q]->quad->order() == QUAD){
                part_tets[q]->quad->pos() = part_tets[q]->quad->pos_next();
                this->check_quadruple_violating_lattice(part_tets[q]);
            }
        }


        //------------------------------------------------------
        // delete cuts of the same interface type
        //------------------------------------------------------        
        for(unsigned int e=0; e < part_edges.size(); e++){

            // check if same as one of the violating interface types
            bool affected = false;

            if(part_edges[e]->cut->order() == CUT)
            {
                for(unsigned int c=0; c < viol_edges.size(); c++)
                {
                    bool same = true;
                    for(int m=0; m < lattice->materials(); m++){
                        if(part_edges[e]->cut->lbls[m] != viol_edges[c]->cut->lbls[m]){
                            same = false;
                            break;
                        }
                    }
                    affected = same;
                    if(affected)
                        break;
                }

                //-----------------------
                // If Affected, Snap it
                //-----------------------
                if(affected)
                {                    
                    snap_cut_to_vertex(part_edges[e]->cut, vertex);                  
                }
            }
        }



        //------------------------------------------------------------------
        //  Delete Cut if Projection Makes it Violate New Vertex Location
        //------------------------------------------------------------------
        for (unsigned int e=0; e < part_edges.size(); e++){
            if(part_edges[e]->cut->order() == CUT && part_edges[e]->cut->violating)
            {
                // if now violating this vertex, snap to it
                if(part_edges[e]->cut->closestGeometry == vertex)
                {                    
                    snap_cut_to_vertex(part_edges[e]->cut, vertex);
                }
                // else if violating an already warped vertex, snap to it
                else if(((Vertex3D*)part_edges[e]->cut->closestGeometry)->warped)
                {                                        
                    snap_cut_to_vertex(part_edges[e]->cut, (Vertex3D*)part_edges[e]->cut->closestGeometry);

                    // Probably should call resolve_degeneracies around vertex(closestGeometry); to be safe
                    resolve_degeneracies_around_vertex((Vertex3D*)part_edges[e]->cut->closestGeometry);         // added 12/7/11
                }
                // else the vertex will be warped and cut snapped when its turn comes
            }
        }


        //---------------------------------------------------------------------
        //  Delete Triple if Projection Makies it Violate New Vertex Location
        //---------------------------------------------------------------------
        for (unsigned int f=0; f < part_faces.size(); f++){
            if(part_faces[f]->triple->order() == TRIP && part_faces[f]->triple->violating)
            {
                // if now violating this vertex, snap to it
                if(part_faces[f]->triple->closestGeometry == vertex)
                {                    
                    snap_triple_to_vertex(part_faces[f]->triple, vertex);
                }
                // else if violating an already warped vertex, snap to it
                else if(((Vertex3D*)part_faces[f]->triple->closestGeometry)->warped)
                {
                    snap_triple_to_vertex(part_faces[f]->triple, (Vertex3D*)part_faces[f]->triple->closestGeometry);

                    // Probably should call resolve_degeneracies around vertex(closestGeometry); to be safe
                    resolve_degeneracies_around_vertex((Vertex3D*)part_faces[f]->triple->closestGeometry);  // added 12/7/11
                }
            }
        }

        //------------------------------------------------------------------------
        //  Delete Quadruple If Projection Makies it Violate New Vertex Location
        //------------------------------------------------------------------------
        for (unsigned int q=0; q < part_tets.size(); q++){
            if(part_tets[q]->quad->order() == QUAD && part_tets[q]->quad->violating && part_tets[q]->quad->closestGeometry == vertex)
            {                
                snap_quad_to_vertex(part_tets[q]->quad, vertex);
            }
        }

        //------------------------
        // Delete Violating Cuts
        //------------------------
        for(unsigned int c=0; c < viol_edges.size(); c++)
        {            
            snap_cut_to_vertex(viol_edges[c]->cut, vertex);
        }

        //--------------------------
        // Delete Violating Trips
        //--------------------------
        for(unsigned int f=0; f < viol_faces.size(); f++)
        {         
            snap_triple_to_vertex(viol_faces[f]->triple, vertex);
        }

        //------------------------------
        // Snap Violating Quadruples
        //------------------------------
        for(unsigned int q=0; q < viol_tets.size(); q++)
        {            
            snap_quad_to_vertex(viol_tets[q]->quad, vertex);
        }

        //--------------------------------------
        //  Resolve Degeneracies Around Vertex
        //--------------------------------------
        resolve_degeneracies_around_vertex(vertex);
    }
}



//=====================================================================================
// - conformQuadruple()
//
//  This method enforces that the location of a quad undergoing a warp remains in
// the interior of the bounding tetrahdron. If it does not, it is 'conformed' to a
// face, edge or vertex, by projecting any negative barycentric coordinates to be zero.
//=====================================================================================
void BCCLattice3DMesher::conformQuadruple(Tet3D *tet, Vertex3D *warp_vertex, const vec3 &warp_pt)
{
    double EPS = 1E-3;

    //-------------------------------------
    //    Compute Barycentric Coordinates
    //-------------------------------------
    Vertex3D *quad = tet->quad;
    Vertex3D *verts[4];
    Edge3D *edges[6];
    Face3D *faces[4];

    lattice->getAdjacencyLists(tet, verts, edges, faces);

    quad->conformedFace = NULL;
    quad->conformedEdge = NULL;
    quad->conformedVertex = NULL;

    for (int i=0; i < 4; i++)
    {
        if (verts[i] == warp_vertex)
        {
            // swap so moving vertex is first in list
            verts[i] = verts[0];
            verts[0] = warp_vertex;
            break;
        }
    }

    // Create Matrix A and solve for Inverse
    double A[3][3];
    vec3 quadruple = quad->pos();
    vec3 inv1,inv2,inv3;
    vec3 v1 = warp_pt;
    vec3 v2 = verts[1]->pos();
    vec3 v3 = verts[2]->pos();
    vec3 v4 = verts[3]->pos();

    if(quadruple != quadruple)
    {
        std::cerr << "PROBLEM!" << std::endl;
    }

    // Fill Coordinate Matrix
    A[0][0] = v1.x - v4.x; A[0][1] = v2.x - v4.x; A[0][2] = v3.x - v4.x;
    A[1][0] = v1.y - v4.y; A[1][1] = v2.y - v4.y; A[1][2] = v3.y - v4.y;
    A[2][0] = v1.z - v4.z; A[2][1] = v2.z - v4.z; A[2][2] = v3.z - v4.z;

    // Solve Inverse
    double det = +A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2])
                 -A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
                 +A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
    double invdet = 1/det;
    inv1.x =  (A[1][1]*A[2][2]-A[2][1]*A[1][2])*invdet;
    inv2.x = -(A[1][0]*A[2][2]-A[1][2]*A[2][0])*invdet;
    inv3.x =  (A[1][0]*A[2][1]-A[2][0]*A[1][1])*invdet;

    inv1.y = -(A[0][1]*A[2][2]-A[0][2]*A[2][1])*invdet;
    inv2.y =  (A[0][0]*A[2][2]-A[0][2]*A[2][0])*invdet;
    inv3.y = -(A[0][0]*A[2][1]-A[2][0]*A[0][1])*invdet;

    inv1.z =  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
    inv2.z = -(A[0][0]*A[1][2]-A[1][0]*A[0][2])*invdet;
    inv3.z =  (A[0][0]*A[1][1]-A[1][0]*A[0][1])*invdet;

    // Multiply Inverse*Coordinate to get Lambda (Barycentric)
    vec3 lambda;
    lambda.x = inv1.x*(quadruple.x - v4.x) + inv1.y*(quadruple.y - v4.y) + inv1.z*(quadruple.z - v4.z);
    lambda.y = inv2.x*(quadruple.x - v4.x) + inv2.y*(quadruple.y - v4.y) + inv2.z*(quadruple.z - v4.z);
    lambda.z = inv3.x*(quadruple.x - v4.x) + inv3.y*(quadruple.y - v4.y) + inv3.z*(quadruple.z - v4.z);

    //--------------------------------------------------------------
    // Is any coordinate negative?
    // If so, make it 0, adjust other weights but keep ratio
    //--------------------------------------------------------------

    double lambda_w = 1.0 - (lambda.x + lambda.y + lambda.z);

    if(lambda.x < EPS){

        // two negatives
        if(lambda.y < EPS){

            lambda.x = 0;
            lambda.y = 0;

            for(int i=0; i < 6; i++){
                if(edges[i]->containsBoth(verts[2],verts[3])){
                    quad->conformedEdge = edges[i];
                    //cout << "conformed to Edge" << endl;
                    break;
                }
            }
        }
        else if(lambda.z < EPS){

            lambda.x = 0;
            lambda.z = 0;

            for(int i=0; i < 6; i++){
                if(edges[i]->containsBoth(verts[1],verts[3])){
                    quad->conformedEdge = edges[i];
                    //cout << "conformed to Edge" << endl;
                    break;
                }
            }
        }
        else if(lambda_w < EPS){

            lambda.x = 0;
            lambda_w = 0;

            for(int i=0; i < 6; i++){
                if(edges[i]->containsBoth(verts[1],verts[2])){
                    quad->conformedEdge = edges[i];
                    //cout << "conformed to Edge" << endl;
                    break;
                }
            }
        }
        // one negative
        else{

            lambda.x = 0;

            for(int i=0; i < 4; i++)
            {
                if(!lattice->contains(faces[i], verts[0])){
                    quad->conformedFace = faces[i];
                    //cout << "Conformed to Face" << endl;
                    break;
                }
            }
        }

    }
    else if(lambda.y < EPS){
        // two negatives
        if(lambda.z < EPS){

            lambda.y = 0;
            lambda.z = 0;

            for(int i=0; i < 6; i++){
                if(edges[i]->containsBoth(verts[0],verts[3])){
                    quad->conformedEdge = edges[i];
                    //cout << "conformed to Edge" << endl;
                    break;
                }
            }
        }
        else if(lambda_w < EPS){

            lambda.y = 0;
            lambda_w = 0;

            for(int i=0; i < 6; i++){
                if(edges[i]->containsBoth(verts[0],verts[2])){
                    quad->conformedEdge = edges[i];
                    //cout << "conformed to Edge" << endl;
                    break;
                }
            }
        }
        // one negative
        else{

            lambda.y = 0;

            for(int i=0; i < 4; i++)
            {
                if(!lattice->contains(faces[i], verts[1])){
                    quad->conformedFace = faces[i];
                    //cout << "Conformed to Face" << endl;
                    break;
                }
            }
        }
    }
    else if(lambda.z < EPS){
        // two negatives
        if(lambda_w < EPS){

            lambda.z = 0;
            lambda_w = 0;

            for(int i=0; i < 6; i++){
                if(edges[i]->containsBoth(verts[0],verts[1])){
                    quad->conformedEdge = edges[i];
                    //cout << "conformed to Edge" << endl;
                    break;
                }
            }
        }
        // one negative
        else{

            lambda.z = 0;

            for(int i=0; i < 4; i++)
            {
                if(!lattice->contains(faces[i], verts[2])){
                    quad->conformedFace = faces[i];
                    //cout << "Conformed to Face" << endl;
                    break;
                }
            }
        }
    }
    else if(lambda_w < EPS)
    {
        // one negative
        lambda_w = 0;

        for(int i=0; i < 4; i++)
        {
            if(!lattice->contains(faces[i], verts[3])){
                quad->conformedFace = faces[i];
                //cout << "Conformed to Face" << endl;
                break;
            }
        }
    }
    else
    {
        quad->conformedFace = NULL;
        quad->conformedEdge =  NULL;
        quad->conformedVertex = NULL;
    }

    if(quad->conformedVertex != NULL){
        cerr << "unhandled exception: quad->conformedVertex != NULL" << endl;
        exit(-1);
    }

    double L1 = lambda.x + lambda.y + lambda.z + lambda_w;
    lambda /= L1;

    // Compute New Triple Coordinate
    quadruple.x = lambda.x*v1.x + lambda.y*v2.x + lambda.z*v3.x + (1.0 - (lambda.x + lambda.y + lambda.z))*v4.x;
    quadruple.y = lambda.x*v1.y + lambda.y*v2.y + lambda.z*v3.y + (1.0 - (lambda.x + lambda.y + lambda.z))*v4.y;
    quadruple.z = lambda.x*v1.z + lambda.y*v2.z + lambda.z*v3.z + (1.0 - (lambda.x + lambda.y + lambda.z))*v4.z;

    quad->pos_next() = quadruple;
}


//--------------------------------------------------------------------------------------------
// projectTriple()
//
// Triplepoints are the endpoints of 1-d interfaces that divide 3 materials. The other endpoint
// must either be another triplepoint or a quadruplepoint. (or some other vertex to which
// one of these has snapped. When a lattice vertex is moved, the triple point must be projected
// so that it remains with the plane of a lattice face. This projection works by intersecting
// this interface edge with the new lattice face plane, generated by warping a lattice vertex.
//--------------------------------------------------------------------------------------------
vec3 BCCLattice3DMesher::projectTriple(Face3D *face, Vertex3D *quad, Vertex3D *warp_vertex, const vec3 &warp_pt)
{   
    Vertex3D *trip = face->triple;
    Vertex3D *verts[3];
    lattice->getVertsAroundFace(face, verts);

    for(int i=0; i < 3; i++)
    {
        if(verts[i] == warp_vertex)
        {
            // swap so moving vertex is first in list
            verts[i] = verts[0];
            verts[0] = warp_vertex;
            break;
        }
    }

    vec3 p_0 = warp_pt;
    vec3 p_1 = verts[1]->pos();
    vec3 p_2 = verts[2]->pos();
    vec3 n = normalize(cross(p_1 - p_0, p_2 - p_0));
    vec3 I_a = trip->pos();
    vec3 I_b = quad->pos();
    vec3 l = I_b - I_a;

    // Check if I_b/I_a (Q/T) interface as collapsed)
    if (length(l) < 1E-5 || dot(l,n) == 0)
        return trip->pos();

    double d = dot(p_0 - I_a, n) / dot(l,n);


    vec3 intersection = I_a + d*l;

    // Debugging code 10/6/11
    /*
    if(intersection.x != intersection.x)
    {
        cerr << "project triple returned NaN interesction point" << endl;        
        exit(-1);
    }
    if(intersection == vec3::zero)
    {
        cerr << "project triple returned Zero intersection point" << endl;
        exit(-1);
    }
    */



    return intersection;
}


//=====================================================================================
// - conformTriple()
//
//  This method enforces that the location of a triple undergoing a warp remains in
// the interior of the bounding triangular face. If it does not, it is 'conformed'
// to the edge, by projecting any negative barycentric coordinates to be zero.
//=====================================================================================
void BCCLattice3DMesher::conformTriple(Face3D *face, Vertex3D *warp_vertex, const vec3 &warp_pt)
{
    double EPS = 1E-3;

    //-------------------------------------
    //    Compute Barycentric Coordinates
    //-------------------------------------

    Vertex3D *trip = face->triple;
    Vertex3D *verts[VERTS_PER_FACE];
    Edge3D *edges[EDGES_PER_FACE];

    // TODO:  rename and use adjacency for "neighbor adajacency"
    lattice->getAdjacencyLists(face, verts, edges);


    fixTriangleOrdering(edges, verts);

    for (int i=0; i < 3; i++)
    {
        if (verts[i] == warp_vertex)
        {
            // swap so moving vertex is first in list
            verts[i] = verts[0];
            verts[0] = warp_vertex;
            break;
        }
    }

    // Create Matrix A and solve for Inverse
    double A[3][3];
    vec3 triple = trip->pos_next();   // was ->pos  8/11/11
    vec3 inv1,inv2,inv3;
    vec3 v1 = warp_pt;     //verts[0]->pos;
    vec3 v2 = verts[1]->pos();
    vec3 v3 = verts[2]->pos();
    vec3 v4 = v1 + normalize(cross(normalize(v3 - v1), normalize(v2 - v1)));

    // Fill Coordinate Matrix
    A[0][0] = v1.x - v4.x; A[0][1] = v2.x - v4.x; A[0][2] = v3.x - v4.x;
    A[1][0] = v1.y - v4.y; A[1][1] = v2.y - v4.y; A[1][2] = v3.y - v4.y;
    A[2][0] = v1.z - v4.z; A[2][1] = v2.z - v4.z; A[2][2] = v3.z - v4.z;

    // Solve Inverse
    double det = +A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2])
                 -A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
                 +A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
    double invdet = 1/det;
    inv1.x =  (A[1][1]*A[2][2]-A[2][1]*A[1][2])*invdet;
    inv2.x = -(A[1][0]*A[2][2]-A[1][2]*A[2][0])*invdet;
    inv3.x =  (A[1][0]*A[2][1]-A[2][0]*A[1][1])*invdet;

    inv1.y = -(A[0][1]*A[2][2]-A[0][2]*A[2][1])*invdet;
    inv2.y =  (A[0][0]*A[2][2]-A[0][2]*A[2][0])*invdet;
    inv3.y = -(A[0][0]*A[2][1]-A[2][0]*A[0][1])*invdet;

    inv1.z =  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
    inv2.z = -(A[0][0]*A[1][2]-A[1][0]*A[0][2])*invdet;
    inv3.z =  (A[0][0]*A[1][1]-A[1][0]*A[0][1])*invdet;

    // Multiply Inverse*Coordinate to get Lambda (Barycentric)
    vec3 lambda;
    lambda.x = inv1.x*(triple.x - v4.x) + inv1.y*(triple.y - v4.y) + inv1.z*(triple.z - v4.z);
    lambda.y = inv2.x*(triple.x - v4.x) + inv2.y*(triple.y - v4.y) + inv2.z*(triple.z - v4.z);
    lambda.z = inv3.x*(triple.x - v4.x) + inv3.y*(triple.y - v4.y) + inv3.z*(triple.z - v4.z);

    //--------------------------------------------------------------
    // Is any coordinate negative?
    // If so, make it 0, adjust other weights but keep ratio
    //--------------------------------------------------------------

    if(lambda.x < EPS){
        lambda.x = 0;

        for(int i=0; i < EDGES_PER_FACE; i++){
            if(edges[i]->containsBoth(verts[1],verts[2])){
                trip->conformedEdge = edges[i];                
                break;
            }
        }
    }
    else if(lambda.y < EPS){
        lambda.y = 0;

        for(int i=0; i < EDGES_PER_FACE; i++){
            if(edges[i]->containsBoth(verts[0],verts[2])){
                trip->conformedEdge = edges[i];                
                break;
            }
        }
    }
    else if(lambda.z < EPS){
        lambda.z = 0;

        for(int i=0; i < EDGES_PER_FACE; i++){
            if(edges[i]->containsBoth(verts[0],verts[1])){
                trip->conformedEdge = edges[i];                
                break;
            }
        }
    }
    else
    {
        trip->conformedEdge = NULL;
    }

    lambda /= L1(lambda);


    // Compute New Triple Coordinate
    triple.x = lambda.x*v1.x + lambda.y*v2.x + lambda.z*v3.x;
    triple.y = lambda.x*v1.y + lambda.y*v2.y + lambda.z*v3.y;
    triple.z = lambda.x*v1.z + lambda.y*v2.z + lambda.z*v3.z;

    if(triple == vec3::zero || triple != triple)
    {
        std::cerr << "Error Conforming Triple!" << std::endl;
        exit(-1);
    }

    face->triple->pos_next() = triple;
}

vec3 BCCLattice3DMesher::projectCut(Edge3D *edge, Tet3D *tet, Vertex3D *warp_vertex, const vec3 &warp_pt)
{
    Vertex3D *static_vertex = NULL;
    Vertex3D *quad = tet->quad;
    Vertex3D *verts[15];
    lattice->getRightHandedVertexList(tet, verts);

    if(edge->v1 == warp_vertex)
        static_vertex = edge->v2;
    else if(edge->v2 == warp_vertex)
        static_vertex = edge->v1;

    vec3 static_pt = static_vertex->pos();
    vec3 pt = vec3::zero;

    //------------------------
    // Form Intersecting Ray
    //------------------------
    vec3 ray = normalize(warp_pt - static_pt);
    double min_error = 10000;

    vec3  point[12];
    float error[12];
    bool  valid[12] = {0};


    // check intersection with each triangle face
    int interface_count = 0;
    if(parity_flip[tet->tet_index])
    {
        for(int i=0; i < 12; i++)
        {
            if(interfaceTableEven[tet->key][i][0] < 0)
                break;

            Vertex3D *v1 = verts[interfaceTableEven[tet->key][i][0]];
            Vertex3D *v2 = verts[interfaceTableEven[tet->key][i][1]];
            Vertex3D *v3 = quad;

            if(v1->isEqualTo(edge->cut) || v2->isEqualTo(edge->cut) || v3->isEqualTo(edge->cut) ||
                    L2(v1->pos() - edge->cut->pos()) < 1E-7 || L2(v2->pos() - edge->cut->pos()) < 1E-7 || L2(v3->pos() - edge->cut->pos()) < 1E-7){
                valid[i] = triangle_intersect(v1,v2,v3, static_pt, ray, point[i], error[i]);
                interface_count++;
            }
            else{
                // skip it
            }
        }
    }
    else
    {
        for(int i=0; i < 12; i++)
        {
            if(interfaceTableOdd[tet->key][i][0] < 0)
                break;

            Vertex3D *v1 = verts[interfaceTableOdd[tet->key][i][0]];
            Vertex3D *v2 = verts[interfaceTableOdd[tet->key][i][1]];
            Vertex3D *v3 = quad;

            if(v1->isEqualTo(edge->cut) || v2->isEqualTo(edge->cut) || v3->isEqualTo(edge->cut) ||
                    L2(v1->pos() - edge->cut->pos()) < 1E-7 || L2(v2->pos() - edge->cut->pos()) < 1E-7 || L2(v3->pos() - edge->cut->pos()) < 1E-7){
                valid[i] = triangle_intersect(v1,v2,v3, static_pt, ray, point[i], error[i]);
                interface_count++;
            }
            else{
                // skip it
            }

        }
    }


    // pick the one with smallest error
    for(int i=0; i < 12; i++)
    {
        if(valid[i] && error[i] < min_error)
        {
            pt = point[i];
            min_error = error[i];
        }
    }

    // if no intersections, don't move it at all
    if(pt == vec3::zero){        
        pt = edge->cut->pos();
    }

    // Conform Point!!!
    vec3 newray = pt - static_vertex->pos();
    double t1 = newray.x / ray.x;

    if(t1 < 0 || t1 > 1)
    {
        // clamp it
        t1 = max(t1, 0.0);
        t1 = min(t1, 1.0);
        pt = static_pt + t1*ray;
    }

    return pt;
}


//===========================================================================================
//  triangle_intersect()
//
//  This method computes the intersection of a ray and triangle, in a slower way,
//  but in a way that allows an epsilon error around each triangle.
//===========================================================================================
bool BCCLattice3DMesher::triangle_intersect(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, vec3 origin, vec3 ray, vec3 &pt, float &error, float EPS)
{
    double epsilon = 1E-7;

    //-------------------------------------------------
    // if v1, v2, and v3 are not unique, return FALSE
    //-------------------------------------------------
    if(v1->isEqualTo(v2) || v2->isEqualTo(v3) || v3->isEqualTo(v1))
    {
        pt = vec3(-2, -2, -2); // Debug J.R.B. 11/22/11
        return false;
    }
    else if(L2(v1->pos() - v2->pos()) < epsilon || L2(v2->pos() - v3->pos()) < epsilon || L2(v3->pos() - v1->pos()) < epsilon)
    {
        pt = vec3(-3, -3, -3); // Debug J.R.B. 11/22/11
        return false;
    }

    //----------------------------------------------
    // compute intersection with plane, store in pt
    //----------------------------------------------
    //bool plane_hit = plane_intersect(v1, v2, v3, origin, ray, pt);
    plane_intersect(v1, v2, v3, origin, ray, pt);
    vec3 tri_pt = vec3::zero;

    //----------------------------------------------
    //      Compute Barycentric Coordinates
    //----------------------------------------------
    // Create Matrix A and solve for Inverse
    double A[3][3];
    vec3 inv1,inv2,inv3;
    vec3 p1 = v1->pos();
    vec3 p2 = v2->pos();
    vec3 p3 = v3->pos();
    vec3 p4 = p1 + normalize(cross(normalize(p3 - p1), normalize(p2 - p1)));

    // Fill Coordinate Matrix
    A[0][0] = p1.x - p4.x; A[0][1] = p2.x - p4.x; A[0][2] = p3.x - p4.x;
    A[1][0] = p1.y - p4.y; A[1][1] = p2.y - p4.y; A[1][2] = p3.y - p4.y;
    A[2][0] = p1.z - p4.z; A[2][1] = p2.z - p4.z; A[2][2] = p3.z - p4.z;

    // Solve Inverse
    double det = +A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2])
                 -A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
                 +A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
    double invdet = 1/det;
    inv1.x =  (A[1][1]*A[2][2]-A[2][1]*A[1][2])*invdet;
    inv2.x = -(A[1][0]*A[2][2]-A[1][2]*A[2][0])*invdet;
    inv3.x =  (A[1][0]*A[2][1]-A[2][0]*A[1][1])*invdet;

    inv1.y = -(A[0][1]*A[2][2]-A[0][2]*A[2][1])*invdet;
    inv2.y =  (A[0][0]*A[2][2]-A[0][2]*A[2][0])*invdet;
    inv3.y = -(A[0][0]*A[2][1]-A[2][0]*A[0][1])*invdet;

    inv1.z =  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
    inv2.z = -(A[0][0]*A[1][2]-A[1][0]*A[0][2])*invdet;
    inv3.z =  (A[0][0]*A[1][1]-A[1][0]*A[0][1])*invdet;

    // Multiply Inverse*Coordinate to get Lambda (Barycentric)
    vec3 lambda;
    lambda.x = inv1.x*(pt.x - p4.x) + inv1.y*(pt.y - p4.y) + inv1.z*(pt.z - p4.z);
    lambda.y = inv2.x*(pt.x - p4.x) + inv2.y*(pt.y - p4.y) + inv2.z*(pt.z - p4.z);
    lambda.z = inv3.x*(pt.x - p4.x) + inv3.y*(pt.y - p4.y) + inv3.z*(pt.z - p4.z);

    //----------------------------------------------
    //   Project to Valid Coordinate
    //----------------------------------------------
    // clamp to borders    
    lambda.x = std::max(0.0, lambda.x);
    lambda.y = std::max(0.0, lambda.y);
    lambda.z = std::max(0.0, lambda.z);

    // renormalize
    lambda /= L1(lambda);

    // compute new coordinate in triangle
    tri_pt.x = lambda.x*p1.x + lambda.y*p2.x + lambda.z*p3.x;
    tri_pt.y = lambda.x*p1.y + lambda.y*p2.y + lambda.z*p3.y;
    tri_pt.z = lambda.x*p1.z + lambda.y*p2.z + lambda.z*p3.z;

    // project pt onto ray
    vec3 a = tri_pt - origin;
    vec3 b = ray;
    vec3 c = (a.dot(b) / b.dot(b)) * b;

    // find final position;
    double t = length(c);
    if(c.dot(b) < 0)
        t *= -1;

    pt = origin + t*ray;

    // how far are we from the actual triangle pt?
    error = (float)L2(tri_pt - pt);

    //if(error != error)
    //    cerr << "TriangleIntersect2 error = NaN" << endl;

    //----------------------------------------------
    //  If Made It This Far,  Return Success
    //----------------------------------------------
    return true;
}


//======================================================
//  detect_trips_violating_edges()
//
// This method is to detect triple points that violate
// tet edges after general lattice warping has taken place.
// This first implementation is noticeably naive, and
// can be incorporated into previous actions in later
// implementations. It is presented here for clarity.
//======================================================
void BCCLattice3DMesher::detect_trips_violating_edges()
{
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){
        OTCell *cell = lattice->cut_cells[c];

        for(int f=0; f < FACES_PER_CELL; f++)
        {
            check_triple_violating_edges(cell->face[f]);
        }
    }
}


//----------------------------------------------------------------
// check_triple_violating_edges()
//
//  This method checks whether a triple violates either of 3 edges
// that surround its face. This is used in the second phase of
// the warping algorithm.
//----------------------------------------------------------------
void BCCLattice3DMesher::check_triple_violating_edges(Face3D *face)
{
    // Return immediately if triple doesn't exist
    if(!face->triple || face->triple->order() != TRIP)
        return;

    Vertex3D *triple = face->triple;
    triple->violating = false;

    double d[3];
    double d_min = 100000;
    bool violating[3] = {0};

    Edge3D *edges[3];
    Vertex3D *verts[3];
    lattice->getAdjacencyLists(face, verts, edges);

    fixTriangleOrdering(edges, verts);

    vec3 v1 = verts[0]->pos();
    vec3 v2 = verts[1]->pos();
    vec3 v3 = verts[2]->pos();
    vec3 trip = triple->pos();

    // check violating edge0
    {
        vec3 e1 = normalize(v3 - v2);
        vec3 e2 = normalize(v2 - v3);
        vec3 t1 = normalize(trip - v2);
        vec3 t2 = normalize(trip - v3);

        double alpha1 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        double alpha2 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());

        vec3 c1 = v3*(1 - alpha1) + alpha1*v1;
        vec3 c2 = v2*(1 - alpha2) + alpha2*v1;

        c1 = normalize(c1 - v2);
        c2 = normalize(c2 - v3);

        if(dot(e1, t1) > dot(e1, c1) ||
           dot(e2, t2) > dot(e2, c2)){

            double dot1 = clamp(dot(e1, t1), -1, 1);
            double dot2 = clamp(dot(e2, t2), -1, 1);

            if(dot1 > dot2)
                d[0] = acos(dot1);
            else
                d[0] = acos(dot2);

            violating[0] = true;
        }
    }

    // check violating edge1
    {
        vec3 e1 = normalize(v3 - v1);
        vec3 e2 = normalize(v1 - v3);
        vec3 t1 = normalize(trip - v1);
        vec3 t2 = normalize(trip - v3);

        double alpha1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        double alpha2 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());

        vec3 c1 = v3*(1 - alpha1) + alpha1*v2;
        vec3 c2 = v1*(1 - alpha2) + alpha2*v2;

        c1 = normalize(c1 - v1);
        c2 = normalize(c2 - v3);

        if(dot(e1, t1) > dot(e1, c1) ||
           dot(e2, t2) > dot(e2, c2)){

            double dot1 = clamp(dot(e1, t1), -1, 1);
            double dot2 = clamp(dot(e2, t2), -1, 1);

            if(dot1 > dot2)
                d[1] = acos(dot1);
            else
                d[1] = acos(dot2);

            violating[1] = true;
        }
    }

    // check violating edge2
    {
        vec3 e1 = normalize(v2 - v1);
        vec3 e2 = normalize(v1 - v2);
        vec3 t1 = normalize(trip - v1);
        vec3 t2 = normalize(trip - v2);

        double alpha1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        double alpha2 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());

        vec3 c1 = v2*(1 - alpha1) + alpha1*v3;
        vec3 c2 = v1*(1 - alpha2) + alpha2*v3;

        c1 = normalize(c1 - v1);
        c2 = normalize(c2 - v2);

        if(dot(e1, t1) > dot(e1, c1) ||
           dot(e2, t2) > dot(e2, c2)){

            double dot1 = clamp(dot(e1, t1), -1, 1);
            double dot2 = clamp(dot(e2, t2), -1, 1);

            if(dot1 > dot2)
                d[2] = acos(dot1);
            else
                d[2] = acos(dot2);

            violating[2] = true;
        }
    }

    // compare violatings, choose minimum
    for(int i=0; i < 3; i++){
        if(violating[i] && d[i] < d_min){
            triple->violating = true;
            triple->closestGeometry = edges[i];
            d_min = d[i];
        }
    }
}


//======================================================
//  detect_quads_violating_edges()
//
// This method is to detect quad points that violate
// tet edges after general lattice warping has taken place.
// This first implementation is noticeably naive, and
// can be incorporated into previous actions in later
// implementations. It is presented here for clarity.
//======================================================
void BCCLattice3DMesher::detect_quads_violating_edges()
{
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){
        OTCell *cell = lattice->cut_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            check_quadruple_violating_edges(cell->tets[t]);
        }
    }

    lattice->setPhase2ViolationsFound(true);
}


//=====================================================================================
//  check_quadruple_violating_edges
//
// This method generalizes the rules that dictate whether a triplepoint violates a lattice
// edge. Similarly, we also want to know when a Quadruple Point violates such an edge.
//=====================================================================================
void BCCLattice3DMesher::check_quadruple_violating_edges(Tet3D *tet)
{
    if(!tet->quad || tet->quad->order() != QUAD)
        return;

    tet->quad->violating = false;

    Vertex3D *verts[4];
    Edge3D *edges[6];
    Face3D *faces[4];

    lattice->getAdjacencyLists(tet, verts, edges, faces);

    vec3  q = tet->quad->pos();
    vec3 v1 = verts[0]->pos();
    vec3 v2 = verts[1]->pos();
    vec3 v3 = verts[2]->pos();
    vec3 v4 = verts[3]->pos();


    // check e1
    if(!tet->quad->violating)
    {
        float t1 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        float t2 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        float t3 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        float t4 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());

        vec3 c1 = (1 - t1)*v1 + t1*v3;
        vec3 c2 = (1 - t2)*v1 + t2*v4;
        vec3 n1 = normalize(cross(c1 - v2, c2 - v2));   // verified
        vec3 q1 = q - v2;

        vec3 c3 = (1 - t3)*v2 + t3*v3;
        vec3 c4 = (1 - t4)*v2 + t4*v4;
        vec3 n2 = normalize(cross(c4 - v1, c3 - v1));   // fixed
        vec3 q2 = q - v1;

        if(dot(n1,q1) > 0 && dot(n2,q2) > 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = edges[0];
        }
    }

    // check e2
    if(!tet->quad->violating)
    {
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        float t2 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        float t3 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        float t4 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());

        vec3 c1 = (1 - t1)*v1 + t1*v2;
        vec3 c2 = (1 - t2)*v1 + t2*v4;
        vec3 n1 = normalize(cross(c2 - v3, c1 - v3));  // fixed
        vec3 q1 = q - v3;

        vec3 c3 = (1 - t3)*v3 + t3*v2;
        vec3 c4 = (1 - t4)*v3 + t4*v4;
        vec3 n2 = normalize(cross(c3 - v1, c4 - v1));  // verified
        vec3 q2 = q - v1;

        if(dot(n1,q1) > 0 && dot(n2,q2) > 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = edges[1];
        }
    }

    // check e3
    if(!tet->quad->violating)
    {
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        float t2 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        float t3 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());
        float t4 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());

        vec3 c1 = (1 - t1)*v1 + t1*v2;
        vec3 c2 = (1 - t2)*v1 + t2*v3;
        vec3 n1 = normalize(cross(c2 - v4, c1 - v4));   // fixed
        vec3 q1 = q - v4;

        vec3 c3 = (1 - t3)*v4 + t3*v2;
        vec3 c4 = (1 - t4)*v4 + t4*v3;
        vec3 n2 = normalize(cross(c4 - v1, c3 - v1));   // fixed
        vec3 q2 = q - v1;

        if(dot(n1,q1) > 0 && dot(n2,q2) > 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = edges[2];
        }
    }

    // check e4
    if(!tet->quad->violating)
    {
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        float t2 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());
        float t3 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        float t4 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());

        vec3 c1 = (1 - t1)*v2 + t1*v1;
        vec3 c2 = (1 - t2)*v2 + t2*v4;
        vec3 n1 = normalize(cross(c1 - v3, c2 - v3));   // verified
        vec3 q1 = q - v3;

        vec3 c3 = (1 - t3)*v3 + t3*v1;
        vec3 c4 = (1 - t4)*v3 + t4*v4;
        vec3 n2 = normalize(cross(c4 - v2, c3 - v2));   // fixed
        vec3 q2 = q - v2;

        if(dot(n1,q1) > 0 && dot(n2,q2) > 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = edges[3];
        }
    }

    // check e5
    if(!tet->quad->violating)
    {
        float t1 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        float t2 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());
        float t3 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        float t4 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());

        vec3 c1 = (1 - t1)*v4 + t1*v1;
        vec3 c2 = (1 - t2)*v4 + t2*v2;
        vec3 n1 = normalize(cross(c2 - v3, c1 - v3));  // fixed
        vec3 q1 = q - v3;

        vec3 c3 = (1 - t3)*v3 + t3*v1;
        vec3 c4 = (1 - t4)*v3 + t4*v2;
        vec3 n2 = normalize(cross(c3 - v4, c4 - v4));  // verified
        vec3 q2 = q - v4;

        if(dot(n1,q1) > 0 && dot(n2,q2) > 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = edges[4];
        }
    }

    // check e6
    if(!tet->quad->violating)
    {
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        float t2 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        float t3 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        float t4 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());

        vec3 c1 = (1 - t1)*v2 + t1*v1;
        vec3 c2 = (1 - t2)*v2 + t2*v3;
        vec3 n1 = normalize(cross(c2 - v4, c1 - v4));   // fixed
        vec3 q1 = q - v4;

        vec3 c3 = (1 - t3)*v4 + t3*v1;
        vec3 c4 = (1 - t4)*v4 + t4*v3;
        vec3 n2 = normalize(cross(c3 - v2, c4 - v2));   // verified
        vec3 q2 = q - v2;

        if(dot(n1,q1) > 0 && dot(n2,q2) > 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = edges[5];
        }
    }

}


//======================================================
// - warp_violating_trips()
//
// This method is to warp triple points that violate
// edges or cuts on these edges, after general lattice
// warping has taken place.
//======================================================
void BCCLattice3DMesher::warp_violating_trips()
{
    // TODO: if edge contains no cut vertex, insert one and split the adjacent tents
    //       currently, the triple just goes to where the cutpoint ended up

    for(unsigned int c=0; c < lattice->cut_cells.size(); c++)
    {
        OTCell *cell = lattice->cut_cells[c];

        // NAIVE - Loop over all edges in cell
        for(int e=0; e < EDGES_PER_CELL; e++)
        {
            warp_edge(cell->edge[e]);
        }
    }

    lattice->setPhase2Complete(true);
}

//==================================================================
// warp_edge()
//
//  This method warps the given lattice edge, as part of the
//  phase 2 subroutine. It's mostly useful for debugging.
//==================================================================
void BCCLattice3DMesher::warp_edge(Edge3D *edge)
{
    // look at eac=h adjacent face
    Face3D* faces[6] = {0};
    unsigned int face_count;
    lattice->getFacesAroundEdge(edge, faces, &face_count);

    for(unsigned int f=0; f < face_count; f++)
    {
        if(faces[f]->triple->order() == TRIP &&
                faces[f]->triple->violating &&
                faces[f]->triple->closestGeometry == edge)
        {
            snap_triple_to_cut(faces[f]->triple, edge->cut);
        }
    }

    // If triples went to a vertex, resolve degeneracies on that vertex
    if(edge->cut->order() == VERT)
    {
        resolve_degeneracies_around_vertex(edge->cut->root());
    }
    // Else Triple went to the Edge-Cut
    else
    {
        resolve_degeneracies_around_edge(edge);
    }
}


//==================================================================
//  detect_quads_violating_faces()
//
// This method is to detect quadruple points that violate
// tet faces after general lattice warping has taken place.
// This first implementation is noticeably naive, and
// can be incorporated into previous actions in later
// implementations. It is presented here for clarity.
//==================================================================
void BCCLattice3DMesher::detect_quads_violating_faces()
{
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){
        OTCell *cell = lattice->cut_cells[c];

        for(int t=0; t < TETS_PER_CELL; t++)
        {
            check_quadruple_violating_faces(cell->tets[t]);
        }
    }

    lattice->setPhase3ViolationsFound(true);
}


//==================================================================
// check_quadruple_violating_faces()
//
//  This method checks whether a quad violates any of 4 faces
// that surround its tet. This is used in the third phase of
// the warping algorithm.
//==================================================================
void BCCLattice3DMesher::check_quadruple_violating_faces(Tet3D *tet)
{
    if(!tet->quad || tet->quad->order() != QUAD)
        return;

    tet->quad->violating = false;

    Vertex3D *verts[4];
    Edge3D *edges[6];
    Face3D *faces[4];

    lattice->getAdjacencyLists(tet, verts, edges, faces);

    vec3  q = tet->quad->pos();
    vec3 v1 = verts[0]->pos();
    vec3 v2 = verts[1]->pos();
    vec3 v3 = verts[2]->pos();
    vec3 v4 = verts[3]->pos();

    // check face 1
    if(!tet->quad->violating)
    {
        float t1 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());
        float t2 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());
        float t3 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());

        vec3 c1 = (1 - t1)*v1 + t1*v4;
        vec3 c2 = (1 - t2)*v2 + t2*v4;
        vec3 c3 = (1 - t3)*v3 + t3*v4;

        vec3 n1 = normalize(cross(c2 - v1, c3 - v1));  // verified
        vec3 n2 = normalize(cross(c3 - v2, c1 - v2));  // fixed
        vec3 n3 = normalize(cross(c1 - v3, c2 - v3));  // verified

        vec3 q1 = q - v1;
        vec3 q2 = q - v2;
        vec3 q3 = q - v3;

        if(dot(n1,q1) < 0 && dot(n2,q2) < 0 && dot(n3,q3) < 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = faces[0];
        }
    }

    // check face 2
    if(!tet->quad->violating)
    {
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        float t2 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        float t3 = edges[5]->isLong ? lattice->a_long * (lattice->long_length / edges[5]->length()) : lattice->a_short * (lattice->short_length / edges[5]->length());

        vec3 c1 = (1 - t1)*v1 + t1*v2;
        vec3 c2 = (1 - t2)*v3 + t2*v2;
        vec3 c3 = (1 - t3)*v4 + t3*v2;

        vec3 n1 = normalize(cross(c2 - v1, c3 - v1));  // verified
        vec3 n2 = normalize(cross(c3 - v3, c1 - v3));  // fixed
        vec3 n3 = normalize(cross(c1 - v4, c2 - v4));  // verified

        vec3 q1 = q - v1;
        vec3 q2 = q - v3;
        vec3 q3 = q - v4;

        if(dot(n1,q1) < 0 && dot(n2,q2) < 0 && dot(n3,q3) < 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = faces[1];
        }
    }

    // check face 3
    if(!tet->quad->violating)
    {
        float t1 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        float t2 = edges[3]->isLong ? lattice->a_long * (lattice->long_length / edges[3]->length()) : lattice->a_short * (lattice->short_length / edges[3]->length());
        float t3 = edges[4]->isLong ? lattice->a_long * (lattice->long_length / edges[4]->length()) : lattice->a_short * (lattice->short_length / edges[4]->length());

        vec3 c1 = (1 - t1)*v1 + t1*v3;
        vec3 c2 = (1 - t2)*v2 + t2*v3;
        vec3 c3 = (1 - t3)*v4 + t3*v3;

        vec3 n1 = normalize(cross(c3 - v1, c2 - v1));  // fixed
        vec3 n2 = normalize(cross(c1 - v2, c3 - v2));  // verified
        vec3 n3 = normalize(cross(c2 - v4, c1 - v4));  // fixed

        vec3 q1 = q - v1;
        vec3 q2 = q - v2;
        vec3 q3 = q - v4;

        if(dot(n1,q1) < 0 && dot(n2,q2) < 0 && dot(n3,q3) < 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = faces[2];
        }
    }

    // check face 4
    if(!tet->quad->violating)
    {
        float t1 = edges[0]->isLong ? lattice->a_long * (lattice->long_length / edges[0]->length()) : lattice->a_short * (lattice->short_length / edges[0]->length());
        float t2 = edges[1]->isLong ? lattice->a_long * (lattice->long_length / edges[1]->length()) : lattice->a_short * (lattice->short_length / edges[1]->length());
        float t3 = edges[2]->isLong ? lattice->a_long * (lattice->long_length / edges[2]->length()) : lattice->a_short * (lattice->short_length / edges[2]->length());

        vec3 c1 = (1 - t1)*v2 + t1*v1;
        vec3 c2 = (1 - t2)*v3 + t2*v1;
        vec3 c3 = (1 - t3)*v4 + t3*v1;

        vec3 n1 = normalize(cross(c3 - v2, c2 - v2));  // verified
        vec3 n2 = normalize(cross(c1 - v3, c3 - v3));  // verified
        vec3 n3 = normalize(cross(c2 - v4, c1 - v4));  // fixed

        vec3 q1 = q - v2;
        vec3 q2 = q - v3;
        vec3 q3 = q - v4;

        if(dot(n1,q1) < 0 && dot(n2,q2) < 0 && dot(n3,q3) < 0){
            tet->quad->violating = true;
            tet->quad->closestGeometry = faces[3];
        }
    }
}



//======================================================
//  warp_violating_quads()
//
// This method is to warp quadruple points that violate
// faces, or triple points on it, after general lattice
// warping has taken place. This first implementation
// is noticeably naive, and can be accelerated by only
// examining triple points that are known to violate.
//======================================================
void BCCLattice3DMesher::warp_violating_quads()
{
    for(unsigned int c=0; c < lattice->cut_cells.size(); c++)
    {
        OTCell *cell = lattice->cut_cells[c];

        // NAIVE - Loop over all faces in cell
        for(unsigned int f=0; f < FACES_PER_CELL; f++)
        {
            Face3D *face = cell->face[f];

            // look at each adjacent tet
            Tet3D* tets[TETS_PER_FACE];
            lattice->getTetsAroundFace(face, tets);

            for(unsigned int t=0; t < TETS_PER_FACE; t++)
            {
                if(tets[t]->quad->order() == QUAD && tets[t]->quad->violating && tets[t]->quad->closestGeometry == face)
                {
                    // Snap to triple point, wherever it happens to be                    
                    snap_quad_to_triple(tets[t]->quad, face->triple);


                    // check order of vertex now pointed to
                    switch(tets[t]->quad->order())
                    {
                        case VERT:
                        {
                            // If Triple_point is on a Vertex
                            resolve_degeneracies_around_vertex(tets[t]->quad->root());                                                        
                            break;
                        }
                        case CUT:
                        {
                            // If Triple-Point is on an Edge
                            Edge3D *edges[EDGES_PER_FACE];
                            lattice->getEdgesAroundFace(face, edges);
                            for(unsigned int e=0; e < EDGES_PER_FACE; e++)
                            {
                                if(edges[e]->cut->isEqualTo(tets[t]->quad))
                                {
                                    snap_quad_to_edge(tets[t], edges[e]);
                                    resolve_degeneracies_around_edge(edges[e]);
                                }
                            }
                            break;
                        }
                        case TRIP:
                        {
                            // If Triple-Point is on a Face, do nothing
                            break;
                        }
                        default:
                        {
                            cerr << "Fatal Error - Quad order == " << tets[t]->quad->order() << endl;
                            exit(-1);
                        }
                    }
                }
            }
        }
    }

    lattice->setPhase3Complete(true);
}

//=====================================================================
//  snap_quad_to_edge()
//
//  This method handles the complex task of snapping a quadpoint to
// an edge. Task is complicated because it causes a degeneration of
// adjacent triple points. If these triplepoints have already snapped,
// the degeneracy propagates to the next adjacent LatticeTet around
// the edge being snapped to. Hence, this is a recursive function.
//=====================================================================
void BCCLattice3DMesher::snap_quad_to_edge(Tet3D *tet, Edge3D *edge)
{
    // Snap the Quad if it's not already there
    if(!tet->quad->isEqualTo(edge->cut)){
        snap_quad_to_cut(tet->quad, edge->cut);
    }

    // Get Adjacent TripleFaces
    Face3D *adjFaces[2];
    lattice->getFacesAroundEdgeOnTet(tet, edge, adjFaces);

    for(unsigned int f=0; f < 2; f++)
    {
        // if still a triple, snap it
        if(adjFaces[f]->triple->order() == TRIP)
        {            
            snap_triple_to_cut(adjFaces[f]->triple, edge->cut);

            Tet3D *opTet = lattice->getOppositeTet(tet, adjFaces[f]);

            // if adjacent Tet quadpoint is snapped to this triple, snap it next
            if(opTet->quad->isEqualTo(adjFaces[f]->triple))
                snap_quad_to_edge(opTet, edge);

        }

        // if snapped to a different edge
        else if(adjFaces[f]->triple->order() == CUT && !(adjFaces[f]->triple->isEqualTo(edge->cut)))
        {
            Tet3D *opTet = lattice->getOppositeTet(tet, adjFaces[f]);

            // if adjacent Tet quadpoint is snapped to this triple, snap it next
            if(opTet->quad->isEqualTo(adjFaces[f]->triple))
                snap_quad_to_edge(opTet, edge);

            snap_triple_to_cut(adjFaces[f]->triple, edge->cut);
        }
        // otherwise done
        else
        {
            // do nothing
        }
    }
}


//===========================================================================
// - resolve_degeneracies_around_vertex()
//
// - TODO: This currently checks too much, even vertices
//         that were snapped a long time ago. Optimize.
//===========================================================================
void BCCLattice3DMesher::resolve_degeneracies_around_vertex(Vertex3D *vertex)
{
    Face3D *faces[FACES_PER_VERT];
    Tet3D *tets[TETS_PER_VERT];

    lattice->getFacesAroundVertex(vertex, faces);
    lattice->getTetsAroundVertex(vertex, tets);

    bool changed = true;
    while(changed)
    {
        changed = false;

    //--------------------------------------------------------------------------
    // Snap Any Triples or Cuts that MUST follow a Quadpoint
    //--------------------------------------------------------------------------
    for(unsigned int t=0; t < TETS_PER_VERT; t++)
    {
        Tet3D *tet = tets[t];

        // If Quadpoint is snapped to Vertex
        if(tet && tet->quad->isEqualTo(vertex))
        {
            // Check if any cuts exist to snap
            Edge3D *edges[EDGES_PER_TET];
            lattice->getEdgesAroundTet(tet, edges);
            for(int e=0; e < EDGES_PER_TET; e++)
            {
                // cut exists & spans the vertex in question
                if(edges[e]->cut->order() == CUT && (edges[e]->v1 == vertex || edges[e]->v2 == vertex))
                {
                    snap_cut_to_vertex(edges[e]->cut, vertex);
                    changed = true;
                }
            }

            // Check if any triples exist to snap
            Face3D *faces[FACES_PER_TET];
            lattice->getFacesAroundTet(tet, faces);
            for(int f=0; f < FACES_PER_TET; f++)
            {
                // triple exists & spans the vertex in question
                if(faces[f]->triple->order() == TRIP)
                {
                    Vertex3D *verts[3];
                    lattice->getVertsAroundFace(faces[f], verts);
                    if(verts[0] == vertex || verts[1] == vertex || verts[2] == vertex)
                    {                     
                        snap_triple_to_vertex(faces[f]->triple, vertex);
                        changed = true;
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------------------
    // Snap Any Cuts that MUST follow a Triplepoint
    //------------------------------------------------------------------------------------
    for(unsigned int f=0; f < FACES_PER_VERT; f++)
    {
        // If Triplepoint is snapped to Vertex
        if(faces[f] && faces[f]->triple->isEqualTo(vertex))
        {
           // Check if any cuts exist to snap
           Edge3D *edges[EDGES_PER_FACE];
           lattice->getEdgesAroundFace(faces[f], edges);
           for(int e=0; e < EDGES_PER_FACE; e++)
           {
               // cut exists & spans the vertex in question
               if(edges[e]->cut->order() == CUT && (edges[e]->v1 == vertex || edges[e]->v2 == vertex))
               {                 
                   snap_cut_to_vertex(edges[e]->cut, vertex);
                   changed = true;
               }
           }
        }
    }


    //------------------------------------------------------------------------------------
    // Snap Any Triples that have now degenerated
    //------------------------------------------------------------------------------------
    for(unsigned int f=0; f < FACES_PER_VERT; f++)
    {
        if(faces[f] && faces[f]->triple->order() == TRIP)
        {
            Edge3D *edges[EDGES_PER_FACE];
            lattice->getEdgesAroundFace(faces[f], edges);

            // count # cuts snapped to vertex
            int count = 0;
            for(int e=0; e < EDGES_PER_FACE; e++)
                count += (int) edges[e]->cut->isEqualTo(vertex);

            // if two cuts have snapped to vertex, triple degenerates
            if(count == 2)
            {             
                snap_triple_to_vertex(faces[f]->triple, vertex);
                changed = true;
            }
        }
    }

    //------------------------------------------------------------------------------------
    // Snap Any Quads that have now degenerated
    //------------------------------------------------------------------------------------
    for(unsigned int t=0; t < TETS_PER_VERT; t++)
    {
        if(tets[t] && tets[t]->quad->order() == QUAD)
        {
            Face3D *faces[FACES_PER_TET];
            lattice->getFacesAroundTet(tets[t], faces);

            // count # trips snapped to vertex
            int count = 0;
            for(int f=0; f < FACES_PER_TET; f++)
                count += (int) faces[f]->triple->isEqualTo(vertex);

            // if 3 triples have snapped to vertex, quad degenerates
            if(count == 3)
            {              
                snap_quad_to_vertex(tets[t]->quad, vertex);
                changed = true;
            }
        }
    }

    }
}


//=====================================================================
// - resolve_degeneracies_around_edge()
//
//=====================================================================
void BCCLattice3DMesher::resolve_degeneracies_around_edge(Edge3D *edge)
{
    Tet3D *tets[MAX_TETS_PER_EDGE];
    unsigned int tet_count;

    lattice->getTetsAroundEdge(edge, tets, &tet_count);

    //--------------------------------------------------------------------------
    //  Pull Adjacent Triples To Quadpoint  (revise: 11/16/11 J.R.B.)
    //--------------------------------------------------------------------------
    for(unsigned int t=0; t < tet_count; t++)
    {
        if(tets[t]->quad->isEqualTo(edge->cut))
        {
            snap_quad_to_edge(tets[t], edge);
        }
    }

    //--------------------------------------------------------------------------
    // Snap Any Quads that have now degenerated onto the Edge
    //--------------------------------------------------------------------------
    for(unsigned int t=0; t < tet_count; t++)
    {
        if(tets[t]->quad->order() == QUAD)
        {
            Face3D *faces[FACES_PER_TET];
            lattice->getFacesAroundTet(tets[t], faces);

            // count # triples snaped to edge
            int count = 0;
            for(int f=0; f < FACES_PER_TET; f++)
                count += (int) faces[f]->triple->isEqualTo(edge->cut);

            // if two triples have snapped to edgecut, quad degenerates
            if(count == 2)
            {
                snap_quad_to_edge(tets[t], edge);
            }
        }
    }
}


//=====================================================================
// - snap_cut_to_vertex()
//=====================================================================
void BCCLattice3DMesher::snap_cut_to_vertex(Vertex3D *&cut, Vertex3D *vertex)
{
    if(cut->original_order() == CUT)
        cut->parent = vertex;
    else
        cut = vertex;
}

//=====================================================================
// - snap_triple_to_vertex()
//=====================================================================
void BCCLattice3DMesher::snap_triple_to_vertex(Vertex3D *&triple, Vertex3D *vertex)
{
    if(triple->original_order() == TRIP)
        triple->parent = vertex;
    else
        triple = vertex;
}

//=====================================================================
// - snap_triple_to_cut()
//=====================================================================
void BCCLattice3DMesher::snap_triple_to_cut(Vertex3D *&triple, Vertex3D *cut)
{    
    if(triple->original_order() == TRIP)
        triple->parent = cut;
    else
        triple = cut;
}

//=====================================================================
// - snap_quad_to_vertex()
//=====================================================================
void BCCLattice3DMesher::snap_quad_to_vertex(Vertex3D *&quad, Vertex3D *vertex)
{
    if(quad->original_order() == QUAD)
        quad->parent = vertex;
    else
        quad = vertex;
}

//=====================================================================
// - snap_quad_to_cut()
//=====================================================================
void BCCLattice3DMesher::snap_quad_to_cut(Vertex3D *&quad, Vertex3D *cut)
{
    if(quad->original_order() == QUAD)
        quad->parent = cut;
    else
        quad = cut;
}

//=====================================================================
// - snap_quad_to_triple()
//=====================================================================
void BCCLattice3DMesher::snap_quad_to_triple(Vertex3D *&quad, Vertex3D *triple)
{
    if(quad->original_order() == QUAD)
        quad->parent = triple;
    else
        quad = triple;
}



//**********************************************************
//           Other  Helper  Functions
//**********************************************************

//===========================================================
// isTransition()
//
// This method compares to lists of booleans, and returns
// true only if they share no values are TRUE. If a single
// index is TRUE in both lists, FALSE is returned. This
// is useful for finding material transitions on triangles.
//===========================================================
bool BCCLattice3DMesher::isTransition(bool *set1, bool *set2)
{
    int n = lattice->materials();
    for(int i=0; i < n; i++){
        if(set1[i] && set2[i])
            return false;
    }
    return true;
}


//==================================================
// fixTriangleOrdering()
//
// Temporary Helper function
// This is slow but saves headache of changing
// tables again and again until it's clear what
// the proper form hsould be
//==================================================
void BCCLattice3DMesher::fixTriangleOrdering(Edge3D *edges[], Vertex3D *verts[])
{
    Edge3D *tmp_edge;

    // fix e[0] to match v[0]
    for(int e=0; e < 3; e++)
    {
        if(edges[e]->v1 != verts[0] &&
           edges[e]->v2 != verts[0])
        {
            // swap
            tmp_edge = edges[0];
            edges[0] = edges[e];
            edges[e] = tmp_edge;
        }
    }

    // fix e[1] to match v[1]
    for(int e=1; e < 3; e++)
    {
        if(edges[e]->v1 != verts[1] &&
           edges[e]->v2 != verts[1])
        {
            // swap
            tmp_edge = edges[1];
            edges[1] = edges[e];
            edges[e] = tmp_edge;
        }
    }
}


//==================================================
// fixTetrahedronOrdering()
//
// Temporary Helper function
// This is slow but saves headache of changing
// tables again and again until it's clear what
// the proper form should be
//==================================================
void BCCLattice3DMesher::fixTetrahedronOrdering(Face3D *faces[], Edge3D *edges[], Vertex3D *verts[])
{
    for(int j=0; j < 3; j++)
    {
        // fix f[j] to match v[j]
        for(int f=j; f < 4; f++)
        {
            Vertex3D *tri_verts[3];
            lattice->getVertsAroundFace(faces[f], tri_verts);

            // if no vertices shared, must be opposite
            if(tri_verts[0] != verts[j] &&
               tri_verts[1] != verts[j] &&
               tri_verts[2] != verts[j])
            {
                // swap
                Face3D *tmp_face = faces[j];
                faces[j] = faces[f];
                faces[f] = tmp_face;
            }
        }
    }
}

//=======================================================
//    CenterOfMass()
//
//  This method takes two lists of vertices, and finds
// their center of mass. This is useful when performing
// lattice warps to a position that minimizes the local
// distortion of the geometry.
//=======================================================
vec3 BCCLattice3DMesher::centerOfMass(const vector<Edge3D*> &viol_edges, const vector<Face3D*> &viol_faces, const vector<Tet3D*> &viol_tets)
{
    vec3 c = vec3::zero;

    for(unsigned int i=0; i < viol_edges.size(); i++)
        c += (*viol_edges[i]->cut).pos();

    for(unsigned int i=0; i < viol_faces.size(); i++)
        c += (*viol_faces[i]->triple).pos();

    for(unsigned int i=0; i < viol_tets.size(); i++)
        c += (*viol_tets[i]->quad).pos();

    c = c / (double)(viol_edges.size() + viol_faces.size() + viol_tets.size());

    return c;
}


//====================================================================
//   plane_intersect()
//====================================================================
bool BCCLattice3DMesher::plane_intersect(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, vec3 origin, vec3 ray, vec3 &pt, float epsilon)
{
    //-------------------------------------------------
    // if v1, v2, and v3 are not unique, return FALSE
    //-------------------------------------------------
    if(v1->isEqualTo(v2) || v2->isEqualTo(v3) || v1->isEqualTo(v3))
        return false;
    else if(L2(v1->pos() - v2->pos()) < epsilon || L2(v2->pos() - v3->pos()) < epsilon || L2(v1->pos() - v3->pos()) < epsilon)
        return false;


    vec3 p1 = origin;
    vec3 p2 = origin + ray;
    vec3 p3 = v1->pos();

    vec3 n = normalize(cross(normalize(v3->pos() - v1->pos()), normalize(v2->pos() - v1->pos())));

    double top = n.dot(p3 - p1);
    double bot = n.dot(p2 - p1);

    double t = top / bot;

    pt = origin + t*ray;

    if(pt != pt)
        return false;
    else
        return true;
}
