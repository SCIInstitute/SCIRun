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
///@file   FutureValue.h
///@brief  Delayed return values
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_FutureValue_h
#define Core_Thread_FutureValue_h

#include <Core/Thread/Legacy/Legacy/Semaphore.h>
#include <Core/Thread/Legacy/Legacy/Thread.h>

namespace SCIRun {
/**************************************

@class
  FutureValue

  KEYWORDS
  Thread

@details
  Creates a single slot for some return value.  The <i>wait</i> method
  waits for a value to be sent from another thread via the <i>reply</i>
  method.  This is typically used to provide a simple means of returning
  data from a server thread.  An <b>FutureValue</b> object is created on the
  stack, and some request is sent (usually via a <b>Mailbox</b>) to a server
  thread.  Then the thread will block in <i>wait</i> until the server thread
  receives the message and responds using <i>reply</i>.

  <p><b>FutureValue</b> is a one-shot wait/reply pair - a new
  <b>FutureValue</b> object must be created for each reply, and these are
  typically created on the stack.  Only a single thread should
  call <i>wait</i> and a single thread shuold call <i>reply</i>.

****************************************/
template<class Item> class FutureValue {
public:
  //////////
  /// Create the FutureValue object.  <i>name</i> should be a
  /// static string which describes the primitive for debugging
  /// purposes.
  FutureValue(const char* name);

  //////////
  /// Destroy the object
  ~FutureValue();

  //////////
  /// Wait until the reply is sent by anothe thread, then return
  /// that reply.
  Item receive();

  //////////
  /// Send the reply to the waiting thread.
  void send(const Item& reply);

private:
  const char* name;
  Item value;
  Semaphore sema;

  // Cannot copy them
  FutureValue(const FutureValue<Item>&);
  FutureValue<Item>& operator=(const FutureValue<Item>&);
};

template<class Item>
FutureValue<Item>::FutureValue(const char* name)
    : name(name), sema("FutureValue semaphore", 0)
{
}

template<class Item>
FutureValue<Item>::~FutureValue()
{
}

template<class Item>
Item
FutureValue<Item>::receive()
{
  int s=Thread::couldBlock(name);
  sema.down();
  Thread::couldBlockDone(s);
  return value;
} // End namespace SCIRun

template<class Item>
void
FutureValue<Item>::send(const Item& reply)
{
  value=reply;
  sema.up();
}
} // End namespace SCIRun

#endif
