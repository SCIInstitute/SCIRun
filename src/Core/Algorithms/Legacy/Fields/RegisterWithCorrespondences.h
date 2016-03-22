/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),a
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

#ifndef CORE_ALGORITHMS_FIELDS_REGISTERWITHCORRESPONDENCES_H
#define CORE_ALGORITHMS_FIELDS_REGISTERWITHCORRESPONDENCES_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <vector>
#include <Core/Algorithms/Legacy/Fields/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace Fields {

class SCISHARE RegisterWithCorrespondencesAlgo : public AlgorithmBase
{
  public:
    /// Set defaults
    RegisterWithCorrespondencesAlgo();
	static AlgorithmInputName Correspondences1;
	static AlgorithmInputName Correspondences2;
	AlgorithmOutput run(const AlgorithmInput& input) const;
  
    /// run the algorithm
    bool runM(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const;
    bool runA(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const;
    bool runP(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const;
    bool runN(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output) const;
    bool radial_basis_func(VMesh* Cors, VMesh* points, Datatypes::DenseMatrixHandle& Smat)  const;
    bool make_new_points(VMesh* points, VMesh* Cors, const std::vector<double>& coefs, VMesh& omesh, double sumx, double sumy, double sumz) const;
    bool make_new_pointsA(VMesh* points, VMesh* Cors, const std::vector<double>& coefs, VMesh& omesh, double sumx, double sumy, double sumz) const;
	
 
};

}}}}


#endif
