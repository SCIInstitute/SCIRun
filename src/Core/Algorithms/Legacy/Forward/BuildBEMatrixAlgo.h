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
*  BuildBEMatrix.h:  class to build Boundary Elements matrix
*
*  Written by:
*   Saeed Babaeizadeh
*   Northeastern University
*   January 2006
*/

#ifndef CORE_ALGORITHMS_LEGACY_FORWARD_BUILDBEMATRIXALGO_H
#define CORE_ALGORITHMS_LEGACY_FORWARD_BUILDBEMATRIXALGO_H

#include <Core/Datatypes/MatrixFwd.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Legacy/Forward/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Forward {

        ALGORITHM_PARAMETER_DECL(FieldNameList);
        ALGORITHM_PARAMETER_DECL(FieldTypeList);
        ALGORITHM_PARAMETER_DECL(BoundaryConditionList);
        ALGORITHM_PARAMETER_DECL(InsideConductivityList);
        ALGORITHM_PARAMETER_DECL(OutsideConductivityList);

        typedef std::vector<std::string> FieldTypeListType;

        class SCISHARE BuildBEMatrixBase
        {
        protected:
          static void get_g_coef( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            double,
            double,
            const Geometry::Vector&,
            Datatypes::DenseMatrix&);

          static void get_cruse_weights( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            double,
            double,
            double,
            Datatypes::DenseMatrix& );

          static void getOmega( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            Datatypes::DenseMatrix& );

          static double do_radon_g( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            double,
            double,
            Datatypes::DenseMatrix& );

          static void get_auto_g( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            unsigned int,
            Datatypes::DenseMatrix&,
            double,
            double,
            Datatypes::DenseMatrix& );

          static void bem_sing( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector&,
            unsigned int,
            Datatypes::DenseMatrix&,
            double,
            double,
            Datatypes::DenseMatrix& );

          static double get_new_auto_g( const Geometry::Vector&,
            const Geometry::Vector&,
            const Geometry::Vector& );

        public:
          static void make_cross_G( VMesh*,
            VMesh*,
            Datatypes::DenseMatrixHandle&,
            double,
            double,
            double,
            const std::vector<double>& );

          static void make_cross_G_allocate(VMesh*, VMesh*, Datatypes::DenseMatrixHandle&);

          static void make_auto_G( VMesh*,
            Datatypes::DenseMatrixHandle&,
            double,
            double,
            double,
            const std::vector<double>& );

          static void make_auto_G_allocate(VMesh*, Datatypes::DenseMatrixHandle&);

          static void make_auto_P( VMesh*,
            Datatypes::DenseMatrixHandle&,
            double,
            double,
            double );

          static int numNodes(FieldHandle f);
          static int numNodes(VMesh* hsurf);
          static void make_auto_P_allocate( VMesh*,
            Datatypes::DenseMatrixHandle&);

          static void make_cross_P( VMesh*,
            VMesh*,
            Datatypes::DenseMatrixHandle&,
            double,
            double,
            double );

          static void make_cross_P_allocate( VMesh*,
            VMesh*, Datatypes::DenseMatrixHandle&);

          static void pre_calc_tri_areas(VMesh*, std::vector<double>&);

          static int compute_parent(const std::vector<VMesh*> &meshes, int index);
          static bool compute_nesting(std::vector<int> &nesting, const std::vector<VMesh*> &meshes);
          static bool ray_triangle_intersect(double &t,
            const Geometry::Point &point,
            const Geometry::Vector &dir,
            const Geometry::Point &p0,
            const Geometry::Point &p1,
            const Geometry::Point &p2);
          static void compute_intersections(std::vector<std::pair<double, int> >& results,
            const VMesh* mesh,
            const Geometry::Point &p, const Geometry::Vector &v,
            int marker);

        };

        class SCISHARE bemfield
        {
        public:
          explicit bemfield(const FieldHandle& fieldHandle) :
          field_(fieldHandle),
            insideconductivity(0),
            outsideconductivity(0),
            surface(false),
            neumann(false),
            measurement(false),
            dirichlet(false),
            source(false) {}

          void set_source_dirichlet()
          {
            this->source = true;
            this->dirichlet = true;
            this->measurement = false;
            this->neumann = false;
          }

          void set_measurement_neumann()
          {
            this->source = false;
            this->dirichlet = false;
            this->measurement = true;
            this->neumann = true;
          }

          FieldHandle field_; // handle of the field itself
          double insideconductivity; // if it applies, the conductivity inside the surface
          double outsideconductivity; // if it applies, the conductivity outside the surface
          bool surface; // true if a surface, false if just points
          // TODO: setters? should change neumann and dirichlet based on measurement and source?
          // BURAK ANSWER: Yes, for the time being they are directly connected. This structure allows someone to introduce a new algorithm with very few modifications to the module design.
          bool neumann; // true if Neumann boundary conditions are defined on this surface
          bool measurement; // true if a measurement field
          bool dirichlet; // true if Dirichlet boundary conditions are defined on this surface
          bool source; // true if a source field
        };

        typedef std::vector<bemfield> bemfield_vector;

        class SCISHARE BEMAlgoImpl
        {
        public:
          virtual ~BEMAlgoImpl() {}
          virtual Datatypes::MatrixHandle compute(const bemfield_vector& fields) const = 0;
        };

        typedef boost::shared_ptr<BEMAlgoImpl> BEMAlgoPtr;

        class SCISHARE BEMAlgoImplFactory
        {
        public:
          static BEMAlgoPtr create(const bemfield_vector& fields);
        };

      }}}}

#endif
