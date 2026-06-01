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

#ifndef DATAFLOW_NETWORK_MODULEPORTDESCRIPTIONTAGSVARIADIC_H
#define DATAFLOW_NETWORK_MODULEPORTDESCRIPTIONTAGSVARIADIC_H

#include <Dataflow/Network/ModulePortDescriptionTags.h>
#include <Dataflow/Network/share.h>
#include <string_view>
#include <utility>

// Requires C++20 for class-type (structural) non-type template parameters.
static_assert(__cplusplus >= 202002L, "ModulePortDescriptionTagsVariadic.h requires C++20");

namespace SCIRun::Modules
{
  // -------------------------------------------------------------------------
  // FixedString — structural type usable as a non-type template parameter.
  // -------------------------------------------------------------------------
  template <size_t N>
  struct FixedString
  {
    char data[N]{};

    constexpr FixedString(const char (&str)[N])
    {
      for (size_t i = 0; i < N; ++i)
        data[i] = str[i];
    }

    constexpr std::string_view view() const { return {data, N - 1}; }
    constexpr bool operator==(const FixedString&) const = default;
  };

  template <size_t N> FixedString(const char (&)[N]) -> FixedString<N>;

  // -------------------------------------------------------------------------
  // PortTypeName — maps a port tag struct to its datatype string.
  // Dynamic/async wrappers forward to the underlying base tag.
  // -------------------------------------------------------------------------
  // -------------------------------------------------------------------------
  // IsDynamic — detects DynamicPortTag / AsyncDynamicPortTag wrappers so that
  // PortSpec can infer the dynamic flag from the tag type automatically.
  // -------------------------------------------------------------------------
  template <typename Tag> struct IsDynamic : std::false_type {};
  template <typename B>   struct IsDynamic<DynamicPortTag<B>>      : std::true_type {};
  template <typename B>   struct IsDynamic<AsyncDynamicPortTag<B>> : std::true_type {};

  template <typename Tag> struct PortTypeName;

  template <typename Base>
  struct PortTypeName<DynamicPortTag<Base>> : PortTypeName<Base> {};

  template <typename Base>
  struct PortTypeName<AsyncDynamicPortTag<Base>> : PortTypeName<Base> {};

  // -------------------------------------------------------------------------
  // PortSpec — a single port description: name (compile-time string),
  // type tag, and whether it is dynamic.  Used in both HasInputPorts and
  // HasOutputPorts; the input/output distinction comes from which list it
  // appears in, not from the spec type itself.
  //
  // Prefer the per-type aliases below (MatrixPort<"Name">) over using
  // PortSpec directly.
  // -------------------------------------------------------------------------
  template <FixedString Name, typename TypeTag, bool Dynamic = IsDynamic<TypeTag>::value>
  struct PortSpec
  {
    static constexpr auto nameStr = Name;          // FixedString, recoverable as NTTP
    static constexpr std::string_view name = Name.view();
    using tag = TypeTag;
    static constexpr bool dynamic = Dynamic;
  };

  // Dynamic<SomePort<"Name">> — wraps any PortSpec alias and sets dynamic=true.
  template <typename Spec>
  using Dynamic = PortSpec<Spec::nameStr, typename Spec::tag, true>;

  // Per-type aliases — the intended public API.
  // Dynamic ports: add true as the second template argument, e.g. FieldPort<"Inputs", true>.
// Declares PortTypeName<TypeNamePortTag> and the TypeNamePort<"Name"> alias
// from a single token, e.g. DECLARE_PORT_TYPE(Matrix) produces:
//   PortTypeName<MatrixPortTag>::value == "Matrix"
//   template <FixedString Name, bool Dynamic = false> using MatrixPort = ...
#define DECLARE_PORT_TYPE(TypeName) \
  template <> struct PortTypeName<TypeName##PortTag> \
    { static constexpr const char* value = #TypeName; }; \
  template <FixedString Name, bool Dynamic = false> \
  using TypeName##Port = PortSpec<Name, TypeName##PortTag, Dynamic>

  DECLARE_PORT_TYPE(Matrix);
  DECLARE_PORT_TYPE(ComplexMatrix);
  DECLARE_PORT_TYPE(Scalar);
  DECLARE_PORT_TYPE(String);
  DECLARE_PORT_TYPE(Field);
  DECLARE_PORT_TYPE(Geometry);
  DECLARE_PORT_TYPE(OsprayGeometry);
  DECLARE_PORT_TYPE(ColorMap);
  DECLARE_PORT_TYPE(Bundle);
  DECLARE_PORT_TYPE(Nrrd);
  DECLARE_PORT_TYPE(Datatype);
  DECLARE_PORT_TYPE(MetadataObject);

#undef DECLARE_PORT_TYPE

  // -------------------------------------------------------------------------
  // HasInputPorts<Specs...> — variadic replacement for Has1InputPort through
  // Has7InputPorts.
  //
  // HasOutputPorts<Specs...> — variadic replacement for Has1OutputPort through
  // Has9OutputPorts.
  //
  // Both provide the same NumIPorts/NumOPorts enum and inputPortDescription()
  // / outputPortDescription() static methods as the old templates.
  // -------------------------------------------------------------------------
  template <typename... Specs>
  struct HasInputPorts : NumInputPorts<sizeof...(Specs)>
  {
  private:
    template <size_t... Is>
    static std::vector<Dataflow::Networks::InputPortDescription>
    makeDescriptions(std::index_sequence<Is...>)
    {
      return {
        Dataflow::Networks::PortDescription(
          Dataflow::Networks::PortId(Is, std::string(Specs::name)),
          PortTypeName<typename Specs::tag>::value,
          Specs::dynamic
        )...
      };
    }

  public:
    static std::vector<Dataflow::Networks::InputPortDescription> inputPortDescription()
    {
      return makeDescriptions(std::index_sequence_for<Specs...>{});
    }
  };

  template <typename... Specs>
  struct HasOutputPorts : NumOutputPorts<sizeof...(Specs)>
  {
  private:
    template <size_t... Is>
    static std::vector<Dataflow::Networks::OutputPortDescription>
    makeDescriptions(std::index_sequence<Is...>)
    {
      return {
        Dataflow::Networks::PortDescription(
          Dataflow::Networks::PortId(Is, std::string(Specs::name)),
          PortTypeName<typename Specs::tag>::value,
          Specs::dynamic
        )...
      };
    }

  public:
    static std::vector<Dataflow::Networks::OutputPortDescription> outputPortDescription()
    {
      return makeDescriptions(std::index_sequence_for<Specs...>{});
    }
  };

} // namespace SCIRun::Modules

/*
  ============================================================================
  Port declaration styles and conversion guide
  ============================================================================

  BEFORE (old style — ModulePortDescriptionTags.h)
  -------------------------------------------------
  A module header declared ports using:
    1. A Has{N}InputPorts / Has{N}OutputPorts base with explicit type-tag args
    2. INPUT_PORT / OUTPUT_PORT macros inside the class body (one per port)
    3. INITIALIZE_PORT calls in the .cc constructor (one per port, easy to forget)

  Example — SolveLinearSystem (3 inputs, 2 outputs):

    // SolveLinearSystem.h
    class SolveLinearSystem : public Module,
      public Has3InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>,
      public Has2OutputPorts<MatrixPortTag, MatrixPortTag>
    {
    public:
      INPUT_PORT(0, Matrix, Matrix);
      INPUT_PORT(1, RHS,    Matrix);
      INPUT_PORT(2, x0,     Matrix);
      OUTPUT_PORT(0, Solution, Matrix);
      OUTPUT_PORT(1, Residual, Matrix);
      ...
    };

    // SolveLinearSystem.cc  -- constructor
    SolveLinearSystem::SolveLinearSystem() : Module(...)
    {
      INITIALIZE_PORT(Matrix);    // <-- forgetting any of these causes
      INITIALIZE_PORT(RHS);       //     a runtime error with no compile-time
      INITIALIZE_PORT(x0);        //     warning
      INITIALIZE_PORT(Solution);
      INITIALIZE_PORT(Residual);
    }

  Dynamic ports used DynamicPortTag<> as the type-tag argument:

    class BuildBEMatrix : public Module,
      public Has1InputPort<DynamicPortTag<FieldPortTag>>,
      ...

  ============================================================================
  AFTER (new style — this file, requires C++20)
  ============================================================================
  Replace the Has{N} base classes with HasInputPorts<...> / HasOutputPorts<...>.
  Port names move into the type list, so INPUT_PORT / OUTPUT_PORT still declare
  the StaticPortName members used by get_input/send_output, but the name string
  is now redundant with the spec — and INITIALIZE_PORT is gone entirely.

  Same module with the new style:

    // SolveLinearSystem.h
    class SolveLinearSystem : public Module,
      public HasInputPorts<
        MatrixPort<"Matrix">,
        MatrixPort<"RHS">,
        MatrixPort<"x0">
      >,
      public HasOutputPorts<
        MatrixPort<"Solution">,
        MatrixPort<"Residual">
      >
    {
    public:
      INPUT_PORT(0, Matrix,   Matrix);   // StaticPortName member still needed
      INPUT_PORT(1, RHS,      Matrix);   // for get_input(Matrix) / send_output
      INPUT_PORT(2, x0,       Matrix);
      OUTPUT_PORT(0, Solution, Matrix);
      OUTPUT_PORT(1, Residual, Matrix);
      ...
    };

    // SolveLinearSystem.cc  -- constructor
    SolveLinearSystem::SolveLinearSystem() : Module(...)
    {
      // No INITIALIZE_PORT calls needed.
    }

  What can be removed when converting a module:
    - Has{N}InputPorts<...> base class   → replaced by HasInputPorts<...>
    - Has{N}OutputPorts<...> base class  → replaced by HasOutputPorts<...>
    - Every INITIALIZE_PORT(...) call    → gone, no replacement needed
    - inputPort{N}Name() static methods  → generated by the old INPUT_PORT macro;
                                           IPortDescriber no longer calls them
                                           once the module uses HasInputPorts

  What stays:
    - INPUT_PORT / OUTPUT_PORT macros    → still needed for StaticPortName members

  ============================================================================
  Port spec styles — all of these are equivalent for a dynamic Field port:
  ============================================================================

    Dynamic<FieldPort<"Inputs">>                  // preferred: explicit wrapper
    FieldPort<"Inputs", true>                     // alias + explicit bool
    PortSpec<"Inputs", DynamicPortTag<FieldPortTag>>  // old DynamicPortTag wrapper

  For a static port these are all equivalent:

    FieldPort<"Input">                            // preferred
    PortSpec<"Input", FieldPortTag>               // explicit PortSpec
    PortSpec<"Input", FieldPortTag, false>        // explicit PortSpec + bool

  ============================================================================
  Full example — ShowField (1 static input, 1 static output):
  ============================================================================

    public HasInputPorts<FieldPort<"Field">>,
    public HasOutputPorts<GeometryPort<"SceneGraph">>

  ============================================================================
  Full example — module with mixed static and dynamic inputs:
  ============================================================================

    public HasInputPorts<
      MatrixPort<"Stiffness">,
      MatrixPort<"RHS">,
      Dynamic<FieldPort<"BoundaryConditions">>
    >,
    public HasOutputPorts<MatrixPort<"Solution">>

  ============================================================================
  Full example — no inputs (e.g. ReadField):
  ============================================================================

    public HasInputPorts<>,
    public HasOutputPorts<FieldPort<"Field">>

*/

#endif
