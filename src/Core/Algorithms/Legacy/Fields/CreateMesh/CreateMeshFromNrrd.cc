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


#include <Core/Algorithms/Fields/CreateMesh/CreateMeshFromNrrd.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

enum {UNKNOWN=0,UNSTRUCTURED=1,STRUCTURED=2,IRREGULAR=4,REGULAR=8};

bool
StructuredCreateMeshFromNrrdAlgo(AlgoBase* algo,
				 FieldHandle& fHandle,
				 NrrdDataHandle pointsH,
				 size_type idim,
				 size_type jdim,
				 size_type kdim)
{
  VMesh* imesh = fHandle->vmesh();

  size_t rank = pointsH->nrrd_->axis[0].size;
  double xVal = 0.0, yVal = 0.0, zVal = 0.0;

  switch(pointsH->nrrd_->type)
  {
    case nrrdTypeChar:
    {
      char* data = reinterpret_cast<char*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeUChar:
    {
      unsigned char* data = reinterpret_cast<unsigned char*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeShort:
    {
      short* data = reinterpret_cast<short*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeUShort:
    {
      unsigned short* data = reinterpret_cast<unsigned short*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeInt:
    {
      int* data = reinterpret_cast<int*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeUInt:
    {
      unsigned int* data = reinterpret_cast<unsigned int*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeLLong:
    {
      long long* data = reinterpret_cast<long long*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeULLong:
    {
      unsigned long long* data = reinterpret_cast<unsigned long long*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeFloat:
    {
      float* data = reinterpret_cast<float*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    case nrrdTypeDouble:
    {
      double* data = reinterpret_cast<double*>(pointsH->nrrd_->data);
      size_type size = idim*jdim*kdim;
      for(index_type idx=0; idx<size; idx++ )
      {
        // Mesh
        if( rank >= 1 ) xVal = static_cast<double>(data[idx*rank+0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[idx*rank+1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[idx*rank+2]);

        imesh->set_point(Point(xVal, yVal, zVal), VMesh::Node::index_type(idx));
      }
    }
    break;
    default:
      algo->error("The points nrrd is of a type that is not supported in this converter.");
      algo->algo_end(); return (false);
  }
  algo->algo_end(); return (true);
}


bool
UnstructuredCreateMeshFromNrrdAlgo( AlgoBase* algo,
				    FieldHandle& fHandle,
				    NrrdDataHandle pointsH,
				    NrrdDataHandle connectH,
				    size_type nconnections,
				    int which)
{
  VField* ifield = fHandle->vfield();
  VMesh*  imesh  = fHandle->vmesh();

  size_type npts = pointsH->nrrd_->axis[1].size;
  for( size_t i=2; i<pointsH->nrrd_->dim; ++i)
        npts *= pointsH->nrrd_->axis[i].size;
  size_type rank = pointsH->nrrd_->axis[0].size;
  double xVal = 0.0, yVal = 0.0, zVal = 0.0;

  switch(pointsH->nrrd_->type)
  {
    case nrrdTypeChar:
    {
      char *data = reinterpret_cast<char*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeUChar:
    {
      unsigned char *data = reinterpret_cast<unsigned char*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeShort:
    {
      short *data = reinterpret_cast<short*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeUShort:
    {
      unsigned short *data = reinterpret_cast<unsigned short*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeInt:
    {
      int *data = reinterpret_cast<int*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeUInt:
    {
      unsigned int *data = reinterpret_cast<unsigned int*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeLLong:
    {
      long long *data = reinterpret_cast<long long*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeULLong:
    {
      unsigned long long *data = reinterpret_cast<unsigned long long*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeFloat:
    {
      float *data = reinterpret_cast<float*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    case nrrdTypeDouble:
    {
      double *data = reinterpret_cast<double*>(pointsH->nrrd_->data);
      for(size_type index=0; index<npts; index++ )
      {
        if( rank >= 1 ) xVal = static_cast<double>(data[index*rank + 0]);
        if( rank >= 2 ) yVal = static_cast<double>(data[index*rank + 1]);
        if( rank >= 3 ) zVal = static_cast<double>(data[index*rank + 2]);
        imesh->add_point( Point(xVal, yVal, zVal) );
      }
    }
    break;
    default:
      algo->error("The points nrrd is of a type that is not supported in this converter.");
      algo->algo_end(); return (false);
  }

  bool single_element = false;
  if (connectH.get_rep() && connectH->nrrd_->dim == 1) single_element = true;

  if( nconnections > 0 )
  {
    size_type nelements = 0;
    if (single_element) nelements = 1;
    else if (which == 0)
    {
      // p x n
      nelements = connectH->nrrd_->axis[1].size;
    }
    else
    {
      // n x p
      nelements = connectH->nrrd_->axis[0].size;
    }

    VMesh::Node::array_type array(nconnections);

    switch(connectH->nrrd_->type)
    {
      case nrrdTypeChar:
      {
        char* data = reinterpret_cast<char*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeUChar:
      {
        unsigned char* data = reinterpret_cast<unsigned char*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeShort:
      {
        short* data = reinterpret_cast<short*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeUShort:
      {
        unsigned short* data = reinterpret_cast<unsigned short*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeInt:
      {
        int* data = reinterpret_cast<int*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeUInt:
      {
        unsigned int* data = reinterpret_cast<unsigned int*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeLLong:
      {
        long long* data = reinterpret_cast<long long*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeULLong:
      {
        unsigned long long* data = reinterpret_cast<unsigned long long*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeFloat:
      {
        float* data = reinterpret_cast<float*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      case nrrdTypeDouble:
      {
        double* data = reinterpret_cast<double*>(connectH->nrrd_->data);
        if (which == 0)
        {
          // p x n
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[i*nconnections+j]);
            }
            imesh->add_elem( array );
          }
        }
        else
        {
          // n x p
          for( size_type i=0; i<nelements; i++ )
          {
            for( size_type j=0; j<nconnections; j++ )
            {
              array[j] = static_cast<index_type>(data[j*nelements+i]);
            }
            imesh->add_elem( array );
          }
        }
      }
      break;
      default:
        algo->error("The connections nrrd is of a type that is not supported in this converter.");
        algo->algo_end(); return (false);
    }
  }

  ifield->resize_values();
  algo->algo_end(); return (true);
}



bool
CreateMeshFromNrrdAlgo::run(NrrdDataHandle pHandle,
                            NrrdDataHandle cHandle,
                            FieldHandle& fHandle)
{
  algo_start("CreateMeshFromNrrd");

  std::string quad_or_tet = get_option("quad_or_tet");
  std::string struct_or_unstruct = get_option("struct_or_unstruct");

  // initialize to 1 so it will at least go through i,j,k loops once.
  int idim = 1, jdim = 1, kdim = 1;

  std::string property;

  // Make sure the points coming in make sense.
  if( pHandle->nrrd_->axis[0].size != 3 )
  {
    error("Points Nrrd must contain 3D points.");
    error("If the points are 2D use UnuPad to turn in to 3D points.");
    algo_end(); return (false);
  }

  //////////////// DETERMINE MESH BASED ON INPUTS AND GUI INFO ////////////////

  // Have points and connections.
  if ( cHandle.get_rep() )
  {
    if (cHandle->nrrd_ == 0)
    {
      error("Connections Nrrd is empty.");
      algo_end(); return (false);
    }

    if (cHandle->nrrd_->dim != 1 && cHandle->nrrd_->dim != 2)
    {
      error("Connections Nrrd must be two dimensional (number of points in each connection by the number of elements).");
      algo_end(); return (false);
    }

    unsigned int which = 0; // which index contains p
    size_t nconnections = cHandle->nrrd_->axis[which].size;

    if (nconnections != 2 && nconnections != 3 &&
        nconnections != 4 && nconnections != 6 && nconnections != 8)
    {
      if( cHandle->nrrd_->dim == 1 )
      {
        error("Connections nrrd must have one axis with size 2, 3, 4, 6, or 8.");
        algo_end(); return (false);
      }
      else
      {
        which = 1;
        nconnections = cHandle->nrrd_->axis[which].size;

        if (nconnections != 2 && nconnections != 3 &&
            nconnections != 4 && nconnections != 6 && nconnections != 8)
        {
          error("Connections nrrd must have one axis with size 2, 3, 4, 6, or 8.");
          algo_end(); return (false);
        }

        warning("Assuming connection nrrd is transposed.");
      }
    }


    // If there is an elem type property make sure it matches what is
    // found automatically.
    std::string elem_type = "Auto";

    std::string property;
    unsigned int nconnections_prop = 0;

    if ( cHandle->get_property( "Elem Type",property ))
    {
      if( property.find( "Curve" ) != std::string::npos )
      {
        elem_type = "Curve";
        nconnections_prop = 2;
      }
      else if( property.find( "Tri" ) != std::string::npos )
      {
        elem_type = "Tri";
        nconnections_prop = 3;
      }
      else if( property.find( "Tet" ) != std::string::npos )
      {
        elem_type = "Tet";
        nconnections_prop = 4;
      }
      else if( property.find( "Quad" ) != std::string::npos )
      {
        elem_type = "Quad";
        nconnections_prop = 4;
      }
      else if( property.find( "Prism" ) != std::string::npos )
      {
        elem_type = "Prism";
        nconnections = 6;
      }
      else if( property.find( "Hex" ) != std::string::npos )
      {
        elem_type = "Hex";
        nconnections_prop = 8;
      }
    }

    if( nconnections_prop == 0 )
    {
      if (pHandle->get_property( "Elem Type",property ))
      {
        warning("Elem Type defined in Points nrrd instead of Connectivity nrrd.");
        if( property.find( "Curve" ) != std::string::npos )
        {
          elem_type = "Curve";
          nconnections = 2;
        }
        else if( property.find( "Tri" ) != std::string::npos )
        {
          elem_type = "Tri";
          nconnections_prop = 3;
        }
        else if( property.find( "Tet" ) != std::string::npos )
        {
          elem_type = "Tet";
          nconnections_prop = 4;
        }
        else if( property.find( "Quad" ) != std::string::npos )
        {
          elem_type = "Quad";
          nconnections = 4;
        }
        else if( property.find( "Prism" ) != std::string::npos )
        {
          elem_type = "Prism";
          nconnections_prop = 6;
        }
        else if( property.find( "Hex" ) != std::string::npos )
        {
          elem_type = "Hex";
          nconnections_prop = 8;
        }
      }
    }

    if( nconnections_prop && nconnections_prop != nconnections )
    {
      warning("The elem type properties and the number of connections found do not match.");
      warning("Using the number of connections found in the nrrd data.");
    }

    switch (nconnections)
    {
      case 2:
      {
        // 2 -> curve
        FieldInformation fi(CURVEMESH_E,LINEARDATA_E,DOUBLE_E);
        fHandle = CreateField(fi);
      }
      break;
      case 3:
      {
        // 3 -> tri
        FieldInformation fi(TRISURFMESH_E,LINEARDATA_E,DOUBLE_E);
        fHandle = CreateField(fi);
      }
      break;
      case 4:
      {
        // 4 -> quad/tet (ask which if this case)
        if (check_option("quad_or_tet","quad"))
        {
          FieldInformation fi(QUADSURFMESH_E,LINEARDATA_E,DOUBLE_E);
          fHandle = CreateField(fi);
        }
        else  if (check_option("quad_or_tet","tet"))
        {
          FieldInformation fi(TETVOLMESH_E,LINEARDATA_E,DOUBLE_E);
          fHandle = CreateField(fi);
        }
        else if (elem_type == "Tet")
        {
          FieldInformation fi(TETVOLMESH_E,LINEARDATA_E,DOUBLE_E);
          fHandle = CreateField(fi);
        }
        else if (elem_type == "Quad")
        {
          FieldInformation fi(QUADSURFMESH_E,LINEARDATA_E,DOUBLE_E);
          fHandle = CreateField(fi);
        }
        else
        {
          error("Auto detection of Elem Type using properties failed.");
          error("Connections Nrrd indicates 4 points per connection. Please indicate whether a Tet or Quad in UI.");
          algo_end(); return (false);
        }
      }
      break;
      case 6:
      {
        // 6 -> prism
        FieldInformation fi(PRISMVOLMESH_E,LINEARDATA_E,DOUBLE_E);
        fHandle = CreateField(fi);
      }
      break;
      case 8:
      {
        // 8 -> hex
        FieldInformation fi(HEXVOLMESH_E,LINEARDATA_E,DOUBLE_E);
        fHandle = CreateField(fi);
      }
      break;
    }

    if (fHandle.get_rep() == 0)
    {
      error("Could not generate output mesh.");
      algo_end(); return (false);
    }

    return (UnstructuredCreateMeshFromNrrdAlgo(this,fHandle,pHandle,cHandle,
                                                nconnections, which));
  }
  else
  {
    int topology_ = UNKNOWN;

    switch (pHandle->nrrd_->dim)
    {
      case 1:
      {
        topology_ = UNSTRUCTURED;
        // data 1D -> single point in a PointCloud
        FieldInformation fi(POINTCLOUDMESH_E,CONSTANTDATA_E,DOUBLE_E);
        fHandle = CreateField(fi);
      }
      break;
      case 2:
      {
        // data 1D ask if point cloud or structcurvemesh
        if (struct_or_unstruct == "pointcloud")
        {
          topology_ = UNSTRUCTURED;
          // data 2D -> PointCloud
          FieldInformation fi(POINTCLOUDMESH_E,CONSTANTDATA_E,DOUBLE_E);
          fHandle = CreateField(fi);
        }
        else if (struct_or_unstruct == "structcurve")
        {
          topology_ = STRUCTURED;
          // data 2D -> StructCurve
          FieldInformation fi(STRUCTCURVEMESH_E,LINEARDATA_E,DOUBLE_E);
          MeshHandle mesh = CreateMesh(fi,pHandle->nrrd_->axis[1].size);
          fHandle = CreateField(fi,mesh);
          idim = pHandle->nrrd_->axis[1].size;
        }
        else
        {
          // Try to figure out based on properties of the points
          if (pHandle->get_property( "Topology" , property))
          {
            if( property.find( "Unstructured" ) != std::string::npos )
            {
              topology_ = UNSTRUCTURED;
              // data 2D -> PointCloud
              FieldInformation fi(POINTCLOUDMESH_E,CONSTANTDATA_E,DOUBLE_E);
              fHandle = CreateField(fi);
            }
            else if( property.find( "Structured" ) != std::string::npos )
            {
              topology_ = STRUCTURED;
              // data 2D -> StructCurve
              FieldInformation fi(STRUCTCURVEMESH_E,LINEARDATA_E,DOUBLE_E);
              MeshHandle mesh = CreateMesh(fi,pHandle->nrrd_->axis[1].size);
              fHandle = CreateField(fi,mesh);
              idim = pHandle->nrrd_->axis[1].size;
            }
          }

          if( topology_ == UNKNOWN )
          {
            if (pHandle->get_property( "Elem Type", property))
            {
              if ( property.find( "Point") != std::string::npos )
              {
                topology_ = UNSTRUCTURED;
                // data 2D -> PointCloud
                FieldInformation fi(POINTCLOUDMESH_E,CONSTANTDATA_E,DOUBLE_E);
                fHandle = CreateField(fi);
              }
              else if ( property.find( "Curve") != std::string::npos )
              {
                topology_ = STRUCTURED;
                // data 2D -> StructCurve
                FieldInformation fi(STRUCTCURVEMESH_E,LINEARDATA_E,DOUBLE_E);
                MeshHandle mesh = CreateMesh(fi,pHandle->nrrd_->axis[1].size);
                fHandle = CreateField(fi,mesh);
                idim = pHandle->nrrd_->axis[1].size;
              }
            }
          }

          if( topology_ == UNKNOWN )
          {
            warning("Unable to determine if creating Point Cloud or Struct Curve. Defaulting to Point Cloud.");
            FieldInformation fi("PointCloudMesh",0,"double");
            fHandle = CreateField(fi);
            topology_ = UNSTRUCTURED;
          }
        }
      }
      break;
      case 3:
      {
        topology_ = STRUCTURED;
        // data 2D -> StructQuad
        FieldInformation fi(STRUCTQUADSURFMESH_E,LINEARDATA_E,DOUBLE_E);
        MeshHandle mesh = CreateMesh(fi,pHandle->nrrd_->axis[1].size,
                            pHandle->nrrd_->axis[2].size);
        fHandle = CreateField(fi,mesh);
        idim = pHandle->nrrd_->axis[1].size;
        jdim = pHandle->nrrd_->axis[2].size;
        //kdim = 1
      }
      break;
      case 4:
      {
        topology_ = STRUCTURED;
        // data 3D -> StructHexVol
        FieldInformation fi(STRUCTHEXVOLMESH_E,LINEARDATA_E,DOUBLE_E);
        MeshHandle mesh = CreateMesh(fi,pHandle->nrrd_->axis[1].size,
                            pHandle->nrrd_->axis[2].size,
                            pHandle->nrrd_->axis[3].size);
        fHandle = CreateField(fi,mesh);
        idim = pHandle->nrrd_->axis[1].size;
        jdim = pHandle->nrrd_->axis[2].size;
        kdim = pHandle->nrrd_->axis[3].size;
      }
      break;
      default:
        error("Incorrect dimensions for Data Nrrd");
        algo_end(); return(false);
      }

    // Create the mesh
    if (topology_ == UNSTRUCTURED)
    {
      return (UnstructuredCreateMeshFromNrrdAlgo(this,fHandle,pHandle,cHandle,0, 0));
    }
    else
    {
      return (StructuredCreateMeshFromNrrdAlgo(this,fHandle,pHandle,idim,jdim,kdim));
    }
  }
}

} // End namespace SCIRun
