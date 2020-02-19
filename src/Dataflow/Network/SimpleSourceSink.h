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


/// @todo Documentation Dataflow/Network/SimpleSourceSink.h

#ifndef DATAFLOW_NETWORK_SIMPLESOURCESINK_H
#define DATAFLOW_NETWORK_SIMPLESOURCESINK_H

#include <Dataflow/Network/DataflowInterfaces.h>
#include <boost/function.hpp>
#include <set>
#include <Dataflow/Network/share.h>

namespace SCIRun
{
  namespace Dataflow
  {
    namespace Networks
    {
      using WeakDatatypeHandle = boost::weak_ptr<Core::Datatypes::DatatypeHandle::element_type>;

      class SCISHARE SimpleSink : public DatatypeSinkInterface
      {
      public:
        SimpleSink();
        ~SimpleSink();
        void waitForData() override;
        Core::Datatypes::DatatypeHandleOption receive() override;
        DatatypeSinkInterface* clone() const override;
        bool hasChanged() const override;
        void setData(Core::Datatypes::DatatypeHandle data);
        void invalidateProvider() override { /*TODO*/ }
        boost::signals2::connection connectDataHasChanged(const DataHasChangedSignalType::slot_type& subscriber) override;
        void forceFireDataHasChanged() override;

        static bool globalPortCachingFlag();
        static void setGlobalPortCachingFlag(bool value);

      private:
        WeakDatatypeHandle weakData_;
        mutable bool hasChanged_;
        DataHasChangedSignalType dataHasChanged_;
        bool checkForNewDataOnSetting_;
        static bool globalPortCaching_;
        static void invalidateAll();
        static std::set<SimpleSink*> instances_;
      };

      /*
      IDEA

      // hook this up to SinkFactory. Executor needs a hook as well, to clearAllSinks.
      class SinkCacheManager
      {
        void addSink(Sink, sinkId); //adds to map
         void clearAllSinks();  // foreach sink in map, calls setHasData(false), unless cache flag is on
         void setCached(int SinkId, bool cache) // set cache flag for specified sink
      };


      */

      class SCISHARE SimpleSource : public DatatypeSourceInterface
      {
      public:
        SimpleSource();
        ~SimpleSource();
        virtual void cacheData(Core::Datatypes::DatatypeHandle data) override;
        virtual void send(DatatypeSinkInterfaceHandle receiver) const override;
        virtual bool hasData() const override;
        virtual std::string describeData() const override;

        static void clearAllSources();
      protected:
        SCIRun::Core::Datatypes::DatatypeHandle data_;
        static std::set<SimpleSource*> instances_;
      };
    }
  }
}

#endif
