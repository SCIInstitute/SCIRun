# v4 → v5 idiom translation table

Mined from the 77 dormant `Legacy/` sources (v4) diffed against the ~90 active
ported modules (v5) in the same directories. Rows are ordered by how many
dormant files contain the idiom (the count in parentheses = files affected /
total occurrences), so work top-down for the highest payoff.

Regenerate the frequency evidence with `reference/idiom_scan.py` (see bottom).

> **C++20 note.** Rows tagged **[C++20]** show the *conceptual* mapping only; the
> exact final macro/spelling is being changed by branch
> `module-descriptors-metaprogram` (variadic ports via C++20 string NTTPs,
> `getRequiredInput_`/`sendOutput_`, 4-arg `MODULE_TRAITS_AND_INFO`). Do **not**
> hand-freeze those spellings until that branch merges — the file-generation
> templates (Wave 2) will pin them. Everything untagged is stable today.

---

## 1. Structural boilerplate (present in ~all files)

| v4 | v5 | Notes |
|---|---|---|
| `DECLARE_MAKER(Foo)` (74/74) | *(delete)* | Registration is done by the `.module` config + factory. |
| `class Foo : public Module { ... }` in the `.cc` (69/69) | Move the class to `Foo.h`; `.cc` keeps only method bodies | v4 rarely had a header — create one. |
| `#include <Dataflow/Network/Ports/FieldPort.h>` etc. (72/106) | *(delete all `Ports/*Port.h` includes)* | Ports are declared by `INPUT_PORT`/`OUTPUT_PORT` macros **[C++20]**. |
| `Foo::Foo(GuiContext* ctx) : Module("Foo", ctx, ...)` | `Foo::Foo() : Module(staticInfo_)` (add `,false` if no UI) | `staticInfo_` comes from the info macro. |
| *(none — v4 had no info macro)* | `MODULE_TRAITS_AND_INFO(NoAlgoOrUI \| ModuleHasUI \| ModuleHasAlgorithm \| ModuleHasUIAndAlgorithm)` in the header | Replaces the old `MODULE_INFO_DEF`. **[C++20]** picks the 4-arg form. |

## 2. Ports & dataflow (getRequiredInput / sendOutput)

| v4 | v5 | Notes |
|---|---|---|
| `get_input_handle("Name", h, true)` (65/115) | `auto h = getRequiredInput(Name);` | Optional port → `getOptionalInput(Name)` (returns a `boost::optional`-like; check truthiness). **[C++20]** |
| `send_output_handle("Name", h)` (70/130) | `sendOutput(Name, h);` | When the value came straight from an `Algo` output: `sendOutputFromAlgorithm(Name, output);` **[C++20]** |
| Port declared only via `add_input_port` in ctor | `INPUT_PORT(0, Name, Datatype)` / `OUTPUT_PORT(0, Name, Datatype)` in the header | Datatype is the tag (e.g. `Field`, `Matrix`, `String`, `ColorMap`). **[C++20]** |
| `if (!oport_cached("Name"))` re-exec guards | *(delete)* | v5 caching is handled by the engine. |

## 3. GUI state (the #1 conversion surface: GuiVar → state variables)

| v4 | v5 | Notes |
|---|---|---|
| `GuiInt gui_size_;` / `GuiDouble` / `GuiString` / `GuiInt` members (74/481) | *(delete the members)* | State lives in the module's `ModuleStateHandle`, keyed by name. |
| ctor init `gui_size_(ctx->subVar("size"))` | *(delete)* | — |
| `gui_size_.get()` (34/165) | `get_state()->getValue(Parameters::Size).toInt()` | `.toDouble()`, `.toBool()`, `.toString()` for the others. |
| `gui_size_.set(v); gui_size_.reset();` | `get_state()->setValue(Parameters::Size, v);` | — |
| *(v4 defaults set in Tcl/ctor)* | `setStateDefaults()` sets every state var | Use `setStateBoolFromAlgo(Parameters::X)` / `…Double…` / `…Int…` when an `Algo` owns the default; else `get_state()->setValue(Parameters::X, dflt)`. |
| Tcl var names (string literals) | `Parameters::X` (an `AlgorithmParameterName`) | Declare names once in the algo header (or module) — shared by module + dialog + algo. |

## 4. Logging & progress

| v4 | v5 | Notes |
|---|---|---|
| `error("msg")` (37/146) | `error("msg")` | Still a `Module` method — usually unchanged. |
| `warning("msg")` / `remark("msg")` | `warning("msg")` / `remark("msg")` | Unchanged. |
| `update_state(Executing)` / `update_progress(x)` (44/47) | *(delete `update_state`)*; `update_progress(x)` still valid | Executing/Completed transitions are automatic in v5. |
| `gui->execute()` / `ctx->reset()` (8/11) | *(delete)* | No Tcl round-trips. |
| `TCLInterface::…` / `Tcl_*` (17/65) | *(delete / re-express)* | Any remaining Tcl coupling must be reworked as state or dialog logic. |

## 5. Handles, fields, meshes

| v4 | v5 | Notes |
|---|---|---|
| `if (h.get_rep() == 0)` (27/84) | `if (!h)` | Works for all handle types. |
| `out = in; out.detach();` | `FieldHandle out(in->clone());` | Copy field, detach data. |
| `out = in; out.detach(); out->mesh_detach();` | `FieldHandle out(in->deep_clone());` | Copy field, detach data **and** mesh. |
| `h->generation` change tests (8/19) | *(delete)* | Recompute is engine-driven. |
| `field->get_type_description()` + dynamic compilation (`get_compile_info`, `DynamicCompilation`) | *(delete the whole dynamic-compile path)* | v5 has **no** run-time dynamic compilation. Replace with `VField`/`VMesh` virtual interface or `FieldInformation` dispatch. |
| `dynamic_cast<SomeField*>(field.get_rep())` (2/2) | `field->vfield()` / `field->vmesh()` + `FieldInformation fi(field)` | Prefer the virtual interface (`VField*` 15/76, `FieldInformation` 16/45 already appear). |

## 6. Algorithm split (when moving compute into a `*Algo`)

| v4 | v5 |
|---|---|
| free functions / private methods doing the compute | `class FooAlgo : public AlgorithmBase` with `AlgorithmOutput run(const AlgorithmInput&) const override` |
| read GUI var inside compute | `get(Parameters::X).toInt()` inside the algo; module pushes state → algo via `setAlgoIntFromState(...)` |
| return a field | `AlgorithmOutput out; out[Variables::OutputField] = field; return out;` |

---

## Escalation triggers (stop and flag — see `verification.md`)

- Dynamic-compilation path that has no `VField`/`VMesh` equivalent yet.
- Datatype the v5 port tree doesn't have (e.g. old Bundle/Nrrd corners).
- `SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED` / `SCIRUN4_CODE_TO_BE_CONVERTED_LATER`
  markers already in the file — these are known-hard (issues #2530/#2529).
- Anything under `Teem/Tend` or Visualization `Show*Texture*` — usually
  *from-scratch* (no in-tree `.cc`), not a mechanical port.

## Regenerating the evidence

`python3 .claude/skills/convert-scirun-module/reference/idiom_scan.py` rescans the
current dormant set and reprints the frequency table used to order this file.
