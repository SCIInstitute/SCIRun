/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

// THIS FILE CONTAINS MOST OF THE FUNCTIONALITY TO MAP A SCIRUN::FIELD OBJECT
// INTO A MATLAB ARRAY
//
// UNFORTUNATELY SCIRUN::FIELD OBJECT ARE NOT FULLY POLYMORPHIC, MEANING SOME
// FUNCTIONALITY IS MISSING IN CERTAIN CLASSES AND DATA STRUCTURES HAVE BEEN
// NAMED DIFFERENT OVER THE VARIOUS FIELDS.
//
// HENCE THIS CONVERTER IS HUGE AND HAS SPECIFIC CODE SNIPPITS FOR EVERY WEIRD
// SCIRUN DEFINITION. 
//
// THE CONVERTER IS COMPLETELY TEMPLATED AND USES TEMPLATE OVER LOADING TO DIRECT
// THE COMPILER TO INCLUDE TO PROPER PIECES OF CODE AT EACH POSITION. UNLIKE 
// MOST SCIRUN CODE, IT ONLY DOES ONE DYNAMIC COMPILATION AND THAN RELIES ON
// OVERLOADING TEMPALTED FUNCTIONS TO DEFINE A SPECIFIC CONVERTER.
// THE ADVANTAGE OF THIS METHODOLOGY IS THAT PIECES OF CODE CAN BE REUSED AND NOT
// EVERY MESH TYPE NEEDS SPECIALLY DESIGNED CODE.
// 
// THE CURRENT SYSTEM NEEDS A SPECIFIC OVERLOADED FUNCTION FOR EACH MESH TYPE WHICH
// TELLS WHICH PIECES TOGETHER FORM A NEW CONVERTER. IF THE COMPILER IS SMART ENOUGH
// IT ONLY COMPILES THE PIECES IT NEEDS. SINCE IT IS ALL TEMPLATED, THE COMPILER
// CANNOT INSTANTIATE EVERY PIECE, ALTHOUGH NEWER COMPILERS LIKE GCC4, WILL CHECK
// THE CODE EVEN IF IT IS NOT USED.....

#include <math.h>

// STL STUFF

#include <vector>
#include <string>


// Class for reading matlab files
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>

// Field class files
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

/*
 * FILE: matlabconverter_fieldtomatlab.h
 * AUTH: Jeroen G Stinstra
 * DATE: 18 MAR 2004
 */
 
#ifndef JGS_MATLABIO_MATLABCONVERTER_FIELDTOMATLAB_H
#define JGS_MATLABIO_MATLABCONVERTER_FIELDTOMATLAB_H 1

#include <Core/Matlab/share.h>

namespace MatlabIO {


class SCISHARE FieldToMatlabAlgo : public matfilebase
{
  public:

    //////// CONSTRUCTOR ///////////////////////////////////
    inline FieldToMatlabAlgo();
    virtual ~FieldToMatlabAlgo();

    //////// OPTIONS FOR CONVERTER ////////////////////////

    // Set the index base for the matlab code, normally htis one is 1
    inline void option_indexbase(int indexbase);
    // Option to switch on the old way of dealing with tensors, the ordering
    // is different. When switched off, ordering will be same as SCIRun 
    // ordering.
    inline void option_oldtensor(bool value);
    // When isoparametric the connectivity matrix of mesh and field should
    // be the same hence one can specify to remove it if not wanted
    inline void option_nofieldconnectivity(bool value);
    // Force old naming scheme when exporting, this will not work for H.O.
    // elements.
    inline void option_forceoldnames(bool value);
    
    //////// DYNAMIC ALGO ENTRY POINT /////////////////////

    virtual bool execute(SCIRun::FieldHandle fieldH, matlabarray &mlarray);

    inline void setreporter(SCIRun::ProgressReporter* pr);

  protected:

    bool mladdmeshheader(SCIRun::FieldInformation fi, matlabarray mlarray);
    bool mladdnodes(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladdedges(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladdfaces(SCIRun::VMesh* mesh,matlabarray mlarray);    
    bool mladdcells(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladdmeshderivatives(SCIRun::VMesh* mesh,matlabarray mlarray);

    bool mladdtransform(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladdxyzmesh1d(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladdxyzmesh2d(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladdxyzmesh3d(SCIRun::VMesh* mesh,matlabarray mlarray);

    bool mladddimension1d(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladddimension2d(SCIRun::VMesh* mesh,matlabarray mlarray);
    bool mladddimension3d(SCIRun::VMesh* mesh,matlabarray mlarray);

    bool mladdfieldheader(SCIRun::FieldInformation fi, matlabarray mlarray);                          

    bool mladdfielddata(SCIRun::VField* field,SCIRun::VMesh* mesh,matlabarray mlarray);

    bool mladdfieldedges(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);
    bool mladdfieldfaces(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);
    bool mladdfieldcells(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);
    
    bool mladdfieldedgederivatives(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);
    bool mladdfieldfacederivatives(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);
    bool mladdfieldcellderivatives(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);

    bool mladdfieldderivatives(SCIRun::VField *field,SCIRun::VMesh *mesh,matlabarray mlarray);


    //////// ERROR REPORTERING MECHANISM /////////////////

    inline void error(std::string error);
    inline void warning(std::string warning);

    //////// OPTION PARAMETERS //////////////////////////

    bool option_forceoldnames_;
    bool option_nofieldconnectivity_;
    int  option_indexbase_;
    
  private:
    SCIRun::ProgressReporter *pr_;
    
};

inline FieldToMatlabAlgo::FieldToMatlabAlgo() :
  option_forceoldnames_(false),
  option_nofieldconnectivity_(false),  
  option_indexbase_(1),
  pr_(0)
{
}

inline void FieldToMatlabAlgo::setreporter(SCIRun::ProgressReporter* pr)
{
  pr_ = pr;
}


inline void FieldToMatlabAlgo::option_forceoldnames(bool value) 
{
  option_forceoldnames_ = value;
}

inline void FieldToMatlabAlgo::option_nofieldconnectivity(bool value) 
{
  option_nofieldconnectivity_ = value;
}

inline void FieldToMatlabAlgo::option_indexbase(int indexbase) 
{
  option_indexbase_ = indexbase;
}

inline void FieldToMatlabAlgo::error(std::string error)
{
  if(pr_) pr_->error(error);
}

inline void FieldToMatlabAlgo::warning(std::string warning)
{
  if(pr_) pr_->warning(warning);
}


} // end namespace

#endif
