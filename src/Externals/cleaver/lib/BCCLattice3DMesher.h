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

#ifndef BCCLattice3DMesher_H
#define BCCLattice3DMesher_H

#include "BCCLattice3D.h"
#include "TetMesh.h"


namespace Cleaver
{
    extern const float DefaultAlphaShort;
    extern const float DefaultAlphaLong;

class BCCLattice3DMesher
{
public:
    BCCLattice3DMesher(BCCLattice3D *lattice = NULL, float alpha_short=Cleaver::DefaultAlphaShort, float alpha_long=Cleaver::DefaultAlphaLong) :
        lattice(lattice), alpha_short(alpha_short), alpha_long(alpha_long){}

    TetMesh* mesh(bool snap = true, bool verbose = false);

    void compute_all_cuts();
    void compute_all_trips();
    void compute_all_quads();
    void generalize_tets();
    void fill_all_stencils();

    void detect_trips_violating_edges();
    void detect_quads_violating_edges();
    void detect_quads_violating_faces();

    void warp_violating_cuts();
    void warp_violating_trips();
    void warp_violating_quads();
    void warp_vertex(Vertex3D *vertex);
    void warp_edge(Edge3D *edge);

    void setLattice(BCCLattice3D *lattice){ this->lattice = lattice; }
    BCCLattice3D *getLattice() { return this->lattice; }

    void setAlphaShort(float alpha){ this->alpha_short = alpha; }
    void setAlphaLong(float alpha){ this->alpha_long = alpha; }

private:

    BCCLattice3D *lattice;

    void compute_cut(Edge3D *edge);
    void compute_triple(Face3D *face);
    void compute_quadruple(Tet3D *tet);

    void check_cut_violating_lattice(Edge3D *edge);
    void check_triple_violating_lattice(Face3D *face);
    void check_quadruple_violating_lattice(Tet3D *tet);

    void check_triple_violating_edges(Face3D *face);
    void check_quadruple_violating_edges(Tet3D *tet);
    void check_quadruple_violating_faces(Tet3D *tet);

    void resolve_degeneracies_around_vertex(Vertex3D *vertex);
    void resolve_degeneracies_around_edge(Edge3D *edge);

    void snap_quad_to_edge(Tet3D *tet, Edge3D *edge);

    // abstraction of snapping a cell-interface to another cell-interface
    void snap_cut_to_vertex(Vertex3D *&cut, Vertex3D *vertex);
    void snap_triple_to_vertex(Vertex3D *&triple, Vertex3D *vertex);
    void snap_triple_to_cut(Vertex3D *&triple, Vertex3D *cut);
    void snap_quad_to_vertex(Vertex3D *&quad, Vertex3D *vertex);
    void snap_quad_to_cut(Vertex3D *&quad, Vertex3D *cut);
    void snap_quad_to_triple(Vertex3D *&quad, Vertex3D *triple);

    void fill_stencil(Tet3D *tet);    

    bool isTransition(bool *set1, bool *set2);
    void fixTriangleOrdering(Edge3D *edges[], Vertex3D *verts[]);
    void fixTetrahedronOrdering(Face3D *faces[], Edge3D *edges[], Vertex3D *verts[]);

    //bool tets_share_verts(int i, int j, Tet3D *tet);
    vec3 centerOfMass(const std::vector<Edge3D*> &viol_edges, const std::vector<Face3D*> &viol_faces, const std::vector<Tet3D*> &viol_tets);

    void strip_bad_tets(Tet3D *tet);
    bool valid_dihedral_angles(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, Vertex3D *v4);

    void split_tet_on_edge(Tet3D *tet, Edge3D *edge, Vertex3D *split);    
    void swap_stencil(Tet3D *tet, int stencil[24][4]);

    bool triangle_intersect(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, vec3 origin, vec3 ray, vec3 &pt, float &error, float epsilon = 1E-2);
    bool plane_intersect(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, vec3 origin, vec3 ray, vec3 &pt, float epsilon = 1E-4);

    vec3 projectCut(Edge3D *edge, Tet3D *tet, Vertex3D *warp_vertex, const vec3 &warp_pt);
    vec3 projectTriple(Face3D *face, Vertex3D *quad, Vertex3D *warp_vertex, const vec3 &warp_pt);

    void conformTriple(Face3D *face, Vertex3D *warp_vertex, const vec3 &warp_pt);
    void conformQuadruple(Tet3D *quad, Vertex3D *warp_vertex, const vec3 &warp_pt);

    float alpha_short;
    float alpha_long;

    TetMesh *tm;
};

}

#endif // BCCLattice3DMesher_H
