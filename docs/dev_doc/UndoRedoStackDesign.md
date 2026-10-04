# Undo/Redo Stack Redesign

**Relates to:** Issues #41 (Global State Manager), #221 (History: store diffs not full state), #223 (Undo position bug on module-with-connections removal), #300 (Performance of undo/redo when network.size > 60), #393 (Algorithm set/get parameters needs hook for Global State Manager)

---

## Problem Summary

The current provenance/undo system has three compounding failures:

1. **Missing event coverage.** Module parameter changes are never recorded. The
   `connectProvenanceStateChanged` signal fires correctly with old/new values, but the
   subscriber only emits `LOG_TRACE` and discards the data. There is no
   `ModuleStateChangedProvenanceItem` class.

2. **Broken Python-string approach for connections.** `ConnectionAddedProvenanceItem`
   generates `scirun_connect_modules("<connection-id-string>")` — a single opaque
   string — but the actual Python API requires four arguments:
   `connect(moduleIdFrom, int fromPortIndex, moduleIdTo, int toPortIndex)`. The
   `#if 0` block in its own constructor documents the failed attempt to extract port
   indices. Connection undo/redo does not work.

3. **ID mutation breaks sequences.** When undoing a module removal,
   `scirun_add_module("ReadMatrix")` re-adds the module with a fresh
   auto-incremented ID (`ReadMatrix:2` instead of the original `ReadMatrix:0`). Any
   subsequent connection operations in the same undo sequence reference the original
   (now stale) ID. The `mostRecentAddModuleId()` hack is a single-slot variable that
   cannot handle sequences involving more than one re-added module.

4. **Performance scales with network size, not operation size.** Every
   `ProvenanceItemBase` stores a full `NetworkFileHandle` XML snapshot regardless of
   what changed. Undo/redo dispatches through the Python interpreter. Measured cost:
   ~3 s for 64 modules, ~6 s for 120 modules (issue #300).

5. **Dead stub code.** `ModuleAddCommand` / `NetworkCommands.cc` — the
   `RedoableCommand` hierarchy that was the right design — throws `"not implemented"`
   for all three methods and has never been touched.

---

## Design Goals

| Goal | Constraint |
|---|---|
| Cover all editable events | module add/remove, connection add/remove, move, parameter change |
| Delta-only storage | no per-item XML snapshot; store only what changed |
| C++ execution | no Python interpreter on the undo/redo hot path |
| Stable IDs across sequences | undo → redo → undo of multi-step sequences must be consistent |
| Coalesce noisy events | slider/spinbox changes must merge into one undo step |
| Composite steps | multi-select delete must be one undo step |
| Keep the ProvenanceWindow UI | list, click-to-inspect, undo/redo/clear buttons stay |
| Keyboard shortcuts | Ctrl+Z / Ctrl+Y wire to the same stack |

---

## Architecture Overview

```
NetworkEditor (Qt GUI)
     │  signals: moduleAdded, moduleRemoved, connectionAdded,
     │           connectionRemoved, moduleMoved, stateChanged
     ▼
GuiActionProvenanceConverter           ← keep this; change what it creates
     │  Q_EMIT commandRecorded(cmd)
     ▼
NetworkUndoStack                       ← replaces ProvenanceManager
     │  push / undo / redo / clear
     ▼
ProvenanceWindow                       ← adapt display; keep UX
```

The key change: `GuiActionProvenanceConverter` creates `NetworkCommand` objects
instead of `ProvenanceItem` objects. `NetworkCommand` objects execute C++ controller
calls directly; no Python strings are stored or interpreted.

---

## Core Interface

```cpp
// src/Dataflow/Engine/Controller/NetworkUndoCommands.h

class NetworkCommand
{
public:
  virtual ~NetworkCommand() = default;
  virtual void undo() = 0;
  virtual void redo() = 0;  // re-executes the operation
  virtual std::string description() const = 0;

  // Optional: return true and mutate *this to absorb `next`.
  // Used for coalescing move and parameter-change events.
  virtual bool tryMerge(const NetworkCommand& next) { return false; }
};

using NetworkCommandHandle = SharedPointer<NetworkCommand>;
```

`redo()` is what the current system calls `execute()`. Every concrete command
implements both directions entirely in C++, calling the existing
`NetworkEditorController` API.

---

## Concrete Commands

### AddModuleCommand

```
Captured at record time:  moduleName (string)
Captured after first redo: assignedId (ModuleId)
```

- `redo()` — calls `controller_.addModule(moduleName_)`, stores the returned
  `ModuleId` as `assignedId_`. On all subsequent redos it calls
  `controller_.addModuleWithId(moduleName_, assignedId_)` (see §ID Stability).
- `undo()` — calls `controller_.removeModule(assignedId_)`.

### RemoveModuleCommand

```
Captured at record time:
  moduleId, moduleName, position (QPointF),
  savedState (ModuleStateHandle deep copy),
  connections (vector<ConnectionDescription>)
```

Removing a module implicitly removes all its connections. This command must
capture those connections before the removal executes so that `undo()` can
restore them. The connections are re-added as a sub-sequence in `undo()` (see
§Composite Commands for the multi-select case).

- `undo()` — calls `controller_.addModuleWithId(moduleName_, moduleId_)`, sets
  position, restores `savedState_`, then calls `controller_.requestConnection`
  for each captured connection.
- `redo()` — calls `controller_.removeModule(moduleId_)`.

### AddConnectionCommand

```
Captured at record time:
  fromModuleId, fromPortIndex (int),
  toModuleId,   toPortIndex   (int)
```

Port **indices** are captured at record time by calling
`port->getIndex()` inside `GuiActionProvenanceConverter::connectionAdded()`,
where the live port objects are available. This sidesteps the port-ID-to-index
conversion problem that broke the Python approach.

- `undo()` — calls `controller_.removeConnection(ConnectionId::create(desc_))`.
- `redo()` — looks up ports by index on the live modules and calls
  `controller_.requestConnection(from, to)`.

### RemoveConnectionCommand

Symmetric to `AddConnectionCommand`.

### MoveModuleCommand

```
Captured at record time:  moduleId, oldPos, newPos
```

- `undo()` — calls `controller_.moveModule(id_, oldPos_)`.
- `redo()` — calls `controller_.moveModule(id_, newPos_)`.
- `tryMerge(next)` — if `next` is a `MoveModuleCommand` for the same module,
  update `newPos_` to `next.newPos_` and return `true`. This collapses a drag
  sequence into a single undo step.

### SetModuleParameterCommand

```
Captured at record time:  moduleId, parameterName, oldValue, newValue
```

- `undo()` — calls `module->get_state()->setValue(paramName_, oldValue_)`.
- `redo()` — calls `module->get_state()->setValue(paramName_, newValue_)`.
- `tryMerge(next)` — if `next` is a `SetModuleParameterCommand` for the same
  `(moduleId, parameterName)`, update `newValue_` to `next.newValue_` and return
  `true`. This collapses a slider drag into a single undo step.

The `connectProvenanceStateChanged` lambda in `NetworkEditor.cc` (currently just
`LOG_TRACE`) is completed to emit a `SetModuleParameterCommand` through
`GuiActionProvenanceConverter`.

### CompositeNetworkCommand

```
Holds: vector<NetworkCommandHandle> steps_
```

Used for multi-select delete (and any future batch operation).

- `undo()` — iterates `steps_` in **reverse** order, calls `undo()` on each.
- `redo()` — iterates `steps_` in forward order, calls `redo()` on each.
- `description()` — e.g. `"Remove 5 modules"`.

Multi-select delete assembles a `CompositeNetworkCommand` from one
`RemoveConnectionCommand` per affected connection (in the order they are
removed) followed by one `RemoveModuleCommand` per selected module.

---

## NetworkUndoStack

Replaces `ProvenanceManager`. Uses `std::vector` + a cursor index rather than
two `std::stack`s, which lets the `ProvenanceWindow` support click-to-jump
navigation and makes the undo/redo/clear logic straightforward.

```cpp
class NetworkUndoStack
{
public:
  explicit NetworkUndoStack(size_t maxDepth = 50);

  // Record a new command. Discards any redo history above the cursor.
  // If the new command merges with the top item (tryMerge returns true),
  // the stack is not grown.
  void push(NetworkCommandHandle cmd);

  bool undo();   // steps cursor back one, calls cmd->undo()
  bool redo();   // steps cursor forward one, calls cmd->redo()
  void clear();

  bool canUndo() const;
  bool canRedo() const;
  std::string undoDescription() const;  // description of the command that would be undone
  std::string redoDescription() const;

  size_t undoSize() const;
  size_t redoSize() const;
  size_t maxDepth() const;
  void setMaxDepth(size_t max);

  // For ProvenanceWindow list display
  const std::vector<NetworkCommandHandle>& history() const;
  int currentIndex() const;  // cursor position

private:
  std::vector<NetworkCommandHandle> history_;
  int cursor_ {-1};   // index of last executed command; -1 = nothing done
  size_t maxDepth_;
};
```

`push()` logic:

1. Drop all items above `cursor_` (discards redo history on new action).
2. Call `history_.back()->tryMerge(*cmd)`. If `true`, do not push; the top
   command has absorbed the new one.
3. Otherwise append. If `history_.size() > maxDepth_`, drop `history_.front()`
   and decrement `cursor_`.
4. Advance `cursor_`.

---

## ID Stability

The fundamental problem: `addModule("ReadMatrix")` auto-assigns an ID like
`ReadMatrix:2`; the original may have been `ReadMatrix:0`. Connections recorded
against the original ID break on redo.

**Solution: `addModuleWithId`**

Add one method to `NetworkEditorController`:

```cpp
ModuleHandle addModuleWithId(const std::string& name, const ModuleId& requestedId);
```

Behaviour: attempts to register the new module under `requestedId`. If that ID
is already in use (e.g. the user manually added another copy before redoing),
falls back to auto-assignment and logs a warning. This case is inherently
ambiguous; a warning is the right response.

`AddModuleCommand` and `RemoveModuleCommand::undo()` both use this method after
the first execution, so every redo of an add reliably produces the same ID that
downstream connection commands expect.

`mostRecentAddModuleId()` and its single-slot implementation in `PythonImpl` can
be removed once this is in place.

---

## Parameter Change Coalescing

`connectProvenanceStateChanged` fires on every `setValue` call, which includes
every tick of a slider or spinner. Without coalescing, a single slider drag
could push hundreds of commands onto the stack.

`GuiActionProvenanceConverter` (or a thin helper owned by it) maintains:

```cpp
QTimer* coalesceTimer_;                            // single-shot, 300 ms
SharedPointer<SetModuleParameterCommand> pending_; // one per (module, param) in flight
```

On each `stateChanged(moduleId, paramName, oldV, newV)`:

1. If `pending_` is null, create it with `oldV` as the baseline old value.
2. Update `pending_->newValue_` to `newV`.
3. Restart `coalesceTimer_`.

When `coalesceTimer_` fires: emit `commandRecorded(pending_)`, clear `pending_`.

This ensures that after a slider drag, one undo step restores the full before
value, regardless of how many intermediate ticks fired.

The 300 ms window is a good default but should be a user preference (same
settings panel as the undo depth spinner that already exists in
`ProvenanceWindow`).

---

## Guard Against Re-recording During Undo/Redo

The existing `provenanceManagerModifyingNetwork_` boolean in
`GuiActionProvenanceConverter` already handles this correctly. All signals from
`NetworkEditor` that fire during an undo/redo operation are gated by this flag.
No design change needed here.

The flag is set via `Q_EMIT modifyingNetwork(true/false)` in
`ProvenanceWindow::undo()` / `redo()`, which connects to
`GuiActionProvenanceConverter::networkBeingModifiedByProvenanceManager()`. This
chain is kept as-is.

---

## ProvenanceWindow Adaptation

The `ProvenanceWindow` UI requires minimal changes:

- `addProvenanceItem(ProvenanceItemHandle)` → `addCommand(NetworkCommandHandle)`.
  The list item label comes from `cmd->description()`.
- The XML preview pane (currently showing the full network XML snapshot stored in
  each `ProvenanceItemBase`) is removed or repurposed. The per-item snapshot was
  only meaningful in the memento approach, which is being abandoned.
- `undo()` / `redo()` / `undoAll()` / `redoAll()` delegate to `NetworkUndoStack`
  rather than `ProvenanceManager`.
- The `itemMaxSpinBox_` already wires to `setMaxDepth`; keep it.
- Click-to-jump: clicking a list item calls `undoStack_.jumpTo(index)`, which
  calls `undo()` or `redo()` repeatedly until the cursor reaches that index.

---

## Files

### New

| File | Purpose |
|---|---|
| `src/Dataflow/Engine/Controller/NetworkUndoCommands.h` | All concrete command classes |
| `src/Dataflow/Engine/Controller/NetworkUndoCommands.cc` | Implementations |
| `src/Dataflow/Engine/Controller/NetworkUndoStack.h` | Stack class |
| `src/Dataflow/Engine/Controller/NetworkUndoStack.cc` | Stack implementation |

### Modified

| File | Change |
|---|---|
| `src/Dataflow/Engine/Controller/NetworkEditorController.h/.cc` | Add `addModuleWithId()` |
| `src/Interface/Application/NetworkEditor.cc` | Complete `connectProvenanceStateChanged` lambda (lines 475–479) to emit commands |
| `src/Interface/Application/ProvenanceWindow.h/.cc` | Replace `ProvenanceManager` with `NetworkUndoStack`; adapt list display |
| `src/Interface/Application/ProvenanceWindow.h` | Add `GuiActionProvenanceConverter::moduleStateChanged` slot and `NetworkEditor::moduleStateChanged` signal |
| `src/Interface/Application/SCIRunMainWindowSetup.cc` | Wire `commandRecorded` signal to `NetworkUndoStack::push` |

### Deleted / Emptied

| File | Reason |
|---|---|
| `src/Dataflow/Engine/Controller/NetworkCommands.h/.cc` | Dead stub; replaced by `NetworkUndoCommands` |
| `src/Dataflow/Engine/Controller/ProvenanceItemImpl.h/.cc` | Python-string items replaced by commands |
| `src/Dataflow/Engine/Controller/ProvenanceItem.h/.cc` | Abstract base no longer needed |
| `src/Dataflow/Engine/Controller/ProvenanceManager.h/.cc` | Replaced by `NetworkUndoStack` |
| `src/Dataflow/Engine/Python/NetworkEditorPythonInterface.h` | Remove `mostRecentAddModuleId()` pure virtual |
| `src/Dataflow/Engine/Controller/PythonImpl.h/.cc` | Remove `mostRecentAddModuleId_` field and its setter |

`ProvenanceItemFactory.h/.cc` should also be audited; if it only creates
`ProvenanceItemImpl` types it can be deleted.

---

## Performance Impact

| Metric | Current | New |
|---|---|---|
| Storage per undo item | full XML snapshot (~100 KB for large networks) | delta only (bytes to tens of bytes) |
| Undo/redo execution | Python interpreter + network reload | direct C++ controller call |
| 64-module undo latency | ~3 s (measured, issue #300) | ~0 ms (no reload, no Python) |
| Parameter change items | 0 (not recorded) | 1 per coalesced edit |
| Connection undo | broken | works |

---

## What This Resolves

| Issue | Status after this work |
|---|---|
| #41 — Global State Manager | All five event types covered in one unified C++ stack |
| #221 — Store diffs not full network state | Each command is a pure delta; no `NetworkFileHandle` snapshot stored per item |
| #223 — Undo of module-with-connections loses position | `RemoveModuleCommand` captures `position` explicitly at record time; `undo()` restores it directly; no XML round-trip, no invalid-state window |
| #300 — Performance > 60 modules | Delta commands; no network reload on undo/redo |
| Connection undo (broken Python API) | Port indices captured at record time; no Python |
| Module parameter undo (never implemented) | `SetModuleParameterCommand` + coalescing |
| ID mutation in sequences | `addModuleWithId` makes IDs stable across undo/redo |
| #393 — Algorithm set/get parameters hook for GSM | `connectProvenanceStateChanged` already exists as the signal; completing its subscriber to emit `SetModuleParameterCommand` is the "hook/signal/slot" the issue requested |
| `ModuleAddCommand` dead stubs | Replaced by working implementation |
