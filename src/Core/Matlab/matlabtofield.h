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

// Class for reading Matlab files
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/share.h>

/*
 * FILE: matlabconverter_fieldtomatlab.h
 * AUTH: Jeroen G Stinstra
 * DATE: 18 MAR 2004
 */
 
#ifndef CORE_MATLABIO_MATLABCONVERTER_MATLABTOFIELD_H
#define CORE_MATLABIO_MATLABCONVERTER_MATLABTOFIELD_H 1

namespace SCIRun {
namespace MatlabIO {

class SCISHARE MatlabToFieldAlgo : public matfilebase
{
  public:
    int ref_cnt;

    //////// CONSTRUCTOR ///////////////////////////////////
    inline MatlabToFieldAlgo();
    virtual ~MatlabToFieldAlgo();

    //////// DYNAMIC ALGO ENTRY POINT /////////////////////

    virtual bool execute(SCIRun::FieldHandle& fieldhandle, matlabarray& mlarray);

    //////// ANALYZE INPUT FUNCTIONS //////////////////////

    int analyze_iscompatible(matlabarray mlarray, std::string& infotext, bool postremark = true);
    int analyze_fieldtype(matlabarray mlarray, std::string& fielddesc);

    inline void setreporter(LoggerHandle pr);
    
  protected:

    int mlanalyze(matlabarray mlarray, bool postremark);  
    matlabarray findfield(matlabarray mlarray,std::string fieldnames);
  
    matlabarray mlnode; 
    matlabarray mledge;
    matlabarray mlface;
    matlabarray mlcell;

    matlabarray mlmeshderivatives; 
    matlabarray mlmeshscalefactors; 

    matlabarray mlx;
    matlabarray mly;
    matlabarray mlz;

    matlabarray mldims;
    matlabarray mltransform;

    matlabarray mlmeshbasis;
    matlabarray mlmeshbasisorder;
    matlabarray mlmeshtype;

    // Field description classes
    matlabarray mlfield;
    matlabarray mlfieldedge;
    matlabarray mlfieldface;
    matlabarray mlfieldcell;

    matlabarray mlfieldderivatives; 
    matlabarray mlfieldscalefactors; 

    matlabarray mlfieldbasis;
    matlabarray mlfieldbasisorder;
    matlabarray mlfieldtype;

    matlabarray mlchannels;

    std::string fdatatype;
    std::string fieldtype;
    std::string meshtype;
    std::string fieldbasis;
    std::string meshbasis;
    
    std::string meshbasistype;
    std::string fieldbasistype;

    std::vector<int> numnodesvec;
    std::vector<int> numelementsvec;
    
    int              numnodes;
    int              numelements;
    int              numfield;
    int              datasize;

    //////// FUNCTIONS FOR BUIDLIGN THE MESH //////////////
    
  protected:
    // Sub functions for building mesh
    bool addtransform(SCIRun::VMesh *vmesh);
    bool addderivatives(SCIRun::VMesh *vmesh);
    bool addscalefactors(SCIRun::VMesh *vmesh);    
    bool addnodes(SCIRun::VMesh *vmesh);
    bool addedges(SCIRun::VMesh *vmesh);
    bool addfaces(SCIRun::VMesh *vmesh);
    bool addcells(SCIRun::VMesh *vmesh);

    bool addfield(SCIRun::VField* field);
    
    inline void uncompressedtensor(std::vector<double> &fielddata,SCIRun::Tensor &tens, unsigned int p);
    inline void compressedtensor(std::vector<double> &fielddata,SCIRun::Tensor &tens, unsigned int p);	

    //////// ERROR REPORTERING MECHANISM /////////////////

  protected:
    
    inline void error(std::string error);
    inline void warning(std::string warning);
    inline void remark(std::string remark);
    
  private:
    SCIRun::ProgressReporter *pr_;
    
};

inline MatlabToFieldAlgo::MatlabToFieldAlgo() :
  ref_cnt(0), pr_(0)
{
}

inline void MatlabToFieldAlgo::setreporter(LoggerHandle pr)
{
  pr_ = pr;
}

inline void MatlabToFieldAlgo::error(std::string error)
{
  if(pr_) pr_->error(error);
}

inline void MatlabToFieldAlgo::warning(std::string warning)
{
  if(pr_) pr_->warning(warning);
}

inline void MatlabToFieldAlgo::remark(std::string remark)
{
  if(pr_) pr_->remark(remark);
}

inline void MatlabToFieldAlgo::compressedtensor(std::vector<double> &fielddata,SCIRun::Tensor &tens, unsigned int p)
{
   tens.mat_[0][0] = fielddata[p+0];
   tens.mat_[0][1] = fielddata[p+1];
   tens.mat_[0][2] = fielddata[p+2];
   tens.mat_[1][0] = fielddata[p+1];
   tens.mat_[1][1] = fielddata[p+3];
   tens.mat_[1][2] = fielddata[p+4];
   tens.mat_[2][0] = fielddata[p+2];
   tens.mat_[2][1] = fielddata[p+4];
   tens.mat_[2][2] = fielddata[p+5];
}

inline void MatlabToFieldAlgo::uncompressedtensor(std::vector<double> &fielddata,SCIRun::Tensor &tens, unsigned int p)
{
  tens.mat_[0][0] = fielddata[p];
  tens.mat_[0][1] = fielddata[p+1];
  tens.mat_[0][2] = fielddata[p+2];
  tens.mat_[1][0] = fielddata[p+3];
  tens.mat_[1][1] = fielddata[p+4];
  tens.mat_[1][2] = fielddata[p+5];
  tens.mat_[2][0] = fielddata[p+6];
  tens.mat_[2][1] = fielddata[p+7];
  tens.mat_[2][2] = fielddata[p+8];
}


}}


#endif
