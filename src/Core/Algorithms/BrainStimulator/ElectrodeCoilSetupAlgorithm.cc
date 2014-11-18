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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <string>
#include <boost/range/algorithm/count.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/assign.hpp>
//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
#include <iostream>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;
using namespace boost::assign;
using namespace Parameters;

ALGORITHM_PARAMETER_DEF(BrainStimulator, TableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ProtoTypeInputCheckbox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, AllInputsTDCS);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ProtoTypeInputComboBox);

const AlgorithmInputName ElectrodeCoilSetupAlgorithm::SCALP_SURF("SCALP_SURF");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::ELECTRODECOILPROTOTYPES("ELECTRODECOILPROTOTYPES");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::ELECTRODE_SPONGE_LOCATION_AVR("ELECTRODE_SPONGE_LOCATION_AVR");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::COILS_FIELD("COILS_FIELD");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::LOCATIONS("LOCATIONS");

const double ElectrodeCoilSetupAlgorithm::number_of_columns = 10;

ElectrodeCoilSetupAlgorithm::ElectrodeCoilSetupAlgorithm()
{
  addParameter(TableValues, 0);
  addParameter(ProtoTypeInputCheckbox, 0);
  addParameter(AllInputsTDCS, 0);
  addParameter(ProtoTypeInputComboBox, 0);
}


VariableHandle ElectrodeCoilSetupAlgorithm::fill_table(FieldHandle scalp, DenseMatrixHandle locations, const std::vector<FieldHandle>& input) const
{
  Variable::List table;
  if (locations->ncols()!=3)
  {
   THROW_ALGORITHM_PROCESSING_ERROR(" LOCATIONS needs to have dimensions such as: (#CoilsOrElectrodes) x 3 "); 
  }
  
  auto tab_values = get(Parameters::TableValues).toVector();
  
  for (int i=0;i<locations->nrows();i++)
  {  
   Variable::List tmp;
   
   if (tab_values.size()==0)
   {
   tmp += 
     makeVariable("#Input", std::string("???")),
     makeVariable("Type", std::string("???")),
     makeVariable("X", boost::str(boost::format("%.3f") % (* locations)(i,0))),
     makeVariable("Y", boost::str(boost::format("%.3f") % (* locations)(i,1))),
     makeVariable("Z", boost::str(boost::format("%.3f") % (* locations)(i,2))),
     makeVariable("Angle", std::string("???")),
     makeVariable("NX", std::string("???")),
     makeVariable("NY", std::string("???")),
     makeVariable("NZ", std::string("???")),
     makeVariable("thickness", std::string("???")); 
   } else
   {
     if (locations->nrows() != tab_values.size()) /// the input has changed -> reset GUI
     {
       auto col = tab_values[i].toVector();
      std::cout << "wrong" << std::endl;
     } else
     {  /// if the number does not change we keep the GUI as it is for the user to adjust some of the values        
      
      auto col = tab_values[i].toVector();
      if (col.size()!=number_of_columns)
      {
       THROW_ALGORITHM_PROCESSING_ERROR("Internal error: data transfer between GUI and algorithm did not work. Number of table columns does not match.");
      }
      
      std::string str1=col[0].toString();
      std::string str2=col[1].toString();
      std::string str3=col[2].toString();
      std::string str4=col[3].toString();
      std::string str5=col[4].toString();
      std::string str6=col[5].toString();
      std::string str7=col[6].toString();
      std::string str8=col[7].toString();
      std::string str9=col[8].toString();
      std::string str10=col[9].toString();
      
      Variable var1=makeVariable("#Input", boost::str(boost::format("%s") % str1));
      Variable var2=makeVariable("Type",   boost::str(boost::format("%s") % str2));  
      Variable var3=makeVariable("X",      boost::str(boost::format("%s") % str3));
      Variable var4=makeVariable("Y",      boost::str(boost::format("%s") % str4));
      Variable var5=makeVariable("Z",      boost::str(boost::format("%s") % str5));
      Variable var6=makeVariable("Angle",      boost::str(boost::format("%s") % str6));  
      
      ///if this table row was selected as tDCS -> project point to scalp surface and put its normal in table (NX,NY,NZ)
      Variable var7=makeVariable("NX", boost::str(boost::format("%s") % str7));
      Variable var8=makeVariable("NY", boost::str(boost::format("%s") % str8));
      Variable var9=makeVariable("NZ", boost::str(boost::format("%s") % str9));
      Variable var10=makeVariable("thickness",boost::str(boost::format("%s") % str10));
      std::cout << "ok!" << std::endl;
      tmp += var1,var2,var3,var4,var5,var6,var7,var8,var9,var10; 
     }
     
   }  
  
   table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(i), tmp));   
  }  
  VariableHandle output(new Variable(Name("Table"), table));
  
  return output;
}

boost::tuple<VariableHandle, DenseMatrixHandle, FieldHandle> ElectrodeCoilSetupAlgorithm::run(const FieldHandle scalp, const DenseMatrixHandle locations, const std::vector<FieldHandle>& elc_coil_proto) const
{
 VariableHandle table = fill_table(scalp, locations, elc_coil_proto);

 DenseMatrixHandle elc_sponge_locations;
 FieldHandle coils_field;
 
 return boost::make_tuple(table, elc_sponge_locations, coils_field);
} 


AlgorithmOutput ElectrodeCoilSetupAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto scalp = input.get<Field>(SCALP_SURF);
  auto locations = input.get<DenseMatrix>(LOCATIONS);
  auto elc_coil_proto = input.getList<Field>(ELECTRODECOILPROTOTYPES);
  
  if (!scalp)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" SCALP_SURF (first input) field empty. ");
  }
  
  if (!locations)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" LOCATIONS (second input) matrix empty. ");
  }
  
  if (!(elc_coil_proto.size()>=1))
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" At least one prototypical coil (POINTMESH) or electrode (TRISURFMESH) definition as a field input must be provided.");
  }
 
  VariableHandle table;
  DenseMatrixHandle elc_sponge_loc_avr;
  FieldHandle coils_field;
  boost::tie(table, elc_sponge_loc_avr, coils_field) = run(scalp, locations, elc_coil_proto);

  AlgorithmOutput output;
  output[ELECTRODE_SPONGE_LOCATION_AVR] = elc_sponge_loc_avr;
  output[COILS_FIELD] = coils_field;
  
  output.setAdditionalAlgoOutput(table);
  return output;
}
