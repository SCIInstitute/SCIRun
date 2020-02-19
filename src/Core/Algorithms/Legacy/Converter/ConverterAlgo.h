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


#ifndef CORE_ALGORITHMS_CONVERTER_CONVERTERALGO_H
#define CORE_ALGORITHMS_CONVERTER_CONVERTERALGO_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Base/Types.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Algorithms/Legacy/Converter/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {

class SCISHARE ConverterAlgo : public AlgorithmBase
{

  public:
    explicit ConverterAlgo(Core::Logging::LoggerHandle pr);

#ifdef SCIRUN4_CODE_TO_BE_CONVERTED_LATER
    bool MatrixToDouble(Datatypes::MatrixHandle matrix, double &val);
    bool MatrixToInt(Datatypes::MatrixHandle matrix, int &val);
    bool MatrixToIndex(Datatypes::MatrixHandle matrix, index_type &val);
    bool MatrixToUnsignedInt(Datatypes::MatrixHandle matrix, unsigned int &val);
    bool MatrixToVector(Datatypes::MatrixHandle matrix, Geometry::Vector& vec);
    bool MatrixToPoint(Datatypes::MatrixHandle matrix, Geometry::Point& point);
    bool MatrixToDoubleVector(Datatypes::MatrixHandle matrix, std::vector<double>& vec);
    bool MatrixToIntVector(Datatypes::MatrixHandle matrix, std::vector<int>& vec);
    bool MatrixToIndexVector(Datatypes::MatrixHandle matrix, std::vector<index_type>& vec);
    bool MatrixToUnsignedIntVector(Datatypes::MatrixHandle matrix, std::vector<unsigned int>& vec);
    bool MatrixToTensor(Datatypes::MatrixHandle matrix, Geometry::Tensor& ten);
    bool MatrixToTransform(Datatypes::MatrixHandle matrix, Geometry::Transform& trans);

    bool DoubleVectorToMatrix(const std::vector<double>& val, Datatypes::MatrixHandle& matrix);
    bool IntVectorToMatrix(const std::vector<int>& val, Datatypes::MatrixHandle& matrix);
    bool IndexVectorToMatrix(const std::vector<index_type>& val, Datatypes::MatrixHandle& matrix);
    bool UnsignedIntVectorToMatrix(const std::vector<unsigned int>& val, Datatypes::MatrixHandle& matrix);
    bool DoubleToMatrix(double val, Datatypes::MatrixHandle& matrix);
    bool IndexToMatrix(index_type val, Datatypes::MatrixHandle& matrix);
    bool IntToMatrix(int val, Datatypes::MatrixHandle& matrix);
    bool UnsignedIntToMatrix(unsigned int val, Datatypes::MatrixHandle& matrix);
    bool VectorToMatrix(Geometry::Vector& vec, Datatypes::MatrixHandle& matrix);
    bool PointToMatrix(Geometry::Point& point, Datatypes::MatrixHandle& matrix);
    bool TensorToMatrix(Geometry::Tensor& ten, Datatypes::MatrixHandle matrix);
    bool TransformToMatrix(Geometry::Transform& trans, Datatypes::MatrixHandle& matrix);

    bool MatricesToDipoleField(Datatypes::MatrixHandle locations, Datatypes::MatrixHandle strengths, FieldHandle& Dipoles);

    // Converters from regular spaced data to a regular field
    // datalocation specifies whether the data is "Node" or "Element" based
    bool MatrixToField(Datatypes::MatrixHandle input, FieldHandle& output, const std::string& datalocation);
    #endif

    bool nrrdToField(NrrdDataHandle input, FieldHandle& output,
      const std::string& datalocation = "Auto",
      const std::string& fieldtype = "Auto",
      const std::string& convertparity = "Make Right Hand Sided");

    bool fieldToNrrd(FieldHandle input, NrrdDataHandle& output);

    #ifdef SCIRUN4_CODE_TO_BE_CONVERTED_LATER
    bool NrrdToMatrix(NrrdDataHandle input, Datatypes::MatrixHandle& output);
	#endif
    bool MatrixToString(Datatypes::MatrixHandle input, Datatypes::StringHandle& output);


    virtual AlgorithmOutput run(const AlgorithmInput& input) const override;
  private:
    Core::Logging::LoggerHandle pr_;
};
  }
}
}

#endif
