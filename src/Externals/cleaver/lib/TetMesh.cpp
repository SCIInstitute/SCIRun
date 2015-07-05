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

#include "TetMesh.h"

#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <deque>
#include <set>
#include <algorithm>
#include <exception>
#include "BCCLattice3D.h"
#include "Util.h"
#include "Matlab.h"

using namespace std;

#ifndef PI
#define PI 3.14159265
#endif

namespace Cleaver
{

Face::Face() :
        normal(0,0,0)
{
    tets[0] = tets[1] = -1;
    face_index[0] = face_index[1] = -1;
    verts[0] = verts[1] = verts[2] = -1;
}

Face::~Face()
{
}

Tet::Tet() : mat_label(-1)
{    
    faces[0] = faces[1] = faces[2] = faces[3] = -1;    
    marked = false;    
}

Tet::Tet(Vertex3D *v1, Vertex3D *v2, Vertex3D *v3, Vertex3D *v4, int material) :
    mat_label(material)
{    
    // initialize face info to empty
    faces[0] = faces[1] = faces[2] = faces[3] = -1;

    // add adjacency info
    verts[0] = v1;
    verts[1] = v2;
    verts[2] = v3;
    verts[3] = v4;

    // both ways
    v1->tets.push_back(this);
    v2->tets.push_back(this);
    v3->tets.push_back(this);
    v4->tets.push_back(this);

    tets[0] = tets[1] = tets[2] = tets[3] = NULL;
    marked = false;    
}

Tet::~Tet()
{
}



TetMesh::TetMesh(std::vector<Vertex3D*> &verts, std::vector<Tet*> &tets) :
    verts(verts), tets(tets), faces(0), nFaces(0), time(0)
{
}

TetMesh::~TetMesh() {

    // delete tets verts, faces, etc
    if (faces) {
        delete[] faces;
        nFaces = 0;
        faces = NULL;
    }

    for(unsigned int v=0; v < verts.size(); v++)
        delete verts[v];
    for(unsigned int t=0; t < tets.size(); t++)
        delete tets[t];

    std::vector<Vertex3D*> *vlist = &verts;
    std::vector<Tet*> *tlist = &tets;

    delete vlist;
    delete tlist;
}

//===================================================
// writeOff()
//
// Public method that writes the mesh
// in the geomview off file format.
//===================================================
//void TetMesh::writeOff(const string &filename)
//{
//    // to implement
//}


static float INTERFACE_COLORS[12][3] = {
    {141/255.0f, 211/255.0f, 199/255.0f},
    {255/255.0f, 255/255.0f, 179/255.0f},
    {190/255.0f, 186/255.0f, 218/255.0f},
    {251/255.0f, 128/255.0f, 114/255.0f},
    {128/255.0f, 177/255.0f, 211/255.0f},
    {253/255.0f, 180/255.0f, 98/255.0f},
    {179/255.0f, 222/255.0f, 105/255.0f},
    {252/255.0f, 205/255.0f, 229/255.0f},
    {217/255.0f, 217/255.0f, 217/255.0f},
    {188/255.0f, 128/255.0f, 189/255.0f},
    {204/255.0f, 235/255.0f, 197/255.0f}
};



//===================================================
// writePly()
//
// Public method that writes the surface mesh
// in the PLY triangle file format.
//===================================================
void TetMesh::writePly(const std::string &filename, bool verbose)
{
    //-----------------------------------
    //           Initialize
    //-----------------------------------
    if(verbose)
        cout << "Writing mesh ply file: " << filename + ".ply" << endl;
    ofstream file((filename + ".ply").c_str());

    std::vector<unsigned int> interfaces;
    std::vector<unsigned int> colors;
    std::vector<unsigned int> keys;

    // determine output faces and vertices vertex counts
    for(int f=0; f < nFaces; f++)
    {
        int t1_index = faces[f].tets[0];
        int t2_index = faces[f].tets[1];

        if(t1_index < 0 || t2_index < 0){
            continue;
        }

        Tet *t1 = this->tets[t1_index];
        Tet *t2 = this->tets[t2_index];

        if(t1->mat_label != t2->mat_label)
        {            
            interfaces.push_back(f);

            unsigned int color_key = (1 << (int)t1->mat_label) + (1 << (int)t2->mat_label);
            int color_index = -1;
            for(unsigned int k=0; k < keys.size(); k++)
            {
                if(keys[k] == color_key){
                    color_index = k;
                    break;
                }
            }
            if(color_index == -1)
            {
                keys.push_back(color_key);
                color_index = keys.size() - 1;
            }

            colors.push_back(color_index);          
        }
    }

    int face_count = interfaces.size();
    int vertex_count = 3*face_count;

    //-----------------------------------
    //           Write Header
    //-----------------------------------
    file << "ply" << endl;
    file << "format ascii 1.0" << endl;
    file << "element vertex " << vertex_count << endl;
    file << "property float x " << endl;
    file << "property float y " << endl;
    file << "property float z " << endl;
    file << "element face " << face_count << endl;
    file << "property list uchar int vertex_index" << endl;
    file << "property uchar red" << endl;
    file << "property uchar green" << endl;
    file << "property uchar blue" << endl;
    file << "end_header" << endl;

    //-----------------------------------
    //         Write Vertex List
    //-----------------------------------
    for(int f=0; f < face_count; f++)
    {
        Face &face = faces[interfaces[f]];

        Vertex3D *v1 = this->verts[face.verts[0]];
        Vertex3D *v2 = this->verts[face.verts[1]];
        Vertex3D *v3 = this->verts[face.verts[2]];

        file << v1->pos().x << " " << v1->pos().y << " " << v1->pos().z << endl;
        file << v2->pos().x << " " << v2->pos().y << " " << v2->pos().z << endl;
        file << v3->pos().x << " " << v3->pos().y << " " << v3->pos().z << endl;
    }

    //-----------------------------------
    //         Write Face List
    //-----------------------------------
    for(int f=0; f < face_count; f++)
    {
        //Face &face = faces[interfaces[f]];

        // output 3 vertices
        file << "3 " << (3*f + 0) << " " << (3*f + 1) << " " << (3*f + 2) << " ";

        // output 3 color components
        file << (int)(255*INTERFACE_COLORS[colors[f]%12][0]) << " ";
        file << (int)(255*INTERFACE_COLORS[colors[f]%12][1]) << " ";
        file << (int)(255*INTERFACE_COLORS[colors[f]%12][2]) << endl;
    }

    // end with a single blank line
    file << endl;

    //-----------------------------------
    //          Close  File
    //-----------------------------------
    file.close();
}

std::pair<int,int> keyToPair(unsigned int key)
{
    std::pair<int,int> labels;

    int offset = 0;
    unsigned int m = (key >> offset) & 1;

    while(!m)
    {
            offset++;
            m = (key >> offset) & 1;
    }
    labels.first = offset;

    offset++;
    m = (key >> offset) & 1;
    while(!m)
    {
            offset++;
            m = (key >> offset) & 1;
    }
    labels.second = offset;


    return labels;
}

//===================================================
// writeMultiplePly()
//
// Public method that writes the surface mesh
// into multiple PLY files. One for each material.
//===================================================
void TetMesh::writeMultiplePly(const vector<std::string> &inputs, const std::string &filename, bool verbose)
{
    //-----------------------------------
    //           Initialize
    //-----------------------------------    
    std::vector<std::vector<unsigned int> > meshes;
    std::vector<unsigned int> interfaces;
    std::vector<unsigned int> colors;
    std::vector<unsigned int> keys;

    while(meshes.size() < inputs.size())
        meshes.push_back(vector<unsigned int>());

    // determine output faces and vertices vertex counts
    for(int f=0; f < nFaces; f++)
    {
        int t1_index = faces[f].tets[0];
        int t2_index = faces[f].tets[1];

        if(t1_index < 0 || t2_index < 0){
            continue;
        }

        Tet *t1 = this->tets[t1_index];
        Tet *t2 = this->tets[t2_index];

        if(t1->mat_label != t2->mat_label)
        {
            // skip boundary padding
            //if(lattice->padded() && (t1->mat_label == lattice->materials() - 1 || t2->mat_label == lattice->materials() - 1))
            //    continue;

            interfaces.push_back(f);

            unsigned int color_key = (1 << (int)t1->mat_label) + (1 << (int)t2->mat_label);
            int color_index = -1;
            for(unsigned int k=0; k < keys.size(); k++)
            {
                if(keys[k] == color_key){
                    color_index = k;
                    break;
                }
            }
            if(color_index == -1)
            {
                keys.push_back(color_key);
                color_index = keys.size() - 1;
            }

            colors.push_back(color_index);


            if(meshes.size() < keys.size()){
                meshes.push_back(vector<unsigned int>());
            }
            meshes[color_index].push_back(f);           

        }
    }


    for(unsigned int m=0; m < meshes.size(); m++)
    {
        if(meshes[m].empty())
            continue;

        std::pair<int,int> mats = keyToPair(keys[m]);

        int mat1 = mats.first;
        int mat2 = mats.second;

        stringstream fns;
        fns << "interface." << mat1 << "-" << mat2 << ".ply";
        string filename = fns.str();
        ofstream file(filename.c_str());

        if(verbose)
            cout << "Writing mesh ply file: " << filename << endl;

        //-----------------------------------
        //           Write Header
        //-----------------------------------
        file << "ply" << endl;
        file << "format ascii 1.0" << endl;
        file << "element vertex " << 3*meshes[m].size() << endl;
        file << "property float x " << endl;
        file << "property float y " << endl;
        file << "property float z " << endl;
        file << "element face " << meshes[m].size() << endl;
        file << "property list uchar int vertex_index" << endl;
        file << "property uchar red" << endl;
        file << "property uchar green" << endl;
        file << "property uchar blue" << endl;
        file << "end_header" << endl;

        //-----------------------------------
        //         Write Vertex List
        //-----------------------------------
        for(unsigned int f=0; f < meshes[m].size(); f++)
        {
            Face &face = faces[meshes[m][f]];

            Vertex3D *v1 = this->verts[face.verts[0]];
            Vertex3D *v2 = this->verts[face.verts[1]];
            Vertex3D *v3 = this->verts[face.verts[2]];

            file << v1->pos().x << " " << v1->pos().y << " " << v1->pos().z << endl;
            file << v2->pos().x << " " << v2->pos().y << " " << v2->pos().z << endl;
            file << v3->pos().x << " " << v3->pos().y << " " << v3->pos().z << endl;
        }

        //-----------------------------------
        //         Write Face List
        //-----------------------------------
        for(unsigned int f=0; f < meshes[m].size(); f++)
        {
            //Face &face = faces[interfaces[f]];
            //int f = meshes[m][ff];

            // output 3 vertices
            file << "3 " << (3*f + 0) << " " << (3*f + 1) << " " << (3*f + 2) << " ";

            // output 3 color components
            file << (int)(255*INTERFACE_COLORS[m%12][0]) << " ";
            file << (int)(255*INTERFACE_COLORS[m%12][1]) << " ";
            file << (int)(255*INTERFACE_COLORS[m%12][2]) << endl;
        }

        // end with a single blank line
        file << endl;

        //-----------------------------------
        //          Close  File
        //-----------------------------------
        file.close();
    }
}



//===================================================
// writeNodeEle()
//
// Public method that writes the mesh
// in the TetGen node/ele file format.
//===================================================
void TetMesh::writeNodeEle(const string &filename, bool verbose)
{
    bool include_materials = true;
    bool include_angles = false;

    //-----------------------------------
    //  Determine Attributes to Include
    //-----------------------------------
    int attribute_count = 0;
    if(include_materials)
        attribute_count++;
    if(include_angles)
        attribute_count++;


    //-----------------------------------
    //         Write Node File
    //-----------------------------------
    string node_filename = filename + ".node";
    if(verbose)
        cout << "Writing mesh node file: " << node_filename << endl;
    ofstream node_file(node_filename.c_str());

    //---------------------------------------------------------------------------------------------------------
    //  First line: <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
    //---------------------------------------------------------------------------------------------------------
    node_file << "# Node count, 3 dim, no attribute, no boundary marker" << endl;
    node_file << this->verts.size() << "  3  0  0" << endl << endl;

    //-------------------------------------------------------------------------------------------
    //  Remaining lines list # of points:  <point #> <x> <y> <z> [attributes] [boundary marker]
    //-------------------------------------------------------------------------------------------
    for(unsigned int i=0; i < this->verts.size(); i++)
    {
        node_file << i+1 << " " << this->verts[i]->pos().x << " " << this->verts[i]->pos().y << " " << this->verts[i]->pos().z << endl;
    }

    node_file.close();


    //-----------------------------------
    //        Write Element File
    //-----------------------------------
    string elem_filename = filename + ".ele";
    if(verbose)
        cout << "Writing mesh ele file: " << elem_filename << endl;
    ofstream elem_file(elem_filename.c_str());

    //---------------------------------------------------------------------------
    //  First line: <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
    //--------------------------------------------------------------------------
    elem_file << "# Tet count, verts per tet, attribute count" << endl;
    elem_file << this->tets.size() << " 4 " << attribute_count << endl << endl;

    //-----------------------------------------------------------------------------------------------------------
    //  Remaining lines list of # of tetrahedra:  <tetrahedron #> <node> <node> <node> <node> ... [attributes]
    //-----------------------------------------------------------------------------------------------------------
    for(unsigned int i=0; i < this->tets.size(); i++)
    {
        elem_file << i+1;
        for(int v=0; v < 4; v++)
            elem_file << " " << this->tets[i]->verts[v]->tm_v_index + 1;
        if(include_materials)
            elem_file << " " << this->tets[i]->mat_label + 1;
        //if(include_angles)
        //    elem_file << " " << tree->tets_JRB[i]->dihedral_angles[0];
        elem_file << endl;
    }

    elem_file.close();
}


//===================================================
// writeOff()
//
// Public method that writes the mesh
// in the TetGen node/ele file format.
//===================================================
void TetMesh::writePtsEle(const std::string &filename, bool verbose)
{
    //-----------------------------------
    //         Create Pts File
    //-----------------------------------
    string pts_filename = filename + ".pts";
    if(verbose)
        cout << "Writing mesh pts file: " << pts_filename << endl;
    ofstream pts_file(pts_filename.c_str());

    //-------------------------------------------------------------------------------------------
    //  Write each line of file <x> <y> <z>
    //-------------------------------------------------------------------------------------------
    for(unsigned int i=0; i < this->verts.size(); i++)
    {
        pts_file << this->verts[i]->pos().x << " " << this->verts[i]->pos().y << " " << this->verts[i]->pos().z << endl;
    }
    pts_file.close();


    //-----------------------------------
    //        Create Element File
    //-----------------------------------
    string elem_filename = filename + ".elem";
    if(verbose)
        cout << "Writing mesh elem file: " << elem_filename << endl;
    ofstream elem_file(elem_filename.c_str());


    //-----------------------------------------------------------------------------------------------------------
    //  Write each line <node> <node> <node> <node>
    //-----------------------------------------------------------------------------------------------------------
    for(unsigned int i=0; i < this->tets.size(); i++)
    {
        elem_file << this->tets[i]->verts[0]->tm_v_index + 1 << " ";
        elem_file << this->tets[i]->verts[1]->tm_v_index + 1 << " ";
        elem_file << this->tets[i]->verts[2]->tm_v_index + 1 << " ";
        elem_file << this->tets[i]->verts[3]->tm_v_index + 1 << endl;
    }
    elem_file.close();

    //-----------------------------------
    //        Create Material File
    //-----------------------------------
    string mat_filename = filename + ".txt";
    cout << "Writing mesh material file: " << mat_filename << endl;
    ofstream mat_file(mat_filename.c_str());
    for(unsigned int i=0; i < this->tets.size(); i++)
    {
        mat_file << this->tets[i]->mat_label + 1 << endl;
    }
    mat_file.close();
}

void TetMesh::constructFaces()
{
    nFaces = 0;

    //-----------------------------------
    //  First Obtain Tet-Tet Adjacency
    //-----------------------------------
    for(unsigned int i=0; i < this->tets.size(); i++)
    {
        // look for a tet sharing three verts opposite vert[j]
        for (int j=0; j < 4; j++)
        {
            if (this->tets[i]->tets[j] == NULL)
            {
                // grab three vertices to compare against
                Vertex3D *v0 = this->tets[i]->verts[(j+1)%4];
                Vertex3D *v1 = this->tets[i]->verts[(j+2)%4];
                Vertex3D *v2 = this->tets[i]->verts[(j+3)%4];
                bool found_adjacent = false;

                // search over adjacent tets touching these verts
                for (unsigned int k=0; k < v0->tets.size(); k++)
                {
                    Tet* tet = v0->tets[k];
                    if(tet != this->tets[i])
                    {
                        // check if it is adjacent, i.e. has v2 and v3
                        int shared_count = 0;
                        for (int l=0; l < 4; l++){
                            if (tet->verts[l] == v1 || tet->verts[l] == v2)
                                shared_count++;
                        }

                        // if match found
                        if (shared_count == 2)
                        {
                            nFaces++;
                            found_adjacent = true;

                            //--------------------
                            //  set for this tet
                            //--------------------
                            this->tets[i]->tets[j] = tet;

                            //----------------------
                            // set for neighbor tet
                            //----------------------
                            // first figure out reversed face
                            int shared_face = 0;
                            for (int m=1; m<4; m++){
                                if(tet->verts[m] != v0 && tet->verts[m] != v1 && tet->verts[m] != v2){
                                    shared_face = m;
                                }
                            }
                            // then set it
                            tet->tets[shared_face] = this->tets[i];

                            // done searching adjacent tets
                            break;
                        }
                    }
                }

                //if there is no face, this is a border face, up the face count
                if (!found_adjacent) {
                    nFaces++;
                }
            }
        }
    }

    //----------------------------
    //  Allocate and Fill Faces
    //----------------------------
    faces = new Face[nFaces];
    int face_count = 0;

    for(unsigned int i=0; i < this->tets.size(); i++)
    {
        for(int j=0; j < 4; j++)
        {
            Face &f = faces[face_count];
            // Face Is Shared?
            if(this->tets[i]->tets[j] && this->tets[i]->faces[j] == -1)
            {
                // make a new face
                f.tets[0] = i;
                f.face_index[0] = j;

                f.verts[0] = this->tets[i]->verts[(j+1)%4]->tm_v_index;
                f.verts[1] = this->tets[i]->verts[(j+2)%4]->tm_v_index;
                f.verts[2] = this->tets[i]->verts[(j+3)%4]->tm_v_index;

                // find the face that had i
                int shared_face = 0;
                for (int k=1; k < 4; k++)
                {
                    if (this->tets[i]->tets[j]->tets[k] == this->tets[i])
                        shared_face = k;
                }

                f.tets[1] = this->tets[i]->tets[j]->tm_index;
                f.face_index[1] = shared_face;

                this->tets[i]->faces[j] = face_count;
                this->tets[i]->tets[j]->faces[shared_face] = face_count;

                face_count++;
            }
            // Boundary Face
            else if(this->tets[i]->tets[j] == NULL){
                f.tets[0] = i;
                f.face_index[0] = j;
                f.tets[1] = -1;
                f.verts[0] = this->tets[i]->verts[(j+1)%4]->tm_v_index;
                f.verts[1] = this->tets[i]->verts[(j+2)%4]->tm_v_index;
                f.verts[2] = this->tets[i]->verts[(j+3)%4]->tm_v_index;
                this->tets[i]->faces[j] = face_count;
                face_count++;
            }
        }
    }

    //-------------------
    //   Normal Loop
    //-------------------
    for(int i=0; i < nFaces; i++)
    {
        Face &f = faces[i];
        Vertex3D *v0 = this->verts[f.verts[0]];
        Vertex3D *v1 = this->verts[f.verts[1]];
        Vertex3D *v2 = this->verts[f.verts[2]];

        vec3 e10 = v1->pos() - v0->pos();
        vec3 e20 = v2->pos() - v0->pos();
        f.normal = normalize(e10.cross(e20));
        vec3 bary = (1.0/3.0)*(v0->pos() + v1->pos() + v2->pos());

        // flip the normal if it points towards tets[0]

        vec3 pos = this->tets[f.tets[0]]->verts[f.face_index[0]]->pos();
        vec3 dir = pos - bary;
        if (dot(dir, f.normal) > 0) {
            //swap normal direction
            f.normal = -1*f.normal;
        }
    }
}



void TetMesh::computeAngles()
{
    float min = 180;
    float max = 0;
    for (unsigned int i=0; i < this->tets.size(); i++)
    {
        Tet *t = this->tets[i];

        //each tet has 6 dihedral angles between pairs of faces
        //compute the face normals for each face
        vec3 face_normals[4];

        for (int j=0; j<4; j++) {
           vec3 v0 = t->verts[(j+1)%4]->pos();
           vec3 v1 = t->verts[(j+2)%4]->pos();
           vec3 v2 = t->verts[(j+3)%4]->pos();
           vec3 normal = normalize(cross(v1-v0,v2-v0));

           // make sure normal faces 4th (opposite) vertex
           vec3 v3 = t->verts[(j+0)%4]->pos();
           vec3 v3_dir = normalize(v3 - v0);
           if(dot(v3_dir, normal) > 0)
               normal *= -1;

           face_normals[j] = normal;
        }


        //now compute the 6 dihedral angles between each pair of faces
        for (int j=0; j<4; j++) {
           for (int k=j+1; k<4; k++) {
              double dot_product = dot(face_normals[j], face_normals[k]);
              if (dot_product < -1) {
                 dot_product = -1;
              } else if (dot_product > 1) {
                 dot_product = 1;
              }

              double dihedral_angle = 180.0 - acos(dot_product) * 180.0 / PI;

              if (dihedral_angle < min)
              {
                  min = dihedral_angle;
              }
              else if(dihedral_angle > max)
              {
                  max = dihedral_angle;
              }
           }
        }
    }

    min_angle = min;
    max_angle = max;
}

void TetMesh::writeInfo(const string &filename, bool verbose)
{
    //-----------------------------------
    //         Create Pts File
    //-----------------------------------
    std::string info_filename = filename + ".info";
    if(verbose)
        std::cout << "Writing settings file: " << info_filename << std::endl;
    std::ofstream info_file(info_filename.c_str());

    info_file.precision(8);    
    info_file << "min_angle = " << min_angle << std::endl;
    info_file << "max_angle = " << max_angle << std::endl;
    info_file << "tet_count = " << tets.size() << std::endl;
    info_file << "vtx_count = " << verts.size() << std::endl;
    info_file << "mesh time = " << time << "s" << std::endl;

    info_file.close();
}

//===================================================
// writeMatlab()
//
// Public method that writes the mesh
// in the SCIRun-Matlab file format.
//===================================================
void TetMesh::writeMatlab(const std::string &filename, bool verbose)
{
    #ifndef _WIN32
    //-------------------------------
    //         Create File
    //-------------------------------
    std::ofstream file((filename + ".mat").c_str(), std::ios::out | std::ios::binary);
    if(verbose)
        std::cout << "Writing mesh matlab file: " << (filename + ".mat").c_str() << std::endl;

    if(!file.is_open())
    {
        std::cerr << "Failed to create file." << std::endl;
        return;
    }

    //--------------------------------------------------------------
    //        Write Header (128 bytes)
    //
    //      Bytes   1 - 116  : Descriptive Text (116 bytes)
    //      Bytes 117 - 124  : Subsystem Offset (8 bytes)
    //      Bytes 125 - 126  : Matlab Version   ( 2 bytes)
    //      Bytes 127 - 128  : Endian Indicator ( 2 bytes)
    //--------------------------------------------------------------

    // write description
    std::string description = "MATLAB 5.0 MAT-file, SCIRun-TetMesh Created using Cleaver. SCI/Utah | http://www.sci.utah.edu";
    description.resize(116, ' ');
    file.write((char*)description.c_str(), description.length());

    // write offset
    char zeros[32] = {0};
    file.write(zeros, 8);

    // write version
    int16_t version = 0x0100;
    file.write((char*)&version, sizeof(int16_t));

    // write endian
    char endian[2] = {'I','M'};
    file.write(endian, sizeof(int16_t));

    //----------------------------------------------------------
    //  Write Containing Structure
    //  8 byte Tag for Matrix Element
    //  6 Structure SubElements as Data
    //          1 - Array Flags         (8 bytes)
    //          2 - Dimensions Array    numberOfDimensions * sizeOfDataType
    //          3 - Array Name          numberOfCharacters * sizeOfDataType
    //          4 - Field Name Length   (4 bytes)
    //          5 - Field Names         numberOfFields * FieldNameLength;
    //          6 - Node Field
    //          7 - Cell Field
    //          8 - FieldAt Field
    //          9 - Field Field
    //----------------------------------------------------------
    int32_t mainType  = miMATRIX;
    int32_t totalSize = 0;

    // save location, when total size known, come back and fill it in
    long totalSizeAddress = file.tellp();
    totalSizeAddress += sizeof(int32_t);


    file.write((char*)&mainType, sizeof(int32_t));
    file.write((char*)&totalSize, sizeof(int32_t));

    //---------------------------------------------
    //       Write Array Flags SubElement
    //
    //   bytes 1 - 2  : undefined (2 bytes)
    //   byte  3      : flags  (1 byte)
    //   byte  4      : class  (1 byte)
    //   bytes 5 - 8  : undefined (4 bytes)
    //---------------------------------------------

    int32_t flagsType = miUINT32;
    int32_t flagsSize = 8;

    file.write((char*)&flagsType, sizeof(int32_t));
    file.write((char*)&flagsSize, sizeof(int32_t));

    int8_t flagsByte = 0;
    int8_t classByte = mxSTRUCT_CLASS;

    file.write((char*)&classByte, sizeof(int8_t));
    file.write((char*)&flagsByte, sizeof(int8_t));
    file.write(zeros, 2);
    file.write(zeros, 4);

    //---------------------------------------------
    //     Write Dimensions Array SubElement
    //
    //---------------------------------------------
    int32_t dimensionsType = miINT32;
    int32_t dimensionsSize = 8;
    int32_t dimension = 1;

    file.write((char*)&dimensionsType, sizeof(int32_t));
    file.write((char*)&dimensionsSize, sizeof(int32_t));
    file.write((char*)&dimension, sizeof(int32_t));
    file.write((char*)&dimension, sizeof(int32_t));

    //---------------------------------------------
    //     Write Array Name SubElement
    //---------------------------------------------
    int8_t  arrayName[8] = {'t','e','t','m','e','s','h','\0'};
    int32_t arrayNameType = miINT8;
    int32_t arrayNameSize = 7;

    file.write((char*)&arrayNameType, sizeof(int32_t));
    file.write((char*)&arrayNameSize, sizeof(int32_t));
    file.write((char*)arrayName,    8*sizeof(int8_t));


    //---------------------------------------------
    //  Write Field Name Length SubElement
    //---------------------------------------------
    int16_t fieldNameLengthSize = sizeof(int32_t);
    int16_t fieldNameLengthType = miINT32;
    int32_t fieldNameLengthData = 8;

    file.write((char*)&fieldNameLengthType, sizeof(int16_t));
    file.write((char*)&fieldNameLengthSize, sizeof(int16_t));
    file.write((char*)&fieldNameLengthData, sizeof(int32_t));

    //---------------------------------------------
    //  Write Field Names
    //---------------------------------------------
    int32_t fieldNamesType = miINT8;
    int32_t fieldNamesSize = 8*4;

    file.write((char*)&fieldNamesType, sizeof(int32_t));
    file.write((char*)&fieldNamesSize, sizeof(int32_t));

    strcpy(zeros, "node");
    file.write(zeros, 8*sizeof(char));
    memset(zeros, 0, 32);

    strcpy(zeros, "cell");
    file.write(zeros, 8*sizeof(char));
    memset(zeros, 0, 32);

    strcpy(zeros, "field");
    file.write(zeros, 8*sizeof(char));
    memset(zeros, 0, 32);

    strcpy(zeros, "fieldat");
    file.write(zeros, 8*sizeof(char));
    memset(zeros, 0, 32);

    //---------------------------------------------
    //  Write Field Cells
    //---------------------------------------------

    //-------------------------------
    //         Write .node
    //-------------------------------
    int32_t nodeType = miMATRIX;
    int32_t nodeSize = 0;

    long nodeSizeAddress = file.tellp();
    nodeSizeAddress += sizeof(int32_t);

    file.write((char*)&nodeType, sizeof(int32_t));
    file.write((char*)&nodeSize, sizeof(int32_t));

    //--------------------------------
    //  Write Node Array flags
    //
    //   bytes 1 - 2  : undefined (2 bytes)
    //   byte  3      : flags  (1 byte)
    //   byte  4      : class  (1 byte)
    //   bytes 5 - 8  : undefined (4 bytes)
    //--------------------------------
    int32_t nodeFlagsType = miUINT32;
    int32_t nodeFlagsSize = 8;

    file.write((char*)&nodeFlagsType, sizeof(int32_t));
    file.write((char*)&nodeFlagsSize, sizeof(int32_t));

    int8_t nodeFlagsByte = 0;
    int8_t nodeClassByte = mxSINGLE_CLASS;

    file.write((char*)&nodeClassByte, sizeof(int8_t));
    file.write((char*)&nodeFlagsByte, sizeof(int8_t));
    file.write(zeros, 2);
    file.write(zeros, 4);

    //---------------------------------------------
    //   Write Node Dimensions Array
    //---------------------------------------------
    int32_t nodeDimensionType = miINT32;
    int32_t nodeDimensionSize = 8;
    int32_t nodeDimensionRows = 3;
    int32_t nodeDimensionCols = verts.size() ;

    file.write((char*)&nodeDimensionType, sizeof(int32_t));
    file.write((char*)&nodeDimensionSize, sizeof(int32_t));
    file.write((char*)&nodeDimensionRows, sizeof(int32_t));
    file.write((char*)&nodeDimensionCols, sizeof(int32_t));

    //---------------------------------------------
    //     Write Node Array Name SubElement
    //---------------------------------------------
    int32_t nodeArrayNameType = miINT8;
    int32_t nodeArrayNameSize = 0;

    file.write((char*)&nodeArrayNameType, sizeof(int32_t));
    file.write((char*)&nodeArrayNameSize, sizeof(int32_t));

    //----------------------------------------------
    //     Write Node Pr Array Data SubElement
    //----------------------------------------------
    int32_t nodeDataType = miSINGLE;
    int32_t nodeDataSize = nodeDimensionRows*nodeDimensionCols*sizeof(float_t);
    int32_t nodePadding = (8 - (nodeDataSize % 8)) % 8;

    file.write((char*)&nodeDataType, sizeof(int32_t));
    file.write((char*)&nodeDataSize, sizeof(int32_t));

    for(unsigned int i=0; i < verts.size(); i++)
    {
        float_t x = verts[i]->pos().x;
        float_t y = verts[i]->pos().y;
        float_t z = verts[i]->pos().z;

        file.write((char*)&x, sizeof(float_t));
        file.write((char*)&y, sizeof(float_t));
        file.write((char*)&z, sizeof(float_t));
    }
    if(nodePadding)
        file.write((char*)zeros, nodePadding);
    long nodeEndAddress = file.tellp();

    //-------------------------------
    //         Write .cell
    //-------------------------------
    int32_t cellType = miMATRIX;
    int32_t cellSize = 0;

    long cellSizeAddress = file.tellp();
    cellSizeAddress += sizeof(int32_t);

    file.write((char*)&cellType, sizeof(int32_t));
    file.write((char*)&cellSize, sizeof(int32_t));

    //--------------------------------
    //  Write Cell Array flags
    //
    //   bytes 1 - 2  : undefined (2 bytes)
    //   byte  3      : flags  (1 byte)
    //   byte  4      : class  (1 byte)
    //   bytes 5 - 8  : undefined (4 bytes)
    //--------------------------------
    int32_t cellFlagsType = miUINT32;
    int32_t cellFlagsSize = 8;

    file.write((char*)&cellFlagsType, sizeof(int32_t));
    file.write((char*)&cellFlagsSize, sizeof(int32_t));

    int8_t cellFlagsByte = 0;
    int8_t cellClassByte = mxINT32_CLASS;

    file.write((char*)&cellClassByte, sizeof(int8_t));
    file.write((char*)&cellFlagsByte, sizeof(int8_t));
    file.write(zeros, 2);
    file.write(zeros, 4);

    //---------------------------------------------
    //   Write Cell Dimensions Array
    //---------------------------------------------
    int32_t cellDimensionType = miINT32;
    int32_t cellDimensionSize = 8;
    int32_t cellDimensionRows = 4;
    int32_t cellDimensionCols = tets.size();

    file.write((char*)&cellDimensionType, sizeof(int32_t));
    file.write((char*)&cellDimensionSize, sizeof(int32_t));
    file.write((char*)&cellDimensionRows, sizeof(int32_t));
    file.write((char*)&cellDimensionCols, sizeof(int32_t));

    //---------------------------------------------
    //     Write Cell Array Name SubElement
    //---------------------------------------------
    int32_t cellArrayNameType = miINT8;
    int32_t cellArrayNameSize = 0;

    file.write((char*)&cellArrayNameType, sizeof(int32_t));
    file.write((char*)&cellArrayNameSize, sizeof(int32_t));

    //----------------------------------------------
    //     Write Cell Pr Array Data SubElement
    //----------------------------------------------
    int32_t cellDataType = miINT32;
    int32_t cellDataSize = cellDimensionRows*cellDimensionCols*sizeof(int32_t);
    int32_t cellPadding  = (8 - (cellDataSize % 8)) % 8;

    file.write((char*)&cellDataType, sizeof(int32_t));
    file.write((char*)&cellDataSize, sizeof(int32_t));

    for(unsigned int i=0; i < tets.size(); i++)
    {
        for(int v=0; v < 4; v++){
            int32_t index = tets[i]->verts[v]->tm_v_index;
            file.write((char*)&index, sizeof(int32_t));
        }
    }
    if(cellPadding)
        file.write((char*)zeros, cellPadding);
    long cellEndAddress = file.tellp();

    //----------------------------------
    //         Write .field
    //----------------------------------
    int32_t fieldType = miMATRIX;
    int32_t fieldSize = 0;

    long fieldSizeAddress = file.tellp();
    fieldSizeAddress += sizeof(int32_t);

    file.write((char*)&fieldType, sizeof(int32_t));
    file.write((char*)&fieldSize, sizeof(int32_t));

    //--------------------------------
    //  Write Field Array flags
    //
    //   bytes 1 - 2  : undefined (2 bytes)
    //   byte  3      : flags  (1 byte)
    //   byte  4      : class  (1 byte)
    //   bytes 5 - 8  : undefined (4 bytes)
    //--------------------------------
    int32_t fieldFlagsType = miUINT32;
    int32_t fieldFlagsSize = 8;

    file.write((char*)&fieldFlagsType, sizeof(int32_t));
    file.write((char*)&fieldFlagsSize, sizeof(int32_t));

    int8_t fieldFlagsByte = 0;
    int8_t fieldClassByte = mxUINT8_CLASS;

    file.write((char*)&fieldClassByte, sizeof(int8_t));
    file.write((char*)&fieldFlagsByte, sizeof(int8_t));
    file.write(zeros, 2);
    file.write(zeros, 4);

    //---------------------------------------------
    //   Write Field Dimensions Array
    //---------------------------------------------
    int32_t fieldDimensionType = miINT32;
    int32_t fieldDimensionSize = 8;
    int32_t fieldDimensionRows = 1;
    int32_t fieldDimensionCols = tets.size();

    file.write((char*)&fieldDimensionType, sizeof(int32_t));
    file.write((char*)&fieldDimensionSize, sizeof(int32_t));
    file.write((char*)&fieldDimensionRows, sizeof(int32_t));
    file.write((char*)&fieldDimensionCols, sizeof(int32_t));

    //---------------------------------------------
    //     Write Field Array Name SubElement
    //---------------------------------------------
    int32_t fieldArrayNameType = miINT8;
    int32_t fieldArrayNameSize = 0;

    file.write((char*)&fieldArrayNameType, sizeof(int32_t));
    file.write((char*)&fieldArrayNameSize, sizeof(int32_t));

    //----------------------------------------------
    //     Write Field Pr Array Data SubElement
    //----------------------------------------------
    int32_t fieldDataType = miUINT8;
    int32_t fieldDataSize = fieldDimensionRows*fieldDimensionCols*sizeof(int8_t);
    int32_t fieldPadding  = (8 - (fieldDataSize % 8)) % 8;

    file.write((char*)&fieldDataType, sizeof(int32_t));
    file.write((char*)&fieldDataSize, sizeof(int32_t));

    for(unsigned int i=0; i < tets.size(); i++)
    {
        unsigned char m = tets[i]->mat_label;
        file.write((char*)&m, sizeof(int8_t));
    }
    if(fieldPadding)
        file.write(zeros, fieldPadding);
    long fieldEndAddress = file.tellp();


    //-------------------------------
    //         Write .fieldat
    //-------------------------------
    int32_t atType = miMATRIX;
    int32_t atSize = 0;

    long atSizeAddress = file.tellp();
    atSizeAddress += sizeof(int32_t);

    file.write((char*)&atType, sizeof(int32_t));
    file.write((char*)&atSize, sizeof(int32_t));

    //--------------------------------
    //  Write At Array flags
    //
    //   bytes 1 - 2  : undefined (2 bytes)
    //   byte  3      : flags  (1 byte)
    //   byte  4      : class  (1 byte)
    //   bytes 5 - 8  : undefined (4 bytes)
    //--------------------------------
    int32_t atFlagsType = miUINT32;
    int32_t atFlagsSize = 8;

    file.write((char*)&atFlagsType, sizeof(int32_t));
    file.write((char*)&atFlagsSize, sizeof(int32_t));

    int8_t atFlagsByte = 0;
    int8_t atClassByte = mxCHAR_CLASS;

    file.write((char*)&atClassByte, sizeof(int8_t));
    file.write((char*)&atFlagsByte, sizeof(int8_t));
    file.write(zeros, 2);
    file.write(zeros, 4);

    //---------------------------------------------
    //   Write 'At' Dimensions Array
    //---------------------------------------------
    std::string fieldAtString = "cell";
    int32_t atDimensionType = miINT32;
    int32_t atDimensionSize = 8;
    int32_t atDimensionRows = 1;
    int32_t atDimensionCols = fieldAtString.length();

    file.write((char*)&atDimensionType, sizeof(int32_t));
    file.write((char*)&atDimensionSize, sizeof(int32_t));
    file.write((char*)&atDimensionRows, sizeof(int32_t));
    file.write((char*)&atDimensionCols, sizeof(int32_t));

    //---------------------------------------------
    //     Write 'At' Array Name SubElement
    //---------------------------------------------
    int32_t atArrayNameType = miINT8;
    int32_t atArrayNameSize = 0;

    file.write((char*)&atArrayNameType, sizeof(int32_t));
    file.write((char*)&atArrayNameSize, sizeof(int32_t));

    //----------------------------------------------
    //     Write 'At' Pr Array Data SubElement
    //----------------------------------------------
    int32_t atDataType = miUTF8;
    int32_t atDataSize = fieldAtString.length();
    int32_t atPadding = (8 - (atDataSize % 8)) % 8;

    file.write((char*)&atDataType, sizeof(int32_t));
    file.write((char*)&atDataSize, sizeof(int32_t));

    file.write((char*)fieldAtString.c_str(), fieldAtString.length());

    if(atPadding)
        file.write((char*)zeros, atPadding);
    long atEndAddress = file.tellp();
    long fileEndAddress = file.tellp();

    //-------------------------------
    //  Finally, Compute Sizes and
    //         Write Them
    //-------------------------------
    totalSize = fileEndAddress -  (totalSizeAddress + sizeof(int32_t));
    nodeSize  = nodeEndAddress -  (nodeSizeAddress  + sizeof(int32_t));
    cellSize  = cellEndAddress -  (cellSizeAddress  + sizeof(int32_t));
    fieldSize = fieldEndAddress - (fieldSizeAddress + sizeof(int32_t));
    atSize = atEndAddress - (atSizeAddress + sizeof(int32_t));

    file.seekp(totalSizeAddress);
    file.write((char*)&totalSize, sizeof(int32_t));

    file.seekp(nodeSizeAddress);
    file.write((char*)&nodeSize,  sizeof(int32_t));

    file.seekp(cellSizeAddress);
    file.write((char*)&cellSize,  sizeof(int32_t));

    file.seekp(fieldSizeAddress);
    file.write((char*)&fieldSize, sizeof(int32_t));

    file.seekp(atSizeAddress);
    file.write((char*)&atSize,    sizeof(int32_t));

    //-------------------------------
    //   Done
    //-------------------------------
    file.flush();
    file.close();

    #endif
}

//===================================================
//  tet_volume()
//
// Helper function to compute the oriented volume
// of a tet, identified by its 4 vertices.
//===================================================
double Tet::volume() const
{
    vec3 a = verts[0]->pos();
    vec3 b = verts[1]->pos();
    vec3 c = verts[2]->pos();
    vec3 d = verts[3]->pos();

    return dot(a - d, cross(b-d, c-d)) / 6.0;
}

}
