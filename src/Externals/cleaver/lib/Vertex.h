//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
//
// -- Vertex Class
//
// Author: Jonathan Bronson (bronson@sci.utah.ed)
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

#ifndef VERTEX_H
#define VERTEX_H

#include <vector>
#include <cstring>
#include "vec3.h"

namespace Cleaver
{

class OTCell;
class Face3D;
class Edge3D;
class Tet;
class Geometry{};

class Vertex3D : public Geometry{

public:
    Vertex3D(OTCell *cell, int index):cell(cell),lbls(NULL),vert_index(index),violating(false),warped(false),parent(NULL),tm_v_index(-1),m_order(0),m_pos(vec3::zero),m_pos_next(vec3::zero){ }
    Vertex3D():cell(NULL), lbls(NULL),vert_index(-1), violating(false),warped(false), closestGeometry(NULL), conformedFace(NULL), conformedEdge(NULL), conformedVertex(NULL),
        parent(NULL), tm_v_index(-1), m_order(0), m_pos(vec3::zero),m_pos_next(vec3::zero){}
    Vertex3D(int m):cell(NULL), lbls(new bool[m]), vert_index(-1), violating(false), warped(false), closestGeometry(NULL), conformedFace(NULL), conformedEdge(NULL),
        conformedVertex(NULL), parent(NULL), tm_v_index(-1), m_order(0),m_pos(vec3::zero),m_pos_next(vec3::zero)
    {
        memset(lbls, 0, m*sizeof(bool));
    }

    Vertex3D(int m, OTCell *cell, int index):cell(cell), lbls(new bool[m]), vert_index(index), violating(false), warped(false), closestGeometry(NULL), conformedFace(NULL), conformedEdge(NULL),
        conformedVertex(NULL), parent(NULL), tm_v_index(-1), m_order(0),m_pos(vec3::zero),m_pos_next(vec3::zero)
    {
        memset(lbls, 0, m*sizeof(bool));
    }

    ~Vertex3D();

    inline vec3& pos(){
        Vertex3D* ptr = this;
        while(ptr->parent)
            ptr = ptr->parent;
        return ptr->m_pos;
    }
    inline vec3& pos_next(){
        Vertex3D* ptr = this;
        while(ptr->parent)
            ptr = ptr->parent;
        return ptr->m_pos_next;
    }
    inline int& order(){
        Vertex3D *ptr = this;
        while(ptr->parent)
            ptr = ptr->parent;
        return ptr->m_order;
    }

    const int original_order(){
        return m_order;
    }

    inline Vertex3D* root() {
        Vertex3D *ptr = this;
        while(ptr->parent)
            ptr = ptr->parent;
        return ptr;
    }

    inline bool isEqualTo(Vertex3D* vert)
    {
        return (this->root() == vert->root());
    }

    // member variables
    OTCell *cell;
    //float *vals;               // material values
    bool  *lbls;               // material labels
    unsigned char vert_index;
    unsigned char label;       // single label (for generating texture image)
    bool violating:1;            // is this cut violating
    bool warped:1;
    //double t;        // if cut, t value along edge

    Geometry *closestGeometry;
    Face3D   *conformedFace;
    Edge3D   *conformedEdge;
    Vertex3D *conformedVertex;
    Vertex3D *parent;

    std::vector<Tet*> tets;
    int tm_v_index;

private:
    int m_order;        // vertex order
    vec3 m_pos;         // current position
    vec3 m_pos_next;    // next position

};

typedef Vertex3D Vertex;

}

#endif // VERTEX_H
