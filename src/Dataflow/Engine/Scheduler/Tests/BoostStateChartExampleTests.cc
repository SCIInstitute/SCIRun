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


#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>
#include <ctime>
#include <iostream>
#include <Dataflow/Network/ModuleInterface.h>

using namespace SCIRun::Dataflow::Networks;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;

// We are declaring all types as structs only to avoid having to
// type public. If you don't mind doing so, you can just as well
// use class.

// We need to forward-declare the initial state because it can
// only be defined at a point where the state machine is
// defined.
struct Greeting;

// Boost.Statechart makes heavy use of the curiously recurring
// template pattern. The deriving class must always be passed as
// the first parameter to all base class templates.
//
// The state machine must be informed which state it has to
// enter when the machine is initiated. That's why Greeting is
// passed as the second template parameter.
struct Machine : sc::state_machine< Machine, Greeting > {};

// For each state we need to define which state machine it
// belongs to and where it is located in the statechart. Both is
// specified with Context argument that is passed to
// simple_state<>. For a flat state machine as we have it here,
// the context is always the state machine. Consequently,
// Machine must be passed as the second template parameter to
// Greeting's base (the Context parameter is explained in more
// detail in the next example).
struct Greeting : sc::simple_state< Greeting, Machine >
{
  // Whenever the state machine enters a state, it creates an
  // object of the corresponding state class. The object is then
  // kept alive as long as the machine remains in the state.
  // Finally, the object is destroyed when the state machine
  // exits the state. Therefore, a state entry action can be
  // defined by adding a constructor and a state exit action can
  // be defined by adding a destructor.
  Greeting() { std::cout << "Hello World!\n"; } // entry
  ~Greeting() { std::cout << "Bye Bye World!\n"; } // exit
};

TEST(StateChartHelloWorld, Run)
{
  Machine myMachine;
  // The machine is not yet running after construction. We start
  // it by calling initiate(). This triggers the construction of
  // the initial state Greeting
  myMachine.initiate();
  // When we leave main(), myMachine is destructed what leads to
  // the destruction of all currently active states.
}

struct EvStartStop : sc::event< EvStartStop > {};
struct EvReset : sc::event< EvReset > {};

struct IElapsedTime
{
  virtual double ElapsedTime() const = 0;
  virtual ~IElapsedTime() {}
};

struct Active;
struct StopWatch : sc::state_machine< StopWatch, Active >
{
  double ElapsedTime() const
  {
    return state_cast< const IElapsedTime & >().ElapsedTime();
  }
};

struct Stopped;

// The simple_state class template accepts up to four parameters:
// - The third parameter specifies the inner initial state, if
//   there is one. Here, only Active has inner states, which is
//   why it needs to pass its inner initial state Stopped to its
//   base
// - The fourth parameter specifies whether and what kind of
//   history is kept

// Active is the outermost state and therefore needs to pass the
// state machine class it belongs to
struct Active : sc::simple_state<
  Active, StopWatch, Stopped >
  {
  public:
    typedef sc::transition< EvReset, Active > reactions;
    Active() : elapsedTime_( 0.0 ) {}
    double ElapsedTime() const { return elapsedTime_; }
    double & ElapsedTime() { return elapsedTime_; }
  private:
    double elapsedTime_;
  };

// Stopped and Running both specify Active as their Context,
// which makes them nested inside Active
struct Running : IElapsedTime, sc::simple_state< Running, Active >
{
public:
  typedef sc::transition< EvStartStop, Stopped > reactions;
  Running() : startTime_( std::time( 0 ) ) {}
  ~Running()
  {
    // Similar to when a derived class object accesses its
    // base class portion, context<>() is used to gain
    // access to the direct or indirect context of a state.
    // This can either be a direct or indirect outer state
    // or the state machine itself
    // (e.g. here: context< StopWatch >()).
    //context< Active >().ElapsedTime() +=  std::difftime( std::time( 0 ), startTime_ );
    context< Active >().ElapsedTime() = ElapsedTime();
  }
  virtual double ElapsedTime() const override
  {
    return context< Active >().ElapsedTime() +
      std::difftime( std::time( 0 ), startTime_ );
  }
private:
  std::time_t startTime_;
};

struct Stopped : IElapsedTime, sc::simple_state< Stopped, Active >
{
  typedef sc::transition< EvStartStop, Running > reactions;
  // A state can define an arbitrary number of reactions. That's
// why we have to put them into an mpl::list<> as soon as there
// is more than one of them
// (see Specifying multiple reactions for a state).
  virtual double ElapsedTime() const override
  {
    return context< Active >().ElapsedTime();
  }
};

// Because the context of a state must be a complete type (i.e.
// not forward declared), a machine must be defined from
// "outside to inside". That is, we always start with the state
// machine, followed by outermost states, followed by the direct
// inner states of outermost states and so on. We can do so in a
// breadth-first or depth-first way or employ a mixture of the
// two.

TEST(StateChartStopwatch, Run)
{
  StopWatch myWatch;
  myWatch.initiate();
  std::cout << myWatch.ElapsedTime() << "\n";
  myWatch.process_event( EvStartStop() );
  std::cout << myWatch.ElapsedTime() << "\n";
  myWatch.process_event( EvStartStop() );
  std::cout << myWatch.ElapsedTime() << "\n";
  myWatch.process_event( EvStartStop() );
  std::cout << myWatch.ElapsedTime() << "\n";
  myWatch.process_event( EvReset() );
  std::cout << myWatch.ElapsedTime() << "\n";
}

struct ModuleStart : sc::event< ModuleStart > {};
struct ModuleEnd : sc::event< ModuleEnd > {};
struct ModuleQueued : sc::event< ModuleQueued > {};
struct ModuleError : sc::event< ModuleError > {};
struct ModuleReset : sc::event< ModuleReset > {};

struct ModuleColorProvider
{
public:
  virtual ModuleExecutionState::Value stateValue() const = 0;
  virtual std::string color() const = 0;
  virtual ~ModuleColorProvider() {}
};

struct ModuleNotExecuted;
struct ModuleState : sc::state_machine< ModuleState, ModuleNotExecuted >
{
public:
  ModuleState()
  {
    std::cout << "ModuleState()" << std::endl;
  }
  ~ModuleState()
  {
    std::cout << "~ModuleState()" << std::endl;
  }
  ModuleExecutionState::Value currentStateValue() const
  {
    return state_cast<const ModuleColorProvider&>().stateValue();
  }
  std::string currentColor() const
  {
    return state_cast<const ModuleColorProvider&>().color();
  }
};

struct ModuleWaiting;
struct ModuleRunning;
struct ModuleErrored;
struct ModuleCompleted;

struct ModuleNotExecuted : ModuleColorProvider, sc::simple_state< ModuleNotExecuted, ModuleState >
{
public:
  typedef sc::transition< ModuleQueued, ModuleWaiting > reactions;
  ModuleNotExecuted()
  {
    std::cout << "ModuleNotExecuted" << std::endl;
  }
  ~ModuleNotExecuted()
  {
    std::cout << "~ModuleNotExecuted" << std::endl;
  }
  virtual ModuleExecutionState::Value stateValue() const override
  {
    return ModuleExecutionState::NotExecuted;
  }
  virtual std::string color() const override { return "gray"; }
};

struct ModuleWaiting : ModuleColorProvider, sc::simple_state< ModuleWaiting, ModuleState >
{
public:
  typedef sc::transition< ModuleStart, ModuleRunning > reactions;
  ModuleWaiting()
  {
    std::cout << "ModuleWaiting" << std::endl;
  }
  ~ModuleWaiting()
  {
    std::cout << "~ModuleWaiting" << std::endl;
  }
  virtual ModuleExecutionState::Value stateValue() const override
  {
    return ModuleExecutionState::Waiting;
  }
  virtual std::string color() const override { return "yellow"; }
};

struct ModuleRunning : ModuleColorProvider, sc::simple_state< ModuleRunning, ModuleState >
{
public:
  typedef mpl::list<
    sc::transition< ModuleError, ModuleErrored >,
    sc::transition< ModuleEnd, ModuleCompleted >>
    reactions;
  ModuleRunning()
  {
    std::cout << "ModuleRunning" << std::endl;
  }
  ~ModuleRunning()
  {
    std::cout << "~ModuleRunning" << std::endl;
  }
  virtual ModuleExecutionState::Value stateValue() const override
  {
    return ModuleExecutionState::Executing;
  }
  virtual std::string color() const override { return "green"; }
};

struct ModuleErrored : ModuleColorProvider, sc::simple_state< ModuleErrored, ModuleState >
{
public:
  typedef sc::transition< ModuleReset, ModuleNotExecuted > reactions;
  ModuleErrored()
  {
    std::cout << "ModuleErrored" << std::endl;
  }
  ~ModuleErrored()
  {
    std::cout << "~ModuleErrored" << std::endl;
  }
  virtual ModuleExecutionState::Value stateValue() const override
  {
    return ModuleExecutionState::Errored;
  }
  virtual std::string color() const override { return "red"; }
};

struct ModuleCompleted : ModuleColorProvider, sc::simple_state< ModuleCompleted, ModuleState >
{
public:
  typedef sc::transition< ModuleReset, ModuleNotExecuted > reactions;
  ModuleCompleted()
  {
    std::cout << "ModuleCompleted" << std::endl;
  }
  ~ModuleCompleted()
  {
    std::cout << "~ModuleCompleted" << std::endl;
  }
  virtual ModuleExecutionState::Value stateValue() const override
  {
    return ModuleExecutionState::Completed;
  }
  virtual std::string color() const override { return "darkGray"; }
};

TEST(ModuleStateChart, RunNormalTransitions)
{
  ModuleState moduleState;
  moduleState.initiate();
  moduleState.process_event( ModuleQueued() );
  moduleState.process_event( ModuleStart() );
  moduleState.process_event( ModuleEnd() );
}

TEST(ModuleStateChart, RunErrorTransitions)
{
  ModuleState moduleState;
  moduleState.initiate();
  moduleState.process_event( ModuleQueued() );
  moduleState.process_event( ModuleStart() );
  moduleState.process_event( ModuleError() );
  moduleState.process_event( ModuleReset() );
}

TEST(ModuleStateChart, RunNormalTransitionsWithColor)
{
  ModuleState moduleState;
  moduleState.initiate();
  EXPECT_EQ("gray", moduleState.currentColor());
  moduleState.process_event( ModuleQueued() );
  EXPECT_EQ("yellow", moduleState.currentColor());
  moduleState.process_event( ModuleStart() );
  EXPECT_EQ("green", moduleState.currentColor());
  moduleState.process_event( ModuleEnd() );
  EXPECT_EQ("darkGray", moduleState.currentColor());
}

TEST(ModuleStateChart, RunErrorTransitionsWithColor)
{
  ModuleState moduleState;
  moduleState.initiate();
  EXPECT_EQ("gray", moduleState.currentColor());
  moduleState.process_event( ModuleQueued() );
  EXPECT_EQ("yellow", moduleState.currentColor());
  moduleState.process_event( ModuleStart() );
  EXPECT_EQ("green", moduleState.currentColor());
  moduleState.process_event( ModuleError() );
  EXPECT_EQ("red", moduleState.currentColor());
  moduleState.process_event( ModuleStart() );
  EXPECT_EQ("red", moduleState.currentColor());
  moduleState.process_event( ModuleReset() );
  EXPECT_EQ("gray", moduleState.currentColor());
}

class ModuleStateMachine : public ModuleExecutionState
{
public:
  ModuleStateMachine()
  {
    moduleState_.initiate();
  }

  virtual Value currentState() const override
  {
    return moduleState_.currentStateValue();
  }

  virtual boost::signals2::connection connectExecutionStateChanged(const ExecutionStateChangedSignalType::slot_type& subscriber) override
  {
    return signal_.connect(subscriber);
  }

  virtual bool transitionTo(Value state) override
  {
    //Guard g(mutex_.get());
    auto current = currentState();
    switch (state)
    {
      case NotExecuted:
        moduleState_.process_event( ModuleReset() );
        break;
      case Waiting:
        moduleState_.process_event( ModuleQueued() );
        break;
      case Executing:
        moduleState_.process_event( ModuleStart() );
        break;
      case Completed:
        moduleState_.process_event( ModuleEnd() );
        break;
      case Errored:
        moduleState_.process_event( ModuleError() );
        break;
    }
    auto changed = current != currentState();
    if (changed)
      signal_(currentState());
    return changed;
  }

  virtual std::string currentColor() const override
  {
    return moduleState_.currentColor();
  }

  Value expandedState() const override { return currentState(); }
  void setExpandedState(Value state) override { }

private:
  ModuleState moduleState_;
  ExecutionStateChangedSignalType signal_;
};

TEST(ModuleStateMachineTest, NormalSequence)
{
  ModuleStateMachine msm;
  msm.transitionTo(ModuleExecutionState::Waiting);
  msm.transitionTo(ModuleExecutionState::Executing);
  msm.transitionTo(ModuleExecutionState::Completed);
  EXPECT_EQ("darkGray", msm.currentColor());
}

TEST(ModuleStateMachineTest, ErrorSequence)
{
  ModuleStateMachine msm;
  msm.transitionTo(ModuleExecutionState::Waiting);
  msm.transitionTo(ModuleExecutionState::Executing);
  msm.transitionTo(ModuleExecutionState::Errored);
  EXPECT_EQ("red", msm.currentColor());
}

TEST(ModuleStateMachineTest, ErrorSequenceWithComplete)
{
  ModuleStateMachine msm;
  msm.transitionTo(ModuleExecutionState::Waiting);
  msm.transitionTo(ModuleExecutionState::Executing);
  msm.transitionTo(ModuleExecutionState::Errored);
  msm.transitionTo(ModuleExecutionState::Completed);
  EXPECT_EQ("red", msm.currentColor());
}

TEST(ModuleStateMachineTest, OutOfOrderSequence)
{
  ModuleStateMachine msm;
  msm.transitionTo(ModuleExecutionState::Executing);
  msm.transitionTo(ModuleExecutionState::Waiting);
  EXPECT_EQ("yellow", msm.currentColor());
}

TEST(ModuleStateMachineTest, ErrorSequenceWithReset)
{
  ModuleStateMachine msm;
  msm.transitionTo(ModuleExecutionState::Waiting);
  msm.transitionTo(ModuleExecutionState::Executing);
  msm.transitionTo(ModuleExecutionState::Errored);
  msm.transitionTo(ModuleExecutionState::NotExecuted);
  EXPECT_EQ("gray", msm.currentColor());
}
