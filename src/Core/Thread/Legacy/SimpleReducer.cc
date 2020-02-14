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


///
///@file  SimpleReducer.cc
///@brief A barrier with reduction operations
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  June 1997
///

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

#include <Core/Thread/Legacy/SimpleReducer.h>
#include <Core/Thread/Legacy/ThreadGroup.h>
#include <Core/Thread/Legacy/Thread.h>

namespace SCIRun {


SimpleReducer::SimpleReducer(const char* name)
    : Barrier(name)
{
    array_size_=-1;
    p_=0;
    join_[0]=0;
    join_[1]=0;
}

SimpleReducer::~SimpleReducer()
{
    if(p_){
	delete[] join_[0];
	delete[] join_[1];
	delete[] p_;
    }
}

void
SimpleReducer::collectiveResize(int proc, int n)
{
    // Extra barrier here to change the array size...

    // We must wait until everybody has seen the array size change,
    // or they will skip down too soon...
    wait(n);
    if(proc==0)
    {
      if(p_)
      {
          delete[] join_[0];
          delete[] join_[1];
          delete[] p_;
      }
      join_[0]=new joinArray[n];
      join_[1]=new joinArray[n];
      p_=new pdata[n];
      for(int i=0;i<n;i++)
        p_[i].buf_=0;
      array_size_=n;
    }
    wait(n);
}

double
SimpleReducer::sum(int proc, int n, double mysum)
{
    int oldstate=Thread::couldBlock("reducer sum");
    if(n != array_size_){
        collectiveResize(proc, n);
    }

    int buf=p_[proc].buf_;
    p_[proc].buf_=1-buf;

    joinArray* j=join_[buf];
    j[proc].d_.d_=mysum;
    wait(n);
    double sum=0;
    for(int i=0;i<n;i++)
        sum+=j[i].d_.d_;
    Thread::couldBlockDone(oldstate);
    return sum;
}

double
SimpleReducer::max(int proc, int n, double mymax)
{
    if(n != array_size_){
        collectiveResize(proc, n);
    }

    int buf=p_[proc].buf_;
    p_[proc].buf_=1-buf;

    joinArray* j=join_[buf];
    j[proc].d_.d_=mymax;
    Barrier::wait(n);
    double gmax=j[0].d_.d_;
    for(int i=1;i<n;i++)
        if(j[i].d_.d_ > gmax)
	    gmax=j[i].d_.d_;
    return gmax;
}

double
SimpleReducer::min(int proc, int n, double mymin)
{
    if(n != array_size_){
        collectiveResize(proc, n);
    }

    int buf=p_[proc].buf_;
    p_[proc].buf_=1-buf;

    joinArray* j=join_[buf];
    j[proc].d_.d_=mymin;
    Barrier::wait(n);
    double gmin=j[0].d_.d_;
    for(int i=1;i<n;i++)
        if(j[i].d_.d_ < gmin)
	    gmin=j[i].d_.d_;
    return gmin;
}


} // End namespace SCIRun
#endif
