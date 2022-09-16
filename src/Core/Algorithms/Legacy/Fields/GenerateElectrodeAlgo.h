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


#ifndef CORE_ALGORITHMS_FIELDS_SAMPLEFIELD_GENERATEELECTRODEALGO_H
#define CORE_ALGORITHMS_FIELDS_SAMPLEFIELD_GENERATEELECTRODEALGO_H 1

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Fields/share.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/Point.h>


namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Fields
      {
      ALGORITHM_PARAMETER_DECL(ElectrodeLength);
      ALGORITHM_PARAMETER_DECL(ElectrodeThickness);
      ALGORITHM_PARAMETER_DECL(ElectrodeWidth);
      ALGORITHM_PARAMETER_DECL(NumberOfControlPoints);
      ALGORITHM_PARAMETER_DECL(ElectrodeType);
      ALGORITHM_PARAMETER_DECL(ElectrodeResolution);
      ALGORITHM_PARAMETER_DECL(ElectrodeProjection);
      ALGORITHM_PARAMETER_DECL(MoveAll);
      ALGORITHM_PARAMETER_DECL(UseFieldNodes);
      ALGORITHM_PARAMETER_DECL( ProbeColor);
      ALGORITHM_PARAMETER_DECL( ProbeLabel);
      ALGORITHM_PARAMETER_DECL( ProbeSize);

      class GenerateElectrodeImpl
      {
      public:
        void get_points(std::vector<Geometry::Point>& points);
        void get_centers(std::vector<Geometry::Point>& , std::vector<Geometry::Point>& , double , int );
        FieldHandle Make_Mesh_Wire(std::vector<Geometry::Point>& , double , int );
          
          bool CalculateSpline(std::vector<double>&  , std::vector<double>& , std::vector<double>& , std::vector<double>& );
          
          bool CalculateSpline(std::vector<double>& , std::vector<Geometry::Point>& , std::vector<double>&, std::vector<Geometry::Point>&);

        std::vector<Geometry::Point> Previous_points_;
      };
           
      
class SCISHARE GenerateElectrodeAlgo : public AlgorithmBase
{
  public:
    GenerateElectrodeAlgo();
    bool runImpl(FieldHandle input, FieldHandle&, FieldHandle&) const;
    static const AlgorithmOutputName ControlPoints, ElectrodeMesh;
    
    AlgorithmOutput run(const AlgorithmInput& input) const override;
    
    typedef std::pair<double, VMesh::Elem::index_type> weight_type;
    typedef std::vector<weight_type> table_type;

    bool build_table(VMesh *mesh, VField* vfield,
                     std::vector<weight_type> &table,
                     std::string& method);
    static bool
    weight_less(const weight_type &a, const weight_type &b)
    {
      return (a.first < b.first);
    }
    

//    FieldHandle Make_Mesh_Wire(std::vector<Geometry::Point>& final_points, double, int) const;
//    void get_centers(std::vector<Geometry::Point>&, std::vector<Geometry::Point>&, double, int) const;
    private:
      SharedPointer<Core::Algorithms::Fields::GenerateElectrodeImpl> impl_;
    
};
}
}
}
}

#endif
