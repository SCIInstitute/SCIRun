//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
//
// -- TetMesh Class
//
// Primary Author: Josh Levine (jlevine@sci.utah.edu)
// Secondary Author: Jonathan Bronson (bronson@sci.utah.ed)
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

#ifndef TETMESH_H
#define TETMESH_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include "Vertex.h"

namespace Cleaver
{

class Face
{
public:
    Face();
    ~Face();

    int tets[2];
    //which face of the tets[i] i am
    int face_index[2];
    int verts[3];

    vec3 normal;
};

class Tet
{
public:
    Tet(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, Vertex3D *v4, int material);
    Tet();
    ~Tet();

    double volume() const;

    Vertex3D *verts[4];
    Tet* tets[4];

    //face[i] is opposite vertex[i]
    int faces[4];
    int tm_index;
    char mat_label;
    unsigned char key;
    bool marked:1;
};


class TetMesh
{    
public:

    TetMesh(std::vector<Vertex3D*> &verts, std::vector<Tet*> &tets);    
    ~TetMesh();


    //void writeOff(const std::string &filename);
    void writeMatlab(const std::string &filename, bool verbose = false);   // matlab format
    void writeNodeEle(const std::string &filename, bool verbose = false);  // tetview format
    void writePtsEle(const std::string &filename, bool verbose = false);   // scirun format
    //void writePlyConnected(const std::string  &filename);
    void writePly(const std::string &filename, bool verbose = false);
    void writeMultiplePly(const std::vector<std::string> &inputs, const std::string &filename, bool verbose = false);
    void writeInfo(const std::string &filename, bool verbose = false);

    void constructFaces();    
    void computeAngles();    
    void computeDihedralHistograms();    

    std::vector<Vertex3D*> &verts;
    std::vector<Tet*> &tets;
    Face* faces;
    int nFaces;

    double min_angle;   // smallest dihedral angle
    double max_angle;   // largest dihedral angle
    double time;        // time taken to mesh
};

}

#endif // TETMESH_H

