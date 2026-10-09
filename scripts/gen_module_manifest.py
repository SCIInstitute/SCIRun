#!/usr/bin/env python3
"""Generate a machine-readable manifest of SCIRun modules: ports, state variables,
defaults, and legal option values.

Everything here comes from the source tree, because a running SCIRun cannot
supply it:

  - Ports are unreachable from Python. PyModule/PyPort are registered with
    Boost.Python, but no exposed function ever returns one.
  - The legal value sets for option parameters exist only in the source, as
    addOption's third argument ("Negative|Both|Positive").
  - Defaults and types would need a built, running SCIRun; this runs on a
    checkout.

scirun_dump_module_state returns the live state as a dict, which makes it a
cross-check on the state half of this manifest, but not a substitute.

Only modules the factory registers are listed; sources no CMakeLists.txt
compiles are ignored, since the dormant v4 ports reuse live class names.

Usage:
    scripts/gen_module_manifest.py --out module_manifest.json
    scripts/gen_module_manifest.py --gaps module_gaps.md
"""

import argparse
import json
import os
import re
import sys
from collections import defaultdict

# --------------------------------------------------------------------------
# C++ text handling
# --------------------------------------------------------------------------

_STRIP = re.compile(
    r'("(?:\\.|[^"\\])*")'      # string literal, kept
    r"|('(?:\\.|[^'\\])*')"     # char literal, kept
    r'|(//[^\n]*)'              # line comment, dropped
    r'|(/\*.*?\*/)',            # block comment, dropped
    re.DOTALL,
)


def strip_comments(text):
    """Drop comments, preserve string literals and line count."""
    def sub(m):
        if m.group(1) or m.group(2):
            return m.group(0)
        return '\n' * m.group(0).count('\n')
    return _STRIP.sub(sub, text)


def brace_body(text, open_idx):
    """Body between the brace at open_idx and its match. Returns (body, end_idx)."""
    depth = 0
    i = open_idx
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"' or c == "'":
            quote = c
            i += 1
            while i < n and text[i] != quote:
                i += 2 if text[i] == '\\' else 1
        elif c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                return text[open_idx + 1:i], i
        i += 1
    return text[open_idx + 1:], n


def split_args(arglist):
    """Split a C++ argument list on top-level commas."""
    args, depth, cur, i, n = [], 0, [], 0, len(arglist)
    while i < n:
        c = arglist[i]
        if c in '"\'':
            quote = c
            cur.append(c)
            i += 1
            while i < n and arglist[i] != quote:
                if arglist[i] == '\\':
                    cur.append(arglist[i])
                    i += 1
                cur.append(arglist[i])
                i += 1
            cur.append(quote)
        elif c in '(<[{':
            depth += 1
            cur.append(c)
        elif c in ')>]}':
            depth -= 1
            cur.append(c)
        elif c == ',' and depth == 0:
            args.append(''.join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    if cur:
        args.append(''.join(cur).strip())
    return [a for a in args if a]


def call_args(text, paren_idx):
    """Argument list of the call whose opening '(' is at paren_idx.

    Takes the paren itself rather than the callee, so a receiver that is a call
    (`get_state()->setValue(...)`) does not get mistaken for the argument list.
    """
    open_paren = text.find('(', paren_idx)
    if open_paren < 0:
        return []
    depth, i, n = 0, open_paren, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            quote = c
            i += 1
            while i < n and text[i] != quote:
                i += 2 if text[i] == '\\' else 1
        elif c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return split_args(text[open_paren + 1:i])
        i += 1
    return []


_STR_LIT = re.compile(r'^"((?:\\.|[^"\\])*)"$')


_WRAPPER = re.compile(r'^(?:std::)?string\s*\((.*)\)$|^static_cast\s*<[^>]*>\s*\((.*)\)$',
                      re.DOTALL)


def cpp_literal(expr, _depth=0):
    """Convert a C++ literal to a JSON value. Returns (value, is_literal)."""
    e = expr.strip()
    # `std::string("x")` and `static_cast<int>(3)` are literals wearing a hat.
    if _depth < 4:
        m = _WRAPPER.match(e)
        if m:
            inner = m.group(1) if m.group(1) is not None else m.group(2)
            if m.group(1) is not None and not inner.strip():
                return '', True
            value, literal = cpp_literal(inner, _depth + 1)
            if literal:
                return value, True
    m = _STR_LIT.match(e)
    if m:
        return m.group(1).encode().decode('unicode_escape', 'replace'), True
    if e in ('true', 'false'):
        return e == 'true', True
    if re.fullmatch(r'[+-]?\d+[uUlL]*', e):
        return int(re.sub(r'[uUlL]+$', '', e)), True
    if re.fullmatch(r'[+-]?(\d+\.\d*|\.\d+|\d+)([eE][+-]?\d+)?[fF]?', e):
        try:
            return float(e.rstrip('fF')), True
        except ValueError:
            pass
    if e in ('nullptr', 'NULL'):
        return None, True
    return e, False


_CAST = re.compile(r'^static_cast\s*<\s*([\w:\s]+?)\s*>\s*\(')
_CAST_TYPE = {'int': 'int', 'unsigned': 'int', 'unsigned int': 'int', 'size_t': 'int',
              'long': 'int', 'double': 'double', 'float': 'double', 'bool': 'bool',
              'std::string': 'string'}
CONST_DEF = re.compile(
    r'\b(?:static\s+)?(?:constexpr|const)\s+(?:static\s+)?'
    r'(?:int|double|bool|float|size_t|long|unsigned(?:\s+int)?)\s+(?:\w+::)*(\w+)\s*=\s*([^;{]+);')


def default_fields(expr, consts):
    """{'default', 'type'} for a literal or named constant; else the expression
    plus whatever type a cast or string construction gives away."""
    value, literal = cpp_literal(expr)
    if not literal:
        m = re.fullmatch(r'(?:[\w:]*::)?(\w+)', expr.strip())
        if m and m.group(1) in consts:
            value, literal = consts[m.group(1)], True
    if literal:
        return {'default': value, 'type': _JSON_TYPE[type(value)]}
    e = expr.strip()
    m = _CAST.match(e)
    t = _CAST_TYPE.get(m.group(1)) if m else None
    if t is None and (e.startswith('"') or re.match(r'(?:std::)?string\s*\(', e)):
        t = 'string'
    return {'defaultExpr': value, 'type': t or 'unknown'}


def scan_constants(src):
    """Named numeric constants, e.g. SetupTDCSAlgorithm::max_number_of_electrodes.
    A name defined with two different values is dropped as ambiguous."""
    found, clash = {}, set()
    for root in ('Core', 'Modules'):
        for path in walk(os.path.join(src, root), {'.cc', '.h'}):
            for m in CONST_DEF.finditer(read(path)):
                value, literal = cpp_literal(m.group(2))
                if not literal:
                    continue
                if m.group(1) in found and found[m.group(1)] != value:
                    clash.add(m.group(1))
                found[m.group(1)] = value
    return {k: v for k, v in found.items() if k not in clash}


# --------------------------------------------------------------------------
# Name resolution
# --------------------------------------------------------------------------

# ALGORITHM_PARAMETER_DECL(X) makes a parameter whose string name is "X", so a
# qualified Parameters::X resolves to itself. Class-scoped names are declared as
# `const AlgorithmParameterName C::X("X")` and are looked up in PARAM_ALIASES.
_QUALIFIED = re.compile(r'^(?:[\w:]*::)?(\w+)$')


_INDEXED = re.compile(r'^(?:[\w:]*::)?(\w+)\s*\[[^\]]*\]$')


def resolve_param(expr, aliases, arrays=None):
    """Resolve a parameter-name expression to a list of state keys.

    `Ident[i]` inside a loop expands to every name in that array, since the loop
    bound is not knowable here.
    """
    e = expr.strip()
    m = _QUALIFIED.match(e)
    if m:
        ident = m.group(1)
        return [aliases.get(ident, ident)]
    m = _INDEXED.match(e)
    if m and arrays:
        names = arrays.get(m.group(1))
        if names:
            return list(names)
    return None


# --------------------------------------------------------------------------
# Source scanning
# --------------------------------------------------------------------------

PORT_MACRO = re.compile(r'\b(INPUT_PORT_DYNAMIC|INPUT_PORT|OUTPUT_PORT)\s*\(')
CLASS_DEF = re.compile(
    r'\bclass\s+(?:SCISHARE\s+)?(\w+)\s*(?:final\s*)?(?::([^{;]*))?\{')
BASE_CLASS = re.compile(r'\bpublic\s+([\w:]+)')
# Definitions may be out of line and templated: GenericReader<H, P>::setStateDefaults()
STATE_DEFAULTS_DEF = re.compile(
    r'\b(\w+)\s*(?:<[^>]*>)?\s*::\s*setStateDefaults\s*\(\s*\)\s*'
    r'(?:const\s*)?(?:override\s*)?(?:final\s*)?\{')
MODULE_INFO_DEF = re.compile(r'\bMODULE_INFO_DEF\s*\(')
PARAM_ALIAS_DEF = re.compile(
    r'\bAlgorithmParameterName\s+(?:\w+::)*(\w+)::(\w+)\s*\(\s*"([^"]*)"\s*\)')
PARAM_ARRAY_DEF = re.compile(
    r'\bAlgorithmParameterName\s+(?:\w+::)*(\w+)::(\w+)\s*\[\s*\]\s*=\s*\{')
# Batched form of setState*FromAlgo: copyAlgoToState({Parameters::A, Parameters::B}).
# Type comes from the algorithm's addParameter/addOption at merge time.
COPY_ALGO_TO_STATE = re.compile(r'\bcopyAlgoToState\s*\(')
STATE_FROM_ALGO = re.compile(
    r'\bsetState(String|Int|Double|Bool)FromAlgo(Option)?\s*\(')
# Receiver is either a local `state` variable or a direct `get_state()` call.
SET_VALUE = re.compile(
    r'(?:\w+|\bget_state\s*\(\s*\))\s*->\s*set(Transient)?Value\s*\(')
ADD_MODULE_DESC = re.compile(r'\baddModuleDesc\s*<\s*([\w:]+)\s*>\s*\(')
# GenericWriter stores its state key in a member seeded from a ctor parameter,
# so the real key ("Filename") only exists at the subclass's base initializer.
CTOR_DEF = re.compile(r'\b(\w+)\s*(?:<[^>]*>)?\s*::\s*\1\s*\(')
MEMBER_INIT = re.compile(r'\b(\w+_)\s*\(\s*(\w+)\s*\)')
BASE_INIT = re.compile(r':\s*(?:\w+::)*(my_base|\w+)\s*\(')
ALGO_CTOR = re.compile(r'\b(\w+)::\1\s*\(\s*\)\s*(?::[^{]*)?\{')
ADD_PARAM = re.compile(r'\badd(Parameter|Option)\s*\(')

TYPE_OF = {'String': 'string', 'Int': 'int', 'Double': 'double', 'Bool': 'bool'}


# .cc files no CMakeLists.txt compiles: the dormant v4 ports (commented out in
# Legacy/), Examples/, Template/, and orphans. They share class names with live
# modules, so scanning them would mix dead code into live entries.
_NOT_BUILT = set()


def find_unbuilt_sources(src):
    built = set()
    for dirpath, _, filenames in os.walk(src):
        if 'Externals' in dirpath or 'CMakeLists.txt' not in filenames:
            continue
        with open(os.path.join(dirpath, 'CMakeLists.txt'), errors='replace') as fh:
            text = re.sub(r'#[^\n]*', '', fh.read())
        for tok in re.findall(r'[\w${}./+-]+\.cc\b', text):
            tok = tok.replace('${CMAKE_CURRENT_SOURCE_DIR}/', '')
            built.add(os.path.normpath(os.path.abspath(os.path.join(dirpath, tok))))
    return {p for p in (os.path.normpath(os.path.abspath(q)) for q in walk(src, {'.cc'}))
            if p not in built}


def walk(root, exts):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in ('Externals', 'Tests', '.git')]
        for fn in filenames:
            path = os.path.join(dirpath, fn)
            if os.path.splitext(fn)[1] in exts and \
                    os.path.normpath(os.path.abspath(path)) not in _NOT_BUILT:
                yield path


def read(path):
    with open(path, encoding='utf-8', errors='replace') as fh:
        return strip_comments(fh.read())


def scan_param_aliases(src):
    """Class-scoped AlgorithmParameterName definitions.

    Returns ({identifier: "name"}, {identifier: ["name", ...]}) — the second for
    array-valued declarations indexed by a loop variable.
    """
    aliases, arrays = {}, {}
    for path in walk(os.path.join(src, 'Modules'), {'.cc'}):
        text = read(path)
        for m in PARAM_ALIAS_DEF.finditer(text):
            aliases[m.group(2)] = m.group(3)
        for m in PARAM_ARRAY_DEF.finditer(text):
            body, _ = brace_body(text, m.end() - 1)
            names = re.findall(r'"([^"]*)"', body)
            if names:
                arrays[m.group(2)] = names
    return aliases, arrays


def scan_ports(src):
    """({class: [{'input','output','header'}, ...]}, {class: [base class names]})

    A class can be declared in more than one header (an orphan left by a split,
    as with GenerateElectrodeFromWidget.h); choose_ports() resolves that."""
    ports = {}
    bases = {}
    for path in walk(os.path.join(src, 'Modules'), {'.h'}):
        text = read(path)
        classes = []
        for m in CLASS_DEF.finditer(text):
            body, end = brace_body(text, m.end() - 1)
            classes.append((m.group(1), m.end(), end, body))
            if m.group(2):
                found = [b.split('::')[-1] for b in BASE_CLASS.findall(m.group(2))]
                if found:
                    bases.setdefault(m.group(1), []).extend(found)
        for name, start, end, body in classes:
            ins, outs = [], []
            for pm in PORT_MACRO.finditer(body):
                args = call_args(body, pm.end() - 1)
                if len(args) < 3:
                    continue
                idx, pname, ptype = args[0], args[1], args[2]
                try:
                    idx = int(idx)
                except ValueError:
                    continue
                entry = {'index': idx, 'name': pname, 'type': ptype}
                if pm.group(1) == 'OUTPUT_PORT':
                    outs.append(entry)
                else:
                    entry['dynamic'] = pm.group(1).endswith('DYNAMIC')
                    ins.append(entry)
            if ins or outs:
                ports.setdefault(name, []).append({
                    'input': sorted(ins, key=lambda p: p['index']),
                    'output': sorted(outs, key=lambda p: p['index']),
                    'header': os.path.relpath(path, os.path.dirname(src)),
                })
    return ports, bases


def scan_ctor_forwarding(src):
    """Resolve state keys held in a member seeded by a constructor parameter.

    Returns (member_param_index, base_call_args):
      member_param_index[(cls, "member_")] -> index of the ctor parameter it takes
      base_call_args[cls]                  -> literal args cls passes to its base ctor
    """
    member_param_index = {}
    base_call_args = {}
    for path in walk(os.path.join(src, 'Modules'), {'.h', '.cc'}):
        text = read(path)
        for m in CTOR_DEF.finditer(text):
            cls = m.group(1)
            params = call_args(text, m.end() - 1)
            pnames = []
            for p in params:
                ids = re.findall(r'\b(\w+)\b', p)
                pnames.append(ids[-1] if ids else '')
            open_brace = text.find('{', m.end())
            if open_brace < 0:
                continue
            init_list = text[m.end():open_brace]
            for im in MEMBER_INIT.finditer(init_list):
                if im.group(2) in pnames:
                    member_param_index[(cls, im.group(1))] = pnames.index(im.group(2))
            bm = BASE_INIT.search(init_list)
            if bm:
                args = call_args(init_list, bm.end() - 1)
                vals = [cpp_literal(a) for a in args]
                base_call_args[cls] = [v if ok else None for v, ok in vals]
    return member_param_index, base_call_args


def scan_module_info(src):
    """MODULE_INFO_DEF -> {class: {category, package, source}}"""
    info = {}
    for path in walk(os.path.join(src, 'Modules'), {'.cc'}):
        text = read(path)
        for m in MODULE_INFO_DEF.finditer(text):
            args = call_args(text, m.end() - 1)
            if len(args) >= 3:
                info[args[0]] = {
                    'category': args[1],
                    'package': args[2],
                    'source': os.path.relpath(path, os.path.dirname(src)),
                }
    return info


_JSON_TYPE = {bool: 'bool', int: 'int', float: 'double',
              str: 'string', type(None): 'unknown'}


def scan_state_defaults(src, aliases, arrays, consts):
    """setStateDefaults bodies -> {class: [state var records]}"""
    out = defaultdict(list)
    unparsed = defaultdict(list)
    # Headers too: the DataIO family defines setStateDefaults out of line in the
    # GenericReader/GenericWriter template, which subclasses inherit.
    for path in walk(os.path.join(src, 'Modules'), {'.cc', '.h'}):
        text = read(path)
        for m in STATE_DEFAULTS_DEF.finditer(text):
            cls = m.group(1)
            body, _ = brace_body(text, m.end() - 1)
            seen = {r['name'] for r in out[cls] if 'name' in r}

            for sm in STATE_FROM_ALGO.finditer(body):
                args = call_args(body, sm.end() - 1)
                if not args:
                    continue
                names = resolve_param(args[0], aliases, arrays)
                if not names:
                    unparsed[cls].append(args[0])
                    continue
                for name in names:
                    if name in seen:
                        continue
                    seen.add(name)
                    out[cls].append({
                        'name': name,
                        'type': 'option' if sm.group(2) else TYPE_OF[sm.group(1)],
                        'source': 'algorithm',
                    })

            for sm in COPY_ALGO_TO_STATE.finditer(body):
                args = call_args(body, sm.end() - 1)
                if len(args) != 1 or not args[0].startswith('{'):
                    unparsed[cls].extend(args or ['copyAlgoToState(?)'])
                    continue
                for item in split_args(args[0].strip()[1:-1]):
                    names = resolve_param(item, aliases, arrays)
                    if not names:
                        unparsed[cls].append(item)
                        continue
                    for name in names:
                        if name not in seen:
                            seen.add(name)
                            out[cls].append({'name': name, 'source': 'algorithm'})

            for sm in SET_VALUE.finditer(body):
                args = call_args(body, sm.end() - 1)
                if len(args) < 2:
                    continue
                raw = args[0].strip()
                fields = default_fields(args[1], consts)
                # A trailing-underscore member (SCIRun's member convention) holds a
                # key seeded by a subclass ctor; defer it for build() to resolve.
                if re.fullmatch(r'\w+_', raw) and raw not in aliases:
                    out[cls].append(dict(fields, member=raw, source='module'))
                    continue
                names = resolve_param(raw, aliases, arrays)
                if not names:
                    unparsed[cls].append(args[0])
                    continue
                for name in names:
                    if name in seen:
                        continue
                    seen.add(name)
                    rec = {'name': name, 'source': 'module'}
                    if sm.group(1):
                        rec['transient'] = True
                    rec.update(fields)
                    out[cls].append(rec)
    return out, unparsed


def _algo_ctor_bodies(root):
    """(class, ctor body) for out-of-line ctors in .cc and inline ones in .h
    (ModelTMSCoilAlgorithm, BiotSavartSolverAlgorithm define theirs in-class)."""
    for path in walk(root, {'.cc', '.h'}):
        text = read(path)
        if path.endswith('.cc'):
            for m in ALGO_CTOR.finditer(text):
                yield m.group(1), brace_body(text, m.end() - 1)[0]
            continue
        for cm in CLASS_DEF.finditer(text):
            cls = cm.group(1)
            cbody, _ = brace_body(text, cm.end() - 1)
            m = re.search(r'\b' + cls + r'\s*\(\s*\)\s*(?::[^{;]*)?\{', cbody)
            if m:
                yield cls, brace_body(cbody, m.end() - 1)[0]


def scan_algorithms(src, aliases, consts):
    """Algo ctors -> {algo_class: {param: record}} plus a global name fallback."""
    by_class = defaultdict(dict)
    globally = {}
    for cls, body in _algo_ctor_bodies(os.path.join(src, 'Core', 'Algorithms')):
        for am in ADD_PARAM.finditer(body):
            args = call_args(body, am.end() - 1)
            if len(args) < 2:
                continue
            names = resolve_param(args[0], aliases)
            if not names:
                continue
            rec = default_fields(args[1], consts)
            if am.group(1) == 'Option':
                rec['type'] = 'option'
                if len(args) >= 3:
                    opts, is_lit = cpp_literal(args[2])
                    if is_lit and isinstance(opts, str):
                        rec['options'] = opts.split('|')
            for name in names:
                by_class[cls][name] = rec
                globally.setdefault(name, rec)
    return by_class, globally


def scan_configs(src):
    """Factory/Config/*.module -> {module_name: config dict}"""
    cfg_dir = os.path.join(src, 'Modules', 'Factory', 'Config')
    configs = {}
    if not os.path.isdir(cfg_dir):
        return configs
    for fn in sorted(os.listdir(cfg_dir)):
        if not fn.endswith('.module'):
            continue
        path = os.path.join(cfg_dir, fn)
        try:
            with open(path, encoding='utf-8') as fh:
                data = json.load(fh)
        except (ValueError, OSError) as exc:
            print(f'warning: {path}: {exc}', file=sys.stderr)
            continue
        name = data.get('module', {}).get('name') or os.path.splitext(fn)[0]
        configs[name] = data
    return configs


def scan_factory(src):
    """Hand-written addModuleDesc<T>(...) -> {class: {status, description, ...}}"""
    reg = {}
    fac = os.path.join(src, 'Modules', 'Factory')
    for fn in ('ModuleFactoryImpl1.cc', 'ModuleFactoryImpl2.cc'):
        path = os.path.join(fac, fn)
        if not os.path.exists(path):
            continue
        text = read(path)
        for m in ADD_MODULE_DESC.finditer(text):
            cls = m.group(1).split('::')[-1]
            args = call_args(text, m.end() - 1)
            lits = [cpp_literal(a) for a in args]
            strs = [v for v, ok in lits if ok and isinstance(v, str)]
            rec = {'registration': 'hardcoded'}
            if len(strs) >= 5:
                rec.update(category=strs[1], package=strs[2],
                           status=strs[3], description=strs[4])
            elif len(strs) == 2:
                rec.update(status=strs[0], description=strs[1])
            reg[cls] = rec
    return reg


def scan_dialog_keys(src, aliases):
    """Keys a dialog writes but setStateDefaults never initializes.

    Mostly command triggers (ResetCenter, SpecifyROI). They are real keys that
    scirun_set_module_state can drive, so they belong in the manifest.
    """
    found = defaultdict(set)
    root = os.path.join(src, 'Interface', 'Modules')
    if not os.path.isdir(root):
        return found
    for path in walk(root, {'.cc'}):
        base = os.path.basename(path)
        if not base.endswith('Dialog.cc'):
            continue
        module = base[:-len('Dialog.cc')]
        text = read(path)
        for sm in SET_VALUE.finditer(text):
            args = call_args(text, sm.end() - 1)
            if not args:
                continue
            raw = args[0].strip()
            if raw.endswith('_'):
                continue
            names = resolve_param(raw, aliases)
            if names:
                found[module].update(names)
    return found


def scan_docs(docs_root):
    found = {}
    if not os.path.isdir(docs_root):
        return found
    for dirpath, _, filenames in os.walk(docs_root):
        for fn in filenames:
            if fn.endswith('.md'):
                found.setdefault(os.path.splitext(fn)[0],
                                 os.path.join(dirpath, fn))
    return found


# --------------------------------------------------------------------------
# Assembly
# --------------------------------------------------------------------------

def ancestry(name, bases, _seen=None):
    """Base classes of `name`, nearest first, without cycles."""
    _seen = _seen if _seen is not None else set()
    order = []
    for base in bases.get(name, []):
        if base in _seen:
            continue
        _seen.add(base)
        order.append(base)
        order.extend(ancestry(base, bases, _seen))
    return order


def live_includes(src):
    """Header paths (as written in #include) that some built .cc includes."""
    inc = set()
    for path in walk(src, {'.cc'}):
        with open(path, errors='replace') as fh:
            inc.update(re.findall(r'#\s*include\s*[<"]([^>"]+)[>"]', fh.read()))
    return inc


def choose_ports(candidates, included, configs):
    """One port record per class: the header a built .cc actually includes, else
    the one the .module config names. Returns (ports, ambiguous class names)."""
    chosen, ambiguous = {}, []
    for cls, cands in candidates.items():
        if len(cands) > 1:
            live = [c for c in cands if c['header'].split('src/', 1)[-1] in included]
            cfg = configs.get(cls, {}).get('module', {}).get('header')
            named = [c for c in cands if cfg and c['header'].endswith(cfg)]
            pick = live if len(live) == 1 else named
            if len(pick) != 1:
                ambiguous.append(cls)
            cands = pick or cands
        chosen[cls] = cands[0]
    return chosen, sorted(ambiguous)


def build(repo):
    src = os.path.join(repo, 'src')
    _NOT_BUILT.clear()
    _NOT_BUILT.update(find_unbuilt_sources(src))
    configs = scan_configs(src)
    factory = scan_factory(src)
    consts = scan_constants(src)
    aliases, arrays = scan_param_aliases(src)
    port_candidates, bases = scan_ports(src)
    ports, ambiguous = choose_ports(port_candidates, live_includes(src), configs)
    member_param_index, base_call_args = scan_ctor_forwarding(src)
    info = scan_module_info(src)
    state, unparsed = scan_state_defaults(src, aliases, arrays, consts)
    algos_by_class, algos_global = scan_algorithms(src, aliases, consts)
    dialog_keys = scan_dialog_keys(src, aliases)
    docs = scan_docs(os.path.join(repo, 'docs', 'modules'))

    # Only what the factory registers: scirun_add_module rejects anything else,
    # however complete its source looks.
    names = set(configs) | set(factory)
    meta = {
        'unregistered': sorted((set(info) & (set(ports) | set(state))) - names),
        'ambiguousHeaders': [c for c in ambiguous if c in names],
    }
    modules = {}

    for name in sorted(names):
        cfg = configs.get(name, {})
        cfg_mod = cfg.get('module', {})
        cfg_algo = cfg.get('algorithm', {})
        cfg_ui = cfg.get('UI', {})
        fac = factory.get(name, {})
        inf = info.get(name, {})

        entry = {
            'name': name,
            'category': cfg_mod.get('namespace') or fac.get('category') or inf.get('category'),
            'package': fac.get('package') or inf.get('package') or 'SCIRun',
            'registration': 'config' if name in configs else fac.get('registration', 'hardcoded'),
            'status': cfg_mod.get('status') or fac.get('status'),
            'description': cfg_mod.get('description') or fac.get('description'),
        }

        lineage = ancestry(name, bases)
        if lineage:
            entry['bases'] = lineage

        # Ports and state are inherited; a subclass's own declarations win.
        prt = ports.get(name)
        merged_ports = {'input': [], 'output': []}
        for src_cls in [name] + lineage:
            got = ports.get(src_cls)
            if not got:
                continue
            for side in ('input', 'output'):
                taken = {p['index'] for p in merged_ports[side]}
                for p in got[side]:
                    if p['index'] not in taken:
                        merged_ports[side].append(
                            p if src_cls == name else dict(p, inheritedFrom=src_cls))
        for side in ('input', 'output'):
            merged_ports[side].sort(key=lambda p: p['index'])

        entry['header'] = prt['header'] if prt else cfg_mod.get('header')
        entry['ports'] = merged_ports if any(merged_ports.values()) else None

        if inf.get('source'):
            entry['source'] = inf['source']

        algo_name = cfg_algo.get('name')
        if algo_name in (None, 'N/A'):
            algo_name = None
        guess = algo_name or next(
            (c for c in (f'{name}Algo', f'{name}Algorithm') if c in algos_by_class), f'{name}Algo')
        algo_params = algos_by_class.get(guess, {})
        if algo_params:
            entry['algorithm'] = {'name': guess, 'header': cfg_algo.get('header')}
        elif algo_name:
            entry['algorithm'] = {'name': algo_name, 'header': cfg_algo.get('header')}

        ui = cfg_ui.get('name')
        entry['hasUI'] = bool(ui and ui != 'N/A') or None

        # Merge module-declared state with the algorithm's defaults and option sets.
        own = list(state.get(name, []))
        claimed = {r['name'] for r in own if 'name' in r}
        for base in lineage:
            for rec in state.get(base, []):
                if 'member' in rec:
                    # Resolve the member against what this class passes its base.
                    idx = member_param_index.get((base, rec['member']))
                    args = base_call_args.get(name) or []
                    key = args[idx] if idx is not None and idx < len(args) else None
                    if not isinstance(key, str) or key in claimed:
                        continue
                    claimed.add(key)
                    own.append(dict(rec, name=key, inheritedFrom=base))
                    own[-1].pop('member')
                elif rec.get('name') not in claimed:
                    claimed.add(rec.get('name'))
                    own.append(dict(rec, inheritedFrom=base))
        own = [r for r in own if 'name' in r]

        merged = []
        for rec in own:
            rec = dict(rec)
            extra = algo_params.get(rec.get('name')) or (
                algos_global.get(rec.get('name')) if rec['source'] == 'algorithm' else None)
            if extra:
                for k, v in extra.items():
                    if k == 'type':
                        if rec.get('type') in (None, 'unknown'):
                            rec['type'] = v
                        continue
                    rec.setdefault(k, v)
                if 'default' in rec:
                    rec.pop('defaultExpr', None)
                if 'options' in extra:
                    rec['options'] = extra['options']
            rec.setdefault('type', 'unknown')
            merged.append(rec)

        known = {r.get('name') for r in merged}
        for pname, rec in sorted(algo_params.items()):
            if pname not in known:
                merged.append(dict({'type': 'unknown'}, **rec, name=pname, source='algorithm-only'))
                known.add(pname)
        for key in sorted(dialog_keys.get(name, ())):
            if key not in known:
                merged.append({'name': key, 'source': 'dialog', 'type': 'unknown'})

        entry['state'] = sorted(merged, key=lambda r: r.get('name') or r.get('member', ''))
        if unparsed.get(name):
            entry['unresolvedStateKeys'] = sorted(set(unparsed[name]))

        doc = docs.get(name)
        entry['doc'] = os.path.relpath(doc, repo) if doc else None

        modules[name] = {k: v for k, v in entry.items() if v is not None}

    return modules, meta


def gaps_report(modules, meta):
    no_ports = [n for n, m in modules.items() if not m.get('ports')]
    no_state = [n for n, m in modules.items() if not m.get('state')]
    no_doc = [n for n, m in modules.items() if not m.get('doc')]
    unresolved = {n: m['unresolvedStateKeys']
                  for n, m in modules.items() if m.get('unresolvedStateKeys')}
    untyped = defaultdict(list)
    for n, m in modules.items():
        for r in m.get('state', []):
            if r.get('type') == 'unknown' or ('default' not in r and 'options' not in r):
                untyped[n].append(r.get('name') or r.get('member', '?'))

    lines = ['# SCIRun module manifest — coverage gaps', '',
             f'{len(modules)} modules extracted.', '',
             '| Gap | Count |', '| --- | ---: |',
             f'| No ports found | {len(no_ports)} |',
             f'| No state variables found | {len(no_state)} |',
             f'| No doc page | {len(no_doc)} |',
             f'| Modules with unresolved state keys | {len(unresolved)} |',
             f'| Modules with untyped/defaultless state | {len(untyped)} |',
             f'| In source but never registered (excluded) | {len(meta["unregistered"])} |',
             f'| Class declared in several headers, unresolved | {len(meta["ambiguousHeaders"])} |', '']

    def section(title, items):
        lines.append(f'## {title} ({len(items)})')
        lines.append('')
        lines.extend(f'- `{i}`' for i in sorted(items))
        lines.append('')

    section('No ports parsed', no_ports)
    section('No state variables parsed', no_state)
    section('No documentation page', no_doc)
    section('Built or on disk, but never registered with the factory', meta['unregistered'])
    section('Declared in several headers; ports may be from the wrong one', meta['ambiguousHeaders'])

    lines.append(f'## Unresolved state keys ({len(unresolved)})')
    lines.append('')
    for n in sorted(unresolved):
        lines.append(f'- `{n}`: ' + ', '.join(f'`{k}`' for k in unresolved[n]))
    lines.append('')

    lines.append(f'## State variables lacking a type or default ({len(untyped)})')
    lines.append('')
    for n in sorted(untyped):
        lines.append(f'- `{n}`: ' + ', '.join(f'`{k}`' for k in sorted(untyped[n])))
    lines.append('')
    return '\n'.join(lines)


def main():
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--repo', default=here, help='repository root (default: parent of scripts/)')
    ap.add_argument('--out', help='write JSON manifest here (default: stdout)')
    ap.add_argument('--gaps', help='write a markdown coverage report here')
    ap.add_argument('--module', help='print a single module and exit')
    args = ap.parse_args()

    modules, meta = build(args.repo)

    if args.module:
        entry = modules.get(args.module)
        if not entry:
            print(f'no such module: {args.module}', file=sys.stderr)
            return 1
        print(json.dumps(entry, indent=2))
        return 0

    payload = {
        'schema': 1,
        'moduleCount': len(modules),
        'modules': modules,
        'unregisteredModules': meta['unregistered'],
    }
    text = json.dumps(payload, indent=2, sort_keys=False)
    if args.out:
        with open(args.out, 'w', encoding='utf-8') as fh:
            fh.write(text + '\n')
        print(f'{len(modules)} modules -> {args.out}', file=sys.stderr)
    else:
        print(text)

    if args.gaps:
        with open(args.gaps, 'w', encoding='utf-8') as fh:
            fh.write(gaps_report(modules, meta) + '\n')
        print(f'gap report -> {args.gaps}', file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
