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
  // IsDynamic — detects DynamicPortTag / AsyncDynamicPortTag wrappers so that
  // PortSpec can infer the dynamic flag from the tag type automatically.
  // -------------------------------------------------------------------------
  template <typename Tag> struct IsDynamic : std::false_type {};
  template <typename B>   struct IsDynamic<DynamicPortTag<B>>      : std::true_type {};
  template <typename B>   struct IsDynamic<AsyncDynamicPortTag<B>> : std::true_type {};

  // -------------------------------------------------------------------------
  // PortTypeName — maps a port tag to its wire-format datatype string.
  // TagDataType  — maps a port tag to its C++ class (for getRequiredInput_ etc.).
  // Both are populated by DECLARE_PORT_TYPE below.
  // Dynamic/async wrappers forward to the underlying base tag for both traits.
  // -------------------------------------------------------------------------
  template <typename Tag> struct PortTypeName;
  template <typename Tag> struct TagDataType;

  template <typename Base>
  struct PortTypeName<DynamicPortTag<Base>> : PortTypeName<Base> {};
  template <typename Base>
  struct PortTypeName<AsyncDynamicPortTag<Base>> : PortTypeName<Base> {};

  template <typename Base>
  struct TagDataType<DynamicPortTag<Base>>      : TagDataType<Base> {};
  template <typename Base>
  struct TagDataType<AsyncDynamicPortTag<Base>> : TagDataType<Base> {};

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
  // Dynamic ports: use Dynamic<MatrixPort<"Name">> or MatrixPort<"Name", true>.
  //
  // DECLARE_PORT_TYPE(TagPrefix [, CppType]) generates:
  //   PortTypeName<TagPrefixPortTag>::value  == "TagPrefix"  (wire-format string)
  //   TagDataType<TagPrefixPortTag>::type    == Core::Datatypes::CppType
  //   TagPrefixPort<"Name">                  (alias template)
  //
  // One-argument form: CppType defaults to TagPrefix (covers most cases).
  // Two-argument form: use when the C++ class name differs from the tag prefix
  //   (Geometry→GeometryObject, OsprayGeometry→OsprayGeometryObject, Nrrd→NrrdDataType).
#define DECLARE_PORT_TYPE_IMPL(TypeName, CppType) \
  template <> struct PortTypeName<TypeName##PortTag> \
    { static constexpr const char* value = #TypeName; }; \
  template <> struct TagDataType<TypeName##PortTag> \
    { using type = Core::Datatypes::CppType; }; \
  template <FixedString Name, bool Dynamic = false> \
  using TypeName##Port = PortSpec<Name, TypeName##PortTag, Dynamic>

#define DECLARE_PORT_TYPE_1(TypeName)            DECLARE_PORT_TYPE_IMPL(TypeName, TypeName)
#define DECLARE_PORT_TYPE_2(TypeName, CppType)   DECLARE_PORT_TYPE_IMPL(TypeName, CppType)
#define DECLARE_PORT_TYPE_PICK(_1, _2, WHICH, ...) WHICH
#define DECLARE_PORT_TYPE(...) \
  DECLARE_PORT_TYPE_PICK(__VA_ARGS__, DECLARE_PORT_TYPE_2, DECLARE_PORT_TYPE_1)(__VA_ARGS__)

  DECLARE_PORT_TYPE(Matrix);
  DECLARE_PORT_TYPE(ComplexMatrix);
  DECLARE_PORT_TYPE(Scalar);
  DECLARE_PORT_TYPE(String);
  DECLARE_PORT_TYPE(Field);
  DECLARE_PORT_TYPE(Geometry,       GeometryObject);
  DECLARE_PORT_TYPE(OsprayGeometry, OsprayGeometryObject);
  DECLARE_PORT_TYPE(ColorMap);
  DECLARE_PORT_TYPE(Bundle);
  DECLARE_PORT_TYPE(Nrrd,           NrrdDataType);
  DECLARE_PORT_TYPE(Datatype);
  DECLARE_PORT_TYPE(MetadataObject);

#undef DECLARE_PORT_TYPE
#undef DECLARE_PORT_TYPE_PICK
#undef DECLARE_PORT_TYPE_1
#undef DECLARE_PORT_TYPE_2
#undef DECLARE_PORT_TYPE_IMPL

  // -------------------------------------------------------------------------
  // FindSpec<Name, Specs...> — compile-time lookup of a PortSpec by name.
  // Returns the matching spec type, or void if not found.
  // -------------------------------------------------------------------------
  template <FixedString Name, typename... Specs> struct FindSpec;
  template <FixedString Name>
  struct FindSpec<Name> { using type = void; };
  template <FixedString Name, typename First, typename... Rest>
  struct FindSpec<Name, First, Rest...> {
    using type = std::conditional_t<(First::name == Name.view()),
                                    First,
                                    typename FindSpec<Name, Rest...>::type>;
  };

  // -------------------------------------------------------------------------
  // HasInputPorts<Specs...> — variadic replacement for Has1InputPort through
  // Has7InputPorts.
  //
  // HasOutputPorts<Specs...> — variadic replacement for Has1OutputPort through
  // Has9OutputPorts.
  //
  // Both provide the same NumIPorts/NumOPorts enum and inputPortDescription()
  // / outputPortDescription() static methods as the old templates.
  //
  // HasInputPorts additionally provides compile-time lookup by name:
  //   inputPortIndex<FixedString{"Matrix"}>()  → constexpr size_t index
  //   InputDataType<FixedString{"Matrix"}>     → C++ type (e.g. Core::Datatypes::Matrix)
  // These are used by the getRequiredInput_ / getOptionalInput_ macros.
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

    template <FixedString Name>
    static constexpr size_t inputPortIndex()
    {
      size_t result = size_t(-1), i = 0;
      ([&]{ if (Specs::name == Name.view()) result = i; ++i; }(), ...);
      return result;
    }

    template <FixedString Name>
    using InputDataType = typename TagDataType<
      typename FindSpec<Name, Specs...>::type::tag
    >::type;
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

    template <FixedString Name>
    static constexpr size_t outputPortIndex()
    {
      size_t result = size_t(-1), i = 0;
      ([&]{ if (Specs::name == Name.view()) result = i; ++i; }(), ...);
      return result;
    }

    template <FixedString Name>
    using OutputDataType = typename TagDataType<
      typename FindSpec<Name, Specs...>::type::tag
    >::type;
  };

} // namespace SCIRun::Modules

// -----------------------------------------------------------------------------
// getRequiredInput_(portName)  — typed, index-safe replacement for
//   getRequiredInput(PortNameMember)
//
// getOptionalInput_(portName)  — replacement for getOptionalInput(PortNameMember)
//
// Both macros require the enclosing class to inherit from HasInputPorts<...>
// with a spec whose name matches the argument.  The port's C++ datatype and
// index are looked up at compile time; a misspelled name is a compile error.
//
// Usage (inside execute() or any member of a HasInputPorts<...> subclass):
//   auto mat   = getRequiredInput_(Matrix);   // shared_ptr<Core::Datatypes::Matrix>
//   auto field = getOptionalInput_(InputField); // optional<shared_ptr<...>>
// -----------------------------------------------------------------------------
#define getRequiredInput_(name) \
  [this]() { \
    using Self_ = std::remove_pointer_t<decltype(this)>; \
    constexpr auto portName_ = SCIRun::Modules::FixedString{#name}; \
    using DataType_ = typename Self_::template InputDataType<portName_>; \
    constexpr size_t idx_ = Self_::template inputPortIndex<portName_>(); \
    return this->template getRequiredInputAtIndex<DataType_>( \
      SCIRun::Dataflow::Networks::PortId(idx_, #name)); \
  }()

#define getOptionalInput_(name) \
  [this]() { \
    using Self_ = std::remove_pointer_t<decltype(this)>; \
    constexpr auto portName_ = SCIRun::Modules::FixedString{#name}; \
    using DataType_ = typename Self_::template InputDataType<portName_>; \
    constexpr size_t idx_ = Self_::template inputPortIndex<portName_>(); \
    return this->template getOptionalInputAtIndex<DataType_>( \
      SCIRun::Dataflow::Networks::PortId(idx_, #name)); \
  }()

// -----------------------------------------------------------------------------
// sendOutput_(portName, data)  — typed replacement for sendOutput(PortNameMember, data).
//
// Requires the enclosing class to inherit from HasOutputPorts<...> with a spec
// whose name matches the argument.  The port's C++ datatype and index are
// resolved at compile time.  A static_assert enforces that the data type is
// compatible with the declared port type, matching the check in sendOutput().
//
// Usage:
//   sendOutput_(SceneGraph, geomHandle);
// -----------------------------------------------------------------------------
#define sendOutput_(name, data) \
  [this, &data]() { \
    using Self_ = std::remove_pointer_t<decltype(this)>; \
    constexpr auto portName_ = SCIRun::Modules::FixedString{#name}; \
    using PortType_ = typename Self_::template OutputDataType<portName_>; \
    using DataType_ = typename std::remove_reference_t<decltype(data)>::element_type; \
    static_assert(std::is_base_of_v<PortType_, DataType_>, \
      "sendOutput_: data type is not compatible with declared port type"); \
    constexpr size_t idx_ = Self_::template outputPortIndex<portName_>(); \
    this->send_output_handle( \
      SCIRun::Dataflow::Networks::PortId(idx_, #name), data); \
  }()

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
    - INPUT_PORT / OUTPUT_PORT macros    → can be removed once execute() is also
                                           converted to use getRequiredInput_ below
    - StaticPortName member variables    → same; only needed for old-style access

  What stays:
    - Nothing — a fully converted module has only the HasInputPorts /
      HasOutputPorts base classes and the macros in execute()

  ============================================================================
  getRequiredInput_ / getOptionalInput_ — replacing getRequiredInput(Member)
  ============================================================================
  Once the module uses HasInputPorts<...>, the execute() body can use these
  macros instead of getRequiredInput(PortNameMember).  The port's C++ type and
  index are resolved at compile time; a misspelled name is a compile error.

    // Old style — requires StaticPortName member + INITIALIZE_PORT:
    auto mat = getRequiredInput(Matrix);

    // New style — only requires HasInputPorts<MatrixPort<"Matrix">, ...>:
    auto mat = getRequiredInput_(Matrix);

  Once execute() is converted, INPUT_PORT / OUTPUT_PORT and INITIALIZE_PORT
  are all gone.  A fully converted module has only the HasInputPorts /
  HasOutputPorts base classes in the header, and getRequiredInput_() /
  sendOutput_() in execute() — no other port boilerplate anywhere.

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
