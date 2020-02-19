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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshByIsovalue.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateSignedDistanceField.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/SplitByConnectedRegion.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/SplitFieldByDomainAlgo.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/GetFieldBoundaryAlgo.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/FlipSurfaceNormals.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <Core/Math/MiscMath.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(BrainStimulator, NumberOfPrototypes);
ALGORITHM_PARAMETER_DEF(BrainStimulator, TableValues);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ProtoTypeInputCheckbox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, AllInputsTDCS);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ProtoTypeInputComboBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodethicknessCheckBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, ElectrodethicknessSpinBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, InvertNormalsCheckBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, OrientTMSCoilRadialToScalpCheckBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, PutElectrodesOnScalpCheckBox);
ALGORITHM_PARAMETER_DEF(BrainStimulator, InterpolateElectrodeShapeCheckbox);

const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::FINAL_ELECTRODES_FIELD("FINAL_ELECTRODES_FIELD");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::MOVED_ELECTRODES_FIELD("MOVED_ELECTRODES_FIELD");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::SCALP_SURF("SCALP_SURF");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::ELECTRODECOILPROTOTYPES("ELECTRODECOILPROTOTYPES");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::ELECTRODE_SPONGE_LOCATION_AVR("ELECTRODE_SPONGE_LOCATION_AVR");
const AlgorithmOutputName ElectrodeCoilSetupAlgorithm::COILS_FIELD("COILS_FIELD");
const AlgorithmInputName ElectrodeCoilSetupAlgorithm::LOCATIONS("LOCATIONS");

const int ElectrodeCoilSetupAlgorithm::unknown_stim_type = 0;
const int ElectrodeCoilSetupAlgorithm::TMS_stim_type = 1;
const int ElectrodeCoilSetupAlgorithm::tDCS_stim_type = 2;
const double ElectrodeCoilSetupAlgorithm::direction_bound = -0.99;

const AlgorithmParameterName ElectrodeCoilSetupAlgorithm::columnNames[] =
{ Name("Input #"),
Name("Type"),
Name("X"),
Name("Y"),
Name("Z"),
Name("Angle"),
Name("NX"),
Name("NY"),
Name("NZ"),
Name("thickness")
};

ElectrodeCoilSetupAlgorithm::ElectrodeCoilSetupAlgorithm()
{
  using namespace Parameters;
  {
    addParameter(MOVED_ELECTRODES_FIELD, 0);
    addParameter(NumberOfPrototypes, 0);
    addParameter(TableValues, 0);
    addParameter(ProtoTypeInputCheckbox, false);
    addParameter(ProtoTypeInputComboBox, false);
    addParameter(InvertNormalsCheckBox, false);
    addParameter(AllInputsTDCS, false);
    addParameter(ElectrodethicknessCheckBox, false);
    addParameter(ElectrodethicknessSpinBox, 1.0);
    addParameter(OrientTMSCoilRadialToScalpCheckBox, true);
    addParameter(PutElectrodesOnScalpCheckBox, false);
    addParameter(InterpolateElectrodeShapeCheckbox, false);
  }
}


VariableHandle ElectrodeCoilSetupAlgorithm::fill_table(FieldHandle, DenseMatrixHandle locations, const std::vector<FieldHandle>&) const
{
  Variable::List table;
  if (locations->ncols() != 3)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" LOCATIONS needs to have dimensions such as: (#CoilsOrElectrodes) x 3 ");
  }

  auto tab_values = get(Parameters::TableValues).toVector();

  for (int i = 0; i < locations->nrows(); i++)
  {

    Variable::List row;

    if (tab_values.size() < locations->nrows())
    {
      row = {
        Variable(columnNames[0], boost::str(boost::format("%s") % "0")),
        Variable(columnNames[1], boost::str(boost::format("%s") % "0")),
        Variable(columnNames[2], boost::str(boost::format("%.3f") % (*locations)(i, 0))),
        Variable(columnNames[3], boost::str(boost::format("%.3f") % (*locations)(i, 1))),
        Variable(columnNames[4], boost::str(boost::format("%.3f") % (*locations)(i, 2))),
        Variable(columnNames[5], boost::str(boost::format("???"))),
        Variable(columnNames[6], std::string("???")),
        Variable(columnNames[7], std::string("???")),
        Variable(columnNames[8], std::string("???")),
        Variable(columnNames[9], std::string("???")) };
/*
//TODO: this compiles but needs testing by user. Not sure it's an improvement.
      row = makeNamedVariableList(columnNames,
        boost::str(boost::format("%s") % "0"),
        boost::str(boost::format("%s") % "0"),
        boost::str(boost::format("%.3f") % (*locations)(i, 0)),
        boost::str(boost::format("%.3f") % (*locations)(i, 1)),
        boost::str(boost::format("%.3f") % (*locations)(i, 2)),
        boost::str(boost::format("???")),
        std::string("???"),
        std::string("???"),
        std::string("???"),
        std::string("???")
        );
*/
    }
    else
    {
      auto col = tab_values[i].toVector();

      if (col.size() == number_of_columns)
      {
        std::string str1 = col[0].toString();
        std::string str2 = col[1].toString();
        std::string str3 = col[2].toString();
        std::string str4 = col[3].toString();
        std::string str5 = col[4].toString();
        std::string str6 = col[5].toString();
        std::string str7 = col[6].toString();
        std::string str8 = col[7].toString();
        std::string str9 = col[8].toString();
        std::string str10 = col[9].toString();

        Variable var1 = makeVariable("Input #", boost::str(boost::format("%s") % str1));
        Variable var2 = makeVariable("Type", boost::str(boost::format("%s") % str2));

        Variable var3, var4, var5;

        if (str3.compare("???") == 0)
          var3 = makeVariable("X", boost::str(boost::format("%.3f") % (*locations)(i, 0)));
        else
          var3 = makeVariable("X", boost::str(boost::format("%s") % str3));

        if (str4.compare("???") == 0)
          var4 = makeVariable("Y", boost::str(boost::format("%.3f") % (*locations)(i, 1)));
        else
          var4 = makeVariable("Y", boost::str(boost::format("%s") % str4));

        if (str5.compare("???") == 0)
          var5 = makeVariable("Y", boost::str(boost::format("%.3f") % (*locations)(i, 2)));
        else
          var5 = makeVariable("Z", boost::str(boost::format("%s") % str5));

        Variable var6 = makeVariable("Angle", boost::str(boost::format("%s") % str6));

        ///if this table row was selected as tDCS -> project point to scalp surface and put its normal in table (NX,NY,NZ)
        Variable var7 = makeVariable("NX", boost::str(boost::format("%s") % str7));
        Variable var8 = makeVariable("NY", boost::str(boost::format("%s") % str8));
        Variable var9 = makeVariable("NZ", boost::str(boost::format("%s") % str9));
        Variable var10 = makeVariable("thickness", boost::str(boost::format("%s") % str10));
        row = { var1, var2, var3, var4, var5, var6, var7, var8, var9, var10 };

      }
    }

    table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(i), row));
  }
  VariableHandle output(new Variable(Name("Table"), table));

  return output;
}


DenseMatrixHandle ElectrodeCoilSetupAlgorithm::make_rotation_matrix(const double angle, const std::vector<double>& normal) const
{
  DenseMatrixHandle result(new DenseMatrix(3, 3));

  double normal_vector_norm = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
  Eigen::Vector3d normal_vector((double)normal[0] / normal_vector_norm, (double)normal[1] / normal_vector_norm, (double)normal[2] / normal_vector_norm);

  Eigen::Vector3d tan_vector1, tan_vector2;
  if (normal_vector(0) != 0 || normal_vector(2) != 0)
  {
    tan_vector1(0) = normal_vector(2);
    tan_vector1(1) = 0.0;
    if (normal_vector(0) == 0)
      tan_vector1(2) = 0.0;
    else
      tan_vector1(2) = -1 * normal_vector(0);
  }
  else
  {
    tan_vector1(0) = normal_vector(1);
    tan_vector1(1) = 0.0;
    tan_vector1(2) = 0.0;
  }

  normal_vector_norm = sqrt(tan_vector1(0)*tan_vector1(0) + tan_vector1(1)*tan_vector1(1) + tan_vector1(2)*tan_vector1(2));
  tan_vector1(0) /= normal_vector_norm; tan_vector1(1) /= normal_vector_norm; tan_vector1(2) /= normal_vector_norm;
  tan_vector2 = normal_vector.cross(tan_vector1);
  normal_vector_norm = sqrt(tan_vector2(0)*tan_vector2(0) + tan_vector2(1)*tan_vector2(1) + tan_vector2(2)*tan_vector2(2));
  tan_vector2(0) /= normal_vector_norm; tan_vector2(1) /= normal_vector_norm; tan_vector2(2) /= normal_vector_norm;

  (*result)(0, 2) = normal_vector(0);
  (*result)(1, 2) = normal_vector(1);
  (*result)(2, 2) = normal_vector(2);
  (*result)(0, 1) = tan_vector2(0);
  (*result)(1, 1) = tan_vector2(1);
  (*result)(2, 1) = tan_vector2(2);
  (*result)(0, 0) = tan_vector1(0);
  (*result)(1, 0) = tan_vector1(1);
  (*result)(2, 0) = tan_vector1(2);

  return result;
}

DenseMatrixHandle ElectrodeCoilSetupAlgorithm::make_rotation_matrix_around_axis(double angle, std::vector<double>& axis_vector) const
{
  DenseMatrixHandle result(new DenseMatrix(3, 3));

  angle = angle * M_PI / 180.0;
  double cos_angle = cos(angle);
  double sin_angle = sin(angle);
  double ux = axis_vector[0], uy = axis_vector[1], uz = axis_vector[2];
  (*result)(0, 0) = cos_angle + ux*ux*(1 - cos_angle);
  (*result)(0, 1) = ux*uy*(1 - cos_angle) - uz*sin_angle;
  (*result)(0, 2) = ux*uz*(1 - cos_angle) + uy*sin_angle;
  (*result)(1, 0) = uy*ux*(1 - cos_angle) + uz*sin_angle;
  (*result)(1, 1) = cos_angle + uy*uy*(1 - cos_angle);
  (*result)(1, 2) = uy*uz*(1 - cos_angle) - ux*sin_angle;
  (*result)(2, 0) = uz*ux*(1 - cos_angle) - uy*sin_angle;
  (*result)(2, 1) = uz*uy*(1 - cos_angle) + ux*sin_angle;
  (*result)(2, 2) = cos_angle + uz*uz*(1 - cos_angle);

  return result;
}


FieldHandle ElectrodeCoilSetupAlgorithm::make_tms(FieldHandle scalp, const std::vector<FieldHandle>& elc_coil_proto, const std::vector<double>& coil_prototyp_map, const std::vector<double>& coil_x, const  std::vector<double>& coil_y, const std::vector<double>& coil_z, const std::vector<double>& coil_angle_rotation, std::vector<double>& coil_nx, std::vector<double>& coil_ny, std::vector<double>& coil_nz) const
{
  FieldInformation fieldinfo("PointCloudMesh", 0, "Vector");
  FieldHandle tms_coils_field = CreateField(fieldinfo);
  VMesh* tms_coils_vmesh = tms_coils_field->vmesh();
  VField* tms_coils_vfld = tms_coils_field->vfield();
  std::vector<Point> tms_coils_field_values;

  for (int i = 0; i < coil_prototyp_map.size(); i++)
  {
    if (coil_prototyp_map[i] <= elc_coil_proto.size() && coil_prototyp_map[i] >= 0)
    {
      if (!(coil_x.size() - 1 >= i && coil_y.size() - 1 >= i && coil_z.size() - 1 >= i))
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Internal error: definition of coil (x,y,z) seems to be empty.");
      }

      /// 1) move coil to predetermined position and orientation
      FieldHandle coil_fld = elc_coil_proto[coil_prototyp_map[i] - 1];
      if (!coil_fld)
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Internal error: coil field .");
      }

      FieldInformation fi(coil_fld);
      if (fi.is_pointcloudmesh())
      {
        GetFieldDataAlgo algo_getfielddata;
        DenseMatrixHandle fielddata;
        try
        {
          fielddata = algo_getfielddata.runMatrix(coil_fld);
        }
        catch (...)
        {
        }

        GetMeshNodesAlgo algo_getfieldnodes;
        DenseMatrixHandle fieldnodes;
        try
        {
          algo_getfieldnodes.run(coil_fld, fieldnodes);
        }
        catch (...)
        {
          THROW_ALGORITHM_PROCESSING_ERROR("Internal error: could not retrieve coil prototype positions ");
        }

        auto magnetic_dipoles(boost::make_shared<DenseMatrix>(fielddata->nrows(), 3ul));

        /// subtract the mean from the coil positions to move them accourding to GUI table entries
        double mean_loc_x = 0, mean_loc_y = 0, mean_loc_z = 0;
        for (int j = 0; j < fieldnodes->nrows(); j++)
        {
          mean_loc_x += (*fieldnodes)(j, 0);
          mean_loc_y += (*fieldnodes)(j, 1);
          mean_loc_z += (*fieldnodes)(j, 2);
        }
        mean_loc_x /= fieldnodes->nrows();
        mean_loc_y /= fieldnodes->nrows();
        mean_loc_z /= fieldnodes->nrows();

        for (int j = 0; j < fieldnodes->nrows(); j++)
        {
          (*fieldnodes)(j, 0) -= mean_loc_x;
          (*fieldnodes)(j, 1) -= mean_loc_y;
          (*fieldnodes)(j, 2) -= mean_loc_z;
        }

        /// 2) create normals and rotate if needed
        if (coil_nx.size() - 1 >= i && coil_ny.size() - 1 >= i && coil_nz.size() - 1 >= i && coil_angle_rotation.size() - 1 >= i)
        {
          double angle = coil_angle_rotation[i];
          DenseMatrixHandle rotation_matrix, rotation_matrix1, rotation_matrix2;
          // 2.1) create rotation matrices
          std::vector<double> coil_vector;

          bool OrientTMSCoilRadiallyToHead = get(Parameters::OrientTMSCoilRadialToScalpCheckBox).toBool();

          Vector norm;
          if (OrientTMSCoilRadiallyToHead)
          {
            VMesh* scalp_vmesh = scalp->vmesh();
            VMesh::Node::index_type didx;
            Point tms_coil(coil_x[i], coil_y[i], coil_z[i]), r;
            scalp_vmesh->synchronize(Mesh::NODE_LOCATE_E);
            double distance = 0;
            scalp_vmesh->find_closest_node(distance, r, didx, tms_coil);
            scalp_vmesh->synchronize(Mesh::NORMALS_E);
            scalp_vmesh->get_normal(norm, didx);
            if (norm.length())
            {
              coil_nx[i] = norm[0];
              coil_ny[i] = norm[1];
              coil_nz[i] = norm[2];
            }
          }

          coil_vector.push_back(coil_nx[i]);
          coil_vector.push_back(coil_ny[i]);
          coil_vector.push_back(coil_nz[i]);

          rotation_matrix1 = make_rotation_matrix(angle, coil_vector);

          if (angle != 0) /// test it !
          {
            std::vector<double> axis;
            axis.push_back(coil_nx[i]);
            axis.push_back(coil_ny[i]);
            axis.push_back(coil_nz[i]);
            rotation_matrix2 = make_rotation_matrix_around_axis(angle, axis);
            rotation_matrix = boost::make_shared<DenseMatrix>((*rotation_matrix2) * (*rotation_matrix1));
          }

          /// 2.2) apply rotation and move points
          for (int j = 0; j < fieldnodes->nrows(); j++)
          {
            if (coil_x.size() - 1 >= i && coil_y.size() - 1 >= i && coil_z.size() - 1 >= i)
            {
              DenseMatrixHandle pos_vec(boost::make_shared<DenseMatrix>(3, 1));

              (*pos_vec)(0, 0) = (*fieldnodes)(j, 0);
              (*pos_vec)(1, 0) = (*fieldnodes)(j, 1);
              (*pos_vec)(2, 0) = (*fieldnodes)(j, 2);

              DenseMatrixHandle rotated_positions;

              if (angle == 0)
                rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix1) * (*pos_vec));
              else
                rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix) * (*pos_vec));

              (*fieldnodes)(j, 0) = (*rotated_positions)(0, 0) + coil_x[i];
              (*fieldnodes)(j, 1) = (*rotated_positions)(1, 0) + coil_y[i];
              (*fieldnodes)(j, 2) = (*rotated_positions)(2, 0) + coil_z[i];

            }
            else
            {
              THROW_ALGORITHM_PROCESSING_ERROR("Internal error: definition of coil (x,y,z) seems to be empty.");
            }
          }

          /// 2.3) use normal as magnetic dipole orientation if there are no normals defined at prototyp
          if (coil_nx.size() - 1 >= i && coil_ny.size() - 1 >= i && coil_nz.size() - 1 >= i)
          {
            if (fielddata->ncols() == 1) /// if there are no dipoles but only scalar values use GUI normal
            {
              for (int j = 0; j < fielddata->nrows(); j++)
              {
                (*magnetic_dipoles)(j, 0) = (*fielddata)(j, 0)*coil_nx[i];
                (*magnetic_dipoles)(j, 1) = (*fielddata)(j, 0)*coil_ny[i];
                (*magnetic_dipoles)(j, 2) = (*fielddata)(j, 0)*coil_nz[i];
              }
            }
            else
              if (fielddata->ncols() == 3) /// roatate magnetic dipoles
              {
                for (int j = 0; j < fielddata->nrows(); j++)
                {
                  auto pos_vec(boost::make_shared<DenseMatrix>(3, 1));
                  (*pos_vec)(0, 0) = (*fielddata)(j, 0);
                  (*pos_vec)(1, 0) = (*fielddata)(j, 1);
                  (*pos_vec)(2, 0) = (*fielddata)(j, 2);

                  DenseMatrixHandle rotated_positions;
                  if (angle == 0 || IsNan(angle))
                  {
                    rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix1) * (*pos_vec));
                  }
                  else
                    rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix) * (*pos_vec));

                  (*magnetic_dipoles)(j, 0) = (*rotated_positions)(0, 0);
                  (*magnetic_dipoles)(j, 1) = (*rotated_positions)(1, 0);
                  (*magnetic_dipoles)(j, 2) = (*rotated_positions)(2, 0);
                }
              }
            if (fielddata->ncols() != 3)
            {
              std::ostringstream ostr4;
              ostr4 << " Trying to generate magnetic dipoles for TMS coil defined in table row " << i + 1 << " could not find any prototyp normals - using (NX,NY,NZ) from GUI instead! " << std::endl;
              remark(ostr4.str());
            }
          }
        }
        else
        {
          THROW_ALGORITHM_PROCESSING_ERROR("Internal error: coil normals or coil prototype or coil angle rotation did not make it to algorithm. ");
        }

        /// 4) join coil to output coil Field
        for (VMesh::index_type j = 0; j < fieldnodes->nrows(); j++)
        {
          Point p((*fieldnodes)(j, 0), (*fieldnodes)(j, 1), (*fieldnodes)(j, 2));
          tms_coils_vmesh->add_point(p);
          Point vec((*magnetic_dipoles)(j, 0), (*magnetic_dipoles)(j, 1), (*magnetic_dipoles)(j, 2));
          tms_coils_field_values.push_back(vec);
        }
      }
      else
      {
        std::ostringstream ostr3;
        ostr3 << " TMS coil definition for " << i + 1 << ". row needs be a point cloud (with defined scalar or vectors data)." << std::endl;
        remark(ostr3.str());
        continue;
      }
    }
    else
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Internal error: coil definition inconsistent. ");
    }
  }

  VMesh::Node::iterator it, it_end;
  tms_coils_vmesh->begin(it);
  tms_coils_vmesh->end(it_end);
  index_type j = 0;
  tms_coils_vfld->resize_values();
  while (it != it_end)
  {
    Vector vec(tms_coils_field_values[j].x(), tms_coils_field_values[j].y(), tms_coils_field_values[j].z());
    tms_coils_vfld->set_value(vec, *it);
    j++;
    ++it;
  }

  return tms_coils_field;
}

boost::tuple<Variable::List, double, double, double> ElectrodeCoilSetupAlgorithm::make_table_row(int i, double x, double y, double z, double nx, double ny, double nz) const
{
  double out_nx, out_ny, out_nz;
  auto tab_values = get(Parameters::TableValues).toVector();
  auto col = tab_values[i].toVector();
  std::string str1 = col[0].toString();
  std::string str2 = col[1].toString();
  std::string str3 = col[2].toString();
  std::string str4 = col[3].toString();
  std::string str5 = col[4].toString();
  std::string str6 = col[5].toString();

  std::string str7 = col[6].toString();
  std::string str8 = col[7].toString();
  std::string str9 = col[8].toString();

  std::string str10 = col[9].toString();

  if (str7.compare("???") == 0)
  {
    str7 = boost::str(boost::format("%.3f") % nx);
    out_nx = nx;
  }
  else
  {
    try
    {
      out_nx = boost::lexical_cast<double>(col[6].toString());
    }
    catch (boost::bad_lexical_cast&) {}
  }

  if (str8.compare("???") == 0)
  {
    str8 = boost::str(boost::format("%.3f") % ny);
    out_ny = ny;
  }
  else
  {
    try
    {
      out_ny = boost::lexical_cast<double>(col[7].toString());
    }
    catch (boost::bad_lexical_cast&) {}
  }

  if (str9.compare("???") == 0)
  {
    str9 = boost::str(boost::format("%.3f") % nz);
    out_nz = nz;
  }
  else
  {
    try
    {
      out_nz = boost::lexical_cast<double>(col[8].toString());
    }
    catch (boost::bad_lexical_cast&) {}
  }

  Variable var1 = makeVariable("Input #", boost::str(boost::format("%s") % str1));
  Variable var2 = makeVariable("Type", boost::str(boost::format("%s") % str2));

  Variable var3, var4, var5;

  if (str3.compare("???") == 0)
    var3 = makeVariable("X", boost::str(boost::format("%.3f") % x));
  else
    var3 = makeVariable("X", boost::str(boost::format("%s") % str3));

  if (str4.compare("???") == 0)
    var4 = makeVariable("Y", boost::str(boost::format("%.3f") % y));
  else
    var4 = makeVariable("Y", boost::str(boost::format("%s") % str4));

  if (str5.compare("???") == 0)
    var5 = makeVariable("Y", boost::str(boost::format("%.3f") % z));
  else
    var5 = makeVariable("Z", boost::str(boost::format("%s") % str5));

  Variable var6 = makeVariable("Angle", boost::str(boost::format("%s") % str6));

  ///if this table row was selected as tDCS -> project point to scalp surface and put its normal in table (NX,NY,NZ)
  Variable var7 = makeVariable("NX", boost::str(boost::format("%s") % str7));
  Variable var8 = makeVariable("NY", boost::str(boost::format("%s") % str8));
  Variable var9 = makeVariable("NZ", boost::str(boost::format("%s") % str9));
  Variable var10 = makeVariable("thickness", boost::str(boost::format("%s") % str10));

  Variable::List row{ var1, var2, var3, var4, var5, var6, var7, var8, var9, var10 };

  return boost::make_tuple(row, out_nx, out_ny, out_nz);
}


boost::tuple<DenseMatrixHandle, FieldHandle, FieldHandle, VariableHandle> ElectrodeCoilSetupAlgorithm::make_tdcs_electrodes(FieldHandle scalp_mesh, const std::vector<FieldHandle>& elc_coil_proto, const std::vector<double>& elc_prototyp_map, const std::vector<double>& elc_x, const std::vector<double>& elc_y, const std::vector<double>& elc_z, const std::vector<double>& elc_angle_rotation, const std::vector<double>& elc_thickness, VariableHandle table) const
{
  int nr_elc_sponge_triangles = 0, num_valid_electrode_definition = 0, nr_elc_sponge_triangles_on_scalp = 0;
  std::vector<double> field_values, field_values_elc_on_scalp;
  std::vector<int> valid_electrode_definition;
  FieldInformation fieldinfo("TriSurfMesh", CONSTANTDATA_E, "int");
  auto electrode_field = CreateField(fieldinfo);
  auto tdcs_vmesh = electrode_field->vmesh();
  auto tdcs_vfld = electrode_field->vfield();
  FieldInformation fieldinfo3("TriSurfMesh", CONSTANTDATA_E, "int");
  auto output = CreateField(fieldinfo3);
  auto output_vmesh = output->vmesh();
  auto output_vfld = output->vfield();
  Variable::List new_table;
  DenseMatrixHandle elc_sponge_locations;
  auto tab_values = get(Parameters::TableValues).toVector();
  auto flip_normal = false;
  FlipSurfaceNormalsAlgo flipnormal_algo;
  ConvertMeshToTriSurfMeshAlgo conv_algo;
  FieldInformation scalp_info(scalp_mesh);
  FieldHandle scalp;
  if (scalp_info.is_quadsurf())
    conv_algo.run(scalp_mesh, scalp);
  else
    scalp = scalp_mesh;

  auto compute_third_output = get(Parameters::PutElectrodesOnScalpCheckBox).toBool();
  auto interpolate_elec_shape = get(Parameters::InterpolateElectrodeShapeCheckbox).toBool();

  if (tab_values.size() == elc_prototyp_map.size() && elc_thickness.size() == elc_prototyp_map.size() && elc_x.size() == elc_prototyp_map.size() && elc_y.size() == elc_prototyp_map.size() && elc_z.size() == elc_prototyp_map.size() && elc_angle_rotation.size() == elc_prototyp_map.size())
  {
    auto scalp_vmesh = scalp->vmesh();
    auto scalp_vfld = scalp->vfield();
    valid_electrode_definition.resize(elc_prototyp_map.size());
    for (int i = 0; i < elc_prototyp_map.size(); i++)
    {
      if (elc_thickness[i] <= 0 || IsNan(elc_thickness[i]))
      {
        std::ostringstream ostr3;
        ostr3 << " Please provide positive electrode thickness - skip electrode definition: " << i << std::endl;
        remark(ostr3.str());
        continue;
      }

      double distance = 0;
      VMesh::Node::index_type didx;
      Point elc(elc_x[i], elc_y[i], elc_z[i]), r;
      scalp_vmesh->synchronize(Mesh::NODE_LOCATE_E);
      scalp_vmesh->find_closest_node(distance, r, didx, elc);  /// project GUI (x,y,z) onto scalp and ...
      std::ostringstream ostr3;
      ostr3 << " Distance of electrode " << i + 1 << " to scalp surface is " << distance << " [distance units]." << std::endl;
      remark(ostr3.str());
      Vector norm;
      scalp_vmesh->synchronize(Mesh::NORMALS_E);
      scalp_vmesh->get_normal(norm, didx); /// ... get its normal
      /// update GUI table normals
      double nx, ny, nz;
      Variable::List new_row;
      boost::tie(new_row, nx, ny, nz) = make_table_row(i, elc_x[i], elc_y[i], elc_z[i], norm.x(), norm.y(), norm.z());
      new_table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(i), new_row));
      if (!(nx == 0.0 && ny == 0.0 && nz == 0.0))
      {
        Vector norm2(nx, ny, nz);
        double dot_product = Dot(norm, norm2);
        norm = Vector(nx, ny, nz);
        if (dot_product < direction_bound)
        {
          flip_normal = true;
        }
      }

      /// move coil prototype to projected location
      /// first, compute the transfer matrices
      std::vector<double> axis;
      axis.push_back(norm[0]);
      axis.push_back(norm[1]);
      axis.push_back(norm[2]);
      double angle = elc_angle_rotation[i];
      DenseMatrixHandle rotation_matrix, rotation_matrix1, rotation_matrix2;
      rotation_matrix1 = make_rotation_matrix(angle, axis);
      if (elc_angle_rotation[i] != 0)
      {
        rotation_matrix2 = make_rotation_matrix_around_axis(angle, axis);
        rotation_matrix = boost::make_shared<DenseMatrix>((*rotation_matrix2) * (*rotation_matrix1));
      }
      FieldHandle prototype, prototype_mesh = elc_coil_proto[elc_prototyp_map[i] - 1];
      FieldInformation fi(prototype_mesh);

      if (fi.is_quadsurfmesh())
        conv_algo.run(prototype_mesh, prototype);
      else
        prototype = prototype_mesh;

      FieldInformation fi_proto(prototype);

      if (fi_proto.is_trisurfmesh())
      {
        GetMeshNodesAlgo algo_getfieldnodes;
        DenseMatrixHandle fieldnodes;
        try
        {
          algo_getfieldnodes.run(prototype, fieldnodes);
        }
        catch (...)
        {
          THROW_ALGORITHM_PROCESSING_ERROR("Internal error: could not retrieve positions from assigned prototype ");
        }
        if (fieldnodes->nrows() == 0) // put this to tms as well
        {
          THROW_ALGORITHM_PROCESSING_ERROR("Internal error: could not retrieve positions from assigned prototype ");
        }
        ///second, subtract the mean of the prototyp positions to center it in origin
        double mean_loc_x = 0, mean_loc_y = 0, mean_loc_z = 0;
        for (int j = 0; j < fieldnodes->nrows(); j++)
        {
          mean_loc_x += (*fieldnodes)(j, 0);
          mean_loc_y += (*fieldnodes)(j, 1);
          mean_loc_z += (*fieldnodes)(j, 2);
        }
        mean_loc_x /= fieldnodes->nrows();
        mean_loc_y /= fieldnodes->nrows();
        mean_loc_z /= fieldnodes->nrows();

        for (int j = 0; j < fieldnodes->nrows(); j++)
        {
          (*fieldnodes)(j, 0) -= mean_loc_x;
          (*fieldnodes)(j, 1) -= mean_loc_y;
          (*fieldnodes)(j, 2) -= mean_loc_z;
        }

        DenseMatrixHandle rotated_positions;

        for (int j = 0; j < fieldnodes->nrows(); j++)
        {
          auto pos_vec(boost::make_shared<DenseMatrix>(3, 1));

          (*pos_vec)(0, 0) = (*fieldnodes)(j, 0);
          (*pos_vec)(1, 0) = (*fieldnodes)(j, 1);
          (*pos_vec)(2, 0) = (*fieldnodes)(j, 2);

          if (angle == 0)
            rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix1) * (*pos_vec));
          else
            rotated_positions = boost::make_shared<DenseMatrix>((*rotation_matrix) * (*pos_vec));

          (*fieldnodes)(j, 0) = (*rotated_positions)(0, 0) + elc_x[i];
          (*fieldnodes)(j, 1) = (*rotated_positions)(1, 0) + elc_y[i];
          (*fieldnodes)(j, 2) = (*rotated_positions)(2, 0) + elc_z[i];
        }
        VMesh* prototype_vmesh = prototype->vmesh();

        FieldInformation fieldinfo("TriSurfMesh", CONSTANTDATA_E, "double"); /// this is the final moved prototype for elc i
        FieldHandle tmp_tdcs_elc = CreateField(fieldinfo);
        VMesh*  tmp_tdcs_elc_vmesh = tmp_tdcs_elc->vmesh();
        VField* tmp_tdcs_elc_vfld = tmp_tdcs_elc->vfield();

        Point p;
        for (int l = 0; l < fieldnodes->nrows(); l++)
        {
          Point p((*fieldnodes)(l, 0), (*fieldnodes)(l, 1), (*fieldnodes)(l, 2));
          tdcs_vmesh->add_point(p);
          tmp_tdcs_elc_vmesh->add_point(p);
        }

        for (VMesh::Elem::index_type l = 0; l < prototype_vmesh->num_elems(); l++)
        {
          VMesh::Node::array_type onodes(3);
          prototype_vmesh->get_nodes(onodes, l);
          tmp_tdcs_elc_vmesh->add_elem(onodes);
          onodes[0] += nr_elc_sponge_triangles;
          onodes[1] += nr_elc_sponge_triangles;
          onodes[2] += nr_elc_sponge_triangles;
          tdcs_vmesh->add_elem(onodes);
          field_values.push_back(i);
        }
        nr_elc_sponge_triangles += fieldnodes->nrows();
        valid_electrode_definition[i] = 1;
        num_valid_electrode_definition++;

        tmp_tdcs_elc_vfld->set_all_values(0.0);

        if (compute_third_output) /// a lot of code is inside this if (make sure it creates proper results)
        {
          BBox proto_bb = prototype->vmesh()->get_bounding_box();
          Vector proto_diameter = proto_bb.diagonal();
          double maxi = 2 * Max(proto_diameter.x(), proto_diameter.y(), proto_diameter.z()); /// use maximum difference as diameter for sphere, make it bigger to get all needed nodes

          /// since the protoype has to be centered around coordinate origin
          /// it will envelop the scalp/electrode sponge surface at its final location (it is now!)
          /// scalp needs to have data stored on nodes for clipping to prevent having frayed electrode sponge corners
          FieldHandle scalp_linear_data;
          ConvertFieldBasisTypeAlgo convert_field_basis;
          std::vector<double> tmp_field_values;

          for (VMesh::Elem::index_type l = 0; l < scalp_vmesh->num_elems(); l++)
          {
            VMesh::Node::array_type onodes(3);
            scalp_vmesh->get_nodes(onodes, l);
            Point p1, p2, p3;
            scalp_vmesh->get_center(p1, onodes[0]);
            scalp_vmesh->get_center(p2, onodes[1]);
            scalp_vmesh->get_center(p3, onodes[2]);

            double x1 = (p1.x() + p2.x() + p3.x()) / 3 - r.x(),
              y1 = (p1.y() + p2.y() + p3.y()) / 3 - r.y(),
              z1 = (p1.z() + p2.z() + p3.z()) / 3 - r.z();
            x1 *= x1; y1 *= y1; z1 *= z1; double sum = x1 + y1 + z1;

            if (compute_third_output)
            {
              if (sum < (maxi / 2 * maxi / 2))
              {
                tmp_field_values.push_back(1.0);
              }
              else
                tmp_field_values.push_back(0.0);
            }
          }

          scalp_vfld->resize_values();
          scalp_vfld->set_values(tmp_field_values);

          SplitFieldByDomainAlgo algo_splitfieldbydomain;
          algo_splitfieldbydomain.setLogger(getLogger());
          FieldList scalp_elc_sphere;
          algo_splitfieldbydomain.set(SplitFieldByDomainAlgo::SortBySize, true);
          algo_splitfieldbydomain.set(SplitFieldByDomainAlgo::SortAscending, false);
          algo_splitfieldbydomain.runImpl(scalp, scalp_elc_sphere);

          VMesh::Elem::index_type c_ind = 0;
          FieldHandle tmp_fld;
          for (long l = 0; l < scalp_elc_sphere.size(); l++)
          {
            VField* scalp_elc_sphere_fld = scalp_elc_sphere[l]->vfield();

            double tmp_val = std::numeric_limits<double>::quiet_NaN();
            scalp_elc_sphere_fld->get_value(tmp_val, c_ind);
            if (tmp_val == 1.0)
            {
              tmp_fld = scalp_elc_sphere[l];
            }
          }

          SplitFieldByConnectedRegionAlgo algo_splitbyconnectedregion;
          algo_splitbyconnectedregion.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
          algo_splitbyconnectedregion.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);
          auto scalp_result = algo_splitbyconnectedregion.run(tmp_fld);

          FieldHandle small_scalp_surf;

          for (int l = 0; l < scalp_result.size(); l++)
          {
            FieldHandle tmp_fld = scalp_result[l];
            VMesh*  tmp_fld_msh = tmp_fld->vmesh();

            tmp_fld_msh->synchronize(Mesh::NODE_LOCATE_E);
            tmp_fld_msh->find_closest_node(distance, p, didx, r);

            if (distance == 0)
            {
              small_scalp_surf = tmp_fld;
            }
          }

          if (!small_scalp_surf->vfield()->is_lineardata())
          {

            using namespace SCIRun::Core::Algorithms::Fields::Parameters;
            {
              convert_field_basis.setOption(OutputType, "Linear");
              convert_field_basis.set(BuildBasisMapping, false);
              convert_field_basis.runImpl(small_scalp_surf, scalp_linear_data);
            }
          }
          CalculateSignedDistanceFieldAlgo algo_sdf;
          FieldHandle sdf_output;
          if (small_scalp_surf->vfield()->is_lineardata())
            algo_sdf.run(small_scalp_surf, tmp_tdcs_elc, sdf_output);
          else
            algo_sdf.run(scalp_linear_data, tmp_tdcs_elc, sdf_output); /// assumed that CalculateSignedDistanceFieldAlgo output has always values defined on nodes

          VField* tmp_sdf_vfld = sdf_output->vfield();
          VMesh*  tmp_sdf_vmsh = sdf_output->vmesh();

          FieldHandle final_electrode_sponge_surf;
          std::vector<double> tmp_field_bin_values;
          bool found_elc_surf = false;
          std::vector<FieldHandle> result;

          if (interpolate_elec_shape)
          {
            FieldHandle sdf_output_linear;
            using namespace SCIRun::Core::Algorithms::Fields::Parameters;  /// convert the data values (zero's) to elements
            {
              convert_field_basis.setOption(OutputType, "Linear");
              convert_field_basis.set(BuildBasisMapping, false);
              convert_field_basis.runImpl(sdf_output, sdf_output_linear);
            }

            /// use clipvolumebyisovalue to get the electrode patch edges right
            FieldHandle clipmeshbyisoval_output;
            ClipMeshByIsovalueAlgo clipmeshbyisoval_algo;
            clipmeshbyisoval_algo.set(ClipMeshByIsovalueAlgo::ScalarIsoValue, 0.0);
            clipmeshbyisoval_algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, true);
            clipmeshbyisoval_algo.run(sdf_output_linear, clipmeshbyisoval_output);
            /// check if we could find the electrode location r that is projected onto the scalp to ensure its the desired surface
            bool found_correct_surface = false;
            if (clipmeshbyisoval_output)
            {
              VMesh* clipmeshbyisoval_vmesh = clipmeshbyisoval_output->vmesh();
              if (clipmeshbyisoval_vmesh)
              {
                clipmeshbyisoval_vmesh->synchronize(Mesh::NODE_LOCATE_E);
                double distance = 0;
                Point p;
                VMesh::Node::index_type didx;
                for (VMesh::Node::index_type l = 0; l < clipmeshbyisoval_vmesh->num_nodes(); l++)
                {
                  clipmeshbyisoval_vmesh->find_closest_node(distance, p, didx, r);
                  if (distance == 0)
                  {
                    found_correct_surface = true;
                    break;
                  }
                }
              }
              if (!found_correct_surface)
              {
                clipmeshbyisoval_algo.set(ClipMeshByIsovalueAlgo::ScalarIsoValue, 0.0);
                clipmeshbyisoval_algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, false);
                clipmeshbyisoval_algo.run(sdf_output, clipmeshbyisoval_output);
                if (clipmeshbyisoval_output)
                {
                  clipmeshbyisoval_vmesh = clipmeshbyisoval_output->vmesh();
                  if (clipmeshbyisoval_vmesh)
                  {
                    clipmeshbyisoval_vmesh->synchronize(Mesh::NODE_LOCATE_E);
                    for (VMesh::Node::index_type l = 0; l < clipmeshbyisoval_vmesh->num_nodes(); l++)
                    {
                      clipmeshbyisoval_vmesh->find_closest_node(distance, p, l, r);
                      if (distance == 0)
                      {
                        found_correct_surface = true;
                        break;
                      }
                    }
                  }
                }
                if (!found_correct_surface)
                {
                  std::ostringstream ostr3;
                  ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode. Make sure that prototype encapsulated scalp!" << std::endl;
                  remark(ostr3.str());
                  continue;
                }

              }

              using namespace SCIRun::Core::Algorithms::Fields::Parameters;  /// convert the data values (zero's) to elements
              {
                convert_field_basis.setOption(OutputType, "Constant");
                convert_field_basis.set(BuildBasisMapping, false);
                convert_field_basis.runImpl(clipmeshbyisoval_output, final_electrode_sponge_surf);
              }

              final_electrode_sponge_surf->vfield()->set_all_values(0.0); /// Precaution: set data values (defined at elements) to zero
              SplitFieldByConnectedRegionAlgo algo_splitbyconnectedregion_1;
              algo_splitbyconnectedregion_1.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
              algo_splitbyconnectedregion_1.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);
              result = algo_splitbyconnectedregion_1.run(final_electrode_sponge_surf);
            }

          }
          else
          {
            tmp_sdf_vmsh->synchronize(Mesh::NODE_LOCATE_E);
            Point tmp_p;

            double label_switch = 0.0, fld_value = std::numeric_limits<double>::quiet_NaN();

            for (VMesh::Elem::index_type k = 0; k < tmp_sdf_vmsh->num_elems(); k++)
            {
              VMesh::Node::array_type onodes(3);
              tmp_sdf_vmsh->get_nodes(onodes, k);

              for (VMesh::Node::index_type l = 0; l < 3; l++)
              {
                tmp_sdf_vmsh->get_center(tmp_p, onodes[l]);
                if (tmp_p.x() == r.x() && tmp_p.y() == r.y() && tmp_p.z() == r.z())
                {
                  tmp_sdf_vfld->get_value(fld_value, k);
                  if (fld_value >= 0)
                    label_switch = 0.0;
                  else
                    label_switch = 1.0;

                  break;
                }
              }
            }

            if (!tmp_sdf_vfld || tmp_sdf_vfld->num_values() <= 0)
            {
              std::ostringstream ostr3;
              ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode. Make sure that prototype encapsulated scalp." << std::endl;
              remark(ostr3.str());
              continue;/// in that case go to the next electrode -> leave the for loop thats iterating over i
            }

            tmp_field_bin_values.resize(tmp_sdf_vfld->num_values());
            for (VMesh::Node::index_type l = 0; l < tmp_sdf_vfld->num_values(); l++) /// find out which nodes are inside the prototype
            {
              double tmp_sdf_fld_val = std::numeric_limits<double>::quiet_NaN(); /// store binary classification of which scalp nodes are inside prototype and which are outside
              tmp_sdf_vfld->get_value(tmp_sdf_fld_val, l);
              if (tmp_sdf_fld_val <= 0.0)
              {
                tmp_field_bin_values[l] = (1.0 - label_switch);
                found_elc_surf = true;
              }
              else
                tmp_field_bin_values[l] = (label_switch - 0.0);
            }

            if (!found_elc_surf)
            {
              std::ostringstream ostr3;
              ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode (after sdf binarization). Make sure that prototype encapsulates scalp." << std::endl;
              remark(ostr3.str());
              continue;/// in that case go to the next electrode -> leave the for loop thats iterating over
            }

            using namespace SCIRun::Core::Algorithms::Fields::Parameters;  /// convert the data values (zero's) to elements
            {
              convert_field_basis.setOption(OutputType, "Constant");
              convert_field_basis.set(BuildBasisMapping, false);
              convert_field_basis.runImpl(sdf_output, final_electrode_sponge_surf);
            }

            VField* final_electrode_sponge_surf_fld = final_electrode_sponge_surf->vfield();
            VMesh*  final_electrode_sponge_surf_msh = final_electrode_sponge_surf->vmesh();

            if (!final_electrode_sponge_surf_fld || final_electrode_sponge_surf_fld->num_values() <= 0 || !final_electrode_sponge_surf_msh || final_electrode_sponge_surf_msh->num_elems() <= 0)
            {
              std::ostringstream ostr3;
              ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode (conversion to constant data storage). Make sure that prototype encapsulates scalp." << std::endl;
              remark(ostr3.str());
              continue;/// in that case go to the next electrode -> leave the for loop thats iterating over i
            }

            final_electrode_sponge_surf_fld->set_all_values(0.0); /// Precaution: set data values (defined at elements) to zero

            for (VMesh::Elem::index_type l = 0; l < final_electrode_sponge_surf_msh->num_elems(); l++)
            {
              VMesh::Node::array_type onodes(3);
              tmp_sdf_vmsh->get_nodes(onodes, l);
              if (tmp_field_bin_values[onodes[0]] == 1.0 && tmp_field_bin_values[onodes[1]] == 1.0 && tmp_field_bin_values[onodes[2]] == 1.0)
              {
                final_electrode_sponge_surf_fld->set_value(1.0, l);
                found_elc_surf = true;
              }
              else
                final_electrode_sponge_surf_fld->set_value(0.0, l);
            }

            if (!found_elc_surf)
            {
              std::ostringstream ostr3;
              ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode (conversion to constant data storage). Make sure that prototype encapsulates scalp." << std::endl;
              remark(ostr3.str());
              continue;/// in that case go to the next electrode -> leave the for loop thats iterating over i
            }

            /// are there multiple not connected scalp surfaces that are inside the prototype
            /// use projected point r (that was projected on scalp surface) to differentiate which surface is the one to use
            /// but first splitbydomain to get the surface with tha value 1.0
            SplitFieldByDomainAlgo algo_splitfieldbydomain;
            algo_splitfieldbydomain.setLogger(getLogger());
            FieldList final_electrode_sponge_surf_domainsplit;
            algo_splitfieldbydomain.set(SplitFieldByDomainAlgo::SortBySize, true);
            algo_splitfieldbydomain.set(SplitFieldByDomainAlgo::SortAscending, false);
            algo_splitfieldbydomain.runImpl(final_electrode_sponge_surf, final_electrode_sponge_surf_domainsplit);
            found_elc_surf = false;
            FieldHandle find_elc_surf;
            VMesh::Elem::index_type c_ind = 0;
            for (long l = 0; l < final_electrode_sponge_surf_domainsplit.size(); l++)
            {
              VField* final_electrode_sponge_surf_domainsplit_fld = final_electrode_sponge_surf_domainsplit[l]->vfield();
              double tmp_val = std::numeric_limits<double>::quiet_NaN();
              final_electrode_sponge_surf_domainsplit_fld->get_value(tmp_val, c_ind);
              if (tmp_val == 1.0)
              {
                find_elc_surf = final_electrode_sponge_surf_domainsplit[l];
                found_elc_surf = true;
              }
            }

            if (!found_elc_surf)
            {
              std::ostringstream ostr3;
              ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode (after domainsplit). Make sure that prototype encapsulates scalp." << std::endl;
              remark(ostr3.str());
              found_elc_surf = false;
              continue;/// in that case go to the next electrode -> leave the for loop thats iterating over i
            }

            SplitFieldByConnectedRegionAlgo algo_splitbyconnectedregion;
            algo_splitbyconnectedregion.set(SplitFieldByConnectedRegionAlgo::SortDomainBySize(), true);
            algo_splitbyconnectedregion.set(SplitFieldByConnectedRegionAlgo::SortAscending(), false);
            result = algo_splitbyconnectedregion.run(find_elc_surf);
          }

          found_elc_surf = false;
          double distance = std::numeric_limits<double>::quiet_NaN();
          VMesh::Node::index_type didx;

          for (int l = 0; l < result.size(); l++)
          {
            auto tmp_fld_1 = result[l];
            auto tmp_fld_msh = tmp_fld_1->vmesh();

            tmp_fld_msh->synchronize(Mesh::NODE_LOCATE_E);
            tmp_fld_msh->find_closest_node(distance, p, didx, r);

            if (distance == 0)
            {
              found_elc_surf = true;
              scalp_vmesh->synchronize(Mesh::NORMALS_E);
              Point q;
              Vector norm1;

              /// create scalp/electrode sponge triangle nodes
              for (VMesh::Node::index_type k = 0; k < tmp_fld_msh->num_nodes(); k++)
              {
                tmp_fld_msh->get_center(p, k);
                output_vmesh->add_point(p);
              }

              /// create electrode sponge top triangle nodes
              long count_pts = 0;
              for (VMesh::Node::index_type k = 0; k < tmp_fld_msh->num_nodes(); k++)
              {
                tmp_fld_msh->get_center(p, k);
                scalp_vmesh->find_closest_node(distance, q, didx, p);
                scalp_vmesh->get_normal(norm1, didx);
                double x = norm1.x(), y = norm1.y(), z = norm1.z();
                double normal_mag = sqrt(x*x + y*y + z*z);
                x /= normal_mag;
                y /= normal_mag;
                z /= normal_mag;
                Point tmp_pt;
                if (!flip_normal)
                  tmp_pt = Point(p.x() + x*elc_thickness[i], p.y() + y*elc_thickness[i], p.z() + z*elc_thickness[i]);
                else
                  tmp_pt = Point(p.x() - x*elc_thickness[i], p.y() - y*elc_thickness[i], p.z() - z*elc_thickness[i]);

                output_vmesh->add_point(tmp_pt);
                count_pts++;

              }
              int offset = nr_elc_sponge_triangles_on_scalp;

              /// the triangle ordering should form a tringle which normal is opposite to scalp surface normal, check it!
              Point tmp;
              VMesh::Node::index_type scalp_node_idx;
              bool scalp_elec_surf_needs_flipping = false;
              if (scalp_vmesh->find_closest_node(distance, tmp, scalp_node_idx, r))
              {
                scalp_vmesh->get_normal(norm1, scalp_node_idx);
                double dot_product = Dot(norm1, Vector(nx, ny, nz));
                if (dot_product > 0)
                {
                  scalp_elec_surf_needs_flipping = true;
                }
              }

              VMesh::Elem::index_type node_tri_ind = 0; /// use first triangle to determine ordering
              VMesh::Node::array_type onodes(3);
              scalp_vmesh->get_nodes(onodes, node_tri_ind);
              double omin = std::numeric_limits<double>::quiet_NaN(), omax = std::numeric_limits<double>::quiet_NaN(); /// determine tri ordering from scalp mesh
              int imin = -1, imax = -1;

              for (int q = 0; q < 3; q++)
              {
                if (onodes[q] < omin || IsNan(omin))
                {
                  omin = onodes[q];
                  imin = q;
                }
                if (onodes[q] > omax || IsNan(omax))
                {
                  omax = onodes[q];
                  imax = q;
                }
              }

              if (imin == imax) // an triangle that has nodes that are the same (sliver element?) - that shoud not happend
              {
                std::ostringstream ostr1;
                ostr1 << " Triangular definition of scalp surface element contains duplicated node number in electrode/sponge - scalp contact surface. " << std::endl;
                THROW_ALGORITHM_PROCESSING_ERROR(ostr1.str());
              }

              double ordering[3];
              for (int q = 0; q < 3; q++)
              {
                if (q == imin)
                  ordering[q] = 0;
                else
                  if (q == imax)
                    ordering[q] = 2;
                  else
                    ordering[q] = 1;
              }

              if (scalp_elec_surf_needs_flipping)
              {
                double tmp_ordering[3];
                tmp_ordering[0] = ordering[2]; tmp_ordering[1] = ordering[1]; tmp_ordering[2] = ordering[0];
                ordering[0] = tmp_ordering[0]; ordering[1] = tmp_ordering[1]; ordering[2] = tmp_ordering[2];
              }

              for (int iter_tmp = 0; iter_tmp < 2; iter_tmp++)
              {
                for (VMesh::Elem::index_type k = 0; k < tmp_fld_msh->num_elems(); k++)
                {
                  VMesh::Node::array_type onodes(3), tmp_onodes(3);
                  tmp_fld_msh->get_nodes(onodes, k);
                  onodes[0] += offset; onodes[1] += offset; onodes[2] += offset;
                  if (iter_tmp == 0)
                  {
                    tmp_onodes[0] = onodes[0]; tmp_onodes[1] = onodes[1]; tmp_onodes[2] = onodes[2];
                    onodes[ordering[0]] = tmp_onodes[0]; onodes[ordering[1]] = tmp_onodes[1]; onodes[ordering[2]] = tmp_onodes[2];
                  }
                  output_vmesh->add_elem(onodes);
                  field_values_elc_on_scalp.push_back(i);
                }
                offset += tmp_fld_msh->num_nodes();
              }
              MatrixHandle mapping;
              GetFieldBoundaryAlgo algo_getfldbnd;
              FieldHandle boundary;
              algo_getfldbnd.run(tmp_fld_1, boundary, mapping);
              VMesh* boundary_msh = boundary->vmesh();
              /// connect electrode sponge surfaces
              if (boundary->vmesh()->is_curvemesh())
              {
                tmp_fld_msh->synchronize(Mesh::NODE_LOCATE_E);
                VMesh::Edge::iterator meshEdgeIter;
                VMesh::Edge::iterator meshEdgeEnd;
                VMesh::Node::array_type nodesFromEdge(2);
                boundary_msh->end(meshEdgeEnd);
                for (boundary_msh->begin(meshEdgeIter); meshEdgeIter != meshEdgeEnd; ++meshEdgeIter)
                {
                  VMesh::Edge::index_type edgeID = *meshEdgeIter;
                  boundary_msh->get_nodes(nodesFromEdge, edgeID);
                  Point p0, p1;
                  boundary_msh->get_point(p0, nodesFromEdge[0]);
                  boundary_msh->get_point(p1, nodesFromEdge[1]);
                  VMesh::Node::index_type idx1;
                  tmp_fld_msh->find_closest_node(distance, r, idx1, p0);
                  if (distance != 0)
                  {
                    std::ostringstream ostr3;
                    ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode (could not find scalp bnd fragment node in scalp fragment). Make sure that prototype encapsulates scalp." << std::endl;
                    remark(ostr3.str());
                    break;
                  }
                  VMesh::Node::index_type idx2;
                  tmp_fld_msh->find_closest_node(distance, r, idx2, p1);
                  if (distance != 0)
                  {
                    std::ostringstream ostr3;
                    ostr3 << " Electrode sponge/scalp surface could not be found for " << i + 1 << ". electrode (could not find scalp bnd fragment node in scalp fragment). Make sure that prototype encapsulates scalp." << std::endl;
                    remark(ostr3.str());
                    break;
                  }
                  long i1 = (long)idx1, i2 = (long)idx2;
                  long i3 = i1 + tmp_fld_msh->num_nodes(), i4 = i2 + tmp_fld_msh->num_nodes();
                  VMesh::Node::array_type onodes(3);
                  onodes[0] = i3 + nr_elc_sponge_triangles_on_scalp;
                  onodes[1] = i1 + nr_elc_sponge_triangles_on_scalp;
                  onodes[2] = i4 + nr_elc_sponge_triangles_on_scalp;
                  output_vmesh->add_elem(onodes);
                  field_values_elc_on_scalp.push_back(i);
                  onodes[0] = i4 + nr_elc_sponge_triangles_on_scalp;
                  onodes[1] = i1 + nr_elc_sponge_triangles_on_scalp;
                  onodes[2] = i2 + nr_elc_sponge_triangles_on_scalp;
                  output_vmesh->add_elem(onodes);
                  field_values_elc_on_scalp.push_back(i);
                }

                nr_elc_sponge_triangles_on_scalp += tmp_fld_msh->num_nodes() + count_pts;
              }
              else
              {
                std::ostringstream ostr3;
                ostr3 << " Trying to get boundary for scalp peace usind to create electrode " << i + 1 << ". Make sure that prototype encapsulates scalp." << std::endl;
                remark(ostr3.str());
                continue;
              }

            }
          }
        }
      }
      else
      {
        std::ostringstream ostr3;
        ostr3 << " tDCS electrode definition for " << i + 1 << ". row needs be a QuadSurf / TriSurf (with defined scalar or vectors data)." << std::endl;
        remark(ostr3.str());
        continue;
      }

      tdcs_vfld->resize_values();
      tdcs_vfld->set_values(field_values);

      if (compute_third_output)
      {
        output_vfld->resize_values();
        output_vfld->set_values(field_values_elc_on_scalp);
        elc_sponge_locations.reset(new DenseMatrix(static_cast<size_t>(num_valid_electrode_definition), 4ul, 0.0));
        int count = 0;
        for (int j = 0; j < valid_electrode_definition.size(); j++)
        {
          if (valid_electrode_definition[j] == 1)
          {
            if (count < elc_sponge_locations->nrows())
            {
              (*elc_sponge_locations)(count, 0) = elc_x[i];
              (*elc_sponge_locations)(count, 1) = elc_y[i];
              (*elc_sponge_locations)(count, 2) = elc_z[i];
              (*elc_sponge_locations)(count, 3) = elc_thickness[i];
              count++;
            }
          }
        }
      }

    }

    if (compute_third_output)
    {
      VMesh::Face::size_type isize;
      output->vmesh()->size(isize);
      flipnormal_algo.run(output, output);
    }
    else
    {
      FieldHandle tmp;
      output = tmp;
    }

  }
  else
  {
    std::ostringstream ostr3;
    ostr3 << " Internal error: tDCS electrode could not be generated. " << std::endl;
    remark(ostr3.str());
  }

  VariableHandle table2(new Variable(Name("Table"), new_table));
  return boost::make_tuple(elc_sponge_locations, electrode_field, output, table2);
}

boost::tuple<VariableHandle, DenseMatrixHandle, FieldHandle, FieldHandle, FieldHandle> ElectrodeCoilSetupAlgorithm::run(const FieldHandle scalp, const DenseMatrixHandle locations, const std::vector<FieldHandle>& elc_coil_proto) const
{
  FieldInformation fi(scalp);
  if (!(fi.is_trisurfmesh() || fi.is_quadsurfmesh()))
  {
    std::ostringstream ostr;
    ostr << " First input (SCALP_SURF) needs to be a triangluar (TRISURF) of rectangular mesh (QUADSURF)." << std::endl;
    THROW_ALGORITHM_PROCESSING_ERROR(ostr.str());
  }

  VariableHandle table_output = fill_table(scalp, locations, elc_coil_proto);
  DenseMatrixHandle elc_sponge_locations;
  FieldHandle electrodes_field, coils_field, final_electrodes_field;
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
  for (int i = 0; i < table.size(); i++)
  {
    auto row = (table[i]).toVector();

    /// the c* integer variables refine the column vales (1,2,3,...,10) for the current row
    int c1 = std::numeric_limits<double>::quiet_NaN(), c2 = std::numeric_limits<double>::quiet_NaN();
    double c3 = std::numeric_limits<double>::quiet_NaN(), c4 = std::numeric_limits<double>::quiet_NaN(), c5 = std::numeric_limits<double>::quiet_NaN(),
      c6 = std::numeric_limits<double>::quiet_NaN(), c7 = std::numeric_limits<double>::quiet_NaN(), c8 = std::numeric_limits<double>::quiet_NaN(),
      c9 = std::numeric_limits<double>::quiet_NaN(), c10 = std::numeric_limits<double>::quiet_NaN();

    bool valid_position = true, valid_normal = true, row_valid = true;

    try
    {
      c1 = boost::lexical_cast<int>(row[0].toString());
    }
    catch (boost::bad_lexical_cast &)
    {
      c1 = std::numeric_limits<double>::quiet_NaN();
      row_valid = false;
    }

    try
    {
      c2 = boost::lexical_cast<int>(row[1].toString());
    }
    catch (boost::bad_lexical_cast &)
    {
      c2 = std::numeric_limits<double>::quiet_NaN();
      row_valid = false;
    }

    auto str_x = row[2].toString();
    if (str_x.compare("???") == 0)
    {
      valid_position = false;
    }
    else
    {
      try
      {
        c3 = boost::lexical_cast<double>(str_x);
      }
      catch (boost::bad_lexical_cast &)
      {
        c3 = std::numeric_limits<double>::quiet_NaN();
        valid_position = false;
      }
    }

    auto str_y = row[3].toString();
    if (str_y.compare("???") == 0)
    {
      valid_position = false;
    }
    else
    {
      try  ///get the electrode thickness from GUI
      {
        c4 = boost::lexical_cast<double>(str_y);
      }
      catch (boost::bad_lexical_cast &)
      {
        c4 = std::numeric_limits<double>::quiet_NaN();
        valid_position = false;
      }
    }

    auto str_z = row[4].toString();
    if (str_z.compare("???") == 0)
    {
      valid_position = false;
    }
    else
    {
      try  ///get the electrode thickness from GUI
      {
        c5 = boost::lexical_cast<double>(str_z);
      }
      catch (boost::bad_lexical_cast &)
      {
        c5 = std::numeric_limits<double>::quiet_NaN();
        valid_position = false;
      }
    }

    c6 = -1;
    auto angle = row[5].toString();
    if (angle.compare("???") != 0)
    {
      try
      {
        c6 = boost::lexical_cast<double>(row[5].toString());
      }
      catch (boost::bad_lexical_cast &)
      {
        c6 = std::numeric_limits<double>::quiet_NaN();
      }
    }
    else
      c6 = std::numeric_limits<double>::quiet_NaN();

    if (IsNan(c6))
    {
      c6 = 0;
    }

    if (row_valid && c1 > 0 && c2 > 0)   ///both combo boxes are set up, so this could be a valid row but first check if ...
      /// its a tDCS electrode and if so if the thickness is provided in the GUI
      /// or if its a TMS coil check if the prototype has normals - if so lets put the normal in the GUI
    {

      FieldHandle prototyp = elc_coil_proto[c1 - 1];

      if (!prototyp)
      {
        std::ostringstream ostr1;
        ostr1 << "Module input " << (c1 + 2) << " seems to be empty" << std::endl;
        THROW_ALGORITHM_PROCESSING_ERROR(ostr1.str());
      }
      FieldInformation fi_proto(prototyp);
      if (!(fi_proto.is_trisurfmesh() || fi_proto.is_quadsurfmesh() || fi_proto.is_pointcloud()))
      {
        std::ostringstream ostr;
        ostr << (c1 + 2) << ". input needs to be a triangluar (TRISURF), rectangular mesh (QUADSURF) or point cloud." << std::endl;
        THROW_ALGORITHM_PROCESSING_ERROR(ostr.str());
      }

      GetFieldDataAlgo algo_getfielddata;
      DenseMatrixHandle fielddata;
      try
      {
        fielddata = algo_getfielddata.runMatrix(prototyp);
      }
      catch (...)
      {

      }

      GetMeshNodesAlgo algo_getfieldnodes;
      DenseMatrixHandle fieldnodes;
      try
      {
        algo_getfieldnodes.run(prototyp, fieldnodes);
      }
      catch (...)
      {

      }

      if (!(fieldnodes->nrows() == 3 || fieldnodes->ncols() == 3) &&
        !(fielddata->nrows() == fieldnodes->nrows() || fielddata->nrows() == fieldnodes->ncols() ||
        fielddata->ncols() == fieldnodes->nrows() || fielddata->ncols() == fieldnodes->ncols())
        )
      {
        std::ostringstream ostr_;
        ostr_ << "Module input " << (c1 + 2) << " (prototype) does contain corrupted data (mesh locations does not match mesh data dimensions)." << std::endl;
        THROW_ALGORITHM_PROCESSING_ERROR(ostr_.str());
      }

      /// get Coil normal from GUI
      /// get NX
      try
      {
        c7 = boost::lexical_cast<double>(row[6].toString());
      }
      catch (boost::bad_lexical_cast &)
      {
        c7 = std::numeric_limits<double>::quiet_NaN();
        valid_normal = false;
      }

      /// get NY
      try
      {
        c8 = boost::lexical_cast<double>(row[7].toString());
      }
      catch (boost::bad_lexical_cast &)
      {
        c8 = std::numeric_limits<double>::quiet_NaN();
        valid_normal = false;
      }

      /// get NZ
      try
      {
        c9 = boost::lexical_cast<double>(row[8].toString());
      }
      catch (boost::bad_lexical_cast &)
      {
        c9 = std::numeric_limits<double>::quiet_NaN();
        valid_normal = false;
      }

      /// get electrode thickness
      try
      {
        c10 = boost::lexical_cast<double>(row[9].toString());
      }
      catch (boost::bad_lexical_cast &)
      {
        c10 = std::numeric_limits<double>::quiet_NaN();
      }

      if (!valid_normal && fielddata->ncols() == 3 && fielddata->nrows() >= 1) /// use FIRST(!) mag. dipole direction to infer coil orientation. Note that the determined direction could be anti parallel
      {
        double norm = sqrt((*fielddata)(0, 0)*(*fielddata)(0, 0) + (*fielddata)(0, 1)*(*fielddata)(0, 1) + (*fielddata)(0, 2)*(*fielddata)(0, 2)); /// normalize mag. dipole to get coil direction estimate
        c7 = (*fielddata)(0, 0) / norm;
        c8 = (*fielddata)(0, 1) / norm;
        c9 = (*fielddata)(0, 2) / norm;
        valid_normal = true;
      }

      if (c7 == 0 && c8 == 0 && c9 == 0)
      {
        std::ostringstream ostr3;
        ostr3 << " The TMS coil defined in table row " << i + 1 << " could not get any valid normal information, found normal contained only zeros (0,0,0). " << std::endl;
        remark(ostr3.str());
        valid_normal = false;
      }

      if (!valid_position && fieldnodes->nrows() >= 1 && fieldnodes->ncols() == 3)
      {
        double x = 0, y = 0, z = 0;
        for (int j = 0; j < fieldnodes->nrows(); j++)
        {
          x += (*fieldnodes)(j, 0);
          y += (*fieldnodes)(j, 1);
          z += (*fieldnodes)(j, 2);
        }
        x /= fieldnodes->nrows();
        y /= fieldnodes->nrows();
        z /= fieldnodes->nrows();
        c3 = x;
        c4 = y;
        c5 = z;
        valid_position = true;
      }

      if (c2 == tDCS_stim_type) /// tDCS?
      {
        if (valid_position)
        {
          elc_prototyp_map.push_back(c1);
          elc_angle_rotation.push_back(c6);
          elc_x.push_back(c3);
          elc_y.push_back(c4);
          elc_z.push_back(c5);
          elc_thickness.push_back(c10);;
        }

      }
      else

        if (c2 == TMS_stim_type) /// TMS?
        {
          if (valid_normal && valid_position)
          {
            coil_prototyp_map.push_back(c1);
            coil_angle_rotation.push_back(c6);
            coil_x.push_back(c3);
            coil_y.push_back(c4);
            coil_z.push_back(c5);
            coil_nx.push_back(c7);
            coil_ny.push_back(c8);
            coil_nz.push_back(c9);
          }
          else
          {
            std::ostringstream ostr3;
            ostr3 << " The TMS coil defined in table row " << i + 1 << " has no normal defined (NX,NY,NZ). Further, no normal could be taken from the linked prototyp field data. " << std::endl;
            remark(ostr3.str());
          }
        }

    }
  }

  bool valid_tdcs = false, valid_tms = false;
  int t1 = elc_angle_rotation.size();
  int t2 = elc_thickness.size();
  int t3 = elc_x.size();
  int t4 = elc_y.size();
  int t5 = elc_z.size();
  int t14 = elc_coil_proto.size();

  if (t1 == t2 && t1 == t3 && t1 == t4 && t1 == t5 && t14 > 0 && t1 > 0)
  {
    boost::tie(elc_sponge_locations, electrodes_field, final_electrodes_field, table_output) = make_tdcs_electrodes(scalp, elc_coil_proto, elc_prototyp_map, elc_x, elc_y, elc_z, elc_angle_rotation, elc_thickness, table_output);
    valid_tdcs = true;
  }

  int t6 = coil_prototyp_map.size();
  int t7 = coil_angle_rotation.size();
  int t8 = coil_x.size();
  int t9 = coil_y.size();
  int t10 = coil_z.size();
  int t11 = coil_nx.size();
  int t12 = coil_ny.size();
  int t13 = coil_nz.size();

  if (t6 == t7 && t6 == t8 && t6 == t9 && t6 == t10 && t6 == t11 && t6 == t12 && t6 == t13 && t14 > 0 && t6 > 0)
  {
    coils_field = make_tms(scalp, elc_coil_proto, coil_prototyp_map, coil_x, coil_y, coil_z, coil_angle_rotation, coil_nx, coil_ny, coil_nz);
    valid_tms = true;
  }

  if (!valid_tdcs && !valid_tms)
  {
    std::ostringstream ostr0;
    ostr0 << " Not a single table row contains valid information. " << std::endl;
    remark(ostr0.str());
  }

  return boost::make_tuple(table_output, elc_sponge_locations, electrodes_field, final_electrodes_field, coils_field);
}


AlgorithmOutput ElectrodeCoilSetupAlgorithm::run(const AlgorithmInput& input) const
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

  if (!(elc_coil_proto.size() >= 1))
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" At least one prototypical coil (POINTMESH) or electrode (TRISURFMESH) definition as a field input must be provided.");
  }

  if (locations->ncols() != 3)
  {
    THROW_ALGORITHM_PROCESSING_ERROR(" Locations (second module input) needs to be a dense matrix input with dimensions lx3 (l being > 0). ");
  }

  VariableHandle table;
  DenseMatrixHandle elc_sponge_loc_avr;
  FieldHandle coils_field, electrodes_field, final_electrode_field;
  boost::tie(table, elc_sponge_loc_avr, electrodes_field, final_electrode_field, coils_field) = run(scalp, locations, elc_coil_proto);

  AlgorithmOutput output;
  output[ELECTRODE_SPONGE_LOCATION_AVR] = elc_sponge_loc_avr;
  output[MOVED_ELECTRODES_FIELD] = electrodes_field;
  output[FINAL_ELECTRODES_FIELD] = final_electrode_field;
  output[COILS_FIELD] = coils_field;
  output.setAdditionalAlgoOutput(table);

  return output;
}
