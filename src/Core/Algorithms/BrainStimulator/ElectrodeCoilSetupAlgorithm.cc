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
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
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
#include <math.h>
#include <Core/Math/MiscMath.h>
#include <iostream>

#define PI 3.14159265

//////////////////////////////////////////////////////////////////////////
/// @todo MORITZ
//////////////////////////////////////////////////////////////////////////
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;
using namespace boost;
using namespace boost::assign;
using namespace Parameters;
using namespace  Eigen;

ALGORITHM_PARAMETER_DEF(BrainStimulator, AllTDCSInputs);
ALGORITHM_PARAMETER_DEF(BrainStimulator, UseThisPrototype);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ComboBoxesAreSetup);
ALGORITHM_PARAMETER_DEF(BrainStimulator, NumberOfPrototypes);
ALGORITHM_PARAMETER_DEF(BrainStimulator, TableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ProtoTypeInputCheckbox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, AllInputsTDCS);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ProtoTypeInputComboBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodethicknessCheckBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodethicknessSpinBox);

const AlgorithmInputName ElectrodeCoilSetupAlgorithm::ELECTRODES_FIELD("ELECTRODES_FIELD");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::SCALP_SURF("SCALP_SURF");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::ELECTRODECOILPROTOTYPES("ELECTRODECOILPROTOTYPES");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::ELECTRODE_SPONGE_LOCATION_AVR("ELECTRODE_SPONGE_LOCATION_AVR");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::COILS_FIELD("COILS_FIELD");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::LOCATIONS("LOCATIONS");

const int ElectrodeCoilSetupAlgorithm::number_of_columns = 10; 
const int ElectrodeCoilSetupAlgorithm::unknown_stim_type = 0; 
const int ElectrodeCoilSetupAlgorithm::TMS_stim_type     = 1; 
const int ElectrodeCoilSetupAlgorithm::tDCS_stim_type    = 2;

ElectrodeCoilSetupAlgorithm::ElectrodeCoilSetupAlgorithm()
{
  addParameter(ELECTRODES_FIELD, 0);
  addParameter(AllTDCSInputs, 0);
  addParameter(UseThisPrototype, 0);
  addParameter(ComboBoxesAreSetup, 0);
  addParameter(NumberOfPrototypes, 0);
  addParameter(TableValues, 0);
  addParameter(ProtoTypeInputCheckbox, false);
  addParameter(ProtoTypeInputComboBox, false);
  addParameter(AllInputsTDCS, false);
  addParameter(ElectrodethicknessCheckBox, false);
  addParameter(ElectrodethicknessSpinBox, 1.0); 
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

   if (tab_values.size()<locations->nrows())
   {
   tmp += 
     makeVariable("#Input", boost::str(boost::format("%s") % "0")),
     makeVariable("Type", boost::str(boost::format("%s") % "0")),
     makeVariable("X", boost::str(boost::format("%.3f") % (* locations)(i,0))),
     makeVariable("Y", boost::str(boost::format("%.3f") % (* locations)(i,1))),
     makeVariable("Z", boost::str(boost::format("%.3f") % (* locations)(i,2))),
     makeVariable("Angle", boost::str(boost::format("???"))),
     makeVariable("NX", std::string("???")),
     makeVariable("NY", std::string("???")),
     makeVariable("NZ", std::string("???")),
     makeVariable("thickness", std::string("???")); 
     
   } else
   {
      auto col = tab_values[i].toVector();

     if (col.size()==number_of_columns)
     {
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

      Variable var3,var4,var5;
      
      if (str3.compare("???")==0)
       var3=makeVariable("X",     boost::str(boost::format("%.3f") % (* locations)(i,0)));
        else
         var3=makeVariable("X",   boost::str(boost::format("%s") % str3));

      if (str4.compare("???")==0) 
       var4=makeVariable("Y",     boost::str(boost::format("%.3f") % (* locations)(i,1)));
	else
	 var4=makeVariable("Y",   boost::str(boost::format("%s") % str4));

      if (str5.compare("???")==0) 
       var5=makeVariable("Y",     boost::str(boost::format("%.3f") % (* locations)(i,2)));
	else
         var5=makeVariable("Z",   boost::str(boost::format("%s") % str5));

      Variable var6=makeVariable("Angle",  boost::str(boost::format("%s") % str6));  

      ///if this table row was selected as tDCS -> project point to scalp surface and put its normal in table (NX,NY,NZ)
      Variable var7=makeVariable("NX", boost::str(boost::format("%s") % str7));
      Variable var8=makeVariable("NY", boost::str(boost::format("%s") % str8));
      Variable var9=makeVariable("NZ", boost::str(boost::format("%s") % str9));
      Variable var10=makeVariable("thickness",boost::str(boost::format("%s") % str10));
      tmp += var1,var2,var3,var4,var5,var6,var7,var8,var9,var10;
   
     }
   }  
  
   table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(i), tmp));   
  }  
  VariableHandle output(new Variable(Name("Table"), table));
 
  return output;
}


DenseMatrixHandle ElectrodeCoilSetupAlgorithm::make_rotation_matrix(const double angle, const std::vector<double>& normal) const
{
 DenseMatrixHandle result(new DenseMatrix(3, 3));

  double normal_vector_norm=sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]); 
  Vector3d normal_vector((double)normal[0]/normal_vector_norm, (double)normal[1]/normal_vector_norm, (double)normal[2]/normal_vector_norm);
  
  Vector3d tan_vector1, tan_vector2;
  if (normal_vector(0)!=0 || normal_vector(2)!=0)
  {
   tan_vector1(0)= normal_vector(2);
   tan_vector1(1)= 0.0;
   if (normal_vector(0)==0)
    tan_vector1(2) = 0.0;
     else
       tan_vector1(2)= -1*normal_vector(0);
   } else
  {
   tan_vector1(0)= normal_vector(1);
   tan_vector1(1)= 0.0;
   tan_vector1(2)= 0.0;
  }
 
 normal_vector_norm=sqrt(tan_vector1(0)*tan_vector1(0)+tan_vector1(1)*tan_vector1(1)+tan_vector1(2)*tan_vector1(2));
 tan_vector1(0)/=normal_vector_norm; tan_vector1(1)/=normal_vector_norm; tan_vector1(2)/=normal_vector_norm;
 tan_vector2 = normal_vector.cross(tan_vector1);    
 normal_vector_norm=sqrt(tan_vector2(0)*tan_vector2(0)+tan_vector2(1)*tan_vector2(1)+tan_vector2(2)*tan_vector2(2));
 tan_vector2(0)/=normal_vector_norm; tan_vector2(1)/=normal_vector_norm; tan_vector2(2)/=normal_vector_norm;
 DenseMatrixHandle rotation_matrix2;
 Matrix3d rotation_matrix(3,3), rotation_matrix1(3,3);

 (*result)(0,2)=normal_vector(0);
 (*result)(1,2)=normal_vector(1);
 (*result)(2,2)=normal_vector(2);
 (*result)(0,1)=tan_vector2(0);
 (*result)(1,1)=tan_vector2(1);
 (*result)(2,1)=tan_vector2(2);
 (*result)(0,0)=tan_vector1(0);
 (*result)(1,0)=tan_vector1(1);
 (*result)(2,0)=tan_vector1(2);

 return result; 
}

DenseMatrixHandle ElectrodeCoilSetupAlgorithm::make_rotation_matrix_around_axis(double angle, std::vector<double>& axis_vector) const
{
   DenseMatrixHandle result(new DenseMatrix(3, 3));
  
   angle = angle * PI / 180.0;
   double cos_angle = cos(angle);
   double sin_angle = sin(angle);
   double ux=axis_vector[0],uy=axis_vector[1], uz=axis_vector[2];
   (*result)(0,0)=cos_angle+ux*ux*(1-cos_angle);
   (*result)(0,1)=ux*uy*(1-cos_angle)-uz*sin_angle;
   (*result)(0,2)=ux*uz*(1-cos_angle)+uy*sin_angle;
   (*result)(1,0)=uy*ux*(1-cos_angle)+uz*sin_angle;
   (*result)(1,1)=cos_angle+uy*uy*(1-cos_angle);
   (*result)(1,2)=uy*uz*(1-cos_angle)-ux*sin_angle;
   (*result)(2,0)=uz*ux*(1-cos_angle)-uy*sin_angle;
   (*result)(2,1)=uz*uy*(1-cos_angle)+ux*sin_angle;
   (*result)(2,2)=cos_angle+uz*uz*(1-cos_angle);
  
  return result; 
}


FieldHandle ElectrodeCoilSetupAlgorithm::make_tms(FieldHandle scalp, const std::vector<FieldHandle>& elc_coil_proto, const std::vector<double>& coil_prototyp_map, const std::vector<double>& coil_x, const std::vector<double>& coil_y, const std::vector<double>& coil_z, const std::vector<double>& coil_angle_rotation, const std::vector<double>& coil_nx, const std::vector<double>& coil_ny, const std::vector<double>& coil_nz) const 
{  
  FieldInformation fieldinfo("PointCloudMesh", 0, "Vector");
  FieldHandle tms_coils_field = CreateField(fieldinfo);        
  VMesh* tms_coils_vmesh = tms_coils_field->vmesh();  
  VField* tms_coils_vfld  = tms_coils_field->vfield();
  std::vector<Point> tms_coils_field_values;
  for (int i=0; i<coil_prototyp_map.size(); i++)
  {
   if (coil_prototyp_map[i]<=elc_coil_proto.size() && coil_prototyp_map[i]>=0) 
   { 
    if( !(coil_x.size()-1>=i && coil_y.size()-1>=i && coil_z.size()-1>=i))
     {
       THROW_ALGORITHM_PROCESSING_ERROR("Internal error: definition of coil (x,y,z) seems to be empty.");  
     }
      
    /// 1) move coil to predetermined position and orientation
    FieldHandle coil_fld = elc_coil_proto[coil_prototyp_map[i]-1];
    GetFieldDataAlgo algo_getfielddata;
    DenseMatrixHandle fielddata;
    try
    {
     fielddata  = algo_getfielddata.run(coil_fld);
    }
    catch (...)
    {
       
    }
     
    GetMeshNodesAlgo algo_getfieldnodes;
    DenseMatrixHandle fieldnodes;
    try
    {
     algo_getfieldnodes.run(coil_fld,fieldnodes);
    }
    catch (...)
    {
     THROW_ALGORITHM_PROCESSING_ERROR("Internal error: could not retrieve positions from  ");
    }
    
    DenseMatrixHandle magnetic_dipoles(boost::make_shared<DenseMatrix>(fielddata->nrows(),3));
    
    /// subtract the mean from the coil positions to move them accourding to GUI table entries
    double mean_loc_x=0,mean_loc_y=0,mean_loc_z=0; 
    for(int j=0; j<fieldnodes->nrows(); j++)
    {
     mean_loc_x+=(*fieldnodes)(j,0);
     mean_loc_y+=(*fieldnodes)(j,1);
     mean_loc_z+=(*fieldnodes)(j,2);
    }
    mean_loc_x/=fieldnodes->nrows();
    mean_loc_y/=fieldnodes->nrows();
    mean_loc_z/=fieldnodes->nrows();
    
    for(int j=0; j<fieldnodes->nrows(); j++)
    {
     (*fieldnodes)(j,0)-=mean_loc_x;
     (*fieldnodes)(j,1)-=mean_loc_y;
     (*fieldnodes)(j,2)-=mean_loc_z;
    }
    
    /// 2) create normals and rotate if needed
    if (coil_nx.size()-1>=i && coil_ny.size()-1>=i && coil_nz.size()-1>=i && coil_angle_rotation.size()-1>=i)
    {
     double angle = coil_angle_rotation[i];
     DenseMatrixHandle rotation_matrix,rotation_matrix1,rotation_matrix2;
     // 2.1) create rotation matrices
     std::vector<double> coil_vector;
     coil_vector.push_back(coil_nx[i]);
     coil_vector.push_back(coil_ny[i]);
     coil_vector.push_back(coil_nz[i]);

     rotation_matrix1 = make_rotation_matrix(angle, coil_vector);

     if (angle!=0) /// test it !
     {
      std::vector<double> axis;
      axis.push_back(coil_nx[i]);
      axis.push_back(coil_ny[i]);
      axis.push_back(coil_nz[i]);
      rotation_matrix2 = make_rotation_matrix_around_axis(angle, axis);
      rotation_matrix = boost::make_shared<DenseMatrix>((*rotation_matrix2) * (*rotation_matrix1));
     }  
     
    /// 2.2) apply rotation and move points
    for(int j=0; j<fieldnodes->nrows(); j++)
    {
     if(coil_x.size()-1>=i && coil_y.size()-1>=i && coil_z.size()-1>=i)
     { 
       DenseMatrixHandle pos_vec (boost::make_shared<DenseMatrix>(3,1));

       (*pos_vec)(0,0)=(*fieldnodes)(j,0);
       (*pos_vec)(1,0)=(*fieldnodes)(j,1);
       (*pos_vec)(2,0)=(*fieldnodes)(j,2);
       
       DenseMatrixHandle rotated_positions;

       if (angle==0)
        rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix1) * (*pos_vec));
	 else
	   rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix) * (*pos_vec));

      (*fieldnodes)(j,0)=(*rotated_positions)(0,0)+coil_x[i];
      (*fieldnodes)(j,1)=(*rotated_positions)(1,0)+coil_y[i];
      (*fieldnodes)(j,2)=(*rotated_positions)(2,0)+coil_z[i];
      
     } else
     {
       THROW_ALGORITHM_PROCESSING_ERROR("Internal error: definition of coil (x,y,z) seems to be empty.");  
     }
    }

   /// 2.3) use normal as magnetic dipole orientation if there are no normals defined at prototyp
   if(coil_nx.size()-1>=i && coil_ny.size()-1>=i && coil_nz.size()-1>=i) 
    {
     if(fielddata->ncols()==1) /// if there are no dipoles but only scalar values use GUI normal
     {
      for (int j=0; j<fielddata->nrows(); j++)
      {
       (*magnetic_dipoles)(j,0)=(*fielddata)(j,0)*coil_nx[i];
       (*magnetic_dipoles)(j,1)=(*fielddata)(j,0)*coil_ny[i];
       (*magnetic_dipoles)(j,2)=(*fielddata)(j,0)*coil_nz[i];
      }
     } else
      if(fielddata->ncols()==3) /// roatate magnetic dipoles
      { 
       for (int j=0; j<fielddata->nrows(); j++)
       { 
    	 DenseMatrixHandle pos_vec(boost::make_shared<DenseMatrix>(3,1));
         (*pos_vec)(0,0)=(*fielddata)(j,0);
         (*pos_vec)(1,0)=(*fielddata)(j,1);
         (*pos_vec)(2,0)=(*fielddata)(j,2);
	 
	 DenseMatrixHandle rotated_positions;
	 if (angle==0 || IsNan(angle))
	 {
          rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix1) * (*pos_vec));
	 }
	   else
	     rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix) * (*pos_vec));
 
 	(*magnetic_dipoles)(j,0)=(*rotated_positions)(0,0);
	(*magnetic_dipoles)(j,1)=(*rotated_positions)(1,0);
	(*magnetic_dipoles)(j,2)=(*rotated_positions)(2,0);
       }
      } 
      if (fielddata->ncols()!=3)
      {
       std::ostringstream ostr4;
       ostr4 << " Trying to generate magnetic dipoles for TMS coil defined in table row " << i << " could not find any prototyp normals - using (NX,NY,NZ) from GUI instead! " << std::endl;
       remark(ostr4.str());	
      }
    }   
   } else
   {
     THROW_ALGORITHM_PROCESSING_ERROR("Internal error: coil normals or coil prototype or coil angle rotation did not make it to algorithm. "); 
   }

    /// 4) join coil to output coil Field 
    for (VMesh::index_type j=0; j<fieldnodes->nrows(); j++)
    {
      Point p((*fieldnodes)(j,0),(*fieldnodes)(j,1),(*fieldnodes)(j,2));
      tms_coils_vmesh->add_point(p);
      Point vec((*magnetic_dipoles)(j,0),(*magnetic_dipoles)(j,1),(*magnetic_dipoles)(j,2));
      tms_coils_field_values.push_back(vec);
    }  
   
  }
 }

  VMesh::Node::iterator it, it_end;
  tms_coils_vmesh->begin(it);
  tms_coils_vmesh->end(it_end);
  index_type j = 0;    
  tms_coils_vfld->resize_values();
  while (it!=it_end)
  {
   Vector vec(tms_coils_field_values[j].x(),tms_coils_field_values[j].y(),tms_coils_field_values[j].z());
   tms_coils_vfld->set_value(vec,*it);
   j++;
   ++it;
  } 
  
  return tms_coils_field;
} 
  
boost::tuple<DenseMatrixHandle, FieldHandle> ElectrodeCoilSetupAlgorithm::make_tdcs_electrodes(FieldHandle scalp, const std::vector<FieldHandle>& elc_coil_proto, const std::vector<double>& elc_prototyp_map, const std::vector<double>& elc_x, const std::vector<double>& elc_y, const std::vector<double>& elc_z, const std::vector<double>& elc_angle_rotation, const std::vector<double>& elc_thickness) const 
{
 DenseMatrixHandle elc_sponge_locations;
 FieldHandle electrode_field;

 for (int i=0; i<elc_prototyp_map.size(); i++)
 {
  std::cout << "i:" << i << std::endl;
  
 
 }

 return boost::make_tuple(elc_sponge_locations, electrode_field);
}

boost::tuple<VariableHandle, DenseMatrixHandle, FieldHandle, FieldHandle> ElectrodeCoilSetupAlgorithm::run(const FieldHandle scalp, const DenseMatrixHandle locations, const std::vector<FieldHandle>& elc_coil_proto) const
{
 VariableHandle table_output = fill_table(scalp, locations, elc_coil_proto);
 DenseMatrixHandle elc_sponge_locations;
 FieldHandle electrodes_field, coils_field;
 auto table = table_output->toVector();
  
 /// check GUI inputs:
 /// 1) Is there any valid row in the GUI table, so at least one row where both ComboBoxes are set
 ///  
 std::vector<double> elc_prototyp_map; 
 std::vector<double> elc_thickness; 
 std::vector<double> elc_angle_rotation; 
 std::vector<double> elc_x;
 std::vector<double> elc_y;
 std::vector<double> elc_z;
 
 std::vector<double> coil_prototyp_map; 
 std::vector<double> coil_angle_rotation; 
 std::vector<double> coil_x;
 std::vector<double> coil_y;
 std::vector<double> coil_z;
 std::vector<double> coil_nx;
 std::vector<double> coil_ny;
 std::vector<double> coil_nz;
 
 /// The rest of the run function checks the validity of the GUI inputs. If there are not valid (="???") it tries to use the prototype inputs and if valid it calls functions make_tdcs_electrodes or make_tms
 for(int i=0;i<table.size();i++)
 {
  auto row = (table[i]).toVector();

  /// the c* integer variables refine the column vales (1,2,3,...,10) for the current row
  int c1=std::numeric_limits<double>::quiet_NaN(),c2=std::numeric_limits<double>::quiet_NaN();
  double c3=std::numeric_limits<double>::quiet_NaN(),c4=std::numeric_limits<double>::quiet_NaN(),c5=std::numeric_limits<double>::quiet_NaN(),
  c6=std::numeric_limits<double>::quiet_NaN(),c7=std::numeric_limits<double>::quiet_NaN(),c8=std::numeric_limits<double>::quiet_NaN(),
  c9=std::numeric_limits<double>::quiet_NaN(),c10=std::numeric_limits<double>::quiet_NaN();

  bool valid_position=true, valid_normal=true, row_valid=true;

  try
  {
   c1 = lexical_cast<int>(row[0].toString());
  } catch(bad_lexical_cast &)
  {
   c1=std::numeric_limits<double>::quiet_NaN();
   row_valid=false;
  }

  try
  { 
   c2 = lexical_cast<int>(row[1].toString());
  } catch(bad_lexical_cast &)
  {
   c2=std::numeric_limits<double>::quiet_NaN();
   row_valid=false;
  }
   
  auto str_x = row[2].toString();
  if (str_x.compare("???")==0)
  {
    valid_position=false;
  } else
  {
   try  
   {
     c3 = lexical_cast<double>(str_x);
   } catch(bad_lexical_cast &)
   {
     c3=std::numeric_limits<double>::quiet_NaN();
     valid_position=false;
   }
  }
  
  auto str_y = row[3].toString();
  if (str_y.compare("???")==0)
  {
   valid_position=false;
  } else
  {
   try  ///get the electrode thickness from GUI
   {
     c4 = lexical_cast<double>(str_y);
   } catch(bad_lexical_cast &)
   {
     c4=std::numeric_limits<double>::quiet_NaN();
     valid_position=false;
   }
  }
  
  auto str_z = row[4].toString();
  if (str_z.compare("???")==0)
  {
     valid_position=false;
  }  else
  {
   try  ///get the electrode thickness from GUI
   {
     c5 = lexical_cast<double>(str_z);
   } catch(bad_lexical_cast &)
   {
     c5=std::numeric_limits<double>::quiet_NaN(); 
     valid_position=false;
   }
  }
    
  c6=-1;
  auto angle = row[5].toString();
  if (angle.compare("???")!=0)
  {
   try 
   {
    c6 = lexical_cast<double>(row[5].toString());
   } catch(bad_lexical_cast &)
   {
    c6=std::numeric_limits<double>::quiet_NaN();
   }
  } else
    c6=std::numeric_limits<double>::quiet_NaN();  
	
  if (IsNan(c6))	
  {	 
   c6=0;
  }
    
  if(row_valid && c1>0 && c2>0)   ///both combo boxes are set up, so this could be a valid row but first check if ... 
                     /// its a tDCS electrode and if so if the thickness is provided in the GUI
		     /// or if its a TMS coil check if the prototype has normals - if so lets put the normal in the GUI 
  {
    
     FieldHandle prototyp = elc_coil_proto[c1-1]; 

     if (!prototyp)
     {
       std::ostringstream ostr1;
       ostr1 << "Module input " << (c1+2) << " seems to be empty" << std::endl;
       THROW_ALGORITHM_PROCESSING_ERROR(ostr1.str());
     }
     
     GetFieldDataAlgo algo_getfielddata;
     DenseMatrixHandle fielddata;
     try
     {
      fielddata  = algo_getfielddata.run(prototyp);
     }
     catch (...)
     {
       
     }
     
     GetMeshNodesAlgo algo_getfieldnodes;
     DenseMatrixHandle fieldnodes;
     try
     {
      algo_getfieldnodes.run(prototyp,fieldnodes);
     }
     catch (...)
     {
     
     }

     if ( (fieldnodes->nrows()!=fielddata->nrows()) )
     {      
      if( !((fieldnodes->nrows()==3 || fieldnodes->nrows()==1) && (fielddata->nrows()==3 || fielddata->nrows()==1)) )
      { 
       std::ostringstream ostr_;
       ostr_ << "Module input " << (c1+2) << " (assigned tms coil prototype) does not contain same number of nodes as data values (linear basis)." << std::endl;
       THROW_ALGORITHM_PROCESSING_ERROR(ostr_.str());
      }
     }
          
     /// get Coil normal from GUI
     /// get NX
     try
     {
       c7 = lexical_cast<double>(row[6].toString());
     } catch(bad_lexical_cast &)
     {
      c7=std::numeric_limits<double>::quiet_NaN();
      valid_normal=false;
     }
     
     /// get NY
     try
     {
       c8 = lexical_cast<double>(row[7].toString());
     } catch(bad_lexical_cast &)
     {
      c8=std::numeric_limits<double>::quiet_NaN();
      valid_normal=false;
     }
     
     /// get NZ
     try
     {
       c9 = lexical_cast<double>(row[8].toString());
     } catch(bad_lexical_cast &)
     {
      c9=std::numeric_limits<double>::quiet_NaN();
      valid_normal=false;
     } 
     
     /// get electrode thickness
     try
     {
       c10 = lexical_cast<double>(row[9].toString());
     } catch(bad_lexical_cast &)
     {
      c10=std::numeric_limits<double>::quiet_NaN();
     } 
     
     if (!valid_normal && fielddata->ncols()==3 && fielddata->nrows()>=1) /// use FIRST(!) mag. dipole direction to infer coil orientation. Note that the determined direction could be anti parallel
     {
       double norm=sqrt((*fielddata)(0,0)*(*fielddata)(0,0)+(*fielddata)(0,1)*(*fielddata)(0,1)+(*fielddata)(0,2)*(*fielddata)(0,2)); /// normalize mag. dipole to get coil direction estimate
       c7=(*fielddata)(0,0)/norm;
       c8=(*fielddata)(0,1)/norm;
       c9=(*fielddata)(0,2)/norm;
       valid_normal=true;
     }
     
     if(c7==0 && c8==0 && c9==0)
     {
      std::ostringstream ostr3;
      ostr3 << " The TMS coil defined in table row " << i << " could not get any valid normal information, found normal contained only zeros (0,0,0). " << std::endl;
      remark(ostr3.str());	
      valid_normal=false;
     }

     if (!valid_position && fieldnodes->nrows()>=1 && fieldnodes->ncols()==3)
     {
       double x=0,y=0,z=0;
       for(int j=0;j<fieldnodes->nrows();j++)
        {
	 x+=(*fieldnodes)(j,0);
	 y+=(*fieldnodes)(j,1);
	 z+=(*fieldnodes)(j,2);
	}
       x/=fieldnodes->nrows();
       y/=fieldnodes->nrows();
       z/=fieldnodes->nrows();
       c3=x;
       c4=y;
       c5=z;
       valid_position=true;
     }

    if (c2==tDCS_stim_type) /// tDCS?
    {
     std::cout << "tDCS" << std::endl;
    
     if(valid_normal && valid_position)
     {
      elc_prototyp_map.push_back(c1); 
      elc_angle_rotation.push_back(c6); 
      elc_x.push_back(c3);
      elc_y.push_back(c4);
      elc_z.push_back(c5); 
      elc_thickness.push_back(c10);; 
     }
    
    } else
    
    if (c2==TMS_stim_type) /// TMS?
    {
     
     std::cout << "TMS" << std::endl;
         
     if(valid_normal && valid_position)
     {
      coil_prototyp_map.push_back(c1); 
      coil_angle_rotation.push_back(c6); 
      coil_x.push_back(c3);
      coil_y.push_back(c4);
      coil_z.push_back(c5);
      coil_nx.push_back(c7);
      coil_ny.push_back(c8);
      coil_nz.push_back(c9);
     } else
        {
	  std::ostringstream ostr3;
	  ostr3 << " The TMS coil defined in table row " << i << " has no normal defined (NX,NY,NZ). Further, no normal could be taken from the linked prototyp field data. " << std::endl;
	  remark(ostr3.str());	
	}
    }  
      
  }
 }  
  
  bool valid_tdcs=false, valid_tms=false; 
  int t1=elc_angle_rotation.size();
  int t2=elc_thickness.size();
  int t3=elc_x.size();
  int t4=elc_y.size();
  int t5=elc_z.size();
  int t14=elc_coil_proto.size();
  if (t1==t2 && t1==t3 && t1==t4 && t1==t5 && t14>0 && t1>0)
  {
    boost::tie(elc_sponge_locations, electrodes_field) = make_tdcs_electrodes(scalp, elc_coil_proto, elc_prototyp_map, elc_x, elc_y, elc_z, elc_angle_rotation, elc_thickness);
    valid_tdcs=true;
  }
  
  int t6=coil_prototyp_map.size();
  int t7=coil_angle_rotation.size();
  int t8=coil_x.size();
  int t9=coil_y.size();
  int t10=coil_z.size();
  int t11=coil_nx.size();
  int t12=coil_ny.size();
  int t13=coil_nz.size();

  if (t6==t7 && t6==t8 && t6==t9 && t6==t10 && t6==t11 && t6==t12 && t6==t13 && t14>0 && t6>0)
  {
   coils_field = make_tms(scalp, elc_coil_proto, coil_prototyp_map, coil_x, coil_y, coil_z, coil_angle_rotation, coil_nx, coil_ny, coil_nz); 
   valid_tms =true;     
  }
  
  if (!valid_tdcs && !valid_tms)
  {  
   std::ostringstream ostr0;
   ostr0 << " Not a single table row contains valid information. " << std::endl;
   remark(ostr0.str());
  }
 
 return boost::make_tuple(table_output, elc_sponge_locations, electrodes_field, coils_field);
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
 
  if(locations->ncols()!=3)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" Locations (second module input) needs to be a dense matrix input with dimensions lx3 (l being > 0). ");  
  }
  
  VariableHandle table;
  DenseMatrixHandle elc_sponge_loc_avr;
  FieldHandle coils_field,electrodes_field;
  boost::tie(table, elc_sponge_loc_avr, electrodes_field, coils_field) = run(scalp, locations, elc_coil_proto);

  AlgorithmOutput output;
  output[ELECTRODE_SPONGE_LOCATION_AVR] = elc_sponge_loc_avr;
  output[ELECTRODES_FIELD] = electrodes_field;
  output[COILS_FIELD] = coils_field;
  
  output.setAdditionalAlgoOutput(table);
  return output;
}
