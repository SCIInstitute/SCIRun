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


/*
 *  BuildBEMatrix.h:  class to build Boundary Elements matrix
 *
 *  Written by:
 *   Saeed Babaeizadeh
 *   Northeastern University
 *   January 2006
 */

#include <algorithm>
#include <map>
#include <iostream>
#include <string>
#include <fstream>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>
#include <Core/Datatypes/MatrixFwd.h>

#include <Packages/BioPSE/Core/Algorithms/NumApproximation/share.h>

namespace BioPSE {

using namespace SCIRun;

class SCISHARE BuildBEMatrixBase
{
  typedef SCIRun::TriSurfMesh<TriLinearLgn<Point> > TSMesh;

private:

  inline void get_g_coef( const Vector&,
                          const Vector&,
                          const Vector&,
                          const Vector&,
                          double,
                          double,
                          const Vector&,
                          DenseMatrix&);

  inline void get_cruse_weights( const Vector&,
                                 const Vector&,
                                 const Vector&,
                                 double,
                                 double,
                                 double,
                                 DenseMatrix& );

  inline void getOmega( const Vector&,
                        const Vector&,
                        const Vector&,
                        DenseMatrix& );

  inline double do_radon_g( const Vector&,
                            const Vector&,
                            const Vector&,
                            const Vector&,
                            double,
                            double,
                            DenseMatrix& );

  inline void get_auto_g( const Vector&,
                          const Vector&,
                          const Vector&,
                          unsigned int,
                          DenseMatrix&,
                          double,
                          double,
                          DenseMatrix& );
						  
  inline void bem_sing( const Vector&,
                          const Vector&,
                          const Vector&,
                          unsigned int,
                          DenseMatrix&,
                          double,
                          double,
                          DenseMatrix& );

  inline double get_new_auto_g( const Vector&,
                                const Vector&,
                                const Vector& );
   
public:

   void make_cross_G( VMesh*,
                            VMesh*,
                            DenseMatrixHandle&,
                            double,
                            double,
                            double,
			    const std::vector<double>& );

  void make_auto_G( VMesh*,
                           DenseMatrixHandle&,
                           double,
                           double,
                           double,
			   const std::vector<double>& );
 
 void make_auto_P( VMesh*,
                           DenseMatrixHandle&,
                           double,
                           double,
                           double );

void make_cross_P( VMesh*,
                            VMesh*,
                            DenseMatrixHandle&,
                            double,
                            double,
                            double );

void pre_calc_tri_areas(VMesh*, std::vector<double>&);

};


} // end namespace BioPSE
