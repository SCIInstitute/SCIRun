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


/// @todo Documentation Core/Datatypes/Legacy/Field/VFDataT.h

#include <Core/Datatypes/Legacy/Field/VFData.h>

#include <Core/Exceptions/AssertionFailed.h>
#include <sci_debug.h>

#ifdef DEBUG
#define TESTRANGE(c, l, h) \
   if(c < static_cast<index_type>(l) || c >= static_cast<index_type>(h)){ \
      std::ostringstream msg; \
      msg << #l "(value=" << l << ") <= " #c << "(value=" << c << ") < " << #h << "(value=" << h << ")"; \
      throw(SCIRun::AssertionFailed(msg.str(), __FILE__, __LINE__)); \
   }
#else
#define TESTRANGE(c, l, h)
#endif


#define VFDATAT_ACCESS_DEFINITION(type) \
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_value(type &val, VMesh::index_type idx) const \
{ \
  TESTRANGE(idx,0,fdata_.size())\
  val = CastFData<type>(fdata_[idx]); \
} \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_value(const type &val, VMesh::index_type idx) \
{ \
  TESTRANGE(idx,0,fdata_.size())\
  fdata_[idx] =  CastFData<typename FDATA::value_type>(val); \
} \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_evalue(type &val, VMesh::index_type idx) const \
{\
 TESTRANGE(idx,0,efdata_.size())\
 val = CastFData<type>(efdata_[idx]);\
} \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_evalue(const type &val, VMesh::index_type idx) \
{\
  TESTRANGE(idx,0,efdata_.size()) \
 efdata_[idx] =  CastFData<typename FDATA::value_type>(val);\
} \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_values(type *ptr, VMesh::size_type sz, VMesh::size_type offset) const \
{ if (static_cast<size_type>(fdata_.size()) < sz+offset) sz = static_cast<size_type>(fdata_.size())-offset; for (size_type i=0; i< sz; i++) ptr[i] = CastFData<type>(fdata_[i+offset]); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_values(const type *ptr, VMesh::size_type sz, VMesh::size_type offset) \
{ if (static_cast<size_type>(fdata_.size()) < sz+offset) sz = static_cast<size_type>(fdata_.size())-offset; for (size_type i=0; i< sz; i++) fdata_[i+offset] =  CastFData<typename FDATA::value_type>(ptr[i]); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_evalues(type *ptr, VMesh::size_type sz, VMesh::size_type offset) const \
{ if (static_cast<size_type>(efdata_.size()) < sz+offset) sz = static_cast<size_type>(efdata_.size())-offset; for (size_type i=0; i< sz; i++) ptr[i] = CastFData<type>(efdata_[i+offset]); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_evalues(const type *ptr, VMesh::size_type sz, VMesh::size_type offset) \
{ if (static_cast<size_type>(efdata_.size()) < sz+offset) sz = static_cast<size_type>(efdata_.size())-offset; for (size_type i=0; i< sz; i++) efdata_[i+offset] =  CastFData<typename FDATA::value_type>(ptr[i]); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_all_values(const type &val) \
{ typename FDATA::value_type tval =  CastFData<typename FDATA::value_type>(val); \
  size_type sz1 = static_cast<size_type>(fdata_.size()); \
  size_type sz2 = static_cast<size_type>(efdata_.size()); \
  for (size_type i=0; i < sz1; i++) fdata_[i] = tval; \
  for (size_type i=0; i < sz2; i++) efdata_[i] = tval; \
} \
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_weighted_value(type &val, const VMesh::index_type* idx, const VMesh::weight_type* w, VMesh::size_type sz ) const \
{ typename FDATA::value_type tval = typename FDATA::value_type(0); for(size_type i=0; i<sz; i++) { TESTRANGE(idx[i],0,fdata_.size()) tval = tval + static_cast<typename FDATA::value_type>(w[i]*fdata_[idx[i]]); } val = CastFData<type>(tval); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_weighted_evalue(type &val, const VMesh::index_type* idx, const VMesh::weight_type* w, VMesh::size_type sz ) const \
{ typename EFDATA::value_type tval = typename EFDATA::value_type(0); for(size_type i=0; i<sz; i++) { TESTRANGE(idx[i],0,efdata_.size()) tval = tval + static_cast<typename EFDATA::value_type>(w[i]*efdata_[idx[i]]); } val = CastFData<type>(tval); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_values(type *ptr, VMesh::Node::array_type& nodes) const \
{ for(size_t j=0; j<nodes.size(); j++) { TESTRANGE(nodes[j],0,fdata_.size()) ptr[j] = CastFData<type>(fdata_[nodes[j]]); } } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_values(type *ptr, VMesh::Elem::array_type& elems) const\
{ for(size_t j=0; j<elems.size(); j++) { TESTRANGE(elems[j],0,fdata_.size()) ptr[j] = CastFData<type>(fdata_[elems[j]]); } } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_values(const type *ptr, VMesh::Node::array_type& nodes) \
{ for(size_t j=0; j<nodes.size(); j++) { TESTRANGE(nodes[j],0,fdata_.size())  fdata_[nodes[j]] = CastFData<typename FDATA::value_type>(ptr[j]); } } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_values(const type *ptr, VMesh::Elem::array_type& elems) \
{ for(size_t j=0; j<elems.size(); j++) { TESTRANGE(elems[j],0,fdata_.size()) fdata_[elems[j]] = CastFData<typename FDATA::value_type>(ptr[j]);} } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::get_values(type *ptr, index_type* idx, size_type size) const\
{ for(index_type j=0; j<size; j++) { TESTRANGE(idx[j],0,fdata_.size()) ptr[j] = CastFData<type>(fdata_[idx[j]]); } } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::set_values(const type *ptr, index_type* idx, size_type size) \
{ for(index_type j=0; j<size; j++) { TESTRANGE(idx[j],0,fdata_.size()) fdata_[idx[j]] = CastFData<typename FDATA::value_type>(ptr[j]);} } \


#define VFDATAT_ACCESS_DEFINITION2(type) \
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::interpolate(type &val, VMesh::ElemInterpolate &interp, type defval) const \
{ interpolateT<type>(val,interp,defval); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::minterpolate(std::vector<type> &vals, VMesh::MultiElemInterpolate &interp, type defval) const \
{ minterpolateT<type>(vals,interp,defval); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::gradient(StackVector<type,3> &val, VMesh::ElemGradient &interp, type defval) const \
{ gradientT<type>(val,interp,defval); } \
\
template<class FDATA, class EFDATA, class HFDATA> \
void VFDataT<FDATA,EFDATA,HFDATA>::mgradient(std::vector<StackVector<type,3> > &vals, VMesh::MultiElemGradient &interp, type defval) const \
{ mgradientT<type>(vals,interp,defval); }

#define VFDATA_FUNCTION_SCALAR_DEFINITION(type) \
VFData* CreateVFData(std::vector<type>& fdata,std::vector<type>& efdata,std::vector<std::vector<type> >& hfdata) \
{ return new VFDataScalarT<std::vector<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); } \
\
VFData* CreateVFData(Array2<type>& fdata,std::vector<type>& efdata,std::vector<std::vector<type> >& hfdata) \
{ return new VFDataScalarT<Array2<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); } \
\
VFData* CreateVFData(Array3<type>& fdata,std::vector<type>& efdata, std::vector<std::vector<type> >& hfdata) \
{ return new VFDataScalarT<Array3<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); }

#define VFDATA_FUNCTION_VECTOR_DEFINITION(type) \
VFData* CreateVFData(std::vector<type>& fdata,std::vector<type>& efdata,std::vector<std::vector<type> >& hfdata) \
{ return new VFDataVectorT<std::vector<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); } \
\
VFData* CreateVFData(Array2<type>& fdata,std::vector<type>& efdata,std::vector<std::vector<type> >& hfdata) \
{ return new VFDataVectorT<Array2<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); } \
\
VFData* CreateVFData(Array3<type>& fdata,std::vector<type>& efdata, std::vector<std::vector<type> >& hfdata) \
{ return new VFDataVectorT<Array3<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); }

#define VFDATA_FUNCTION_TENSOR_DEFINITION(type) \
VFData* CreateVFData(std::vector<type>& fdata,std::vector<type>& efdata,std::vector<std::vector<type> >& hfdata) \
{ return new VFDataTensorT<std::vector<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); } \
\
VFData* CreateVFData(Array2<type>& fdata,std::vector<type>& efdata,std::vector<std::vector<type> >& hfdata) \
{ return new VFDataTensorT<Array2<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); } \
\
VFData* CreateVFData(Array3<type>& fdata,std::vector<type>& efdata, std::vector<std::vector<type> >& hfdata) \
{ return new VFDataTensorT<Array3<type>,std::vector<type>,std::vector<std::vector<type> > >(fdata,efdata,hfdata); }


namespace SCIRun {

/// Implementation layer of the functions we actually need. These classes are
/// defined here to prevent overload of the header files:

template<class FDATA, class EFDATA, class HFDATA>
class VFDataT : public VFData {

private:
  template<class T>
  void resize(std::vector<T>& fdata,VMesh::dimension_type dim)
  {
    VMesh::size_type sz = 1;
    if (dim.size() > 0) sz = dim[0];
    if (dim.size() > 1) sz *= dim[1];
    if (dim.size() > 2) sz *= dim[2];
    fdata.resize(sz);
  }

  template<class T>
  void resize(Array2<T>& fdata,VMesh::dimension_type dim)
  {
    VMesh::size_type sz1 = 1;
    VMesh::size_type sz2 = 1;
    if (dim.size() > 0) sz1 = dim[0];
    if (dim.size() > 1) sz2 = dim[1];
    fdata.resize(sz2,sz1);
  }

  template<class T>
  void resize(Array3<T>& fdata,VMesh::dimension_type dim)
  {
    VMesh::size_type sz1 = 1;
    VMesh::size_type sz2 = 1;
    VMesh::size_type sz3 = 1;
    if (dim.size() > 0) sz1 = dim[0];
    if (dim.size() > 1) sz2 = dim[1];
    if (dim.size() > 2) sz3 = dim[2];
    fdata.resize(sz3,sz2,sz1);
  }

public:
  // constructor
  VFDataT(FDATA& fdata, EFDATA& efdata, HFDATA& hfdata) :
  fdata_(fdata), efdata_(efdata), hfdata_(hfdata)
  { }

  // destructor
  virtual ~VFDataT() {}


  virtual void resize_fdata(VMesh::dimension_type dim)
  {
    resize(fdata_,dim);
  }

  virtual void resize_efdata(VMesh::dimension_type dim)
  {
    resize(efdata_,dim);
  }

  virtual VMesh::size_type fdata_size() const
    { return (fdata_.size()); }
  virtual VMesh::size_type efdata_size() const
    { return (efdata_.size()); }

  virtual void* fdata_pointer() const
    {
      if (fdata_.size() == 0) return (0);
      return (&(fdata_[0]));
    }
  virtual void* efdata_pointer() const
    {
      if (efdata_.size() == 0) return (0);
      return (&(efdata_[0]));
    }

  VFDATA_ACCESS_DECLARATION(char)
  VFDATA_ACCESS_DECLARATION(unsigned char)
  VFDATA_ACCESS_DECLARATION(short)
  VFDATA_ACCESS_DECLARATION(unsigned short)
  VFDATA_ACCESS_DECLARATION(int)
  VFDATA_ACCESS_DECLARATION(unsigned int)
  VFDATA_ACCESS_DECLARATION(long long)
  VFDATA_ACCESS_DECLARATION(unsigned long long)
  VFDATA_ACCESS_DECLARATION(float)
  VFDATA_ACCESS_DECLARATION(double)
  VFDATA_ACCESS_DECLARATION(std::complex<double>)
  VFDATA_ACCESS_DECLARATION(Core::Geometry::Vector)
  VFDATA_ACCESS_DECLARATION(Core::Geometry::Tensor)


  VFDATA_ACCESS_DECLARATION2(int)
  VFDATA_ACCESS_DECLARATION2(float)
  VFDATA_ACCESS_DECLARATION2(double)
  VFDATA_ACCESS_DECLARATION2(Core::Geometry::Vector)
  VFDATA_ACCESS_DECLARATION2(Core::Geometry::Tensor)

  virtual void copy_value(VFData* fdata,
                          VMesh::index_type vidx,
                          VMesh::index_type idx)
  {
    fdata->get_value(fdata_[idx],vidx);
  }

  virtual void copy_values(VFData* fdata,
                           VMesh::index_type vidx,
                           VMesh::index_type idx,
                           VMesh::size_type num)
  {
    fdata->get_values(&(fdata_[idx]),num,vidx);
  }


  virtual void copy_weighted_value(VFData* fdata,
                                   const VMesh::index_type* vidx,
                                   const VMesh::weight_type* vw,
                                   VMesh::size_type sz,
                                   VMesh::index_type idx)
  {
    fdata->get_weighted_value(fdata_[idx],vidx,vw,sz);
  }

  virtual void copy_evalue(VFData* fdata,
                           VMesh::index_type vidx,
                           VMesh::index_type idx)
  {
    typename EFDATA::value_type val;
    fdata->get_value(val,vidx);
    efdata_[idx] = val;
  }

  virtual void copy_evalues(VFData* fdata,
                            VMesh::index_type vidx,
                            VMesh::index_type idx,
                            VMesh::size_type num)
  {
    fdata->get_evalues(&(fdata_[idx]),num,vidx);
  }


  virtual void copy_weighted_evalue(VFData* fdata,
                                    const VMesh::index_type* vidx,
                                    const VMesh::weight_type* vw,
                                    VMesh::size_type sz,
                                    VMesh::index_type idx)
  {
    typename FDATA::value_type val;
    fdata->get_weighted_evalue(val,vidx,vw,sz);
    efdata_[idx] = val;
  }

  virtual void copy_values(VFData* fdata)
  {
    VMesh::size_type sz1 = fdata_.size();
    VMesh::size_type sz2 = fdata->fdata_size();
    if (sz1 == 0) return;
    if (sz2 < sz1) sz1 = sz2;
    fdata->get_values(&(fdata_[0]),sz1,0);
  }

  virtual void copy_evalues(VFData* fdata)
  {
    VMesh::size_type sz1 = efdata_.size();
    VMesh::size_type sz2 = fdata->efdata_size();
    if (sz2 < sz1) sz1 = sz2;
    fdata->get_evalues(&(efdata_[0]),sz1,0);
  }

  template<class T>
  inline void interpolateT(T& val, VMesh::ElemInterpolate& ei, T defval) const
  {
    switch(ei.basis_order)
    {
      case 0:
      {
        if (ei.elem_index >= 0)
        {
          TESTRANGE(ei.elem_index,0,fdata_.size())
          val = CastFData<T>(fdata_[ei.elem_index]);
        }
        else
          val = defval;
        return;
      }
      case 1:
      {
        if (ei.elem_index >= 0)
        {
          val = static_cast<T>(0.0);
          for (size_t p=0;p<ei.node_index.size(); p++)
          {
            TESTRANGE(ei.node_index[p],0,fdata_.size())
            val += CastFData<T>(fdata_[ei.node_index[p]]*ei.weights[p]);
          }
        }
        else
        {
          val = defval;
        }
        return;
      }
      case 2:
      {
        if (ei.elem_index >= 0)
        {
          index_type k = 0;
          val = static_cast<T>(0.0);
          for (size_t p=0;p<ei.node_index.size(); p++)
          {
            TESTRANGE(ei.node_index[p],0,fdata_.size())
            val += CastFData<T>(fdata_[ei.node_index[p]]*ei.weights[k]); k++;
          }
          for (size_t p=0;p<ei.edge_index.size(); p++)
          {
            TESTRANGE(ei.edge_index[p],0,efdata_.size())
            val += CastFData<T>(efdata_[ei.edge_index[p]]*ei.weights[k]); k++;
          }
        }
        else
        {
          val =  defval;
        }
        return;
      }
      case 3:
      {
        if (ei.elem_index >= 0)
        {
          val = static_cast<T>(0.0);
          index_type k = 0;
          for (size_t p=0;p<ei.node_index.size();p++)
          {
            TESTRANGE(ei.node_index[p],0,fdata_.size())
            val += CastFData<T>(fdata_[ei.node_index[p]]*ei.weights[k]); k++;
            for (index_type q=0;q<ei.num_hderivs;q++)
            {
              TESTRANGE(ei.node_index[p],0,hfdata_.size())
              val += CastFData<T>(hfdata_[ei.node_index[p]][q]*ei.weights[k]); k++;
            }
          }
        }
        else
        {
          val = defval;
        }
        return;
      }
    }
    ASSERTFAIL("Interpolate encountered an unknown basis_order");
  }

  template<class T>
  inline void minterpolateT(std::vector<T>& vals, VMesh::MultiElemInterpolate& ei, T defval) const
  {
    vals.resize(ei.size());

    if (ei.size() == 0) return;

    switch(ei[0].basis_order)
    {
      case 0:
      {
        for (size_t j=0; j<ei.size(); j++)
          if (ei[j].elem_index >= 0)
            vals[j] = CastFData<T>(fdata_[ei[j].elem_index]);
          else
            vals[j] = defval;
        return;
      }
      case 1:
      {
        for (size_t j=0; j<ei.size(); j++)
        {
          if (ei[j].elem_index >= 0)
          {
            vals[j] = static_cast<T>(0.0);
            for (size_t p=0;p<ei[j].node_index.size(); p++)
              vals[j] += CastFData<T>(fdata_[ei[j].node_index[p]]*ei[j].weights[p]);
          }
          else
          {
            vals[j] = defval;
          }
        }
        return;
      }
      case 2:
      {
        for (size_t j=0; j<ei.size(); j++)
        {
          if (ei[j].elem_index >= 0)
          {
            vals[j] = static_cast<T>(0.0);
            index_type k = 0;
            for (size_t p=0;p<ei[j].node_index.size(); p++)
             { vals[j] += CastFData<T>(fdata_[ei[j].node_index[p]]*ei[j].weights[k]); k++; }
            for (size_t p=0;p<ei[j].edge_index.size(); p++)
             { vals[j] += CastFData<T>(efdata_[ei[j].edge_index[p]]*ei[j].weights[k]); k++; }
          }
          else
          {
            vals[j] = defval;
          }
        }
        return;
      }
      case 3:
      {
        for (size_t j=0; j<ei.size(); j++)
        {
          if (ei[j].elem_index >= 0)
          {
            index_type k = 0;
            vals[j] = static_cast<T>(0.0);
            for (size_t p=0;p<ei[j].node_index.size();p++)
            {
              vals[j] += CastFData<T>(fdata_[ei[j].node_index[p]]*ei[j].weights[k]); k++;
              for (index_type q=0;q<ei[j].num_hderivs;q++)
               { vals[j] += CastFData<T>(hfdata_[ei[j].node_index[p]][q]*ei[j].weights[k]); k++; }
            }
          }
          else
          {
            vals[j] = defval;
          }
        }
        return;
      }
    }
    ASSERTFAIL("Interpolate encountered an unknown basis_order");
  }


  template<class T>
  inline void mgradientT(std::vector<StackVector<T,3> >& vals, VMesh::MultiElemGradient& eg, T defval) const
  {
    vals.resize(eg.size());

    if (eg.size() == 0) return;
    T grad;

    switch(eg[0].basis_order)
    {
      case 0:
        for (size_t j=0; j<eg.size(); j++)
        {
          vals[j].resize(3);
          if (eg[j].elem_index >= 0)
          {
            vals[j][0] = T(0); vals[j][1] = T(0); vals[j][2] = T(0);
          }
          else
          {
            vals[j][0] = defval; vals[j][1] = defval; vals[j][2] = defval;
          }

        }
        return;
      case 1:
      {
        for (size_t j=0; j<eg.size(); j++)
        {
          vals[j].resize(3);
          vals[j][0] = T(0); vals[j][1] = T(0); vals[j][2] = T(0);
          if (eg[j].elem_index >= 0)
          {
            for (index_type k=0, q = 0; k<eg[j].num_derivs; k++)
            {
              grad = T(0);
              for (size_t p=0;p<eg[j].node_index.size();p++)
                { grad += CastFData<T>(fdata_[eg[j].node_index[p]]*eg[j].weights[q]); q++; }

              vals[j][0] += CastFData<T>(grad*eg[j].inverse_jacobian[k]);
              vals[j][1] += CastFData<T>(grad*eg[j].inverse_jacobian[k+3]);
              vals[j][2] += CastFData<T>(grad*eg[j].inverse_jacobian[k+6]);
            }
          }
          else
          {
            vals[j][0] = defval; vals[j][1] = defval; vals[j][2] = defval;
          }
        }
        return;
      }
      case 2:
      {
        for (size_t j=0; j<eg.size(); j++)
        {
          vals[j].resize(3);
          vals[j][0] = T(0); vals[j][1] = T(0); vals[j][2] = T(0);

          if (eg[j].elem_index >= 0)
          {
            index_type q = 0;
            for (index_type k=0; k<eg[j].num_derivs; k++)
            {
              grad = T(0);
              for (size_t p=0;p<eg[j].node_index.size();p++)
               { grad += CastFData<T>(fdata_[eg[j].node_index[p]]*eg[j].weights[q]); q++; }
              for (size_t p=0;p<eg[j].edge_index.size();p++)
               { grad += CastFData<T>(fdata_[eg[j].edge_index[p]]*eg[j].weights[q]); q++; }

              vals[j][0] += CastFData<T>(grad*eg[j].inverse_jacobian[k]);
              vals[j][1] += CastFData<T>(grad*eg[j].inverse_jacobian[k+3]);
              vals[j][2] += CastFData<T>(grad*eg[j].inverse_jacobian[k+6]);
            }
          }
          else
          {
            vals[j][0] = defval; vals[j][1] = defval; vals[j][2] = defval;
          }

        }
        return;
      }
      case 3:
      {
        for (size_t j=0; j<eg.size(); j++)
        {
          vals[j].resize(3);
          vals[j][0] = T(0); vals[j][1] = T(0); vals[j][2] = T(0);

          if (eg[j].elem_index >= 0)
          {
            index_type q = 0;
            for (index_type k=0; k<eg[j].num_derivs; k++)
            {
              grad = T(0);

              for (size_t p=0;p<eg[j].node_index.size();p++)
              {
                grad += CastFData<T>(fdata_[eg[j].node_index[p]]*eg[j].weights[q]); q++;
                for (index_type r=1;r<eg[j].num_hderivs;r++)
                  { grad += CastFData<T>(hfdata_[eg[j].node_index[p]][r]*eg[j].weights[q]); q++; }
              }

              vals[j][0] += CastFData<T>(grad*eg[j].inverse_jacobian[k]);
              vals[j][1] += CastFData<T>(grad*eg[j].inverse_jacobian[k+3]);
              vals[j][2] += CastFData<T>(grad*eg[j].inverse_jacobian[k+6]);
            }
          }
          else
          {
            vals[j][0] = defval; vals[j][1] = defval; vals[j][2] = defval;
          }

        }
        return;
      }
    }
    ASSERTFAIL("Gradient encountered an unknown basis_order");
  }

  template<class T>
  inline void gradientT(StackVector<T,3>& val, VMesh::ElemGradient& eg, T defval) const
  {
    T grad;

    switch(eg.basis_order)
    {
      case 0:
        val.resize(3);
        if (eg.elem_index >= 0)
        {
          val[0] = T(0); val[1] = T(0); val[2] = T(0);
        }
        else
        {
          val[0] = defval; val[1] = defval; val[2] = defval;
        }

        return;
      case 1:
      {
        val.resize(3);
        val[0] = T(0); val[1] = T(0); val[2] = T(0);

        if (eg.elem_index >= 0)
        {
          for (index_type k=0, q = 0; k<eg.num_derivs; k++)
          {
            grad = T(0);
            for (size_t p=0;p<eg.node_index.size();p++)
              { grad += CastFData<T>(fdata_[eg.node_index[p]]*eg.weights[q]); q++; }

            val[0] += CastFData<T>(grad*eg.inverse_jacobian[k]);
            val[1] += CastFData<T>(grad*eg.inverse_jacobian[k+3]);
            val[2] += CastFData<T>(grad*eg.inverse_jacobian[k+6]);
          }
        }
        else
        {
          val[0] = defval; val[1] = defval; val[2] = defval;
        }
        return;
      }
      case 2:
      {
        val.resize(3);
        val[0] = T(0); val[1] = T(0); val[2] = T(0);

        if (eg.elem_index >= 0)
        {
          index_type q = 0;
          for (index_type k=0; k<eg.num_derivs; k++)
          {
            grad = T(0);
            for (size_t p=0;p<eg.node_index.size();p++)
             { grad += CastFData<T>(fdata_[eg.node_index[p]]*eg.weights[q]); q++; }
            for (size_t p=0;p<eg.edge_index.size();p++)
             { grad += CastFData<T>(fdata_[eg.edge_index[p]]*eg.weights[q]); q++; }

            val[0] += CastFData<T>(grad*eg.inverse_jacobian[k]);
            val[1] += CastFData<T>(grad*eg.inverse_jacobian[k+3]);
            val[2] += CastFData<T>(grad*eg.inverse_jacobian[k+6]);
          }
        }
        else
        {
          val[0] = defval; val[1] = defval; val[2] = defval;
        }

        return;
      }
      case 3:
      {
        val.resize(3);
        val[0] = T(0); val[1] = T(0); val[2] = T(0);

        if (eg.elem_index >= 0)
        {
          index_type q = 0;
          for (index_type k=0; k<eg.num_derivs; k++)
          {
            grad = T(0);

            for (size_t p=0;p<eg.node_index.size();p++)
            {
              grad += CastFData<T>(fdata_[eg.node_index[p]]*eg.weights[q]); q++;
              for (index_type r=1;r<eg.num_hderivs;r++)
                { grad += CastFData<T>(hfdata_[eg.node_index[p]][r]*eg.weights[q]); q++; }
            }

            val[0] += CastFData<T>(grad*eg.inverse_jacobian[k]);
            val[1] += CastFData<T>(grad*eg.inverse_jacobian[k+3]);
            val[2] += CastFData<T>(grad*eg.inverse_jacobian[k+6]);
          }
        }
        else
        {
          val[0] = defval; val[1] = defval; val[2] = defval;
        }

        return;
      }
    }
    ASSERTFAIL("Gradient encountered an unknown basis_order");
  }

  virtual VMesh::size_type size() { return (VMesh::size_type(fdata_.size())); }

protected:
  FDATA& fdata_;
  EFDATA& efdata_;  // Additional data for lagrangian interpolation data
  HFDATA& hfdata_;  // Additional data for hermitian interpolation data
};

template <typename Scalar>
struct LessThan
{
  bool operator()(const Scalar& lhs, const Scalar& rhs) const
  {
    return lhs < rhs;
  }
};

template <>
struct SCISHARE LessThan<std::complex<double>>
{
  bool operator()(const std::complex<double>& lhs, const std::complex<double>& rhs) const
  {
    return std::norm(lhs) < std::norm(rhs);
  }
};

template <typename Scalar>
struct GreaterThan
{
  bool operator()(const Scalar& lhs, const Scalar& rhs) const
  {
    return lhs > rhs;
  }
};

template <>
struct SCISHARE GreaterThan<std::complex<double>>
{
  bool operator()(const std::complex<double>& lhs, const std::complex<double>& rhs) const
  {
    return std::norm(lhs) > std::norm(rhs);
  }
};


template<class FDATA, class EFDATA, class HFDATA>
class VFDataScalarT : public VFDataT<FDATA,EFDATA,HFDATA> {
public:
  // constructor
  VFDataScalarT(FDATA& fdata, EFDATA& efdata, HFDATA& hfdata) :
    VFDataT<FDATA,EFDATA,HFDATA>(fdata,efdata,hfdata)
  {}

  // destructor
  virtual ~VFDataScalarT() {}

  virtual bool min(double& val, VMesh::index_type& idx) const
  {
    typename FDATA::value_type tval(0);
    idx = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0)
      tval = this->fdata_[0];
    else
      return (false);

    LessThan<typename FDATA::value_type> less;
    for (size_type p=1; p<sz; p++)
    {
      if (less(this->fdata_[p], tval))
      {
        tval = this->fdata_[p];
        idx = VMesh::index_type(p);
      }
    }
    val = CastFData<double>(tval);
    return (true);
  }

  virtual bool max(double& val, VMesh::index_type& idx) const
  {
    typename FDATA::value_type tval(0);
    idx = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) tval = this->fdata_[0]; else return (false);
    GreaterThan<typename FDATA::value_type> greater;
    for (size_type p = 1; p < sz; p++)
    {
      if (greater(this->fdata_[p], tval))
      {
        tval = this->fdata_[p];
        idx = VMesh::index_type(p);
      }
    }
    val = CastFData<double>(tval);
    return (true);
  }

  virtual bool minmax(double& min, VMesh::index_type& idxmin,
                      double& max, VMesh::index_type& idxmax) const
  {
    typename FDATA::value_type tval(0);
    typename FDATA::value_type tval2(0);
    idxmin = 0;
    idxmax = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) { tval = this->fdata_[0]; tval2 = tval; } else return (false);
    LessThan<typename FDATA::value_type> less;
    GreaterThan<typename FDATA::value_type> greater;
    for (size_type p=1; p<sz; p++)
    {
      if (less(this->fdata_[p], tval)) { tval = this->fdata_[p]; idxmin = VMesh::index_type(p); }
      if (greater(this->fdata_[p], tval2)) { tval2 = this->fdata_[p]; idxmax = VMesh::index_type(p); }
    }
    min = CastFData<double>(tval);
    max = CastFData<double>(tval2);
    return (true);
  }
};

template<class FDATA, class EFDATA, class HFDATA>
class VFDataVectorT : public VFDataT<FDATA,EFDATA,HFDATA> {
public:
  // constructor
  VFDataVectorT(FDATA& fdata, EFDATA& efdata, HFDATA& hfdata) :
    VFDataT<FDATA,EFDATA,HFDATA>(fdata,efdata,hfdata)
  {}

  // destructor
  virtual ~VFDataVectorT() {}

  virtual bool min(double& val, VMesh::index_type& idx) const
  {
    double tval = 0;
    idx = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) tval = this->fdata_[0].length();

    for (size_type p=1; p<sz; p++)
    {
      double len = this->fdata_[p].length();
      if (len < tval) { tval = len; idx = VMesh::index_type(p); }
    }

    val = CastFData<double>(tval);
    return (true);
  }

  virtual bool max(double& val, VMesh::index_type& idx) const
  {
    double tval = 0;
    idx = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) tval = this->fdata_[0].length();
    for (size_type p=1; p<sz; p++)
    {
      double len = this->fdata_[p].length();
      if (len > tval) { tval = len; idx = VMesh::index_type(p); }
    }

    val = CastFData<double>(tval);
    return (true);
  }

  virtual bool minmax(double& min, VMesh::index_type& idxmin,
                      double& max, VMesh::index_type& idxmax) const
  {
    double tval = 0;
    double tval2 = 0;
    idxmin = 0;
    idxmax = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) { tval = this->fdata_[0].length(); tval2 = tval; }
    for (size_type p=1; p<sz; p++)
    {
      if (this->fdata_[p].length() < tval) { tval = this->fdata_[p].length(); idxmin = VMesh::index_type(p); }
      if (this->fdata_[p].length() > tval2) { tval2 = this->fdata_[p].length(); idxmax = VMesh::index_type(p); }
    }
    min = CastFData<double>(tval);
    max = CastFData<double>(tval2);
    return (true);
  }
};


template<class FDATA, class EFDATA, class HFDATA>
class VFDataTensorT : public VFDataT<FDATA,EFDATA,HFDATA> {
public:
  // constructor
  VFDataTensorT(FDATA& fdata, EFDATA& efdata, HFDATA& hfdata) :
    VFDataT<FDATA,EFDATA,HFDATA>(fdata,efdata,hfdata)
  {}

  // destructor
  virtual ~VFDataTensorT() {}

  virtual bool min(double& val, VMesh::index_type& idx) const
  {
    double tval = 0;
    idx = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) tval = this->fdata_[0].norm();

    for (size_type p=1; p<sz; p++)
    {
      double len = this->fdata_[p].norm();
      if (len < tval) { tval = len; idx = VMesh::index_type(p); }
    }

    val = CastFData<double>(tval);
    return (true);
  }

  virtual bool max(double& val, VMesh::index_type& idx) const
  {
    double tval = 0;
    idx = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) tval = this->fdata_[0].norm();
    for (size_type p=1; p<sz; p++)
    {
      double len = this->fdata_[p].norm();
      if (len > tval) { tval = len; idx = VMesh::index_type(p); }
    }

    val = CastFData<double>(tval);
    return (true);  }

  virtual bool minmax(double& min, VMesh::index_type& idxmin,
                      double& max, VMesh::index_type& idxmax) const
  {
    double tval = 0;
    double tval2 = 0;
    idxmin = 0;
    idxmax = 0;
    size_type sz = static_cast<size_type>(this->fdata_.size());
    if (sz > 0) { tval = this->fdata_[0].norm(); tval2 = tval; }
    for (size_type p=1; p<sz; p++)
    {
      if (this->fdata_[p].norm() < tval) { tval = this->fdata_[p].norm(); idxmin = VMesh::index_type(p); }
      if (this->fdata_[p].norm() > tval2) { tval2 = this->fdata_[p].norm(); idxmax = VMesh::index_type(p); }
    }
    min = CastFData<double>(tval);
    max = CastFData<double>(tval2);
    return (true);
  }
};


VFDATAT_ACCESS_DEFINITION(double)
VFDATAT_ACCESS_DEFINITION(std::complex<double>)
VFDATAT_ACCESS_DEFINITION(Core::Geometry::Vector)
VFDATAT_ACCESS_DEFINITION(Core::Geometry::Tensor)
VFDATAT_ACCESS_DEFINITION(char)
VFDATAT_ACCESS_DEFINITION(unsigned char)
VFDATAT_ACCESS_DEFINITION(short)
VFDATAT_ACCESS_DEFINITION(unsigned short)
VFDATAT_ACCESS_DEFINITION(int)
VFDATAT_ACCESS_DEFINITION(unsigned int)
VFDATAT_ACCESS_DEFINITION(float)
VFDATAT_ACCESS_DEFINITION(long long)
VFDATAT_ACCESS_DEFINITION(unsigned long long)

VFDATAT_ACCESS_DEFINITION2(int)
VFDATAT_ACCESS_DEFINITION2(float)
VFDATAT_ACCESS_DEFINITION2(double)
//VFDATAT_ACCESS_DEFINITION2(std::complex<double>)
VFDATAT_ACCESS_DEFINITION2(Core::Geometry::Vector)
VFDATAT_ACCESS_DEFINITION2(Core::Geometry::Tensor)


}
