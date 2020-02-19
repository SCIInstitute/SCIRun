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


#ifndef DATAFLOW_NETWORK_MODULETEMPLATEIMPL_H
#define DATAFLOW_NETWORK_MODULETEMPLATEIMPL_H

  template <class T>
  boost::shared_ptr<T> Module::getRequiredInputAtIndex(const PortId& id)
  {
    auto inputOpt = get_input_handle(id);
    if (!inputOpt)
      MODULE_ERROR_WITH_TYPE(NoHandleOnPortException, "Input data required on port " + id.name);

    return checkInput<T>(inputOpt, id);
  }

  template <class T, size_t N>
  boost::shared_ptr<T> Module::getRequiredInput(const StaticPortName<T,N>& port)
  {
    return getRequiredInputAtIndex<T>(port.toId());
  }

  template <class T>
  boost::optional<boost::shared_ptr<T>> Module::getOptionalInputAtIndex(const PortId& id)
  {
    auto inputOpt = get_input_handle(id);
    if (!inputOpt)
      return {};

    return checkInput<T>(inputOpt, id);
  }

  template <class T, size_t N>
  std::vector<boost::shared_ptr<T>> Module::getRequiredDynamicInputs(const DynamicPortName<T,N>& port)
  {
    auto handleOptions = get_dynamic_input_handles(port.id_);
    std::vector<boost::shared_ptr<T>> handles;
    auto check = [&, this](Core::Datatypes::DatatypeHandleOption opt) { return this->checkInput<T>(opt, port.id_); };
    auto end = handleOptions.end() - 1; //leave off empty final port
    std::transform(handleOptions.begin(), end, std::back_inserter(handles), check);
    if (handles.empty())
      MODULE_ERROR_WITH_TYPE(NoHandleOnPortException, "Input data required on port " + port.id_.name);
    return handles;
  }

  template <class T, size_t N>
  std::vector<boost::shared_ptr<T>> Module::getValidDynamicInputs(const DynamicPortName<T,N>& port)
  {
    auto handleOptions = get_dynamic_input_handles(port.id_);
    std::vector<boost::shared_ptr<T>> handles;
    for (auto& opt : handleOptions)
    {
      if (opt && *opt)
        handles.push_back(boost::dynamic_pointer_cast<T>(*opt));
    }
    return handles;
  }

  template <class T, size_t N>
  std::vector<boost::shared_ptr<T>> Module::getOptionalDynamicInputs(const DynamicPortName<T,N>& port)
  {
    auto handleOptions = get_dynamic_input_handles(port.id_);
    std::vector<boost::shared_ptr<T>> handles;
    auto check = [&, this](Core::Datatypes::DatatypeHandleOption opt) { return this->checkInput<T>(opt, port.id_); };
    auto end = handleOptions.end() - 1; //leave off empty final port
    std::transform(handleOptions.begin(), end, std::back_inserter(handles), check);
    return handles;
  }

  template <class T, size_t N>
  boost::optional<boost::shared_ptr<T>> Module::getOptionalInput(const StaticPortName<T,N>& port)
  {
    return getOptionalInputAtIndex<T>(port.id_);
  }

  template <class T, class D, size_t N>
  void Module::sendOutput(const StaticPortName<T,N>& port, boost::shared_ptr<D> data)
  {
    const bool datatypeForThisPortMustBeCompatible = boost::is_base_of<T,D>::value;
    BOOST_STATIC_ASSERT(datatypeForThisPortMustBeCompatible);
    send_output_handle(port.id_, data);
  }

  template <class T, size_t N, typename F>
  void Module::computeOutputAndSendIfConnected(const StaticPortName<T, N>& port, F evalFunc)
  {
    if (oport_connected(port))
    {
      sendOutput(port, evalFunc());
    }
  }

  template <class T, size_t N>
  void Module::sendOutputFromAlgorithm(const StaticPortName<T,N>& port, const Core::Algorithms::AlgorithmOutput& output)
  {
    sendOutput<T, T, N>(port, output.get<T>(Core::Algorithms::AlgorithmParameterName(port)));
  }

  template <class T>
  boost::shared_ptr<T> Module::checkInput(Core::Datatypes::DatatypeHandleOption inputOpt, const PortId& id)
  {
    if (!inputOpt)
      MODULE_ERROR_WITH_TYPE(NoHandleOnPortException, "Input data required on port " + id.name);

    if (!*inputOpt)
      MODULE_ERROR_WITH_TYPE(NullHandleOnPortException, "Null handle on port " + id.name);

    auto data = boost::dynamic_pointer_cast<T>(*inputOpt);

    if (!data)
    {
      std::ostringstream ostr;
      ostr << "Wrong datatype on port " << id.name << "; expected " << typeid(T).name() << " but received " << typeid(*inputOpt).name();
      MODULE_ERROR_WITH_TYPE(WrongDatatypeOnPortException, ostr.str());
    }
    return data;
  }

#endif
