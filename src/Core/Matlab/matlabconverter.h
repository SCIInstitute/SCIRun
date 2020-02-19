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


/*
 * FILE: matlabconverter.h
 * AUTH: Jeroen G Stinstra
 * DATE: 18 MAR 2004
 */

#ifndef CORE_MATLABIO_MATLABCONVERTER_H
#define CORE_MATLABIO_MATLABCONVERTER_H 1

/*
 *  This class converts Matlab matrices into SCIRun objects and vice versa.
 *  The class is more a collection of functions then a real object.
 *
 *  The functions in this class are an attempt to bridge between the C++ kind
 *  of Object Oriented data management, towards a more classical way organizing
 *  data in an ensemble of arrays of different types
 *
 *  As SCIRun was not designed to easily extract and insert data the conversion
 *  algorithms are far from perfect and will need constant updating. A different
 *  way of managing data within SCIRun would greatly enhance the usability of
 *  SCIRun and make the conversions less cumbersome
 *
 */

#include <Core/Matlab/matfilebase.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Logging/LoggerFwd.h>
#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Matlab/share.h>

/*
 * CLASS DESCRIPTION
 * Class for dealing with the conversion of "Matlab" objects into
 * SCIRun objects.
 *
 * MEMORY MODEL
 * Functions use the matlabarray / matfiledata classes to allocate data.
 * All memory allocate is associated with the objects and is deallocated
 * by their destructors
 *
 * ERROR HANDLING
 * All errors are reported as exceptions:
 * the matlabconverter_error class.
 *
 * COPYING/ASSIGNMENT
 * Only the converter options are stored in the object and thence the
 * object can be copied without any problems.
 *
 * RESOURCE ALLOCATION
 * no external resources are used
 *
 */

namespace SCIRun {
namespace MatlabIO {

class SCISHARE matlabconverter : public matfilebase
{

  public:

    // Exception class indicating an error in the converter
    // class itself.
    class error_type : public matfileerror
    {
    public:
      explicit error_type(const std::string& msg = "") : matfileerror(msg) {}
    };

    // Functions for converting back and forward of
    // Compatible function:
    // indicates whether a matrix can be converted, for this purpose only
    // the header of the Matlab matrix is read into memory. The function also
    // returns a information string describing the object for use in the GUI
    // This function returns a value 0 if the object cannot be converted and
    // a positive number if it is compatible. The higher the number the more
    // likely it is the users wants to read this Matlab array.
    // The latter classification is based on some simple rules, like matrices
    // are probably of the double format etc.
    //
    // mlArrayTO.... function:
    // Convert a Matlab array into a SCIRun object. If the object is compatible
    // this function should deal with the conversion. Depending on the information
    // in the object, fields like the property manager will be filled out.
    //
    // ....TOmlMatrix function:
    // Convert a SCIRun object into a matlabarray. This version will produce a
    // pure numeric array, with only the numeric values, every other field will
    // be stripped and does not reappear in Matlab.
    //
    // ....TOmlArray function:
    // Try to convert a SCIRun object as complete as possible into a Matlab field
    // properties are stored and as well axis names, units etc. This function will
    // create a Matlab structured array, with each field representing different
    // parts of the original SCIRun object.
    //
    // Limitations:
    // PropertyManager:
    // Current implementation only allows for translation of strings. Any numeric
    // property like vectors and scalars are still ignored. (Limitation is partly
    // due to unavailability of type information in the property manager)
    // Complex numbers:
    // Currently only the real parts of complex numbers are taken. SCIRun does not
    // support complex numbers!
    // Nrrd key value pairs:
    // These key value pairs are not supported yet, like in the rest of SCIRun

    matlabconverter();
    explicit matlabconverter(SCIRun::Core::Logging::LoggerHandle pr);

    // SET CONVERTER OPTIONS:
    // Data type sets the export type of the data
    void setdatatype(matlabarray::mitype dataformat);
    // Index base sets the index base used for indices in for example geometries
    void setindexbase(int indexbase);
    // In a numeric matrix all data will be stripped and the data will be saved as
    // a plain dense or sparse matrix.
    void setdisabletranspose(bool dt);

    void converttonumericmatrix();
    void converttostructmatrix();

    // The following options are for controlling the conversion to bundles
    // In case prefernrrds is set, numerical data is converted into nrrds
    // only sparse matrices become matrices. If prefermatrices is set, the
    // behavior is opposite and only ND (N>2) matrices become nrrds.

    void prefernrrds();
    void preferfields();
    void prefermatrices();

    // Since Bundles can be bundled, a choice needs to be made whether structured
    // Matlab matrices should become bundles or if possible should be converted into
    // matrices/nrrds or fields. In case prefer bundles is set, a Matlab structure will
    // be decomposed into bundles of sub bundles and of nrrds and matrices. In case
    // prefersciobjects is set each structure is read and if it can be translated into
    // a sciobject it will be come a field, nrrd or matrix and only at the last
    // resort it will be a bundle. Note that the comparison is done to see whether the
    // required number of fields is there if so other fields are ignored.

    void preferbundles();
    void prefersciobjects();
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // SCIRun STRINGS
    int sciStringCompatible(const matlabarray& mlarray, std::string& infostring, bool postremarks = true);
    void mlArrayTOsciString(const matlabarray& mlmat, SCIRun::StringHandle& scistr);
    void sciStringTOmlArray(SCIRun::StringHandle scistr, matlabarray& mlmat);
#endif
    // SCIRun MATRICES
    int sciMatrixCompatible(const matlabarray& mlarray, std::string& infostring, bool postremarks = true);
    void mlArrayTOsciMatrix(const matlabarray& mlmat, SCIRun::Core::Datatypes::MatrixHandle& scimat);
    void sciMatrixTOmlArray(SCIRun::Core::Datatypes::MatrixHandle scimat, matlabarray &mlmat);

    // SCIRun NRRDS
    int sciNrrdDataCompatible(const matlabarray& mlarray, std::string& infostring, bool postremarks = true);
    void mlArrayTOsciNrrdData(const matlabarray& mlmat, SCIRun::NrrdDataHandle& scinrrd);
    void sciNrrdDataTOmlArray(SCIRun::NrrdDataHandle scinrrd, matlabarray& mlmat);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // SCIRun BUNDLES
    int sciBundleCompatible(matlabarray &mlarray, std::string &infostring, bool postremarks = true);
    void mlArrayTOsciBundle(matlabarray &mlmat, SCIRun::BundleHandle &scibundle);
    void sciBundleTOmlArray(SCIRun::BundleHandle &scibundle, matlabarray &mlmat);

    // SCIRun ColorMaps (Reader only)
    int sciColorMapCompatible(matlabarray &mlarray, std::string &infostring, bool postremarks = true);
    void mlArrayTOsciColorMap(matlabarray &mlmat,SCIRun::ColorMapHandle &scinrrd);
#endif
    // SCIRun Fields/Meshes
    int sciFieldCompatible(const matlabarray& mlarray,std::string &infostring, bool postremarks = true);
    void mlArrayTOsciField(const matlabarray& mlarray, SCIRun::FieldHandle& scifield);
    void sciFieldTOmlArray(SCIRun::FieldHandle scifield, matlabarray& mlarray);

    // SUPPORT FUNCTIONS
    // Test whether the proposed name of a Matlab matrix is valid.
    bool isvalidmatrixname(const std::string& name);

  private:

    // FUNCTIONS FOR COMMUNICATING WITH THE USER

    SCIRun::Core::Logging::LoggerHandle pr_;

    void error(const std::string& error);
    void warning(const std::string& warning);
    void remark(const std::string& remark);

    // FUNCTION FOR TRANSLATING THE CONTENTS OF A MATRIX (THE NUMERIC PART OF THE DATA)
    void sciMatrixTOmlMatrix(SCIRun::Core::Datatypes::MatrixHandle scimat,matlabarray &mlmat);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    // FUNCTIONS FOR TRANSLATING THE PROPERTY MANAGER
    void mlPropertyTOsciProperty(matlabarray &ma,SCIRun::PropertyManager *handle);
    void sciPropertyTOmlProperty(SCIRun::PropertyManager *handle,matlabarray &ma);
#endif
    // FUNCTIONS FOR TRANSLATING THE CONTENTS OF A NRRD (THE NUMERIC PART OF THE DATA)
    void sciNrrdDataTOmlMatrix(SCIRun::NrrdDataHandle scinrrd, matlabarray &mlmat);

    unsigned int convertmitype(matlabarray::mitype type);
    matlabarray::mitype convertnrrdtype(int type);

    // Matrix should be translated as a numeric matrix directly
    bool numericarray_;
    // Specify the indexbase for the output
    int indexbase_;
    // Specify the data of output data
    matlabarray::mitype datatype_;
    // Disable transposing matrices from Fortran format to C++ format
    bool disable_transpose_;

    // Options for translation of structures into bundled objects
    bool prefer_nrrds;
    bool prefer_bundles;
    bool prefer_fields;

};


}} // end namespace

#endif
