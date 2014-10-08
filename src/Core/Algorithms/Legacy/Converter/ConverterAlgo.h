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


#ifndef CORE_ALGORITHMS_CONVERTER_CONVERTERALGO_H
#define CORE_ALGORITHMS_CONVERTER_CONVERTERALGO_H 1


#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Geometry/Transform.h>

#include <Core/Algorithms/Util/AlgoLibrary.h>
#include <Core/Algorithms/Converter/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE ConverterAlgo : public AlgoLibrary {

  public:
    ConverterAlgo(ProgressReporter* pr); // normal case

    // Conversion tools for Matrices
    bool MatrixToDouble(MatrixHandle matrix, double &val);
    bool MatrixToInt(MatrixHandle matrix, int &val);
    bool MatrixToIndex(MatrixHandle matrix, index_type &val);
    bool MatrixToUnsignedInt(MatrixHandle matrix, unsigned int &val);
    bool MatrixToVector(MatrixHandle matrix, Vector& vec);
    bool MatrixToPoint(MatrixHandle matrix, Point& point);
    bool MatrixToDoubleVector(MatrixHandle matrix, std::vector<double>& vec);
    bool MatrixToIntVector(MatrixHandle matrix, std::vector<int>& vec);
    bool MatrixToIndexVector(MatrixHandle matrix, std::vector<index_type>& vec);
    bool MatrixToUnsignedIntVector(MatrixHandle matrix, std::vector<unsigned int>& vec);
    bool MatrixToTensor(MatrixHandle matrix, Tensor& ten);
    bool MatrixToTransform(MatrixHandle matrix, Transform& trans);

    bool DoubleVectorToMatrix(std::vector<double> val, MatrixHandle& matrix);
    bool IntVectorToMatrix(std::vector<int> val, MatrixHandle& matrix);    
    bool IndexVectorToMatrix(std::vector<index_type> val, MatrixHandle& matrix);    
    bool UnsignedIntVectorToMatrix(std::vector<unsigned int> val, MatrixHandle& matrix);    
    bool DoubleToMatrix(double val, MatrixHandle& matrix);
    bool IndexToMatrix(index_type val, MatrixHandle& matrix);
    bool IntToMatrix(int val, MatrixHandle& matrix);
    bool UnsignedIntToMatrix(unsigned int val, MatrixHandle& matrix);
    bool VectorToMatrix(Vector& vec, MatrixHandle& matrix);
    bool PointToMatrix(Point& point, MatrixHandle& matrix);
    bool TensorToMatrix(Tensor& ten, MatrixHandle matrix);
    bool TransformToMatrix(Transform& trans, MatrixHandle& matrix);
    
    bool MatricesToDipoleField(MatrixHandle locations,MatrixHandle strengths,FieldHandle& Dipoles);
    
    // Converters from regular spaced data to a regular field
    // datalocation specifies whether the data is "Node" or "Element" based
    bool MatrixToField(MatrixHandle input, FieldHandle& output, const std::string& datalocation);
    bool NrrdToField(NrrdDataHandle input, FieldHandle& output, const std::string& datalocation = "Auto", const std::string& fieldtype = "Auto", const std::string& convertparity= "Make Right Hand Sided");

    // For who ever insists on using nrrds
    bool FieldToNrrd(FieldHandle input, NrrdDataHandle& output);

    // Converter from NrrdToMatrix
    bool NrrdToMatrix(NrrdDataHandle input, MatrixHandle& output);

    // Convert Matrix to String
    bool MatrixToString(MatrixHandle input, StringHandle& output);
};

} // SCIRunAlgo

#endif
