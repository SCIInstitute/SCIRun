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
// Copyright (c) 2011, Scientific Computing & Imaging Institute and
// the University of Utah. All Rights Reserved.
//
// Redistribution and use in source and binary forms is strictly
// prohibited without the direct consent of the author.
//
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------


#include "Image3DLoader.h"
//#include "ImageField3D.h"
//#include <teem/nrrd.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include "TetMesh.h"

using namespace std;
using namespace Cleaver;


#define REALLY_SMALL -10000;
#define REALLY_LARGE +10000;


//===============================================
// load()
//
// - This is the main access point to load a 3D
//   dataset. It expects the .stuff fileformat
//   and parses it to determine input parameters.
//================================================
BCCLattice3D* Image3DLoader::load(const std::string &filename, bool pad)
{
    string fullname = filename;
    int index = fullname.rfind(".");

    string extension = fullname.substr(index+1, fullname.length() - index);

    if(extension.compare("stuff") == 0 || extension.compare("STUFF") == 0)
    {
        return loadStuffFile(filename, pad);
    }
    else if(extension.compare("nrrd") == 0 || extension.compare("NRRD") == 0)
    {
        return loadNRRDFile(filename, pad);
    }
    else if(extension.compare("nhdr") == 0 || extension.compare("NHDR") == 0)
    {
        return loadNHDRFile(filename, pad);
    }
    else
    {
        cerr << "Failed to identify file extension: " << extension << endl;
        return NULL;
    }
}

//=================================================================================================================================
// loadStuffFile()
//
// This routine should only be called if the filename given is already identified as
// being of the *.stuff file format.
//
//=================================================================================================================================
BCCLattice3D* Image3DLoader::loadStuffFile(const std::string &filename, bool pad)
{
    //----------------------------------------------------------
    // open .stuff file, get type information, create lattice
    //----------------------------------------------------------
    cout << "Attempting to Load Stuff File: " << filename << endl;

    string fullname = filename;
    int index = fullname.rfind("/");
    string path = fullname.substr(0, index+1);


    ifstream file;
    format f;
    type t;
    int w, h, d, m, dim;
    string datafilename;
    string token1, token2;

    BCCLattice3D *lattice3D = NULL;
    file.open(filename.c_str());

    if(!file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return NULL;
    }

    file >> token1 >> m;
    if(token1 == "IMG"){
        f = IMG;
        file >> token2 >> dim;

        if(token2 == "BYTE")
            t = BYTE;
        else if(token2 == "UBYTE")
            t = UBYTE;
        else if(token2 == "FLOAT")
            t = FLOAT;
        else if(token2 == "DOUBLE")
            t = DOUBLE;
        else if(token2 == "INT")
            t = INT;
        else{
            cerr << "Invalid File Format: Type == " << token2.c_str() << endl;
            file.close();
            return NULL;
        }

        if(dim == 2)
            file >> w >> h;
        else if(dim == 3)
            file >> w >> h >> d;
        else{
            cerr << "Invalid File Format: Dimensions == " << dim << endl;
            file.close();
            return NULL;
        }

        string padding_string;
        vector<string> filenames(m);

        file >> padding_string;
        if(padding_string.compare("REQUIRES_PADDING") == 0)
        {
            pad = true;

            // now read in file names
            for(int i=0; i < m; i++){
                file >> filenames[i];
                filenames[i] = path + filenames[i];
            }
        }
        else{
            // already read first filename
            filenames[0] = path + padding_string;

            // read the rest
            for(int i=1; i < m; i++){
                file >> filenames[i];
                filenames[i] = path + filenames[i];
            }

        }

        file.close();

        if(dim == 3)
        {
            //float *data = Image3DLoader::readImageData(w, h, d, m, t, f, filenames, pad);
            //ImageField3D *imageField = new ImageField3D(w,h,d,m,data);
            //lattice3D  = Image3DLoader::constructLatticeFromField(imageField, pad);

            //lattice3D   = Image3DLoader::constructLatticeFromArray(w, h, d, m, data, pad);
        }
        else{
            cerr << "Unsupported Image Dimension: " << dim << endl;
            return NULL;
        }

    }
    else if(token1 == "LBL"){
        f = LBL;
        file >> token2 >> dim;

        if(token2 == "BYTE")
            t = BYTE;
        else if(token2 == "UBYTE")
            t = UBYTE;
        else if(token2 == "FLOAT")
            t = FLOAT;
        else if(token2 == "DOUBLE")
            t = DOUBLE;
        else if(token2 == "INT")
            t = INT;
        else{
            cerr << "Invalid File Format: Type == " << token2.c_str() << endl;
            file.close();
            return NULL;
        }
        if(dim == 3)
            file >> w >> h >> d;
        else{
            cerr << "Invalid File Format: Dimensions == " << dim << endl;
            file.close();
            return NULL;
        }
        file >> datafilename;
        datafilename = path + datafilename;
        file.close();

        if(dim == 3){
            cerr << "Error: 3D Label Image Data Not Yet Supported." << endl;
            return NULL;
        }

    }
    else{
        cerr << "Invalid File Format: Format == " << token1.c_str() << endl;
        file.close();
        return NULL;
    }

    if(lattice3D == NULL){
        cerr << "Error loading data file (NULL Lattice Returned)" << endl;
        return NULL;
    }
    else
        return lattice3D;
}

/*
BCCLattice3D* Image3DLoader::loadNRRDFiles(const std::vector<std::string> &filenames, bool pad)
{
    //--------------------------------------------
    //  Read only headers of each file
    //-------------------------------------------
    vector<Nrrd*> nins;
    for(unsigned int i=0; i < filenames.size(); i++)
    {
        // create empty nrrd file container
        nins.push_back(nrrdNew());

        // load only the header, not the data
        NrrdIoState *nio = nrrdIoStateNew();
        nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);

        // Read in the Header
        if(nrrdLoad(nins[i], filenames[i].c_str(), nio))
        {
            char *err = biffGetDone(NRRD);
            cerr << "trouble reading file: " << filenames[i] << " : " << err << endl;
            free(err);
            nio = nrrdIoStateNix(nio);
            continue;
        }

        // Done with nrrdIoState
        nio = nrrdIoStateNix(nio);

        if(nins[i]->dim != 3)
        {
            cout << "Fatal Error: volume dimension " << nins[i]->dim << ", expected 3." << endl;
            return NULL;
        }
    }

    //-----------------------------------
    // Verify contents match
    //-----------------------------------
    bool match = true;

    for(unsigned i=1; i < filenames.size(); i++)
    {
        if(nins[i]->dim != nins[0]->dim){
            cerr << "Error, file " << i << " # dims don't match." << endl;
            match = false;
        }

        for(unsigned int j=0; j < nins[0]->dim-1; j++)
            if(nins[i]->axis[j].size != nins[0]->axis[j].size)
            {
                cerr << "Error, file " << j << " dimensions don't match." << endl;
                match = false;
                break;
            }

        if((int)nrrdElementNumber(nins[i]) != (int)nrrdElementNumber(nins[0]))
        {
            cerr << "Error, file " << i << " does not contain expected number of elements" << endl;
            cerr << "Expected: " << (int)nrrdElementNumber(nins[0]) << endl;
            cerr << "Found:    " << (int)nrrdElementNumber(nins[i]) << endl;

            match = false;
            break;
        }
    }

    if(!match)
    {
        return NULL;
    }



    //-----------------------------------
    //       Account For Padding
    //-----------------------------------
    int w = nins[0]->axis[0].size;
    int h = nins[0]->axis[1].size;
    int d = nins[0]->axis[2].size;
    int m = filenames.size();
    int p = 0;

    if(pad){
        p = 2;

        // correct for new grid size
        w += 2*p;
        h += 2*p;
        d += 2*p;

        // add material for 'outside'
        m++;
    }

    //---------------------------------------
    //     Allocate Sufficient Data
    //---------------------------------------
    float *data = new float[w * h * d * m];


    //----------------------------------------
    //  Deferred  Data Load/Copy
    //----------------------------------------
    for(unsigned int f=0; f < filenames.size(); f++)
    {
        // load nrrd data, reading memory into data array
        if(nrrdLoad(nins[f], filenames[f].c_str(), NULL))
        {
            char *err = biffGetDone(NRRD);
            cerr << "trouble reading data in file: " << filenames[f] << " : " << err << endl;
            free(err);
            return NULL;
        }

        float (*lup)(const void *, size_t I);
        lup = nrrdFLookup[nins[f]->type];

        // cast and copy into large array
        int s=0;
        for(int k=p; k < d-p; k++){
            for(int j=p; j < h-p; j++){
                for(int i=p; i < w-p; i++){
                    DATA3D(i,j,k,m,f) = lup(nins[f]->data, s++);
                }
            }
        }

        // free local copy
        nrrdNuke(nins[f]);
    }

    //----------------------------------------------
    //      Copy Boundary Values If Necessary
    //----------------------------------------------
    if(pad)
    {
        // set 'outside' material small everywhere except exterior
        for(int k=0; k < d; k++){
            for(int j=0; j < h; j++){
                for(int i=0; i < w; i++){

                    // is distance to edge less than padding
                    if(k < p || k >= (d - p) ||
                       j < p || j >= (h - p) ||
                       i < p || i >= (w - p))
                    {
                        // set all other materials really small
                        for(unsigned int f=0; f < filenames.size(); f++)
                            DATA3D(i,j,k,m,f) = REALLY_SMALL;
                        // set 'outside' material really large
                        DATA3D(i,j,k,m,m-1) = REALLY_LARGE;
                    }
                    // otherwise
                    else{
                        // set 'outside' material very small
                        DATA3D(i,j,k,m,m-1) = REALLY_SMALL;
                    }

                }
            }
        }
    }

    //------------------------
    //  Set up Projections
    //------------------------
    //vec3 origin = vec3::zero;
    //vec3 directions[3] = {vec3::unitX, vec3::unitY, vec3::unitZ};

    //----------------------------------------
    //   Construct the Lattice
    //----------------------------------------
    //BCCLattice3D *lattice = Image3DLoader::constructLatticeFromArray(w, h, d, m, data, pad);
    //lattice->setOrigin(origin);
    //lattice->setFrame(directions);
    //return lattice;
    return NULL;
}
*/

/*

//=================================================================================================================================
// loadNRRDFile()
//
// This is the new preferred way to load data. A list of input NRRD files is specified. This loader verifies that each file
// is self-consistent (the size specified by its dimensions) and compatible with eachother (same resolution). If these check
// out, the data is loaded and returned as a BCC Lattice structure.
//
//=================================================================================================================================
BCCLattice3D* Image3DLoader::loadNRRDFiles_OLD(const std::vector<std::string> &filenames, bool pad)
{   
    int w=0, h=0, d=0, p=0;
    int m = filenames.size();
    std::string line, token, datafilename;
    std::vector<std::string> tokens;
    std::vector<std::string> datafiles;
    std::stringstream ss;
    type t = FLOAT;
    vec3 origin = vec3::zero;
    vec3 directions[3] = {vec3::unitX, vec3::unitY, vec3::unitZ};

    std::vector<std::ifstream*> files(filenames.size(), NULL);
    std::vector<long> rawStarts(filenames.size(), 0);
    cout << "Loading " << filenames.size() << " input files:" << endl;

    //---------------------------------------
    //    Verify Integrity of Inputs
    //---------------------------------------
    bool problem_flag = false;
    for(unsigned int f=0; f < filenames.size(); f++)
    {
        tokens.clear();

        if(problem_flag)
            break;

        std::string fullname = filenames[f];
        int index = fullname.rfind("/");

        std::string path = fullname.substr(0, index+1);

        cout << filenames[f] << endl;
        files[f] = new std::ifstream(filenames[f].c_str(), ios::in | ios::binary);

        if(!files[f]->is_open())
        {
            cerr << "Failed to open file: " << filenames[f] << endl;
            problem_flag = true;
            continue;
        }

        //--------------------------------------
        //  Read Each Line of the Header File
        //--------------------------------------
        while(getline(*files[f], line))
        {
            //---------------------
            // skip comment lines
            //---------------------
            if(line[0] == '#')
                continue;

            //----------------------
            //  Split Into Tokens
            //----------------------
            ss << line;
            while(getline(ss, token, ':'))
            {
                tokens.push_back(token);
            }

            //------------------------
            //  Done if blank line
            //------------------------
            if(line.empty())
            {
                ss.str("");
                ss.clear();
                tokens.clear();
                break;
            }

            //-----------------------------------
            // Strip leading Whitespace
            //-----------------------------------
            for(unsigned int i=1; i < tokens.size(); i++)
            {
                tokens[i] = tokens[i].substr(1);
            }

            //-----------------------------------
            //      parse 'type' field
            //-----------------------------------
            if(tokens[0].compare("type") == 0)
            {
                if(tokens[1].compare("unsigned char") == 0)
                    t = UBYTE;
                else if(tokens[1].compare("float") == 0)
                    t = FLOAT;
                else if(tokens[1].compare("double") == 0)
                    t = DOUBLE;
                else{
                    cerr << "unsupported file format: Type = " << tokens[1] << endl;
                    problem_flag = true;
                    break;
                }

                //cout << "Read Type as: " << tokens[1] << endl;
            }
            //-----------------------------------
            //      parse 'dimension' field
            //-----------------------------------
            else if(tokens[0].compare("dimension") == 0)
            {
                int dim = atoi(tokens[1].c_str());

                if(dim != 3)
                {
                    cerr << "unsupported dimensionality: " << dim << endl;
                    problem_flag = true;
                    break;
                }
                //cout << "Read dimension as \"" << dim << "\"" << endl;
            }
            //-----------------------------------
            //      parse 'sizes' field
            //-----------------------------------
            else if(tokens[0].compare("sizes") == 0)
            {
                ss.str("");
                ss.clear();

                ss << tokens[1];
                if(f == 0)
                    ss >> w >> h >> d;
                else
                {
                    int w2=0,h2=0,d2=0;
                    ss >> w2 >> h2 >> d2;
                    if(w != w2 || h != h2 || d != d2)
                    {
                        cerr << "Dimensions of inputs do not agree." << endl;
                        cerr << filenames[0] << " [ " << w << " x " << h << " x " << d << endl;
                        cerr << filenames[f] << " [ " << w2 << " x " << h2 << " x " << d2 << endl;
                        problem_flag = true;
                        break;
                    }
                }

                //cout << "Read sizes as [" << w << ", " << h << ", " << d << "]" << endl;
            }
            //-----------------------------------
            //   parse 'space directions' field
            //-----------------------------------
            else if(tokens[0].compare("space directions") == 0)
            {
                ss.str("");
                ss.clear();
                ss << tokens[1];

                string token;
                for(int i=0; i < 3; i++)
                {
                    getline(ss, token, ' ');
                    string vector = token.substr(1, token.length()-2);
                    string value;
                    stringstream vs(vector);
                    for(int j=0; j < 3; j++)
                    {
                        getline(vs, value, ',');
                        directions[i][j] = atof(value.c_str());
                    }
                }


                //cout << "Read space directions as: ";
                //for(int i=0; i < 3; i++)
                //    cout << directions[i].toString() << " ";
                //cout << endl;
                //
            }
            //-----------------------------------
            //    parse 'space origin' field
            //-----------------------------------
            else if(tokens[0].compare("space origin") == 0)
            {
                ss.str("");
                ss.clear();
                ss << tokens[1].substr(2, token.length()-3);
                string value;

                for(int i=0; i < 3; i++)
                {
                    getline(ss, value, ',');
                    origin[i] = atof(value.c_str());
                }
                //cout << "Read space origin as: " << origin.toString() << endl;
            }

            //--------------------------------
            //  Reset StringStream + Tokens
            //--------------------------------
            ss.str("");
            ss.clear();
            tokens.clear();
        }

        //------------------------------------
        // Now verify remaining portion of
        // file is the correct size.
        //------------------------------------
        long correctSize = sizeof(float)*w*h*d;
        long here = rawStarts[f] = files[f]->tellg();
        files[f]->seekg(0, ios::end);
        long end = files[f]->tellg();
        long actualSize = end - here;

        if(correctSize != actualSize)
        {
            cerr << "Raw data wrong size. Does not match dimensions." << endl;
            cerr << "Expected Size = " << correctSize << endl;
            cerr << "Actual Size   = " << actualSize << endl;
            problem_flag = true;
        }
    }

    // 1 Last Chance to Cleanup Problems
    if(problem_flag)
    {
        for(unsigned int i=0; i < files.size(); i++)
        {
            if(files[i])
            {
                files[i]->close();
                delete files[i];
            }
            return NULL;
        }
    }

    //-----------------------------------
    //       Account For Padding
    //-----------------------------------
    if(pad){
        p = 2;

        // correct for new grid size
        w += 2*p;
        h += 2*p;
        d += 2*p;

        // add material for 'outside'
        m++;
    }


    //---------------------------------------
    //     Allocate Sufficient Data
    //---------------------------------------
    float *data = new float[w * h * d * m];
    float val = 0;

    //----------------------------------------
    //      Copy Data Form Each File
    //----------------------------------------
    for(unsigned int f=0; f < files.size(); f++)
    {
        files[f]->seekg(rawStarts[f]);

        for(int k=p; k < d-p; k++){
            for(int j=p; j < h-p; j++){
                for(int i=p; i < w-p; i++){

                    files[f]->read((char *)&val, sizeof(float));
                    DATA3D(i,j,k,m,f) = (float)val;
                }
            }
        }
    }   

    //----------------------------------------
    //    Release File Pointers
    //----------------------------------------
    for(unsigned int i=0; i < files.size(); i++)
    {
        if(files[i])
        {
            files[i]->close();
            delete files[i];
        }
    }


    //----------------------------------------------
    //      Copy Boundary Values If Necessary
    //----------------------------------------------
    if(pad)
    {
        // set 'outside' material small everywhere except exterior
        for(int k=0; k < d; k++){
            for(int j=0; j < h; j++){
                for(int i=0; i < w; i++){

                    // is distance to edge less than padding
                    if(k < p || k >= (d - p) ||
                       j < p || j >= (h - p) ||
                       i < p || i >= (w - p))
                    {
                        // set all other materials really small
                        for(unsigned int f=0; f < files.size(); f++)
                            DATA3D(i,j,k,m,f) = REALLY_SMALL;
                        // set 'outside' material really large
                        DATA3D(i,j,k,m,m-1) = REALLY_LARGE;
                    }
                    // otherwise
                    else{
                        // set 'outside' material very small
                        DATA3D(i,j,k,m,m-1) = REALLY_SMALL;
                    }

                }
            }
        }
    }

    //----------------------------------------
    //   Construct the Lattice
    //----------------------------------------
    //BCCLattice3D *lattice = Image3DLoader::constructLatticeFromArray(w, h, d, m, data, pad);
    //lattice->setOrigin(origin);
    //lattice->setFrame(directions);
    //return lattice;
    return NULL;
}
*/


//=================================================================================================================================
// loadNRRDFile()
//
// This routine should only be called if the filename given is already identified as
// being of the *.nrrd file format.
//
//=================================================================================================================================
BCCLattice3D* Image3DLoader::loadNRRDFile(const std::string &filename, bool pad)
{
    //----------------------------------------------------------
    // open .nrrd file, get type information, create lattice
    //----------------------------------------------------------
    cout << "Attempting to load NRRD File: " << filename << endl;

    cerr << "Unable to load nrrd files. Instead use .nhdr  & .raw pair" << endl;
    return NULL;
}

//=================================================================================================================================
// loadNRRDFile()
//
// This routine should only be called if the filename given is already identified as
// being of the *.nrrd file format.
//
//=================================================================================================================================
BCCLattice3D* Image3DLoader::loadNHDRFile(const std::string &filename, bool pad)
{
    std::string fullname = filename;
    int index = fullname.rfind("/");
    std::string path = fullname.substr(0, index+1);


    //----------------------------------------------------------
    // open .nrrd file, get type information, create lattice
    //----------------------------------------------------------
    cout << "Attempting to load nhdr File: " << filename << endl;

    ifstream hdr_file(filename.c_str());
    if(!hdr_file.is_open())
    {
        cerr << "Failed to open file: " << filename << endl;
        return NULL;
    }

    std::string line, token, datafilename;
    std::vector<std::string> tokens;
    std::vector<std::string> datafiles;
    std::stringstream ss;
    //format f;
    type t = FLOAT;
    int w, h, d, m;
    int dim = 3, material_count;
    vec3 directions[3] = {vec3::unitX, vec3::unitY, vec3::unitZ};
    vec3 origin;
    int p = 0;
    bool NRRD_PAD = false;
    float *data = NULL;

    //--------------------------------------
    //  Read Each Line of the Header File
    //--------------------------------------
    while(getline(hdr_file, line))
    {
        //---------------------
        // skip comment lines
        //---------------------
        if(line[0] == '#')
            continue;

        //----------------------
        //  Split Into Tokens
        //----------------------
        ss << line;
        while(getline(ss, token, ':'))
        {
            tokens.push_back(token);
        }

        //-----------------------------------
        // Strip leading Whitespace
        //-----------------------------------
        for(unsigned int i=1; i < tokens.size(); i++)
        {
            tokens[i] = tokens[i].substr(1);
        }

        //-----------------------------------
        //      parse 'type' field
        //-----------------------------------
        if(tokens[0].compare("type") == 0)
        {
            if(tokens[1].compare("unsigned char") == 0)
                t = UBYTE;
            else if(tokens[1].compare("float") == 0)
                t = FLOAT;
            else if(tokens[1].compare("double") == 0)
                t = DOUBLE;
            else{
                cerr << "unsupported file format: Type = " << tokens[1] << endl;
                hdr_file.close();
                return NULL;
            }

            cout << "Read Type as: " << tokens[1] << endl;
        }
        //-----------------------------------
        //      parse 'dimension' field
        //-----------------------------------
        else if(tokens[0].compare("dimension") == 0)
        {
            dim = atoi(tokens[1].c_str());

            if(dim != 3)
            {
                cerr << "unsupported dimensionality: " << dim << endl;
                return NULL;
            }
            cout << "Read dimension as \"" << dim << "\"" << endl;
        }
        //-----------------------------------
        //      parse 'sizes' field
        //-----------------------------------
        else if(tokens[0].compare("sizes") == 0)
        {
            ss.str("");
            ss.clear();

            ss << tokens[1];
            ss >> w >> h >> d;

            cout << "Read sizes as [" << w << ", " << h << ", " << d << "]" << endl;
        }
        //-----------------------------------
        //   parse 'space directions' field
        //-----------------------------------
        else if(tokens[0].compare("space directions") == 0)
        {
            ss.str("");
            ss.clear();
            ss << tokens[1];

            string token;
            for(int i=0; i < dim; i++)
            {
                getline(ss, token, ' ');
                string vector = token.substr(1, token.length()-2);
                string value;
                stringstream vs(vector);
                for(int j=0; j < dim; j++)
                {
                    getline(vs, value, ',');
                    directions[i][j] = atof(value.c_str());
                }
            }

            cout << "Read space directions as: ";
            for(int i=0; i < dim; i++)
                cout << directions[i].toString() << " ";
            cout << endl;
        }
        //-----------------------------------
        //    parse 'space origin' field
        //-----------------------------------
        else if(tokens[0].compare("space origin") == 0)
        {
            ss.str("");
            ss.clear();
            ss << tokens[1].substr(2, token.length()-3);
            string value;

            for(int i=0; i < dim; i++)
            {
                getline(ss, value, ',');
                origin[i] = atof(value.c_str());
            }
            cout << "Read space origin as: " << origin.toString() << endl;
        }
        //-----------------------------------
        //      parse 'data file' field
        //-----------------------------------
        else if(tokens[0].compare("data file") == 0)
        {
            datafilename = tokens[1];
            cout << "Read data file as \"" << datafilename << "\"" << endl;
            datafilename = path + datafilename;
            datafiles.push_back(datafilename);
        }
        //-----------------------------------
        //      parse 'requires padding' field
        //-----------------------------------
        else if(tokens[0].compare("requires padding") == 0)
        {
            if(tokens[1].compare("yes") == 0)
            {
                NRRD_PAD = true;
            }
        }
        else
        {
            // ignore other fields for now
        }

        //--------------------------------
        //  Reset StringStream + Tokens
        //--------------------------------
        ss.str("");
        ss.clear();
        tokens.clear();
    }

    //------------------------
    //  Finished With Header
    //------------------------
    hdr_file.close();


    //------------------------------------------
    // If only 1 data file - it's a label map
    //------------------------------------------
    if(datafiles.size() == 1)
    {
        //----------------------------------
        //     Now Read In Data File
        //----------------------------------
        cout << "Attempting to load datafile: \"" << datafilename << "\"" << endl;

        ifstream datafile(datafilename.c_str(), ios::in | ios::binary);
        if(!datafile.is_open())
        {
            cerr << "Failed to load datafile." << endl;
            return NULL;
        }

        //--------------------------------------------------------
        // make pass over data, determine number of materials
        //--------------------------------------------------------
        unsigned char val;
        int max = 0;

        for(int k=0; k < d; k++){
            for(int j=0; j < h; j++){
                for(int i=0; i < w; i++){
                    datafile.read((char *)&val, sizeof(unsigned char));
                    if(val > max)
                        max = val;
                }
            }
        }

        m = max+1;
        cout << "Data spans " << m << " unique material labels" << endl;


        material_count = m;
        if (NRRD_PAD){
            p = 2;

            // correct for new grid size
            w += 2*p;
            h += 2*p;
            d += 2*p;

            // add material for 'outside'
            m++;
        }


        //----------------------------------------------
        //        Allocate Sufficient Data
        //----------------------------------------------
        data = new float[w * h * d * m];


        //---------------------------------------------------
        //   Make pass to set data with appropriate values
        //--------------------------------------------------
        datafile.clear();
        datafile.seekg(0, ios::beg);


        for(int k=p; k < d-p; k++){
            for(int j=p; j < h-p; j++){
                for(int i=p; i < w-p; i++){

                    datafile.read((char *)&val, sizeof(unsigned char));

                    for(unsigned char mat=0; mat < material_count; mat++)
                    {
                        if(mat == val)
                            DATA3D(i,j,k,m,mat) = 1.0f;
                        else
                            DATA3D(i,j,k,m,mat) = 0.0f;
                    }
                }
            }
        }

        datafile.close();
    }

    //--------------------------------------------------
    // otherwise, each file is a unique material field
    //--------------------------------------------------
    else
    {
        //------------------------------------
        // Determine Material Count + Padding
        //------------------------------------
        material_count = m = datafiles.size();
        if (NRRD_PAD){
            p = 2;

            // correct for new grid size
            w += 2*p;
            h += 2*p;
            d += 2*p;

            // add material for 'outside'
            m++;
        }

        //----------------------------------------------
        //        Allocate Sufficient Data
        //----------------------------------------------
        data = new float[w * h * d * m];

        if(t == DOUBLE)
        {
            double val;
            //----------------------------------
            //     Go Through Each Data File
            //----------------------------------
            for(int mat=0; mat < material_count; mat++)
            {
                //------------------------------------
                //            Open File
                //------------------------------------
                cout << "Attempting to load datafile: " << datafiles[mat] << endl;
                ifstream datafile(datafiles[mat].c_str(), ios::in | ios::binary);
                if(!datafile.is_open())
                {
                    cerr << "Failed to load datafile." << endl;
                    return NULL;
                }

                //------------------------------------
                //         Read In All Data
                //------------------------------------
                for(int k=p; k < d-p; k++){
                    for(int j=p; j < h-p; j++){
                        for(int i=p; i < w-p; i++){

                            datafile.read((char *)&val, sizeof(double));
                            DATA3D(i,j,k,m,mat) = (float)val;
                        }
                    }
                }

                //-------------------------
                //    Done With File
                //-------------------------
                datafile.close();
            }
        }
        else if(t == FLOAT)
        {
            float val;
            //----------------------------------
            //     Go Through Each Data File
            //----------------------------------
            for(int mat=0; mat < material_count; mat++)
            {
                //------------------------------------
                //            Open File
                //------------------------------------
                cout << "Attempting to load datafile: " << datafiles[mat] << endl;
                ifstream datafile(datafiles[mat].c_str(), ios::in | ios::binary);
                if(!datafile.is_open())
                {
                    cerr << "Failed to load datafile." << endl;
                    return NULL;
                }

                //------------------------------------
                //         Read In All Data
                //------------------------------------
                for(int k=p; k < d-p; k++){
                    for(int j=p; j < h-p; j++){
                        for(int i=p; i < w-p; i++){

                            datafile.read((char *)&val, sizeof(float));
                            DATA3D(i,j,k,m,mat) = val;
                        }
                    }
                }

                //-------------------------
                //    Done With File
                //-------------------------
                datafile.close();
            }
        }
    }

    //----------------------------------------------
    //      Copy Boundary Values If Necessary
    //----------------------------------------------
    if(NRRD_PAD)
    {
        // set 'outside' material small everywhere except exterior
        for(int k=0; k < d; k++){
            for(int j=0; j < h; j++){
                for(int i=0; i < w; i++){

                    // is distance to edge less than padding
                    if(k < p || k >= (d - p) ||
                       j < p || j >= (h - p) ||
                       i < p || i >= (w - p))
                    {
                        // set all other materials really small
                        for(int mat=0; mat < material_count; mat++)
                            DATA3D(i,j,k,m,mat) = REALLY_SMALL;
                        // set 'outside' material really large
                        DATA3D(i,j,k,m,m-1) = REALLY_LARGE;
                    }
                    // otherwise
                    else{
                        // set 'outside' material very small
                        DATA3D(i,j,k,m,m-1) = REALLY_SMALL;
                    }

                }
            }
        }
    }



    //-----------------------------------------------------------------------
    //         Construct New Lattice From Data and Return It
    //-----------------------------------------------------------------------
    //ImageField3D *imageField = new ImageField3D(w,h,d,m,data);

    //float sw = 1.0;
    //float sh = 1.0;
    //float sd = 1.0;

    //return Image3DLoader::constructLatticeFromField(imageField, NRRD_PAD, sw, sh, sd);
    //BCCLattice3D *lattice = Image3DLoader::constructLatticeFromArray(w, h, d, m, data, NRRD_PAD);
    //lattice->setOrigin(origin);
    //lattice->setFrame(directions);
    //return lattice;
    return NULL;
}



//=================================================================================================================================
// readImageData()
//
// - This method reads the data from a set of input files and stores them into a single array.
//   This array will then be used for constructing the BCC Lattice.
//=================================================================================================================================
float* Image3DLoader::readImageData(int &w, int &h, int &d, int &m, type t, format f, std::vector<std::string> filenames, const bool &USE_PADDING)
{
    if(USE_PADDING)
        cout << "Loading and Padding " << m << " material files: " << endl;
    else
        cout << "Loading " << m << " material files: " << endl;


    //----------------------------------------------
    //        Factor in Padding if Necessary
    //----------------------------------------------
    int material_count = m;
    int p = 0;
    if (USE_PADDING){
        p = 2;

        // correct for new grid size
        w += 2*p;
        h += 2*p;
        d += 2*p;

        // add material for 'outside'
        m++;
    }


    //----------------------------------------------
    //        Allocate Sufficient Data
    //----------------------------------------------
    float *data = new float[w * h * d * m];

    //----------------------------------------------
    //      Grab Data From Each File
    //----------------------------------------------
    for(int mat=0; mat < material_count; mat++){
        cout << "    " << filenames[mat] << " ..." << flush;

        ifstream file(filenames[mat].c_str(), ios::in | ios::binary);

        if(file.is_open()){
            if(t == FLOAT){
                for(int k=p; k < d-p; k++){
                    for(int j=p; j < h-p; j++){
                        for(int i=p; i < w-p; i++){
                            float *val = &DATA3D(i,j,k,m,mat);
                            file.read((char *)(val), sizeof(float));
                        }
                    }
                }
            }
            else{
                cerr << "Non-FLOAT Data not yet implemented." << endl;
                exit(1337);
            }

            cout << "Successful." << endl;
        }
        else{
            cerr << "Failed; " << endl;
            delete data;
            return NULL;
        }
    }


    //----------------------------------------------
    //      Copy Boundary Values If Necessary
    //----------------------------------------------
    if(USE_PADDING)
    {
        // set 'outside' material small everywhere except exterior
        for(int k=0; k < d; k++){
            for(int j=0; j < h; j++){
                for(int i=0; i < w; i++){

                    // is distance to edge less than padding
                    if(k < p || k >= (d - p) ||
                       j < p || j >= (h - p) ||
                       i < p || i >= (w - p))
                    {
                        // set all other materials really small
                        for(int mat=0; mat < material_count; mat++)
                            DATA3D(i,j,k,m,mat) = REALLY_SMALL;
                        // set 'outside' material really large
                        DATA3D(i,j,k,m,m-1) = REALLY_LARGE;
                    }
                    // otherwise
                    else{
                        // set 'outside' material very small
                        DATA3D(i,j,k,m,m-1) = REALLY_SMALL;
                    }

                }
            }
        }
    }

    return data;
}


//==========================================================================================================
// constructLatticeFromField()
//
// - This method takes in a data field object and uses it to construct the BCC Lattice.
//   This field may be from a discrete array or from analytic data. This approach does
//   not enforce linear interpolation on dual vertices.
//==========================================================================================================
/*
BCCLattice3D* Image3DLoader::constructLatticeFromField(const Field3D *field, bool padded, float sw, float sh, float sd)
{
    //-----------------------------------------------------
    //           Create Storage For Lattice
    //-----------------------------------------------------
    int w = (int)sw*field->width();
    int h = (int)sh*field->height();
    int d = (int)sd*field->depth();
    int m = field->numMaterials();
    BCCLattice3D *lattice = new BCCLattice3D(w,h,d,m);

    lattice->setPadded(padded);

    //-----------------------------------------------------
    //           Sample Grid Values
    //-----------------------------------------------------
    for(int k=0; k < d; k++){
        for(int j=0; j < h; j++){
            for(int i=0; i < w; i++){
                for(int mat=0; mat < m; mat++)
                {
                    float x = (i/(float)w) * field->width();
                    float y = (j/(float)h) * field->height();
                    float z = (k/(float)d) * field->depth();

                    lattice->DATA3D(i,j,k,m,mat) = field->value(x,y,z,mat);
                    //lattice->DATA3D(i,j,k,m,mat) = field->value(i,j,k,mat);
                }
            }
        }
    }

    //-----------------------------------------------------
    //            Find Dominant Materials
    //-----------------------------------------------------
    for(int k=0; k < d; k++){
        for(int j=0; j < h; j++){
            for(int i=0; i < w; i++){

                unsigned char dom = 0;
                float max = lattice->DATA3D(i,j,k,m,dom);
                for(int mat=1; mat < m; mat++){

                    float val = lattice->DATA3D(i,j,k,m,mat);
                    if(val > max){
                        max = val;
                        dom = mat;
                    }
                }
                // make sure no values are equivalent to max
                for(int mat=0; mat < m; mat++){
                    if(lattice->DATA3D(i,j,k,m,mat) >= max && mat != dom){
                        double diff = 1E-6*fabs(max);
                        diff = std::max(1E-6, diff);
                        lattice->DATA3D(i,j,k,m,mat) = (float)(max - diff);
                    }
                }

                // sanity check
                int count = 0;
                for (int mat=0; mat < m; mat++)
                {
                    float val = lattice->DATA3D(i,j,k,m,mat);
                    if(val == max)
                        count++;
                }
                if(count != 1)
                    cout << "Failed to find dominant material in cell [ " << i << "," << j << "," << k << " ]!!" << endl;

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

//                float *LLF_vals = &lattice->DATA3D(i,j,k,m,0);
//                float *LLB_vals = &lattice->DATA3D(i,j,(k+1),m,0);
//                float *LRF_vals = &lattice->DATA3D((i+1),j,k,m,0);
//                float *LRB_vals = &lattice->DATA3D((i+1),j,(k+1),m,0);
//                float *ULF_vals = &lattice->DATA3D(i,(j+1),k,m,0);
//                float *ULB_vals = &lattice->DATA3D(i,(j+1),(k+1),m,0);
//                float *URF_vals = &lattice->DATA3D((i+1),(j+1),k,m,0);
//                float *URB_vals = &lattice->DATA3D((i+1),(j+1),(k+1),m,0);

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
                    if(cell->vert != NULL){
                        cout << "WHOA" << endl;
                        exit(13);
                    }
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
                    // create interpolated center vertex
                    //---------------------------------------
                    //interpolate_cell(cell,lattice);
                    create_dual_vertex(cell,lattice,field);


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

    // TODO: Evaluate the value of using getNeighbor() addNeighbor()  vs. getCell() addCell()
    // Loop over cut cells and add their one rings, so we miss no cuts

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

    // Loop over cut cells, populated and connect central edges and tetrahedra and faces
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


    // Repeat the same for the buffer cells;
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
            //no down cell, this means we need two pyramids to match
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
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p3, v_p0->label);
                lattice->tree->createTet(v_c, v_p1, v_p2, v_p3, v_p0->label);
            }
            else{
                lattice->tree->createTet(v_c, v_p0, v_p1, v_p2, v_p0->label);
                lattice->tree->createTet(v_c, v_p2, v_p3, v_p0, v_p0->label);

            }
        }
    }


    //enforce_field_constraint(lattice);

    lattice->tree->balance_tree_new();
    lattice->tree->create_background_grid(lattice->buffer_cells);
    lattice->tree->label_background_tets(lattice->labels);

    lattice->setDataLoaded(true);

    return lattice;
}
*/

/*
void Image3DLoader::enforce_field_constraint(BCCLattice3D *lattice)
{
    int m = lattice->materials();

    // loop over cut cells
    //for(int c=0; c < lattice->cut_cells.size(); c++)
    for(int c=lattice->cut_cells.size()-1; c >= 0; c--)
    {
        OTCell *cell = lattice->cut_cells[c];

        // loop over 8 primal vertices  (repeats some work)
        for(int v=0; v < 8; v++)
        {
            // gather 8 octants surrounding current vertex
            OTCell *octants[8];
            lattice->getCellsAroundVertex(cell->vert[v], octants);

            int mat = cell->vert[v]->label;

            // look at dual vertices to find label
            bool found = false;
            for(int o=0; o < 8; o++)
            {
                if(octants[o]->vert[C]->label == mat)
                {
                    found = true;
                    break;
                }
            }

            // correct the issue if necessary
            if(!found)
            {

                int dom = -1;
                double max = 0;

                for(int mat=0; mat < m; mat++){

                    bool found = false;
                    for(int o=0; o < 8; o++){
                        found = octants[o]->vert[C]->lbls[mat];
                        if(found)
                            break;
                    }

                    // only allow maximums that occured on corners
                    if(!found)
                        continue;

                    if(dom < 0 || cell->vert[v]->vals[mat] > max){
                        max = cell->vert[v]->vals[mat];
                        dom = mat;
                    }
                }

                // make sure no values are equivalent to max
                for(int mat=0; mat < m; mat++){
                    if(cell->vert[v]->vals[mat] >= max && mat != dom){
                        float diff = 1E-6*fabs(max);
                        diff = fmax(1E-6, diff);
                        cell->vert[v]->vals[mat] = max - diff;
                    }
                }

                // now double check values aren't equivalent
                int max_count = 0;
                for(int mat=0; mat < m; mat++){
                    if(cell->vert[v]->vals[mat] == max){
                        max_count++;
                    }
                }
                if(max_count != 1)
                    cerr << "Problem in enforce_field_restriction!!! - Primal Vertex has no clear max value" << endl;

                memset(cell->vert[v]->lbls, 0, m*sizeof(bool));
                cell->vert[v]->lbls[dom] = true;
                cell->vert[v]->label = dom;
            }
        }
    }





    // if primal max material is not max on 1 of these 8, drop redo
    // labeling of primal vertex and drop other materials to be lower
    // than it.
}
*/

//==========================================================================================================
// constructLatticeFromArray()
//
// - This method takes in a data array, with dimensions and materia
//   counts, and then uses this array to construct the BCC Lattice.
//==========================================================================================================
/*
BCCLattice3D* Image3DLoader::constructLatticeFromArray(int w, int h, int d, int m, float *data, bool padded)
{
    //-----------------------------------------------------
    //           Create Storage For Lattice
    //-----------------------------------------------------
    BCCLattice3D *lattice = new BCCLattice3D(w,h,d,m,data);
    lattice->setPadded(padded);

    //-----------------------------------------------------
    //            Find Dominant Materials
    //-----------------------------------------------------
    for(int k=0; k < d; k++){
        for(int j=0; j < h; j++){
            for(int i=0; i < w; i++){

                unsigned char dom = 0;
                float max = lattice->DATA3D(i,j,k,m,dom);
                for(int mat=1; mat < m; mat++){

                    float val = lattice->DATA3D(i,j,k,m,mat);
                    if(val > max){
                        max = val;
                        dom = mat;
                    }
                }
                // make sure no values are equivalent to max
                for(int mat=0; mat < m; mat++){
                    if(lattice->DATA3D(i,j,k,m,mat) >= max && mat != dom){
                        float diff = 1E-6*fabs(max);
                        diff = fmax(1E-6, diff);
                        lattice->DATA3D(i,j,k,m,mat) = max - diff;
                    }
                }

                // sanity check
                int count = 0;
                for (int mat=0; mat < m; mat++)
                {
                    float val = lattice->DATA3D(i,j,k,m,mat);
                    if(val == max)
                        count++;
                }
                if(count != 1){
                    cout << "Failed to find dominant material in cell [ " << i << "," << j << "," << k << " ]!!" << endl;

                    cout << "Values: = ";
                    for (int mat=0; mat < m; mat++)
                    {
                        float val = lattice->DATA3D(i,j,k,m,mat);
                        cout << ", " << val;
                    }
                    exit(10);
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

                float *LLF_vals = &lattice->DATA3D(i,j,k,m,0);
                float *LLB_vals = &lattice->DATA3D(i,j,(k+1),m,0);
                float *LRF_vals = &lattice->DATA3D((i+1),j,k,m,0);
                float *LRB_vals = &lattice->DATA3D((i+1),j,(k+1),m,0);
                float *ULF_vals = &lattice->DATA3D(i,(j+1),k,m,0);
                float *ULB_vals = &lattice->DATA3D(i,(j+1),(k+1),m,0);
                float *URF_vals = &lattice->DATA3D((i+1),(j+1),k,m,0);
                float *URB_vals = &lattice->DATA3D((i+1),(j+1),(k+1),m,0);

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
                    if(cell->vert != NULL){
                        cout << "WHOA" << endl;
                        exit(13);
                    }
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
                        cell->vert[ULF]->vals = ULF_vals;

                        cell->vert[ULF]->pos().x = cell->xLocCode;
                        cell->vert[ULF]->pos().y = cell->yLocCode + 1;
                        cell->vert[ULF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[ULB]){
                        cell->vert[ULB] = new Vertex3D(m, cell, ULB);
                        cell->vert[ULB]->label = mULB;
                        cell->vert[ULB]->lbls[mULB] = true;
                        cell->vert[ULB]->vals = ULB_vals;

                        cell->vert[ULB]->pos().x = cell->xLocCode;
                        cell->vert[ULB]->pos().y = cell->yLocCode + 1;
                        cell->vert[ULB]->pos().z = cell->zLocCode + 1;
                    }
                    if(!cell->vert[URF]){
                        cell->vert[URF] = new Vertex3D(m, cell, URF);
                        cell->vert[URF]->label = mURF;
                        cell->vert[URF]->lbls[mURF] = true;
                        cell->vert[URF]->vals = URF_vals;

                        cell->vert[URF]->pos().x = cell->xLocCode + 1;
                        cell->vert[URF]->pos().y = cell->yLocCode + 1;
                        cell->vert[URF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[URB]){
                        cell->vert[URB] = new Vertex3D(m, cell, URB);
                        cell->vert[URB]->label = mURB;
                        cell->vert[URB]->lbls[mURB] = true;
                        cell->vert[URB]->vals = URB_vals;

                        cell->vert[URB]->pos().x = cell->xLocCode + 1;
                        cell->vert[URB]->pos().y = cell->yLocCode + 1;
                        cell->vert[URB]->pos().z = cell->zLocCode + 1;
                    }

                    if(!cell->vert[LLF]){
                        cell->vert[LLF] = new Vertex3D(m, cell, LLF);
                        cell->vert[LLF]->label = mLLF;
                        cell->vert[LLF]->lbls[mLLF] = true;
                        cell->vert[LLF]->vals = LLF_vals;

                        cell->vert[LLF]->pos().x = cell->xLocCode;
                        cell->vert[LLF]->pos().y = cell->yLocCode;
                        cell->vert[LLF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[LLB]){
                        cell->vert[LLB] = new Vertex3D(m, cell, LLB);
                        cell->vert[LLB]->label = mLLB;
                        cell->vert[LLB]->lbls[mLLB] = true;
                        cell->vert[LLB]->vals = LLB_vals;

                        cell->vert[LLB]->pos().x = cell->xLocCode;
                        cell->vert[LLB]->pos().y = cell->yLocCode;
                        cell->vert[LLB]->pos().z = cell->zLocCode + 1;
                    }
                    if(!cell->vert[LRF]){
                        cell->vert[LRF] = new Vertex3D(m, cell, LRF);
                        cell->vert[LRF]->label = mLRF;
                        cell->vert[LRF]->lbls[mLRF] = true;
                        cell->vert[LRF]->vals = LRF_vals;

                        cell->vert[LRF]->pos().x = cell->xLocCode + 1;
                        cell->vert[LRF]->pos().y = cell->yLocCode;
                        cell->vert[LRF]->pos().z = cell->zLocCode;
                    }
                    if(!cell->vert[LRB]){
                        cell->vert[LRB] = new Vertex3D(m, cell, LRB);
                        cell->vert[LRB]->label = mLRB;
                        cell->vert[LRB]->lbls[mLRB] = true;
                        cell->vert[LRB]->vals = LRB_vals;

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




    // TODO: Evaluate the value of using getNeighbor() addNeighbor()  vs. getCell() addCell()
    // Loop over cut cells and add their one rings, so we miss no cuts

    for(unsigned int c=0; c < lattice->cut_cells.size(); c++){

        int i = lattice->cut_cells[c]->xLocCode;
        int j = lattice->cut_cells[c]->yLocCode;
        int k = lattice->cut_cells[c]->zLocCode;


        // Add ALL neighbors - Because this setup sucks
        for(int dz=-1; dz <= 1; dz++){
            for(int dy=-1; dy <= 1; dy++){
                for(int dx=-1; dx <= 1; dx++){

                    //JAL believes he's fixed this condition
//                    if(!(dx == dy == dz == 0) &&
//                        i+dx >= 0 && i+dx < w-1 &&
//                        j+dy >= 0 && j+dy < h-1 &&
//                        k+dz >= 0 && k+dz < d-1)
//                    {
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

    // Loop over cut cells, populated and connect central edges and tetrahedra and faces
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


    // Repeat the same for the buffer cells;
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

    lattice->tree->balance_tree_new();
    lattice->tree->create_background_grid(lattice->buffer_cells);
    lattice->tree->label_background_tets(lattice->labels);

    lattice->setDataLoaded(true);

    return lattice;
}
*/

/*
OTCell* Image3DLoader::addCell(BCCLattice3D *lattice, int i, int j, int k, const Field3D *field)
{

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

            float *LLF_vals = &lattice->DATA3D(i,j,k,m,0);
            float *LLB_vals = &lattice->DATA3D(i,j,(k+1),m,0);
            float *LRF_vals = &lattice->DATA3D((i+1),j,k,m,0);
            float *LRB_vals = &lattice->DATA3D((i+1),j,(k+1),m,0);
            float *ULF_vals = &lattice->DATA3D(i,(j+1),k,m,0);
            float *ULB_vals = &lattice->DATA3D(i,(j+1),(k+1),m,0);
            float *URF_vals = &lattice->DATA3D((i+1),(j+1),k,m,0);
            float *URB_vals = &lattice->DATA3D((i+1),(j+1),(k+1),m,0);

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
                cell->vert[ULF]->vals = ULF_vals;
                cell->vert[ULF]->pos().x = cell->xLocCode;
                cell->vert[ULF]->pos().y = cell->yLocCode + 1;
                cell->vert[ULF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[ULB]){
                cell->vert[ULB] = new Vertex3D(m, cell, ULB);
                cell->vert[ULB]->label = mULB;
                cell->vert[ULB]->lbls[mULB] = true;
                cell->vert[ULB]->vals = ULB_vals;
                cell->vert[ULB]->pos().x = cell->xLocCode;
                cell->vert[ULB]->pos().y = cell->yLocCode + 1;
                cell->vert[ULB]->pos().z = cell->zLocCode + 1;
            }
            if(!cell->vert[URF]){
                cell->vert[URF] = new Vertex3D(m, cell, URF);
                cell->vert[URF]->label = mURF;
                cell->vert[URF]->lbls[mURF] = true;
                cell->vert[URF]->vals = URF_vals;
                cell->vert[URF]->pos().x = cell->xLocCode + 1;
                cell->vert[URF]->pos().y = cell->yLocCode + 1;
                cell->vert[URF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[URB]){
                cell->vert[URB] = new Vertex3D(m, cell, URB);
                cell->vert[URB]->label = mURB;
                cell->vert[URB]->lbls[mURB] = true;
                cell->vert[URB]->vals = URB_vals;
                cell->vert[URB]->pos().x = cell->xLocCode + 1;
                cell->vert[URB]->pos().y = cell->yLocCode + 1;
                cell->vert[URB]->pos().z = cell->zLocCode + 1;
            }

            if(!cell->vert[LLF]){
                cell->vert[LLF] = new Vertex3D(m, cell, LLF);
                cell->vert[LLF]->label = mLLF;
                cell->vert[LLF]->lbls[mLLF] = true;
                cell->vert[LLF]->vals = LLF_vals;

                cell->vert[LLF]->pos().x = cell->xLocCode;
                cell->vert[LLF]->pos().y = cell->yLocCode;
                cell->vert[LLF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[LLB]){
                cell->vert[LLB] = new Vertex3D(m, cell, LLB);
                cell->vert[LLB]->label = mLLB;
                cell->vert[LLB]->lbls[mLLB] = true;
                cell->vert[LLB]->vals = LLB_vals;
                cell->vert[LLB]->pos().x = cell->xLocCode;
                cell->vert[LLB]->pos().y = cell->yLocCode;
                cell->vert[LLB]->pos().z = cell->zLocCode + 1;
            }
            if(!cell->vert[LRF]){
                cell->vert[LRF] = new Vertex3D(m, cell, LRF);
                cell->vert[LRF]->label = mLRF;
                cell->vert[LRF]->lbls[mLRF] = true;
                cell->vert[LRF]->vals = LRF_vals;
                cell->vert[LRF]->pos().x = cell->xLocCode + 1;
                cell->vert[LRF]->pos().y = cell->yLocCode;
                cell->vert[LRF]->pos().z = cell->zLocCode;
            }
            if(!cell->vert[LRB]){
                cell->vert[LRB] = new Vertex3D(m, cell, LRB);
                cell->vert[LRB]->label = mLRB;
                cell->vert[LRB]->lbls[mLRB] = true;
                cell->vert[LRB]->vals = LRB_vals;
                cell->vert[LRB]->pos().x = cell->xLocCode + 1;
                cell->vert[LRB]->pos().y = cell->yLocCode;
                cell->vert[LRB]->pos().z = cell->zLocCode + 1;
            }

            //---------------------------------------
            // create interpolated center vertex
            //---------------------------------------
            if(field)
                create_dual_vertex(cell, lattice, field);
            else
                interpolate_cell(cell,lattice);

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
*/

/*
void Image3DLoader::interpolate_cell(OTCell *cell, BCCLattice3D *lattice)
{
    int m = lattice->materials();

    cell->vert[C] = new Vertex3D(m, cell, C);
    cell->vert[C]->vals = new float[m];
    memset(cell->vert[C]->vals, 0, m*sizeof(float));
    cell->vert[C]->pos().x = cell->xLocCode + 0.5f;
    cell->vert[C]->pos().y = cell->yLocCode + 0.5f;
    cell->vert[C]->pos().z = cell->zLocCode + 0.5f;

    int dom = -1;
    double max = -10000;

    for(int mat=0; mat < m; mat++){

        // copy all values, in case one appears in neighbor dual
        for(int i=0; i < 8; i++){
            cell->vert[C]->vals[mat] += cell->vert[i]->vals[mat];
        }
        cell->vert[C]->vals[mat] *= 0.125f;  // * 1/8

        bool found = false;
        for(int i=0; i < 8; i++){
            found = cell->vert[i]->lbls[mat];
            if(found)
                break;
        }

        // only allow maximums that occured on corners
        if(!found)
            continue;

        if(cell->vert[C]->vals[mat] > max || dom < 0){
            max = cell->vert[C]->vals[mat];
            dom = mat;
        }
    }

    // make sure no values are equivalent to max
    for(int mat=0; mat < m; mat++){
        if(cell->vert[C]->vals[mat] == max && mat != dom){            
            float diff = 1E-6*fabs(max);
            diff = fmax(1E-6, diff);
            cell->vert[C]->vals[mat] = max - diff;
        }
    }

    //max = cell->vert[C]->vals[dom] += 10.1;

    // now double check values aren't equivalent
    int max_count = 0;
    for(int mat=0; mat < m; mat++){
        if(cell->vert[C]->vals[mat] == max){
            max_count++;
        }
    }
    if(max_count != 1)
        cerr << "Problem!! - Dual Vertex has no clear max value" << endl;

    cell->vert[C]->lbls[dom] = true;
    cell->vert[C]->label = dom;
}
*/

/*
void Image3DLoader::create_dual_vertex(OTCell *cell, BCCLattice3D *lattice, const Field3D *field)
{
    int m = lattice->materials();

    cell->vert[C] = new Vertex3D(m, cell, C);
    cell->vert[C]->vals = new float[m];
    memset(cell->vert[C]->vals, 0, m*sizeof(float));
    cell->vert[C]->pos().x = cell->xLocCode + 0.5f;
    cell->vert[C]->pos().y = cell->yLocCode + 0.5f;
    cell->vert[C]->pos().z = cell->zLocCode + 0.5f;

    int dom = -1;
    double max = -10000;

    for(int mat=0; mat < m; mat++){

        float x = (cell->vert[C]->pos().x / lattice->width()) * field->width();
        float y = (cell->vert[C]->pos().y / lattice->height()) * field->height();
        float z = (cell->vert[C]->pos().z / lattice->depth()) * field->depth();


        // grab value from field
        cell->vert[C]->vals[mat] = field->value(x,y,z,mat);
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

        if(cell->vert[C]->vals[mat] > max || dom < 0){
            max = cell->vert[C]->vals[mat];
            dom = mat;
        }
    }

    // make sure no values are equivalent to max
    for(int mat=0; mat < m; mat++){
        if(cell->vert[C]->vals[mat] == max && mat != dom){
            float diff = 1E-6*fabs(max);
            diff = fmax(1E-6, diff);
            cell->vert[C]->vals[mat] = max - diff;
        }
    }


    //max = cell->vert[C]->vals[dom] += 10.1;

    // now double check values aren't equivalent
    int max_count = 0;
    for(int mat=0; mat < m; mat++){
        if(cell->vert[C]->vals[mat] == max){
            max_count++;
        }
    }
    if(max_count != 1)
        cerr << "Problem!! - Dual Vertex has no clear max value" << endl;

    cell->vert[C]->lbls[dom] = true;
    cell->vert[C]->label = dom;
}
*/
