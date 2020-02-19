/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
//  - Command Line Program
//
// Primary Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#include <iostream>
#include <string>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <Cleaver/Cleaver.h>
#include <Cleaver/InverseField.h>
#include <Cleaver/PaddedVolume.h>
#include <Cleaver/Volume.h>
#include "nrrd2cleaver.h"

void parse_input(int argc, char *argv[]);
void print_usage();
void print_help();

const std::string scirun = "scirun";
const std::string tetgen = "tetgen";
const std::string matlab = "matlab";

std::vector<std::string> inputs;
std::string output = "output";
std::string format = "tetgen";
bool padding = false;
bool verbose = true;
bool absolute_resolution = false;
bool scaled_resolution = false;
int rx,ry,rz;
float sx,sy,sz;
float alpha_short = Cleaver::DefaultAlphaShort;
float alpha_long  = Cleaver::DefaultAlphaLong;


int main(int argc, char *argv[])
{
    //-------------------------------
    //  Parse Inputs
    //-------------------------------
    parse_input(argc, argv);

    //-------------------------------
    //  Load Data & Create Lattice
    //-------------------------------

     //
    //std::vector<Cleaver::ScalarField*> fields;
    //for (each input field F)
    //{
    //   Cleaver::FloatField *field = new FloatField(F.rawDataPointer(), F.width, F.height, F.depth);
    //   fields.push_back(field);
   // }



    std::vector<Cleaver::ScalarField*> fields = loadNRRDFiles(inputs, verbose);
    if(fields.empty()){
        std::cerr << "Failed to load image data. Terminating." << std::endl;
        return 0;
    }
    else if(fields.size() == 1)
        fields.push_back(new Cleaver::InverseField(fields[0]));

    Cleaver::AbstractVolume *volume = new Cleaver::Volume(fields);


    if(absolute_resolution)
        ((Cleaver::Volume*)volume)->setSize(rx,ry,rz);
    if(scaled_resolution)
        ((Cleaver::Volume*)volume)->setSize(sx*volume->size().x,
                                            sy*volume->size().y,
                                            sz*volume->size().z);

    if(padding)
        volume = new Cleaver::PaddedVolume(volume);

    std::cout << "Creating Mesh with Volume Size " << volume->size().toString() << std::endl;

    //--------------------------------
    //  Create Mesher & TetMesh
    //--------------------------------
    Cleaver::TetMesh *mesh = Cleaver::createMeshFromVolume(volume, verbose);

    // mesh.verts[i].pos().x ; mesh.verts[i].pos().y ; mesh.verts[i].pos().z
    // mesh.tets[i].verts[0].tm_vindex
    // mesh.tets[i].mat_label

    //------------------
    //  Compute Angles
    //------------------
    mesh->computeAngles();
    if(verbose){
        std::cout.precision(12);
        std::cout << "Worst Angles:" << std::endl;
        std::cout << "min: " << mesh->min_angle << std::endl;
        std::cout << "max: " << mesh->max_angle << std::endl;
    }

    //----------------------
    //  Write Info File
    //----------------------
    mesh->writeInfo(output, verbose);

    //----------------------
    // Write Tet Mesh Files
    //----------------------
    if(format == tetgen)
        mesh->writeNodeEle(output, verbose);
    else if(format == scirun)
        mesh->writePtsEle(output, verbose);
    else if(format == matlab)
        mesh->writeMatlab(output, verbose);

    //----------------------
    // Write Surface Files
    //----------------------
    mesh->constructFaces();
    mesh->writePly(output, verbose);
    //mesh->writeMultiplePly(inputs, output, verbose);


    //-----------
    // Cleanup
    //-----------
    if(verbose)
        std::cout << "Cleaning up." << std::endl;
    delete mesh;
    for(unsigned int f=0; f < fields.size(); f++)
        delete fields[f];
    delete volume;

    //-----------
    //  Done
    //-----------
    if(verbose)
        std::cout << "Done." << std::endl;

    return 0;
}


void parse_input(int argc, char *argv[])
{
    // Verify input is minimal length
   int expected_length = 3;
   if(argc < 2)
       print_usage();

   int a = 1;
   std::string token = argv[a];

   //------------------------
   //  Parse Help Flag
   //------------------------
   if(token.compare("-h") == 0)
   {
       print_help();
       exit(0);
   }

   //------------------------
   //  Parse Silent Flag
   //------------------------
   if(token.compare("-s") == 0)
   {

       if(argc < ++expected_length)
           print_usage();
       verbose = false;
       token = argv[++a];
   }

   //------------------------
   //  Parse Padding Flag
   //------------------------
   if(token.compare("-p") == 0)
   {

       if(argc < ++expected_length)
           print_usage();
       padding = true;
       token = argv[++a];
   }

   //--------------------------
   //  Parse Alpha Flags
   //--------------------------
   if(token.compare("-as") == 0)
   {
       expected_length += 2;
       if(argc < expected_length)
           print_usage();

       token = argv[++a];
       if(token[0] == '-')
       {
           print_usage();
           exit(0);
       }

       alpha_short = atoi(token.c_str());
       token = argv[++a];
   }
   if(token.compare("-al") == 0)
   {
       expected_length += 2;
       if(argc < expected_length)
           print_usage();

       token = argv[++a];
       if(token[0] == '-')
           print_usage();

       alpha_long = atoi(token.c_str());
       token = argv[++a];
   }

   //--------------------------
   //   Parse Input Flag
   //--------------------------
   if(token.compare("-i") == 0)
   {
       while(a+1 < argc)
       {
           token = argv[++a];
           if(token[0] == '-')
               break;

           inputs.push_back(token);
       }

       expected_length += std::max<int>(0, inputs.size() - 1);
   }
   else
       print_usage();

   //--------------------------
   //   Parse Output Flag
   //--------------------------
   if(token.compare("-o") == 0)
   {
       expected_length += 2;
       if(argc < expected_length)
           print_usage();

       token = argv[++a];
       if(token[0] == '-')
       {
           std::cerr << "Error:  no output name specified with -o flag" << std::endl;
           print_usage();
       }

       output = token;

       if(a+1 < argc)
           token = argv[++a];
   }

   //--------------------------
   //   Parse Format Flag
   //--------------------------
   if(token.compare("-f") == 0)
   {
       expected_length += 2;
       if(argc < expected_length)
           print_usage();

           format = argv[++a];

       if(format.length() != 6){
           std::cerr << "invalid output format: " << format << std::endl;
           print_usage();
       }

       if(format != scirun &&
          format != tetgen &&
          format != matlab)
       {
           std::cerr << "invalid output format: " << format << std::endl;
           print_usage();
       }
       if(a+1 < argc)
           token = argv[++a];
   }

   //-------------------------
   //   Resolution Flag
   //-------------------------
   if(token.compare("-ra") == 0)
   {
       absolute_resolution = true;

       if(a+3 < argc)
       {
           rx = atoi(argv[++a]);
           ry = atoi(argv[++a]);
           rz = atoi(argv[++a]);
       }
       else{
           print_usage();
       }
   }
   else if(token.compare("-rs") == 0)
   {
        scaled_resolution = true;

        if(a+3 < argc)
        {
            sx = atof(argv[++a]);
            sy = atof(argv[++a]);
            sz = atof(argv[++a]);
        }
        else{
            print_usage();
        }
   }

}





void print_usage()
{
    std::cerr << "usage: cleaver -s -p -as [val] -al [val] -i [input1.nrrd input2.nrrd ...] -o [output] -f [format] -r[a] [x] [y] [z]" << std::endl;
    std::cerr << "   -h   help                            " << std::endl;
    std::cerr << "   -s   silent mode       default=off   " << std::endl;
    std::cerr << "   -p   use padding       default=no    " << std::endl;
    std::cerr << "   -as  alpha short       default=0.357 " << std::endl;
    std::cerr << "   -al  alpha long        default=0.203 " << std::endl;
    std::cerr << "   -i   input filenames   minimimum=1   " << std::endl;
    std::cerr << "   -o   output filename   default=output" << std::endl;
    std::cerr << "   -f   output format     default=tetgen" << std::endl;
    std::cerr << "   -ra  x y z absolute  resolution      " << std::endl;
    std::cerr << "   -rs  sx sy sz scaled resolution      " << std::endl;

    std::cerr << std::endl;

    std::cerr << "   Valid Parameters:                " << std::endl;
    std::cerr << "        alpha short       0.0 to 1.0" << std::endl;
    std::cerr << "        alpha long        0.0 to 1.0" << std::endl;
    std::cerr << "        tetmesh formats   tetgen, scirun, matlab" << std::endl << std::endl;
    //cerr << "        trimesh formats   ply, stl, obj" << endl << endl;


    std::cerr << "Examples:" << std::endl;
    std::cerr << "cleaver -h                                         print help guide" << std::endl;
    std::cerr << "cleaver -i mat1.nrrd mat2.nrrd                     basic use" << std::endl;
    std::cerr << "cleaver -p -i mat1.nrrd mat2.nrrd                  use padding" << std::endl;
    std::cerr << "cleaver -i mat1.nrrd mat2.nrrd -o mesh             specify target name" << std::endl;
    std::cerr << "cleaver -i mat1.nrrd mat2.nrrd -o mesh -f scirun   specify target format" << std::endl;
    std::cerr << "cleaver -as 0.15 -i mat1.nrrd mat2.nrrd            specify alpha short" << std::endl << std::endl;

    exit(0);
}

void print_help()
{
    std::cerr << "Cleaver" << std::endl;
    std::cerr << "A Conforming, Multimaterial, Tetrahedral Mesh Generator" << std::endl;
    std::cerr << "with Bounded Element Quality." << std::endl;
    std::cerr << Cleaver::Version << std::endl << std::endl;

    std::cerr << "Copyright (C) 2013" << std::endl;
    std::cerr << "Jonathan Bronson" << std::endl;
    std::cerr << "bronson@sci.utah.edu" << std::endl;
    std::cerr << "Scientific Computing & Imaging Institute" << std::endl;
    std::cerr << "University of Utah, School of Computing" << std::endl;
    std::cerr << "Salt Lake City, Utah" << std::endl << std::endl;

    std::cerr << "What Can Cleaver Do?" << std::endl << std::endl;
    std::cerr << "  " << "Cleaver generates conforming tetrahedral meshes for" << std::endl;
    std::cerr << "  " << "multimaterial or multiphase volumetric data. Both  " << std::endl;
    std::cerr << "  " << "geometric accuracy and element quality are bounded." << std::endl;
    std::cerr << "  " << "The method is a stencil-based approach, and relies " << std::endl;
    std::cerr << "  " << "on an octree structure to provide a coarse level of" << std::endl;
    std::cerr << "  " << "grading in regions of homogeneity.                 " << std::endl;

    std::cerr << "What does Cleaver use as input?" << std::endl << std::endl;
    std::cerr << "  " << "The cleaving algorithm works by utilizing indicator" << std::endl;
    std::cerr << "  " << "functions. These functions indicate the strength or" << std::endl;
    std::cerr << "  " << "relative presence of a particular material. At each" << std::endl;
    std::cerr << "  " << "point, only the material with the largest indicator" << std::endl;
    std::cerr << "  " << "value is considered present. In practice, inside-  " << std::endl;
    std::cerr << "  " << "outside and distance functions are most common.    " << std::endl << std::endl;

    std::cerr << "What is the input format?" << std::endl;
    std::cerr << "  " << "Cleaver takes the Nearly Raw Raster Data format, or" << std::endl;
    std::cerr << "  " << "NRRD, as input. Information on the format is avail-" << std::endl;
    std::cerr << "  " << "able at the Teem website:                          " << std::endl;
    std::cerr << "  " << "http://teem.sourceforge.net/nrrd/format.html       " << std::endl << std::endl;

    std::cerr << "What is the output format?" << std::endl;
    std::cerr << "  " << "Cleaver can output into three mesh formats." << std::endl;
    std::cerr << "  " << "1) TetGen:  .node, .ele                    " << std::endl;
    std::cerr << "  " << "2) SCIRun:  .pts,  .elem, .txt             " << std::endl;
    std::cerr << "  " << "3) Matlab:  .mat                           " << std::endl;
    std::cerr << "  " <<  std::endl;
    std::cerr << "  " << "In addition, Cleaver outputs a .info file  " << std::endl;
    std::cerr << "  " << "with more details about the output mesh.   " << std::endl << std::endl;

    print_usage();

    std::cerr << std::endl << std::endl;
}
