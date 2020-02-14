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
///@file   Mailbox.h
///@brief  Threadsafe FIFO
///
///@author Steve Parker
///        Department of Computer Science
///        University of Utah
///@date   June 1997
///

#ifndef Core_Thread_Mailbox_h
#define Core_Thread_Mailbox_h

#include <Core/Thread/Legacy/Legacy/ConditionVariable.h>
#include <Core/Thread/Legacy/Legacy/Mutex.h>
#include <Core/Thread/Legacy/Legacy/Semaphore.h>
#include <Core/Thread/Legacy/Legacy/Thread.h>

#include <vector>


namespace SCIRun {
/**************************************

@class
  Mailbox

  KEYWORDS
  Thread, FIFO

@details
  A thread-safe, fixed-length FIFO queue which allows multiple
  concurrent senders and receivers.  Multiple threads send <b>Item</b>s
  to the mailbox, and multiple thread may receive <b>Item</b>s from the
  mailbox.  Items are typically pointers to a message structure.

****************************************/
template<class Item> class Mailbox {
public:
  //////////
  /// Create a mailbox with a maximum queue size of <i>size</i>
  /// items. If size is zero, then the mailbox will use
  /// <i>rendevous semantics</i>, where a sender will block
  /// until a reciever is waiting for the item.  The item will
  /// be handed off synchronously. <i>name</i> should be a
  /// static string which describes the primitive for debugging
  /// purposes.
  Mailbox(const char* name, int size);

  //////////
  /// Destroy the mailbox.  All items in the queue are silently
  /// dropped.
  ~Mailbox();

  //////////
  /// Puts <i>msg</i> in the queue.  If the queue is full, the
  /// thread will be blocked until there is room in the queue.
  /// Messages from the same thread will be placed in the
  /// queue in a first-in/first out order. Multiple threads may
  /// call <i>send</i> concurrently, and the messages will be
  /// placed in the queue in an arbitrary order.
  void send(const Item& msg);

  //////////
  /// Attempt to send <i>msg</i> to the queue.  If the queue is
  /// full, the thread will not be blocked, and <i>trySend</i>
  /// will return false.  Otherwise, <i>trySend</i> will return
  /// true.  This may never complete if the reciever only uses
  /// <i>tryRecieve</i>.
  bool trySend(const Item& msg);


  //////////
  /// Send <i>msg</i> to the queue only if the <i>checker</i> function
  /// fails to compare the <i>msg</i> to what is already the last item
  /// there.  This is useful if we want to make certain that at least
  /// one of a particular message (like a viewer resize redraw) gets
  /// put on the mailbox without spamming it.  This blocks like
  /// <i>send</i> does.  It returns true if <i>msg</i> was added to the
  /// queue and false if it wasn't.
  bool sendIfNotSentLast(const Item& msg,
                         bool (*checker)(const Item &a, const Item &b));

  //////////
  /// Receive an item from the queue.  If the queue is empty,
  /// the thread will block until another thread sends an item.
  /// Multiple threads may call <i>recieve</i> concurrently, but
  /// no guarantee is made as to which thread will recieve the
  /// next token.  However, implementors should give preference
  /// to the thread that has been waiting the longest.
  Item receive();

  //////////
  /// Attempt to recieve <i>item</i> from the mailbox.  If the
  /// queue is empty, the thread is blocked and <i>tryRecieve</i>
  /// will return false.  Otherwise, <i>tryRecieve</i> returns true.
  bool tryReceive(Item& item);

  //////////
  /// Return the maximum size of the mailbox queue, as given in the
  /// constructor.
  int size() const;

  //////////
  /// Return the number of items currently in the queue.
  int numItems() const;

private:
  const char* name_;
  Mutex mutex_;
  std::vector<Item> ring_buffer_;
  int head_;
  int len_;
  int max_;
  ConditionVariable empty_;
  ConditionVariable full_;
  Semaphore rendezvous_;
  int send_wait_;
  int recv_wait_;
  inline int ringNext(int inc);

  // Cannot copy them
  Mailbox(const Mailbox<Item>&);
  Mailbox<Item> operator=(const Mailbox<Item>&);
};

template<class Item> inline
int
Mailbox<Item>::ringNext(int inc)
{
    return max_==0?0:((head_+inc)%max_);
}

template<class Item>
Mailbox<Item>::Mailbox(const char* name, int size)
    : name_(name), mutex_("Mailbox lock"), ring_buffer_(size==0?1:size),
      empty_("Mailbox empty condition"), full_("Mailbox full condition"),
      rendezvous_("Mailbox rendezvous semaphore", 0)
{
    head_=0;
    len_=0;
    send_wait_=0;
    recv_wait_=0;
    max_=size;
}

template<class Item>
Mailbox<Item>::~Mailbox()
{
  mutex_.tryLock();
  mutex_.unlock();

  // release all waiting threads
  //  empty_.conditionBroadcast();
  //  full_.conditionBroadcast();
}

template<class Item>
void
Mailbox<Item>::send(const Item& msg)
{
    int s=Thread::couldBlock(name_);
    mutex_.lock();
    // See if the message buffer is full...
    int rmax=max_==0?1:max_;
    while(len_ == rmax){
        send_wait_++;
        full_.wait(mutex_);
        send_wait_--;
    }
    ring_buffer_[ringNext(len_)]=msg;
    len_++;
    if(recv_wait_)
        empty_.conditionSignal();
    mutex_.unlock();
    if(max_==0)
        rendezvous_.down();
    Thread::couldBlockDone(s);
}

template<class Item>
bool
Mailbox<Item>::trySend(const Item& msg)
{
    mutex_.lock();
    // See if the message buffer is full...
    int rmax=max_==0?1:max_;
    if(len_ == rmax){
        mutex_.unlock();
        return false;
    }
    if(max_ == 0 && recv_wait_==0){
        // No receivers waiting, so rendezvous will fail. Return now.
        mutex_.unlock();
        return false;
    }

    ring_buffer_[ringNext(len_)]=msg;
    len_++;
    if(recv_wait_)
        empty_.conditionSignal();
    mutex_.unlock();
    if(max_==0)
        rendezvous_.down();  // Won't block for long, since a receiver
                            // will wake us up
    return true;
}


template<class Item>
bool
Mailbox<Item>::sendIfNotSentLast(const Item& msg,
                                 bool (*checker)(const Item &a, const Item &b))
{
    int s = Thread::couldBlock(name_);
    mutex_.lock();
    // See if the message buffer is full...
    int rmax=max_==0?1:max_;
    while(len_ == rmax){
        send_wait_++;
        full_.wait(mutex_);
        send_wait_--;
    }
    if (len_ < 1 || checker(ring_buffer_[ringNext(len_-1)], msg))
    {
      ring_buffer_[ringNext(len_)]=msg;
      len_++;
      if(recv_wait_)
        empty_.conditionSignal();
      mutex_.unlock();
      if(max_==0)
        rendezvous_.down();
      Thread::couldBlockDone(s);
      return true;
    }
    else
    {
      mutex_.unlock();
      Thread::couldBlockDone(s);
      return false;
    }
}


template<class Item>
Item
Mailbox<Item>::receive()
{
    int s=Thread::couldBlock(name_);
    mutex_.lock();
    while(len_ == 0){
        recv_wait_++;
        empty_.wait(mutex_);
        recv_wait_--;
    }
    Item val=ring_buffer_[head_];
    ring_buffer_[head_] = 0; //if it is a handle clear it.
    head_=ringNext(1);
    len_--;
    if(send_wait_)
        full_.conditionSignal();
    mutex_.unlock();
    if(max_==0)
        rendezvous_.up();
    Thread::couldBlockDone(s);
    return val;
}

template<class Item>
bool
Mailbox<Item>::tryReceive(Item& item)
{
    mutex_.lock();
    if(len_ == 0){
        mutex_.unlock();
        return false;
    }
    item=ring_buffer_[head_];
    ring_buffer_[head_] = 0; //if it is a handle clear it.
    head_=ringNext(1);
    len_--;
    if(send_wait_)
        full_.conditionSignal();
    mutex_.unlock();
    if(max_==0)
        rendezvous_.up();
    return true;
}

template<class Item>
int
Mailbox<Item>::size() const
{
    return max_;
} // End namespace SCIRun

template<class Item>
int
Mailbox<Item>::numItems() const
{
    return len_;
}

} // End namespace SCIRun
#endif
