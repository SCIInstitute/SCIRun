#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
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



///
///@file  WorkQueue.cc
///@brief Manage assignments of work
///
///@author
///       Steve Parker
///       Department of Computer Science
///       University of Utah
///@date  June 1997
///


#include <Core/Thread/Legacy/WorkQueue.h>
namespace SCIRun {

WorkQueue::WorkQueue(const char* name)
  : name_(name),
    num_threads_(-1),
    total_assignments_(-1),
    granularity_(-1),
    current_assignment_("WorkQueue counter")
{
}

WorkQueue::~WorkQueue()
{
}

bool
WorkQueue::nextAssignment(int& start, int& end)
{
    int i=current_assignment_++; // Atomic ++
    if(i >= (int)assignments_.size()-1)
	return false;
    start=assignments_[i];
    end=assignments_[i+1];
    return true;
}

void
WorkQueue::refill(int new_ta, int new_nthreads, int new_granularity)
{
    if(new_ta == total_assignments_ && new_nthreads == num_threads_
       && new_granularity == granularity_){
	current_assignment_.set(0);
    } else {
	total_assignments_=new_ta;
	num_threads_=new_nthreads;
	granularity_=new_granularity;
	fill();
    }
}

void
WorkQueue::fill()
{
    current_assignment_.set(0);

    if(total_assignments_==0){
	assignments_.resize(0);
	return;
    }

    // Since we are using push_back every time we call fill (and make
    // assignments into the array, we need to remove the existing entries,
    // otherwise we get more assignments then we should.
    assignments_.clear();
    // make sure we only allocate memory once for the vector
    assignments_.reserve(total_assignments_+1);
    int current_assignment=0;
    int current_assignmentsize=(2*total_assignments_)/(num_threads_*(granularity_+1));
    int decrement=current_assignmentsize/granularity_;
    if(current_assignmentsize==0)
	current_assignmentsize=1;
    if(decrement==0)
	decrement=1;
    for(int i=0;i<granularity_;i++){
	for(int j=0;j<num_threads_;j++){
	    assignments_.push_back(current_assignment);
	    current_assignment+=current_assignmentsize;
	    if(current_assignment >= total_assignments_){
		break;
	    }
	}
	if(current_assignment >= total_assignments_){
	  break;
	}
	current_assignmentsize-=decrement;
	if(current_assignmentsize<1)
	    current_assignmentsize=1;
	if(current_assignment >= total_assignments_){
	    break;
	}
    }
    while(current_assignment < total_assignments_){
	assignments_.push_back(current_assignment);
	current_assignment+=current_assignmentsize;
    }
    assignments_.push_back(total_assignments_);
}


} // End namespace SCIRun
#endif
