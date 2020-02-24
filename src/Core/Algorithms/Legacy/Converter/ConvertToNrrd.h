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

	 Author:	Martin Cole
	 Date:		January 7 2003
*/


#ifndef CORE_ALOGRITHMS_CONVERT_CONVERTTONRRD_H
#define CORE_ALOGRITHMS_CONVERT_CONVERTTONRRD_H 1

#include <Core/Datatypes/DatatypeFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Converter/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
				namespace Converters {

ALGORITHM_PARAMETER_DECL(BuildPoints);
ALGORITHM_PARAMETER_DECL(BuildConnections);
ALGORITHM_PARAMETER_DECL(BuildData);
ALGORITHM_PARAMETER_DECL(DataLabel);

class SCISHARE ConvertToNrrdAlgo : public AlgorithmBase
{
  public:
    ConvertToNrrdAlgo();

    bool runImpl(FieldHandle input, NrrdDataHandle& points,
             NrrdDataHandle& connections,NrrdDataHandle& data) const;

    virtual AlgorithmOutput run(const AlgorithmInput& input) const override;
    static const AlgorithmOutputName Data;
    static const AlgorithmOutputName Points;
    static const AlgorithmOutputName Connections;
};

}}}}

#endif // ConvertToNrrd_h
