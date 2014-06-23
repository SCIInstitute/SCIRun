/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef ALGORITHMS_MATH_SETUPRHSFORTDCSANDTMSALGORITHM_H
#define ALGORITHMS_MATH_SETUPRHSFORTDCSANDTMSALGORITHM_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Math/AlgorithmFwd.h>
#include <Core/Algorithms/BrainStimulator/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace BrainStimulator {
  
  class SCISHARE SetupRHSforTDCSandTMSAlgorithm : public AlgorithmBase
  {
  public:
    //Outputs run(const Inputs& input, const Parameters& params = 0) const;
    SetupRHSforTDCSandTMSAlgorithm();
    AlgorithmOutput run_generic(const AlgorithmInput& input) const;
    SCIRun::Core::Datatypes::DenseMatrixHandle run(FieldHandle fh, int num_of_elc) const;
    
    static AlgorithmParameterName SetupRHSforTDCSandTMSAlgorithm::Test();
    static AlgorithmParameterName Elc0(); static AlgorithmParameterName Elc1(); static AlgorithmParameterName Elc2(); static AlgorithmParameterName Elc3(); static AlgorithmParameterName Elc4(); static AlgorithmParameterName Elc5(); static AlgorithmParameterName Elc6(); static AlgorithmParameterName Elc7(); static AlgorithmParameterName Elc8(); static AlgorithmParameterName Elc9(); static AlgorithmParameterName Elc10(); static AlgorithmParameterName Elc11(); static AlgorithmParameterName Elc12(); static AlgorithmParameterName Elc13(); static AlgorithmParameterName Elc14(); static AlgorithmParameterName Elc15(); static AlgorithmParameterName Elc16(); static AlgorithmParameterName Elc17(); static AlgorithmParameterName Elc18(); static AlgorithmParameterName Elc19(); static AlgorithmParameterName Elc20(); static AlgorithmParameterName Elc21(); static AlgorithmParameterName Elc22(); static AlgorithmParameterName Elc23(); static AlgorithmParameterName Elc24(); static AlgorithmParameterName Elc25(); static AlgorithmParameterName Elc26(); static AlgorithmParameterName Elc27(); static AlgorithmParameterName Elc28(); static AlgorithmParameterName Elc29(); static AlgorithmParameterName Elc30(); static AlgorithmParameterName Elc31(); static AlgorithmParameterName Elc32(); static AlgorithmParameterName Elc33(); static AlgorithmParameterName Elc34(); static AlgorithmParameterName Elc35(); static AlgorithmParameterName Elc36(); static AlgorithmParameterName Elc37(); static AlgorithmParameterName Elc38(); static AlgorithmParameterName Elc39(); static AlgorithmParameterName Elc40(); static AlgorithmParameterName Elc41(); static AlgorithmParameterName Elc42(); static AlgorithmParameterName Elc43(); static AlgorithmParameterName Elc44(); static AlgorithmParameterName Elc45(); static AlgorithmParameterName Elc46(); static AlgorithmParameterName Elc47(); static AlgorithmParameterName Elc48(); static AlgorithmParameterName Elc49(); static AlgorithmParameterName Elc50(); static AlgorithmParameterName Elc51(); static AlgorithmParameterName Elc52(); static AlgorithmParameterName Elc53(); static AlgorithmParameterName Elc54(); static AlgorithmParameterName Elc55(); static AlgorithmParameterName Elc56(); static AlgorithmParameterName Elc57(); static AlgorithmParameterName Elc58(); static AlgorithmParameterName Elc59(); static AlgorithmParameterName Elc60(); static AlgorithmParameterName Elc61(); static AlgorithmParameterName Elc62(); static AlgorithmParameterName Elc63(); static AlgorithmParameterName Elc64(); static AlgorithmParameterName Elc65(); static AlgorithmParameterName Elc66(); static AlgorithmParameterName Elc67(); static AlgorithmParameterName Elc68(); static AlgorithmParameterName Elc69(); static AlgorithmParameterName Elc70(); static AlgorithmParameterName Elc71(); static AlgorithmParameterName Elc72(); static AlgorithmParameterName Elc73(); static AlgorithmParameterName Elc74(); static AlgorithmParameterName Elc75(); static AlgorithmParameterName Elc76(); static AlgorithmParameterName Elc77(); static AlgorithmParameterName Elc78(); static AlgorithmParameterName Elc79(); static AlgorithmParameterName Elc80(); static AlgorithmParameterName Elc81(); static AlgorithmParameterName Elc82(); static AlgorithmParameterName Elc83(); static AlgorithmParameterName Elc84(); static AlgorithmParameterName Elc85(); static AlgorithmParameterName Elc86(); static AlgorithmParameterName Elc87(); static AlgorithmParameterName Elc88(); static AlgorithmParameterName Elc89(); static AlgorithmParameterName Elc90(); static AlgorithmParameterName Elc91(); static AlgorithmParameterName Elc92(); static AlgorithmParameterName Elc93(); static AlgorithmParameterName Elc94(); static AlgorithmParameterName Elc95(); static AlgorithmParameterName Elc96(); static AlgorithmParameterName Elc97(); static AlgorithmParameterName Elc98(); static AlgorithmParameterName Elc99(); static AlgorithmParameterName Elc100(); static AlgorithmParameterName Elc101(); static AlgorithmParameterName Elc102(); static AlgorithmParameterName Elc103(); static AlgorithmParameterName Elc104(); static AlgorithmParameterName Elc105(); static AlgorithmParameterName Elc106(); static AlgorithmParameterName Elc107(); static AlgorithmParameterName Elc108(); static AlgorithmParameterName Elc109(); static AlgorithmParameterName Elc110(); static AlgorithmParameterName Elc111(); static AlgorithmParameterName Elc112(); static AlgorithmParameterName Elc113(); static AlgorithmParameterName Elc114(); static AlgorithmParameterName Elc115(); static AlgorithmParameterName Elc116(); static AlgorithmParameterName Elc117(); static AlgorithmParameterName Elc118(); static AlgorithmParameterName Elc119(); static AlgorithmParameterName Elc120(); static AlgorithmParameterName Elc121(); static AlgorithmParameterName Elc122(); static AlgorithmParameterName Elc123(); static AlgorithmParameterName Elc124(); static AlgorithmParameterName Elc125(); static AlgorithmParameterName Elc126(); static AlgorithmParameterName Elc127();
    
    static AlgorithmInputName ELECTRODE_COIL_POSITIONS_AND_NORMAL;
    static AlgorithmInputName ELECTRODE_COUNT;
    static AlgorithmOutputName RHS;
    
//    static const AlgorithmInputName ELECTRODE_TRIANGULATION;
//    static const AlgorithmInputName ELECTRODE_TRIANGULATION2;
//    static const AlgorithmInputName COIL;
//    static const AlgorithmInputName COIL2;
//    static const AlgorithmOutputName ELECTRODES_FIELD;
//    static const AlgorithmOutputName COILS_FIELD;
  };

}}}}

#endif