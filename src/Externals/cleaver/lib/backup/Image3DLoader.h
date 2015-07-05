//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// Cleaver - A MultiMaterial Tetrahedral Mesher
// -- Image Loader
//
//  Author: Jonathan Bronson (bronson@sci.utah.edu)
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  Copyright (C) 2012, Jonathan Bronson
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


#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "BCCLattice3D.h"
//#include "Field3D.h"
#include <string>

namespace Cleaver
{

enum type { BYTE, UBYTE, FLOAT, DOUBLE, INT };
enum format { IMG, LBL, PNG };

class Image3DLoader
{
public:

    static BCCLattice3D* load(const std::string &filename, bool pad = false);
    //static BCCLattice3D* constructLatticeFromArray(int w, int h, int d, int m, float *data, bool padded = false);
    //static BCCLattice3D* constructLatticeFromField(const Field3D *field, bool padded = false, float sw = 1.0, float sh = 1.0, float sd = 1.0);

    static BCCLattice3D* loadStuffFile(const std::string &filename, bool pad = false);
    static BCCLattice3D* loadNRRDFile(const std::string &filename, bool pad = false);
    static BCCLattice3D* loadNHDRFile(const std::string &filename, bool pad = false);

    static BCCLattice3D* loadNRRDFiles(const std::vector<std::string> &filenames, bool pad = false);
    static BCCLattice3D* loadNRRDFiles_OLD(const std::vector<std::string> &filenames, bool pad = false);

private:
    static float* readImageData(int &w, int &h, int &d, int &m, type t, format f, std::vector<std::string> filenames, const bool &pad = false);


    //static BCCLattice3D* constructLatticeFromField(Field3D *field);

    //static void enforce_field_constraint(BCCLattice3D *lattice);
    //static void interpolate_dual(BCCLattice3D *lattice);
    //static void interpolate_cell(OTCell *cell, BCCLattice3D *lattice);
    //static void create_dual_vertex(OTCell *cell, BCCLattice3D *lattice, const Field3D *field);
    //static OTCell* addCell(BCCLattice3D *lattice, int i, int j, int k, const Field3D *field = NULL);
};

}

#endif // IMAGELOADER_H
