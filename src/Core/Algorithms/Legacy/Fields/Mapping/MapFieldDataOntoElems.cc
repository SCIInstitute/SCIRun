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

#include <Core/Thread/Thread.h>
#include <Core/Thread/Barrier.h>

#include <Core/Algorithms/Fields/Mapping/MapFieldDataOntoElems.h>
#include <Core/Algorithms/Fields/Mapping/MappingDataSource.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

#ifdef _WIN32
typedef __int64 int64;  
typedef unsigned __int64 uint64;  
#define INT64_VAL(x) x##i64
#define UINT64_VAL(x) x##ui64
#else
typedef long long int64;
typedef unsigned long long uint64;
#define INT64_VAL(x) x##ll
#define UINT64_VAL(x) x##ull
#endif

using namespace SCIRun;

class MapFieldDataOntoElemsPAlgo
{
  public:
    MapFieldDataOntoElemsPAlgo() :
      barrier_("MapFieldDataOntoElemsPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);

    FieldHandle sfield_;
    FieldHandle wfield_;
    FieldHandle ofield_;
    
    AlgoBase * algo_;
    
    bool is_flux_;
    bool has_nan_;
    
    double def_value_;
    
    std::vector<bool> success_;
  
  private:
    Barrier  barrier_;
    
    inline bool exist(double d)
    {
      uint64* n = reinterpret_cast<uint64 *>(&d);
      if ( (*n & UINT64_VAL(0x7ff0000000000000)) ==
           UINT64_VAL(0x7ff0000000000000)) return (false); else return (true);
    }

    inline bool exist(Vector& d)
    {
      return (exist(d.x())&&exist(d.y())&&exist(d.z()));
    }

    inline bool exist(Tensor& d)
    {
      return (exist(d.xx())&&exist(d.xy())&&exist(d.xz())&&
              exist(d.yy())&&exist(d.yz())&&exist(d.zz()));
    }
    
};

void
MapFieldDataOntoElemsPAlgo::parallel(int proc, int nproc)
{
  // Each thread has its own Datasource class, so it can preallocate array internally
  MappingDataSourceHandle datasource;
  success_[proc] = true;

  barrier_.wait(nproc);
  
  if(!(CreateDataSource(datasource,sfield_,wfield_,algo_))) 
  {
    success_[proc] = false;
  }

  barrier_.wait(nproc);
  
  for (int j=0; j<nproc; j++)
  {
    if (success_[j] == false)
    {
      if (proc == 0) algo_->error("Could not allocate data source.");
      return;
    }
  }

  barrier_.wait(nproc);
  
  def_value_ = algo_->get_scalar("outside_value");
  
  VMesh* omesh = ofield_->vmesh();
  VField* ofield = ofield_->vfield();
  
  VMesh::Elem::size_type  num_elems = omesh->num_elems();
  VField::size_type       localsize = num_elems/nproc;
  VField::index_type      start = localsize*proc;
  VField::index_type      end = localsize*(proc+1);
  if (proc == nproc-1) end = num_elems;

  std::vector<VMesh::coords_type> coords; 
  std::vector<double> weights;
  std::vector<Point> points;
              
  std::string sample_points = algo_->get_option("sample_points");            
  std::string sample_method = algo_->get_option("sample_method");            
              
  if (sample_points == "regular1")
  {
    omesh->get_regular_scheme(coords,weights,1);
  }
  else if (sample_points == "regular2")
  {
    omesh->get_regular_scheme(coords,weights,2);
  }
  else if (sample_points == "regular3")
  {
    omesh->get_regular_scheme(coords,weights,3);
  }
  else if (sample_points == "regular4")
  {
    omesh->get_regular_scheme(coords,weights,4);
  }
  else if (sample_points == "regular5")
  {
    omesh->get_regular_scheme(coords,weights,5);
  }
  else if (sample_points == "gaussian1")
  {
    omesh->get_gaussian_scheme(coords,weights,1);
  }
  else if (sample_points == "gaussian2")
  {
    omesh->get_gaussian_scheme(coords,weights,2);
  }
  else if (sample_points == "gaussian3")
  {
    omesh->get_gaussian_scheme(coords,weights,3);
  }
  else
  {
    if (proc == 0) algo_->error("Sampling points are not defined for this type of mesh");
    success_[proc] = false;
    barrier_.wait(nproc); 
    return;     
  }
               
  int cnt = 0;
  
  if (has_nan_)
  {
    if (is_flux_)
    {
      // To compute flux through a surface
      std::vector<Vector> norms;
      std::vector<Vector> grads;
      std::vector<double> values;
      double val; double rval; double vol;
      values.resize(coords.size());
      if (sample_method== "average")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = 0.0; size_t num = 0;
          for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]; num++; }
          if (num > 0) val /= num; else val = def_value_;
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else if (sample_method== "integrate")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          vol = omesh->get_size(idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = 0.0; size_t num = 0;
          for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]*weights[j]; num++; }
          if (num > 0) val *= vol; else val = def_value_;
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }      
      }
      else if (sample_method== "min")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = DBL_MAX;
          for (size_t j=0; j<values.size(); j++) if (exist(values[j])) if (values[j] < val) val = values[j];
          if (val == DBL_MAX) val = def_value_; 
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }      
      }    
      else if (sample_method== "max")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = -DBL_MAX;
          for (size_t j=0; j<values.size(); j++) if (exist(values[j])) if (values[j] > val) val = values[j];
          if (val == -DBL_MAX) val = def_value_; 
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }      
      }
      else if (sample_method== "sum")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = 0.0; size_t num = 0;
          for (size_t j=0; j<values.size(); j++) if(exist(val)) { val += values[j]; num++; }
          if (num == 0) val = def_value_;
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else if (sample_method== "mostcommon")
      {
        std::vector<double> common;
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          common.clear();
          for (size_t j=0; j<values.size();j++) if (exist(values[j])) common.push_back(values[j]);
          if (common.size() > 0)
          {
            sort(common.begin(),common.end());
            val =0.0; rval =0.0;
            size_t rnum =0; size_t p = 0; size_t n = 0;
            while (p <common.size())
            {
              n = 1;
              val = common[p];
              p++;
              while(p <common.size() && values[p] == val) {n++; p++;}
              if (n >= rnum) {rnum = n; rval = val;}
            }
          }
          else
          {
            rval = def_value_;
          }
          ofield->set_value(rval,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else if (sample_method== "median")
      {
        std::vector<double> median;      
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          median.clear();
          for (size_t j=0; j<values.size();j++) if (exist(values[j])) median.push_back(values[j]);
          if (median.size() > 0)
          {
            sort(median.begin(),median.end());
            int x = static_cast<index_type>(median.size()/2);
            if (median.size()%2) val = (median[x]+median[x+1])*0.5;
            else val = median[x];
          }
          else
          {
            val = def_value_;
          }
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else
      {
        if (proc == 0) algo_->error("Sampling method was not defined for flux data");
        success_[proc] = false;
        barrier_.wait(nproc); 
        return;       
      }
    }
    else
    {
      // To map value, gradient, or gradientnorm
      if (datasource->is_scalar())
      {
        std::vector<double> values;
        double val; double rval; double vol;
        if (sample_method== "average")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = 0.0; size_t num = 0;
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]; num++; }
            if (num > 0) val = val/num; else val = def_value_;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "integrate")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            vol = omesh->get_size(idx);
            datasource->get_data(values,points);
            val = 0.0; size_t num = 0;
            for (size_t j=0; j<values.size(); j++) if (exist(val)) { val += values[j]*weights[j]; num++; }
            if (num > 0) val *= vol;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "min")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = DBL_MAX;
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { if (values[j] < val) val = values[j]; }
            if (val == DBL_MAX) val = def_value_;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }    
        else if (sample_method== "max")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = -DBL_MAX;
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { if (values[j] > val) val = values[j]; }
            if (val == -DBL_MAX) val = def_value_;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "sum")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = 0.0; size_t num = 0;
            for (size_t j=0; j<values.size(); j++) { if (exist(values[j])) { val += values[j]; num++;} }
            if (num == 0) val = def_value_;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "mostcommon")
        { 
          std::vector<double> common;
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            common.clear();
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { common.push_back(values[j]); }
            if (common.size() > 0)
            {
              sort(common.begin(),common.end());
              val =0.0; rval =0.0;
              size_t rnum =0; size_t p = 0; size_t n = 0;
              while (p <common.size())
              {
                n = 1;
                val = common[p];
                p++;
                while(p <common.size() && common[p] == val) {n++; p++;}
                if (n >= rnum) {rnum = n; rval = val;}
              }
            }
            else
            {
              rval = def_value_;
            }
            ofield->set_value(rval,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "median")
        {
          std::vector<double> median;
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            median.clear();
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { median.push_back(values[j]); }
            if (median.size())
            {
              sort(median.begin(),median.end());
              int x = static_cast<index_type>(median.size()/2);
              if (median.size()%2) val = (median[x]+median[x+1])*0.5;
              else val = median[x];
            }
            else
            {
              val = def_value_;
            }
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else
        {
          if (proc == 0) algo_->error("Sampling method was not defined for this type of data");
          success_[proc] = false;
          barrier_.wait(nproc); 
          return;       
        }
      }
      else if (datasource->is_vector())
      {
        std::vector<Vector> values;
        Vector val; double vol;
        if (sample_method== "average")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = Vector(0,0,0); size_t num = 0;
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]; num++; }
            if (num > 0) val = val*(1.0/num); else val = def_value_;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "integrate")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            vol = omesh->get_size(idx);
            datasource->get_data(values,points);
            val = Vector(0,0,0); 
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]*weights[j]; }
            val *= vol;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "sum")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = Vector(0,0,0);
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]; }
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else
        {
          if (proc == 0) algo_->error("Sampling method was not defined for this type of data");
          success_[proc] = false;
          barrier_.wait(nproc); 
          return;       
        }    
      }
      else
      {
        std::vector<Tensor> values;
        Tensor val; double vol;
        if (sample_method== "average")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = Tensor(0); size_t num = 0;
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]; num++; }
            if (num > 0) val = val*(1.0/num);
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "integrate")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            vol = omesh->get_size(idx);
            datasource->get_data(values,points);
            val = Tensor(0);
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += weights[j]*values[j]; }
            val = val*vol;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "sum")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = Tensor(0);
            for (size_t j=0; j<values.size(); j++) if (exist(values[j])) { val += values[j]; }
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else
        {
          if (proc == 0) algo_->error("Sampling method was not defined for this type of data");
          success_[proc] = false;
          barrier_.wait(nproc); 
          return;       
        }    
      }  
    }
  }
  else
  {
    if (is_flux_)
    {
      // To compute flux through a surface
      std::vector<Vector> norms;
      std::vector<Vector> grads;
      std::vector<double> values;
      double val; double rval; double vol;
      double scale = 1.0/coords.size();
      values.resize(coords.size());
      if (sample_method== "average")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = values[0];
          for (size_t j=1; j<values.size(); j++) val += values[j];
          val *= scale;
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else if (sample_method== "integrate")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          vol = omesh->get_size(idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = values[0];
          for (size_t j=1; j<values.size(); j++) val += values[j]*weights[j];
          val *= vol;
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }      
      }
      else if (sample_method== "min")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = values[0];
          for (size_t j=1; j<values.size(); j++) if (values[j] < val) val = values[j];
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }      
      }    
      else if (sample_method== "max")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = values[0];
          for (size_t j=1; j<values.size(); j++) if (values[j] > val) val = values[j];
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }      
      }
      else if (sample_method== "sum")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          val = values[0];
          for (size_t j=1; j<values.size(); j++) val += values[j];
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else if (sample_method== "mostcommon")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          sort(values.begin(),values.end());
          val =0.0; rval =0.0;
          size_t rnum =0; size_t p = 0; size_t n = 0;
          while (p <values.size())
          {
            n = 1;
            val = values[p];
            p++;
            while(p <values.size() && values[p] == val) {n++; p++;}
            if (n >= rnum) {rnum = n; rval = val;}
          }

          ofield->set_value(rval,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else if (sample_method== "median")
      {
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          omesh->minterpolate(points,coords,idx);
          omesh->get_normals(norms,coords,idx);
          datasource->get_data(grads,points);
          for (size_t j=0; j<grads.size();j++) values[j] = Dot(grads[j],norms[j]);
          sort(values.begin(),values.end());
          int x = static_cast<index_type>(values.size()/2);
          if (values.size()%2) val = (values[x]+values[x+1])*0.5;
          else val = values[x];
          ofield->set_value(val,idx);
          if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
        }
      }
      else
      {
        if (proc == 0) algo_->error("Sampling method was not defined for flux data");
        success_[proc] = false;
        barrier_.wait(nproc); 
        return;       
      }
    }
    else
    {
      // To map value, gradient, or gradientnorm
      if (datasource->is_scalar())
      {
        std::vector<double> values;
        double val; double rval; double vol;
        double scale = 1.0/coords.size();
        if (sample_method== "average")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j];
            val *= scale;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "integrate")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            vol = omesh->get_size(idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j]*weights[j];
            val *= vol;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "min")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) if (values[j] < val) val = values[j];
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }    
        else if (sample_method== "max")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) if (values[j] > val) val = values[j];
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "sum")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j];
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "mostcommon")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            sort(values.begin(),values.end());
            val =0.0; rval =0.0;
            size_t rnum =0; size_t p = 0; size_t n = 0;
            while (p <values.size())
            {
              n = 1;
              val = values[p];
              p++;
              while(p <values.size() && values[p] == val) {n++; p++;}
              if (n >= rnum) {rnum = n; rval = val;}
            }

            ofield->set_value(rval,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "median")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            sort(values.begin(),values.end());
            int x = static_cast<index_type>(values.size()/2);
            if (values.size()%2) val = (values[x]+values[x+1])*0.5;
            else val = values[x];
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else
        {
          if (proc == 0) algo_->error("Sampling method was not defined for this type of data");
          success_[proc] = false;
          barrier_.wait(nproc); 
          return;       
        }
      }
      else if (datasource->is_vector())
      {
        std::vector<Vector> values;
        Vector val; double vol;
        double scale = 1.0/coords.size();
        if (sample_method== "average")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j];
            val *= scale;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "integrate")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            vol = omesh->get_size(idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j]*weights[j];
            val *= vol;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "sum")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j];
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else
        {
          if (proc == 0) algo_->error("Sampling method was not defined for this type of data");
          success_[proc] = false;
          barrier_.wait(nproc); 
          return;       
        }    
      }
      else
      {
        std::vector<Tensor> values;
        Tensor val; double vol;
        double scale = 1.0/coords.size();
        if (sample_method== "average")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j];
            val = val* scale;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else if (sample_method== "integrate")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            vol = omesh->get_size(idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j]*weights[j];
            val = val*vol;
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }      
        }
        else if (sample_method== "sum")
        {
          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            omesh->minterpolate(points,coords,idx);
            datasource->get_data(values,points);
            val = values[0];
            for (size_t j=1; j<values.size(); j++) val += values[j];
            ofield->set_value(val,idx);
            if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress(idx,end); } }
          }
        }
        else
        {
          if (proc == 0) algo_->error("Sampling method was not defined for this type of data");
          success_[proc] = false;
          barrier_.wait(nproc); 
          return;       
        }    
      }  
    }
  }
  
  // Wait until all of the threads are done
  success_[proc] = true;
  barrier_.wait(nproc);
}


bool
MapFieldDataOntoElemsAlgo::
run(FieldHandle source, FieldHandle weights,
    FieldHandle destination, FieldHandle& output)
{
  algo_start("MapFieldDataOntoElems");
  
  if (source.get_rep() == 0)
  {
    error("No source field");
    algo_end(); return (false);
  }

  if (destination.get_rep() == 0)
  {
    error("No destination field");
    algo_end(); return (false);
  }

  FieldInformation fi(source);
  FieldInformation fo(destination);
  fo.make_constantdata();

  std::string quantity = get_option("quantity");
  std::string value = get_option("value");
  
  if (value == "closestnodedata")
  {
    if (!fi.is_lineardata())
    {
      error("Closest node data only works for source data located at the nodes.");
      algo_end(); return (false);      
    }
  }

  if (fi.is_nodata())
  {
    error("No data in source field.");
    algo_end(); return (false);       
  }

  if (weights.get_rep())
  {
    FieldInformation wfi(weights);
    if (value == "closestnodedata")
    {
      if (!wfi.is_lineardata())
      {
        error("Closest node data only works for weights data located at the nodes.");
        algo_end(); return (false);      
      }
    }
    
    if (wfi.is_nodata())
    {
      error("No data in weights field.");
      algo_end(); return (false);       
    }
  }
  
  // Make sure output equals quantity to be computed

  if (quantity == "value")
  {
    // Copy the output datatype
    fo.set_data_type(fi.get_data_type());
  }
  else  if (quantity == "gradient")
  {
    // Output will be a vector
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      algo_end(); return (false);
    }
    fo.make_vector();
  }
  else if (quantity == "gradientnorm")
  {
    // Output will be a double
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      algo_end(); return (false);
    }
    fo.make_double();
  }
  else if (quantity == "flux")
  {
    // Only for output surfaces (only field with normals) and output will
    // be double
    if (!fi.is_scalar())
    {
      error("Flux can only be calculated on a scalar field.");
      algo_end(); return (false);
    }
    if (!fo.is_surface())
    {
      error("Flux can only be computed for surfaces meshes as destination");
      algo_end(); return (false);
    }
    fo.make_double();
  }

  // Incorporate the weights and alter the datatype to reflect that
  if (weights.get_rep())
  {
    FieldInformation wfi(weights);
    if ((!wfi.is_tensor())&&(!wfi.is_scalar()))
    {
      error("Weights field needs to be a scalar or a tensor.");
      algo_end(); return (false);
    }
  
    if (fo.is_scalar() && wfi.is_tensor()) 
    {
      fo.make_tensor();
    }
    
    if (fo.is_tensor() && wfi.is_tensor())
    {
      error("Weights and source field cannot be both tensor data.");
      algo_end(); return (false);
    }
  }

  // Create new output field
  output = CreateField(fo,destination->mesh());
  output->vfield()->resize_values();
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  // Run algorithm in parallel
  MapFieldDataOntoElemsPAlgo algo;  
  
  algo.sfield_ = source;
  algo.wfield_ = weights;
  algo.ofield_ = output;
  algo.algo_ = this;
    
  // Number of threads is equal to the number of cores
  int np = Thread::numProcessors();
  algo.success_.resize(np,true);
  // Mark whether it is a flux computation
  algo.is_flux_ = false;
  if (quantity == "flux") algo.is_flux_ = true;

  algo.has_nan_ = false;
  std::string valuestr = get_option("value");
  if (valuestr == "interpolateddataonly") algo.has_nan_ = true;

  // Parallel algorithm
  Thread::parallel(&algo,&MapFieldDataOntoElemsPAlgo::parallel,np,np);

 // Check whether algorithm succeeded
  for (int j=0; j<np; j++)
  {
    if (algo.success_[j] == false)
    {
      // We get herer if sample_method is incorrect
      algo_end(); return (false);
    }
  }
  // Copy properties
  output->vfield()->copy_properties(destination->vfield());
  
  algo_end(); return (true);
}




bool
MapFieldDataOntoElemsAlgo::
run(FieldHandle source, FieldHandle destination, FieldHandle& output)
{
  algo_start("MapFieldDataOntoElems");
  
  if (source.get_rep() == 0)
  {
    error("No source field");
    algo_end(); return (false);
  }

  if (destination.get_rep() == 0)
  {
    error("No destination field");
    algo_end(); return (false);
  }

  FieldInformation fi(source);
  FieldInformation fo(destination);
  fo.make_constantdata();

  std::string quantity = get_option("quantity");
  std::string value = get_option("value");
  
  if (value == "closestnodedata")
  {
    if (!fi.is_lineardata())
    {
      error("Closest node data only works for source data located at the nodes.");
      algo_end(); return (false);      
    }
  }

  if (fi.is_nodata())
  {
    error("No data in source field.");
    algo_end(); return (false);       
  }
  
  // Make sure output equals quantity to be computed

  if (quantity == "value")
  {
    // Copy the output datatype
    fo.set_data_type(fi.get_data_type());
  }
  else  if (quantity == "gradient")
  {
    // Output will be a vector
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      algo_end(); return (false);
    }
    fo.make_vector();
  }
  else if (quantity == "gradientnorm")
  {
    // Output will be a double
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      algo_end(); return (false);
    }
    fo.make_double();
  }
  else if (quantity == "flux")
  {
    // Only for output surfaces (only field with normals) and output will
    // be double
    if (!fi.is_scalar())
    {
      error("Flux can only be calculated on a scalar field.");
      algo_end(); return (false);
    }
    if (!fo.is_surface())
    {
      error("Flux can only be computed for surfaces meshes as destination");
      algo_end(); return (false);
    }
    fo.make_double();
  }

  // Create new output field
  output = CreateField(fo,destination->mesh());
  output->vfield()->resize_values();
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  // Run algorithm in parallel
  MapFieldDataOntoElemsPAlgo algo;  
  
  algo.sfield_ = source;
  algo.ofield_ = output;
  algo.algo_ = this;
    
  // Number of threads is equal to the number of cores
  int np = Thread::numProcessors();
  algo.success_.resize(np,true);
  // Mark whether it is a flux computation
  algo.is_flux_ = false;
  if (quantity == "flux") algo.is_flux_ = true;

  algo.has_nan_ = false;
  std::string valuestr = get_option("value");
  if (valuestr == "interpolateddataonly") algo.has_nan_ = true;

  // Parallel algorithm
  Thread::parallel(&algo,&MapFieldDataOntoElemsPAlgo::parallel,np,np);

 // Check whether algorithm succeeded
  for (int j=0; j<np; j++)
  {
    if (algo.success_[j] == false)
    {
      // We get herer if sample_method is incorrect
      algo_end(); return (false);
    }
  }
  // Copy properties
  output->vfield()->copy_properties(destination->vfield());
  
  algo_end(); return (true);
}



} // end namespace SCIRunAlgo
